; Assembly code to push registers and call a C handler
extern isr_handler

global isr_common_stub

isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    ; Set up segment registers
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp             ; Pass pointer to registers
    call isr_handler     ; Call the C ISR handler
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 4           ; pop error code or fake placeholder
    iret

