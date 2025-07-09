#include "idt.h"
#include <string.h> // For memset

struct IDTEntry idt[256];
struct IDTDescriptor idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

void idt_install() {
    idtp.limit = sizeof(struct IDTEntry) * 256 - 1; // Calculate the correct limit
    idtp.base = (uint32_t)&idt;
    memset(&idt, 0, sizeof(idt));
    idt_load();
}

