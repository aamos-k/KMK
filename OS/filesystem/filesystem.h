#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include "helpers/disk.h"


#define BLOCK_SIZE 512
#define FILE_TABLE_BLOCKS ((MAX_FILE_ENTRIES * sizeof(FileEntry) + BLOCK_SIZE - 1) / BLOCK_SIZE)
#define MAX_FILE_ENTRIES 1024

FileEntry file_table[MAX_FILE_ENTRIES];  // loaded from disk at startup
Superblock superblock;

void save_file_table(void) {
    log("Saving file table to disk...\n");
    
    uint8_t sector[BLOCK_SIZE];
    size_t entries_per_sector = BLOCK_SIZE / sizeof(FileEntry);
    
    for (int sector_num = 0; sector_num < FILE_TABLE_BLOCKS; sector_num++) {
        memset(sector, 0, BLOCK_SIZE);
        
        // Copy file entries to this sector
        size_t start_entry = sector_num * entries_per_sector;
        size_t entries_to_copy = entries_per_sector;
        
        if (start_entry < MAX_FILE_ENTRIES) {
            if (start_entry + entries_to_copy > MAX_FILE_ENTRIES) {
                entries_to_copy = MAX_FILE_ENTRIES - start_entry;
            }
            
            memcpy(sector, &file_table[start_entry], entries_to_copy * sizeof(FileEntry));
        }
        
        uint32_t lba = superblock.file_table_start + sector_num;
        if (ata_write_sector(lba, sector) != 0) {
            log("Error writing file table sector ");
            char buf[12];
            int_to_chars(sector_num, buf, sizeof(buf));
            log_buffer(buf);
            log("\n");
            return;
        }
    }
    
    log("File table saved successfully.\n");
}


void save_superblock() {
    log("Saving superblock...\n");
    int ret = ata_write_sector(0, (uint8_t*)&superblock);
    if (ret != 0) {
        log("Error writing superblock sector!\n");
    }
}

void load_superblock() {
    log("Loading superblock...\n");
    int ret = ata_read_sector(0, (uint8_t*)&superblock);
    if (ret != 0) {
        log("Error reading superblock sector!\n");
        // Clear to zero so you can tell
        memset(&superblock, 0, sizeof(superblock));
    } else {
        log("Superblock magic: ");
        char buf[12];
        int_to_chars(superblock.magic, buf, sizeof(buf));
        log_buffer(buf);
        log("\n");
    }
}

void disk_read_block(uint32_t block_num, void* buffer) {
    if (ata_read_sector(block_num, buffer) != 0) {
        memset(buffer, 0, BLOCK_SIZE);
        return;
    }
}



void disk_write_block(uint32_t block_num, const void* buffer) {
    int ret = ata_write_sector(block_num, buffer);
    if (ret != 0) {
        return;
    }
}


void load_file_table() { 
    int blocks_for_table = (sizeof(file_table) + BLOCK_SIZE - 1) / BLOCK_SIZE;
    for (int i = 0; i < blocks_for_table; i++) {
        disk_read_block(superblock.file_table_start + i, ((char*)file_table) + i * BLOCK_SIZE);
    }
}

FileEntry* find_file(const char* filename) {
    for (int i = 0; i < superblock.file_table_length; i++) {
        if (file_table[i].active) {
            log("Comparing: ");
            log(file_table[i].filename);
            log(" == ");
            log(filename);
            log("\n");

            if (strcmp(file_table[i].filename, filename) == 0) {
                log("Found match!\n");
                return &file_table[i];
            }
        }
    }

    log("No match found for ");
    log(filename);
    log("\n");
    return NULL;
}
#endif
