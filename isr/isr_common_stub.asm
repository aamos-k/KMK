; Assembly code to push registers and call a C handler
extern isr_handler

global isr_common_stub

isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10      ; kernel data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp           ; argument to C handler
    call isr_handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8         ; remove err_code + int_no
    iretd


