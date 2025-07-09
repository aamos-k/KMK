#ifndef DISK_H
#define DISK_H

#include <stdint.h>
#include <stddef.h>
#include "helpers/basics.h"

#define ATA_PRIMARY_CMD  0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

// Add these debug functions AFTER your existing utility functions (after int_to_chars, print, etc.)
void print_ata_status(const char* context) {
    uint8_t status = inb(ATA_PRIMARY_CMD + 7);
    log(context);
    log(" - Status: 0x");
    char hex[3];
    int_to_hex(status, hex);
    log_buffer(hex);
    log(" (BSY=");
    log((status & 0x80) ? "1" : "0");
    log(", DRQ=");
    log((status & 0x08) ? "1" : "0");
    log(")\n");
}

static int ata_wait_bsy_clear(void) {
    uint8_t status;
    log("Waiting for BSY to clear...\n");
    for (int i = 0; i < 50000; i++) {
        status = inb(ATA_PRIMARY_CMD + 7);
        if (!(status & 0x80)) {
            log("BSY cleared after ");
            char buf[12];
            int_to_chars(i, buf, sizeof(buf));
            log_buffer(buf);
            log(" iterations\n");
            return 0;
        }
    }
    log("BSY clear timeout! Final status: 0x");
    char hex[3];
    int_to_hex(status, hex);
    log_buffer(hex);
    log("\n");
    return -1;
}

static int ata_wait_drq_set(void) {
    uint8_t status;
    log("Waiting for DRQ to set...\n");
    for (int i = 0; i < 100000; i++) {
        status = inb(ATA_PRIMARY_CMD + 7);
        if ((status & 0x08) && !(status & 0x80)) {
            log("DRQ set after ");
            char buf[12];
            int_to_chars(i, buf, sizeof(buf));
            log_buffer(buf);
            log(" iterations\n");
            return 0;
        }
    }
    log("DRQ set timeout! Final status: 0x");
    char hex[3];
    int_to_hex(status, hex);
    log_buffer(hex);
    log("\n");
    return -1;
}

int ata_identify_drive(void) {
    log("Identifying ATA drive...\n");
    
    // Select drive 0
    outb(0x1F6, 0xA0);
    
    // Set sector count and LBA to 0
    outb(0x1F2, 0);
    outb(0x1F3, 0);
    outb(0x1F4, 0);
    outb(0x1F5, 0);
    
    // Send IDENTIFY command
    outb(0x1F7, 0xEC);
    
    // Check if drive exists
    uint8_t status = inb(0x1F7);
    if (status == 0) {
        log("No drive detected\n");
        return -1;
    }
    
    print_ata_status("After IDENTIFY");
    
    // Wait for BSY to clear
    if (ata_wait_bsy_clear() != 0) {
        log("IDENTIFY: BSY timeout\n");
        return -2;
    }
    
    // Check for ATA drive (not ATAPI)
    if (inb(0x1F4) != 0 || inb(0x1F5) != 0) {
        log("Not an ATA drive\n");
        return -3;
    }
    
    if (ata_wait_drq_set() != 0) {
        log("IDENTIFY: DRQ timeout\n");
        return -4;
    }
    
    // Read and discard the identify data
    for (int i = 0; i < 256; i++) {
        inw(ATA_PRIMARY_CMD);
    }
    
    log("ATA drive identified successfully\n");
    return 0;
}


int ata_wait_ready() {
    uint8_t status;
    for (int i = 0; i < 100000; i++) {
        status = inb(ATA_PRIMARY_CMD + 7);
        if (!(status & 0x80) && (status & 0x08)) // BSY=0 and DRQ=1
            return 0; // ready
    }
    return -1; // timeout or error
}

int ata_read_sector(uint32_t lba, void* buffer) {
    log("ATA read sector ");
    char buf[12];
    int_to_chars(lba, buf, sizeof(buf));
    log_buffer(buf);
    log("\n");
    
    // Wait for drive to be ready first
    if (ata_wait_bsy_clear() != 0) {
        log("Read: Initial BSY clear failed\n");
        return -1;
    }

    // Select drive 0 with LBA mode
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    
    // Small delay after drive selection
    for(int i = 0; i < 1000; i++) {
        inb(0x1F7); // Status port read for delay
    }
    
    // Set sector count and LBA
    outb(0x1F2, 1);                           // sector count
    outb(0x1F3, (uint8_t)(lba & 0xFF));       // LBA 0-7
    outb(0x1F4, (uint8_t)((lba >> 8) & 0xFF)); // LBA 8-15
    outb(0x1F5, (uint8_t)((lba >> 16) & 0xFF)); // LBA 16-23
    
    // Send READ SECTORS command
    outb(0x1F7, 0x20);

    print_ata_status("After read command");

    if (ata_wait_drq_set() != 0) {
        log("Read: DRQ set failed\n");
        return -2;
    }

    // Read data
    uint16_t* buf16 = (uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        buf16[i] = inw(0x1F0); // Use explicit port instead of ATA_PRIMARY_CMD
    }
    
    log("Read completed successfully\n");
    return 0;
}

int ata_write_sector(uint32_t lba, const void* buffer) {
    log("ATA write sector ");
    char buf[12];
    int_to_chars(lba, buf, sizeof(buf));
    log_buffer(buf);
    log("\n");
    
    print_ata_status("Before write");
    
    if (ata_wait_bsy_clear() != 0) {
        log("Write: BSY clear failed\n");
        return -1;
    }

    // Select drive and set LBA
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)(lba & 0xFF));
    outb(0x1F4, (uint8_t)((lba >> 8) & 0xFF));
    outb(0x1F5, (uint8_t)((lba >> 16) & 0xFF));
    outb(0x1F7, 0x30); // WRITE SECTORS command

    print_ata_status("After write command");

    if (ata_wait_drq_set() != 0) {
        log("Write: DRQ set failed\n");
        return -2;
    }


    // Write data
    const uint16_t* buf16 = (const uint16_t*)buffer;
    for (int i = 0; i < 256; i++) {
        outw(ATA_PRIMARY_CMD, buf16[i]);
    }

    // Flush cache
    outb(0x1F7, 0xE7);
    ata_wait_bsy_clear();
    
    log("Write completed successfully\n");
    return 0;
}

#endif
