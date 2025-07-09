__attribute__((section(".multiboot"))) volatile unsigned long header[] = {
    0x1BADB002, 0x0, -(0x1BADB002)
};

#include <stdint.h>
#include <stddef.h>
#include "helpers/port_io.h"
#include "helpers/basics.h"
#include "helpers/serial.h"
#include "helpers/disk.h"
#include "structs/structs.h"
#include "filesystem/filesystem.h"
#include "posix/posix.h"
#include "helpers/idt.h"
#include "structs/interrupts.h"

#define USER_PROG_LOAD_ADDR 0x400000
#define USER_STACK_TOP      0x800000
#define MAX_INPUT 64
#define MAX_BLOCKS 4096
#define ATA_SR_BSY 0x80
#define ATA_SR_DRQ 0x08
#define FD_PIPE_READ(pipe_id)  (1000 + (pipe_id) * 2)
#define FD_PIPE_WRITE(pipe_id) (1000 + (pipe_id) * 2 + 1)
#define FS_MAGIC 0x5346 // 'SF' in little endian
#define MAX_INTERRUPTS 256

// At the top of kernel.c:
void register_interrupt_handler(int n, void (*handler)(struct registers*));
int load_user_program(const char* filename);
extern void syscall_entry();  // defined in assembly or C stub

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
} __attribute__((packed)) multiboot_info_t;

// Utility function prototypes
void print(const char* str);
void print_buffer(const char *buffer);
void int_to_hex(uint8_t val, char* out);
void int_to_chars(int num, char *buf, int buf_size);
void log_buffer(const char *buffer);
void log_buffer(const char *buffer);
void log(const char* str);

// ATA and Filesystem function prototypes
void print_ata_status(const char* context);
static int ata_wait_bsy_clear(void);
static int ata_wait_drq_set(void);
int ata_identify_drive(void);
int ata_read_sector(uint32_t lba, void* buffer);
int ata_write_sector(uint32_t lba, const void* buffer);
void init_filesystem_if_empty(void);
void dump_block_0(void);
void initialize_next_free_block(void);

char input_buffer[MAX_INPUT];
extern char _text_start[];
extern char _text_end[];
size_t code_size;
int filesystem_initialized = 0;
char mem_buf[24];
extern uint32_t magic_number;
extern uint32_t mb_info_ptr;
static void (*interrupt_handlers[MAX_INTERRUPTS])(struct registers *);

void register_interrupt_handler(int n, void (*handler)(struct registers *r)) {
    if (n >= 0 && n < MAX_INTERRUPTS) {
        interrupt_handlers[n] = handler;
    }
}

int task_create(void (*entry)(void)) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!tasks[i].active) {
            tasks[i].entry = entry;
            tasks[i].active = 1;

            // Zero out the stack (optional)
            for (int j = 0; j < STACK_SIZE; j++)
                tasks[i].stack[j] = 0;

            return i;
        }
    }
    return -1; // No slot
}

void clear_screen() {
    volatile char *video = (volatile char*)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        video[i * 2] = ' ';       // Character
        video[i * 2 + 1] = 0x07;  // Attribute: light gray on black
    }
}

void debug_sector_data(const void* buffer, const char* label) {
    const uint8_t* data = (const uint8_t*)buffer;
    print(label); 
    print(" - First 32 bytes:\n");
    
    for (int i = 0; i < 32; i++) {
        char hex[3];
        int_to_hex(data[i], hex);
        print(hex);
        print(" ");
        if ((i + 1) % 16 == 0) print("\n");
    }
    if (32 % 16 != 0) log("\n");
    
    // Check if it's all zeros
    int all_zeros = 1;
    for (int i = 0; i < BLOCK_SIZE; i++) {
        if (data[i] != 0) {
            all_zeros = 0;
            break;
        }
    }
    log(all_zeros ? "Data is ALL ZEROS\n" : "Data contains non-zero bytes\n");
}

void init_filesystem(void) {
    uint8_t sector[BLOCK_SIZE];
    
    // Actually read the superblock from disk first
    if (ata_read_sector(0, sector) != 0) {
        print("Failed to read superblock from disk\n");
        return;

    }

    // Now check if we have a valid superblock
    Superblock *disk_sb = (Superblock*)sector;
    if (disk_sb->magic == FS_MAGIC) {
        print("Valid filesystem found. Loading...\n");
        memcpy(&superblock, disk_sb, sizeof(Superblock));
        load_file_table();
        filesystem_initialized = 1;
        return;
    }
    
    print("No valid filesystem found. Creating new filesystem...\n");
    
    // Create a new filesystem
    memset(sector, 0, BLOCK_SIZE);
    Superblock *sb = (Superblock*)sector;
    sb->magic = FS_MAGIC;
    sb->total_blocks = MAX_BLOCKS;
    sb->file_table_start = 1;
    sb->file_table_length = MAX_FILE_ENTRIES;
    sb->data_start = FILE_TABLE_BLOCKS + 1;
    
    // Write the superblock to disk
    if (ata_write_sector(0, sector) != 0) {
        print("Failed to write superblock to disk\n");
        return;
    }
    
    // Copy to our global superblock
    memcpy(&superblock, sb, sizeof(superblock));
    
    // Initialize empty file table
    print("Initializing empty file table...\n");
    memset(file_table, 0, sizeof(file_table));
    save_file_table();
    filesystem_initialized = 1;
    print("Filesystem initialized successfully!\n");
}


void initialize_next_free_block() {
    next_free_block = 0;

    for (int i = 0; i < superblock.file_table_length; i++) {
        if (!file_table[i].active) continue;

        uint32_t end_block = file_table[i].start_block + (file_table[i].size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        if (end_block > next_free_block) {
            next_free_block = end_block;
        }
    }

    print("Next free block: ");
    int_to_chars(next_free_block, buffer, sizeof(buffer));
    print_buffer(buffer); print("\n");
}

void dump_block_0() {
    uint8_t buf[BLOCK_SIZE];
    disk_read_block(0, buf);

    log("Block 0 dump:\n");
    for (int i = 0; i < 64; i++) {
        char hex[3];
        int_to_hex(buf[i], hex);
        log_buffer(hex);
        log(" ");
        if ((i + 1) % 16 == 0)
            log("\n");
    }
}

void switch_to_user_mode(void* entry_point, void* user_stack_top) {
    asm volatile (
        "cli\n"
        "mov $0x23, %%ax\n"        // User data segment selector (ring 3, data)
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "mov %0, %%eax\n"
        "mov %%eax, %%esp\n"       // Set stack pointer

        "pushl $0x23\n"            // User data segment selector (stack)
        "pushl %%eax\n"            // Stack pointer

        "pushf\n"                  // Push eflags

        "pushl $0x1B\n"            // User code segment selector (ring 3)
        "pushl %1\n"               // Entry point (EIP)

        "iret\n"
        :
        : "r" (user_stack_top), "r" (entry_point)
    );
}



void syscall_handler(struct registers *r) {
    switch (r->eax) {
        case 1: // sys_exit
            print("Process exited\n");
            kill(current_task);
            r->eax = 0;
            break;

        case 2: // sys_write(filename, data, size)
            r->eax = write((const char*)r->ebx, (const void*)r->ecx, r->edx, DEFAULT_PERMS);
            break;

        case 3: // sys_read(filename, buffer, max_size)
            r->eax = read((const char*)r->ebx, (void*)r->ecx, r->edx);
            break;

        case 4: // sys_unlink(filename)
            r->eax = unlink((const char*)r->ebx);
            break;

        case 5: // sys_rename(oldname, newname)
            r->eax = rename((const char*)r->ebx, (const char*)r->ecx);
            break;

        case 6: // sys_truncate(filename, len)
            r->eax = truncate((const char*)r->ebx, (int)r->ecx);
            break;

        case 7: // sys_chmod(filename, new_perms)
            r->eax = chmod((const char*)r->ebx, (uint8_t)r->ecx);
            break;

        case 8: // sys_pipe(pipe_fds)
            r->eax = pipe((int*)r->ebx);
            break;

        case 9: // sys_sched_yield()
            sched_yield();
            r->eax = 0;
            break;

        case 10: // sys_getchar()
            r->eax = (uint32_t)getpress(); // blocking read from keyboard
            break;

        case 11: // sys_kill(task_id)
            kill((int)r->ebx);
            r->eax = 0;
            break;

        default:
            print("Unknown syscall: ");
            int_to_chars(r->eax, buffer, sizeof(buffer));
            print(buffer);
            print("\n");
            r->eax = -1;
            break;
    }
}

void user_task_entry() {
    load_user_program("init");
}

void kernel_main() {
    multiboot_info_t* mb_info = (multiboot_info_t*)mb_info_ptr;
    serial_init();
    clear_screen();
    idt_install(); 
    print_buffer("Total Memory (MB): ");
    int_to_chars(((mb_info->mem_lower + mb_info->mem_upper) / 1024), mem_buf, sizeof(mem_buf));
    print_buffer(mem_buf);
    print("\n");

    print("Memory Used (MB): ");
    code_size = (size_t)(_text_end - _text_start);
    int_to_chars(code_size / 1024, buffer, sizeof(buffer));
    print_buffer(buffer);
    print("\n");

    // Test ATA drive before filesystem operations
    if (ata_identify_drive() != 0) {
        print("WARNING: No ATA drive detected. Filesystem operations will fail.\n");
        print("Running in read-only mode.\n");
        return;
    }

    init_filesystem();
    if (!filesystem_initialized) {
        print("FATAL: Filesystem initialization failed!\n");
        while(1); // Halt
    }
    
    if (mb_info->flags & MB_INFO_FLAG_ELF_BOOTLOADER) {
    }
    log("Dumping block 0:\n");
    dump_block_0();
    initialize_next_free_block();
    print("superblock.file_table_length: ");
    int_to_chars(superblock.file_table_length, buffer, sizeof(buffer));
    print(buffer);
    print("\n");

    print_logo(".##  .## .##       .## .##  .##");
    print_logo(".## .##  .###     .### .## .## ");
    print_logo(".##.##   .##.##  .#### .##.##  ");
    print_logo(".####    .## .##.##.## .####   ");
    print_logo(".###     .##  .### .## .###    ");
    print_logo(".####    .##       .## .####   ");
    print_logo(".##.##   .##       .## .##.##  ");
    print_logo(".## .##  .##       .## .## .## ");
    print_logo(".##  .## .##       .## .##  .##");

    print_bear("     !       ");
    print_bear("  ()=---=()  ");
    print_bear("  :<O|-|O>:  ");
    print_bear("  X  U-U  X  ");
    print_bear(" ( - #<# - ) ");
    print_bear(" {) (|_|) (} ");
    print_bear("()=-() ()-=()\n");
                        // <- Set up IDT
    register_interrupt_handler(0x80, syscall_handler);  // <- syscalls
    
    int user_task_id = task_create(user_task_entry);
    if (user_task_id >= 0) {
        current_task = user_task_id;
        switch_to_user_mode_with_task(user_task_id);
    }

    while (1); // fallback halt

}
