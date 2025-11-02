global syscall_entry
extern syscall_handler

syscall_entry:
    ; Save all registers
    pusha
    push ds
    push es
    push fs
    push gs
    
    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call C handler (stack pointer = struct registers*)
    push esp
    call syscall_handler
    add esp, 4
    
    ; Restore registers
    pop gs
    pop fs
    pop es
    pop ds
    popa
    
    iret
