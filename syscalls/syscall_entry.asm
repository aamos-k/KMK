extern syscall_handler

[bits 32]
global syscall_entry
syscall_entry:
    pusha
    push ds
    push es
    push fs
    push gs

    pushad
    mov eax, esp
    push eax            ; Pass register state

    call syscall_handler
    add esp, 4          ; Clean up parameter

    popad

    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

