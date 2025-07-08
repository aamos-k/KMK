#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stddef.h>
#include "basics.h"

#define COM1 0x3F8

void serial_init() {
    outb(0x3F8 + 1, 0x00);    // Disable interrupts
    outb(0x3F8 + 3, 0x80);    // Enable DLAB
    outb(0x3F8 + 0, 0x01);    // Divisor LSB (115200 baud)
    outb(0x3F8 + 1, 0x00);    // Divisor MSB
    outb(0x3F8 + 3, 0x03);    // 8 bits, no parity, one stop
    outb(0x3F8 + 2, 0xC7);    // FIFO, clear, 14-byte threshold
    outb(0x3F8 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}


int serial_is_transmit_ready() {
    return inb(COM1 + 5) & 0x20;
}

void serial_write(char a) {
    while (!serial_is_transmit_ready());
    outb(COM1, a);
}

void log(const char* str) {
    while (*str) {
        char c = *str++;
        serial_write(c);    
    }
}

void log_buffer_n(const char *buffer, int len) {
    for (int i = 0; i < len; i++) {
        serial_write(buffer[i]);
    }
}

void log_buffer(const char *buffer) {
    while (*buffer != '\0') {
        char str[2];
        char c = *buffer++;
        char_to_string(c,str);
        log(str);
    }
}

#endif
