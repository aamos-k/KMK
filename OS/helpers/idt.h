#ifndef IDT_H
#define IDT_H

#include <stdint.h>


struct IDTEntry {
    uint16_t base_low;
    uint16_t sel;
    uint8_t  always0;
    uint8_t  flags;
    uint16_t base_high;
} __attribute__((packed));

struct IDTDescriptor {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

extern struct IDTEntry idt[256];
extern struct IDTDescriptor idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_install();
void idt_load(); // Implemented in assembly

#endif

