#ifndef POSIX_H
#define POSIX_H

#include <stdint.h>
#include <stddef.h>
#include "helpers/disk.h"
#include "filesystem/filesystem.h"
#include "structs/structs.h"

#define DEFAULT_PERMS (PERM_READ | PERM_WRITE)
#define MAX_PIPES 8
#define MAX_TASKS 4

uint32_t next_free_block = 0;
char buffer[12];
int pipe_count = 0;
int current_task = 0;
char scancode_to_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,'\\','z','x','c','v','b','n','m',',','.','/',
    0,'*',0,' ',
};


Pipe pipe_table[MAX_PIPES];
Task tasks[MAX_TASKS];

int is_pipe_fd(int fd) {
    return fd >= 1000 && fd < 1000 + MAX_PIPES * 2;
}
int get_pipe_id(int fd) {
    return (fd - 1000) / 2;
}
int is_pipe_read_fd(int fd) {
    return (fd - 1000) % 2 == 0;
}

char getpress() {
    char c = 0;

    while (1) {
        uint8_t scancode = inb(0x60);

        // Ignore releases and unknown keys
        if (scancode >= 0x80 || scancode_to_ascii[scancode] == 0)
            continue;

        // Translate scancode to ASCII
        c = scancode_to_ascii[scancode];
        break;
    }

    // Wait for key release (skip repeated presses)
    while ((inb(0x60) & 0x80) == 0) {
        // Wait for the release of the previous key
    }

    return c;
}

int write(const char* filename, const void* data, uint32_t size, uint8_t perms) {

    FileEntry* existing = find_file(filename);
    uint32_t needed_blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    int slot = -1;
    log("FT length: ");
    int_to_chars(superblock.file_table_length, buffer, sizeof(buffer));
    log_buffer(buffer);
    log("\n");
    if (filename[0] >= '0' && filename[0] <= '9') {
        int fd = 0;
        for (int i = 0; filename[i] && i < 5; i++) {
            if (filename[i] < '0' || filename[i] > '9') {
                fd = -1;
                break;
            }
            fd = fd * 10 + (filename[i] - '0');
        }

        if (fd >= 1000) {
            for (int i = 0; i < pipe_count; i++) {
                Pipe* p = &pipe_table[i];
                if (p->write_fd == fd) {
                    const uint8_t* bytes = (const uint8_t*)data;
                    for (uint32_t j = 0; j < size; j++) {
                        if (p->used >= PIPE_BUFFER_SIZE)
                            return -3; // Pipe full
                        p->buffer[p->end] = bytes[j];
                        p->end = (p->end + 1) % PIPE_BUFFER_SIZE;
                        p->used++;
                    }
                    return 0;
                }
            }
            return -2; // Pipe not found
        }
    }
    if (existing) {
        // Overwrite existing file
        slot = existing - file_table;
        next_free_block = existing->start_block; // Optionally reuse space
    } 
    
    else {
        // Find new slot
        for (int i = 0; i < superblock.file_table_length; i++) {
            if (!file_table[i].active) {
                log("Found empty slot at index: ");
                int_to_chars(i, buffer, sizeof(buffer));
                log_buffer(buffer); 
                log("\n");
        
                slot = i;
                break;
            } else {
                log("Slot ");
                int_to_chars(i, buffer, sizeof(buffer));
                log_buffer(buffer);
                log(": occupied by ");
                log(file_table[i].filename);
                log("\n");
            }
        }

        if (slot == -1) return -4;
    }
    log("About to write file data: ");
    log_buffer(data);
    log("\n");
    int_to_chars(slot, buffer, sizeof(buffer));
    log_buffer(buffer);
    const uint8_t* data_bytes = (const uint8_t*)data;
    uint8_t block_buffer[BLOCK_SIZE];
    for (uint32_t i = 0; i < needed_blocks; i++) {
        uint32_t block_num = superblock.data_start + next_free_block + i;
        uint32_t bytes_left = size - i * BLOCK_SIZE;
        uint32_t chunk_size = (bytes_left < BLOCK_SIZE) ? bytes_left : BLOCK_SIZE;

        memset(block_buffer, 0, BLOCK_SIZE);
        memcpy(block_buffer, data_bytes + i * BLOCK_SIZE, chunk_size);

        disk_write_block(block_num, block_buffer);
        log("Writing to block: ");
        int_to_chars(block_num, buffer, sizeof(buffer));
        log_buffer(buffer);
        log("\n");

    }

    FileEntry* fe = &file_table[slot];
    strncpy(fe->filename, filename, MAX_FILENAME_LEN);
    // Make sure it's null-terminated:
    fe->filename[MAX_FILENAME_LEN - 1] = '\0';
    fe->start_block = next_free_block;
    fe->size = size;
    fe->active = 1;
    fe->permissions = perms;  // Save permissions
    
    next_free_block += needed_blocks;
    save_file_table();
    log("Writing file: ");
    log(filename);
    log("\n");

    log("Size: ");
    int_to_chars(size, buffer, sizeof(buffer));
    log_buffer(buffer); 
    log("\n");

    log("Used slot: ");
    int_to_chars(slot, buffer, sizeof(buffer));
    log_buffer(buffer);
    log("\n");

    log("Start block: ");
    int_to_chars(superblock.data_start + next_free_block, buffer, sizeof(buffer));
    log_buffer(buffer);
    log("\n");

    return 0;
    
}

int writedefper(const char* filename, const void* data, uint32_t size) {
    return write(filename, data, size, DEFAULT_PERMS);
}

int read(const char* filename, void* buffer, uint32_t max_size) {
    char buffer_str[12];

    // Pipe check: see if filename is a pipe FD (pure number >= 1000)
    int fd = 0;
    bool is_pipe = true;

    for (int i = 0; filename[i]; i++) {
        if (filename[i] < '0' || filename[i] > '9') {
            is_pipe = false;
            break;
        }
    }

    if (is_pipe) {
        fd = simple_atoi(filename);
        for (int i = 0; i < pipe_count; i++) {
            if (pipe_table[i].read_fd == fd) {
                Pipe* p = &pipe_table[i];

                if (!p->readable || p->ref_count == 0) return -5;

                uint32_t to_read = (p->used < max_size) ? p->used : max_size;

                for (uint32_t j = 0; j < to_read; j++) {
                    ((char*)buffer)[j] = p->buffer[p->start];
                    p->start = (p->start + 1) % PIPE_BUFFER_SIZE;
                    p->used--;
                }

                // If no more data and no writers, mark unreadable
                if (p->used == 0 && p->writable == 0)
                    p->readable = 0;

                return to_read;
            }
        }
        return -6; // Pipe not found
    }

    // Normal file read
    FileEntry* file = find_file(filename);
    if (!file) return -1;

    uint32_t to_read = (file->size < max_size) ? file->size : max_size;
    uint32_t bytes_read = 0;
    uint8_t block_buffer[BLOCK_SIZE];

    uint32_t current_block = superblock.data_start + file->start_block;

    log("Reading ");
    int_to_chars(to_read, buffer_str, sizeof(buffer_str));
    log_buffer(buffer_str);
    log(" bytes from block ");
    int_to_chars(current_block, buffer_str, sizeof(buffer_str));
    log_buffer(buffer_str);
    log("\n");

    while (bytes_read < to_read) {
        int res = ata_read_sector(current_block, block_buffer);
        if (res != 0) {
            log("Error reading sector\n");
            return -2;
        }

        uint32_t bytes_left = to_read - bytes_read;
        uint32_t chunk = (bytes_left < BLOCK_SIZE) ? bytes_left : BLOCK_SIZE;

        log("Data bytes: ");
        for (int i = 0; i < (chunk < 8 ? chunk : 8); i++) {
            char hex[3];
            int_to_hex(block_buffer[i], hex);
            log_buffer(hex);
            log(" ");
        }
        log("\n");

        memcpy((uint8_t*)buffer + bytes_read, block_buffer, chunk);

        bytes_read += chunk;
        current_block++;
    }

    return bytes_read;
}


static void recompute_next_free_block(void) {
    next_free_block = 0;
    for (int i = 0; i < superblock.file_table_length; ++i) {
        if (!file_table[i].active) continue;
        uint32_t end = file_table[i].start_block +
                       ((file_table[i].size + BLOCK_SIZE - 1) / BLOCK_SIZE);
        if (end > next_free_block) next_free_block = end;
    }
}

int unlink(const char *filename) {
    FileEntry *fe = find_file(filename);
    if (!fe) return -1;

    fe->active       = 0;
    fe->filename[0]  = '\0';
    fe->size         = 0;
    fe->start_block  = 0;

    save_file_table();
    recompute_next_free_block();
    save_superblock();
    return 0;
}

int rename(const char *oldname, const char *newname) {
     FileEntry *fe = find_file(oldname);
     if (!fe) return -1;
     
     int bytes_read = read(oldname, buffer, sizeof(buffer));
     writedefper(newname, &bytes_read, sizeof(bytes_read));
     unlink(oldname);
     // add correct buffers and such as input to funcs
     return 0;
}

int truncate(const char *filename, int len) {
     FileEntry *fe = find_file(filename);
     if (!fe || len == 0)  return -1;
     
     fe->size        = len;
     return 0;
}

void switch_to_user_mode_with_task(int task_id) {
    Task *t = &tasks[task_id];

    asm volatile (
        "cli\n"
        "mov $0x23, %%ax\n"        // User data segment
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "mov %0, %%esp\n"          // Set stack pointer
        "pushl $0x23\n"
        "pushl %%esp\n"
        "pushf\n"
        "pushl $0x1B\n"            // User code segment
        "pushl %1\n"               // EIP (entry point)
        "iret\n"
        :
        : "r" (&t->stack[STACK_SIZE - 1]), "r" (t->entry)
    );
}

void sched_yield() {
    for (int i = 1; i <= MAX_TASKS; i++) {
        int next = (current_task + i) % MAX_TASKS;
        if (tasks[next].active) {
            current_task = next;
            switch_to_user_mode_with_task(next); // set EIP and stack
            return;
        }
    }

    // No tasks left
    print("No tasks left. Halting.\n");
    while (1) asm volatile("hlt");
}


void kill(int id) {
    tasks[id].active = 0;
    if (current_task == id)
        sched_yield();  // choose another task or halt
}


int chmod(const char* filename, uint8_t new_perms) {
    FileEntry* file = find_file(filename);
    if (!file) return -1;
    file->permissions = new_perms;
    save_file_table();
    return 0;
}

int pipe(int* fds) {
    for (int i = 0; i < MAX_PIPES; i++) {
        if (pipe_table[i].ref_count == 0) {
            pipe_table[i].start = 0;
            pipe_table[i].end = 0;
            pipe_table[i].used = 0;
            pipe_table[i].readable = 1;
            pipe_table[i].writable = 1;
            pipe_table[i].ref_count = 2;

            int read_fd = 1000 + i * 2;
            int write_fd = read_fd + 1;

            pipe_table[i].read_fd = read_fd;
            pipe_table[i].write_fd = write_fd;

            fds[0] = read_fd;
            fds[1] = write_fd;

            if (i >= pipe_count) pipe_count = i + 1;
            return 0;
        }
    }
    return -1; // No space for new pipe
}


#endif
