global isr_stub_table
extern isr_common_stub

section .text

isr_stub_table:

; ISRs 0–31
; Some exceptions push an error code automatically.
; We must adjust so every stub pushes a dummy error code if CPU doesn’t.

%macro ISR_NOERR 1
    global isr%1
isr%1:
    push dword 0          ; push dummy error code
    push dword %1         ; push interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
    global isr%1
isr%1:
    push dword %1         ; push interrupt number (error code already on stack)
    jmp isr_common_stub
%endmacro

; CPU exceptions that push an error code:
; 8 (Double Fault), 10 (Invalid TSS), 11 (Segment Not Present),
; 12 (Stack Fault), 13 (General Protection), 14 (Page Fault), 17 (Alignment Check)
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

