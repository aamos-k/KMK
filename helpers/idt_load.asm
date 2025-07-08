global idt_load
extern idtp
section .bss
align 8
idt:       resb 256*8
idt_end:

section .data
idt_descriptor:
    dw idt_end - idt - 1
    dd idt

section .text
idt_load:
    lidt [idtp]
    ret


