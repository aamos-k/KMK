// isr.c
#include <stdint.h>
#include "helpers/basics.h"
#include "structs/registers.h" // make sure this exists and defines `struct registers`

char buffer_hex[12];

void isr_handler(struct registers **r) {
    if ((*r)->int_no == 14) {
        uint32_t cr2;
        asm volatile("mov %%cr2, %0" : "=r"(cr2));
    
        print("PAGE FAULT at ");
        int_to_chars(cr2, buffer_hex, sizeof(buffer_hex));
        print_buffer(buffer_hex);
        print("\n");
        panic("Unhandled page fault");
    }
}

