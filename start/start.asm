global _start
extern kernel_main
global magic_number
global mb_info_ptr

section .text
_start:
    mov [magic_number], eax   ; Save magic number from GRUB
    mov [mb_info_ptr], ebx    ; Save multiboot info pointer from GRUB
    call kernel_main

.halt_loop:
    hlt
    jmp .halt_loop

section .bss
magic_number  resd 1
mb_info_ptr   resd 1

