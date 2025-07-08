#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>
#include <stddef.h>

#define STACK_SIZE 4096
#define PIPE_BUFFER_SIZE 512
#define MAX_FILENAME_LEN 32
#define PERM_READ   0x01  // 00000001
#define PERM_WRITE  0x02  // 00000010
#define PERM_EXEC   0x04  // 00000100


typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    // other fields omitted for now
} __attribute__((packed)) multiboot_info_t;

typedef struct {
    uint16_t magic;
    uint32_t total_blocks;
    uint32_t file_table_start;
    uint32_t file_table_length;
    uint32_t data_start;
} Superblock;

typedef struct {
    int read_fd;
    int write_fd;

    char buffer[PIPE_BUFFER_SIZE];
    int start;       // read position
    int end;         // write position
    int used;        // number of bytes currently in the buffer

    int readable;    // can be read?
    int writable;    // can be written to?
    int ref_count;   // number of endpoints still open
} Pipe;

typedef struct {
    char filename[MAX_FILENAME_LEN];
    uint32_t start_block;
    uint32_t size;
    uint8_t active;
    uint8_t permissions;  // New field
} FileEntry;
typedef struct {
    uint8_t stack[STACK_SIZE];
    void (*entry)(void);
    int active;
} Task;

#endif
