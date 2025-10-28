global isr_stub_table
extern isr_common_stub

section .text
isr_stub_table:
%assign i 0
%rep 32
    push byte i
    jmp isr_common_stub
%assign i i+1
%endrep

