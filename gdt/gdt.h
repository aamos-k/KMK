#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct gdt_entry {
    uint16_t limit_low;     
    uint16_t base_low;      
    uint8_t  base_middle;   
    uint8_t  access;        
    uint8_t  granularity;
    uint8_t  base_high;     
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_entry gdt[6];
struct gdt_ptr gp;

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

void gdt_flush(uint32_t gdt_ptr) {
    asm volatile (
        "lgdt (%0)\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "jmp $0x08, $.flush2\n"
        ".flush2:\n"
        : 
        : "r" (gdt_ptr)
        : "ax"
    );
}

void gdt_install() {
    gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
    gp.base = (uint32_t)&gdt;

    gdt_set_gate(0, 0, 0, 0, 0);                // Null segment
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Kernel code segment
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Kernel data segment
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User code segment (DPL = 3)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User data segment (DPL = 3)

    // Optionally: add TSS at index 5

    gdt_flush((uint32_t)&gp);
}

#endif
