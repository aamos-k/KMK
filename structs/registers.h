#ifndef REGISTERS_H
#define REGISTERS_H

// Structure passed to interrupt handlers from ISR stubs
struct registers {
    uint32_t ds;                  // Data segment selector
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;  // Pushed by pusha
    uint32_t int_no, err_code;    // Interrupt number and error code (if applicable)
    uint32_t eip, cs, eflags, useresp, ss;
};

#endif

