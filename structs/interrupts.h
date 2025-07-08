// structs/interrupts.h
#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
#include "registers.h"

void register_interrupt_handler(int n, void (*handler)(struct registers*));

typedef struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} registers_t;



#endif

