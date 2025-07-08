#include "syscalls.h"

extern char buffer[12];

void load_user_program() {
    const char* msg  = "Hello, kernel!\n";
    const char* name = "hello";
    syscall(2, (int)name, (int)msg, sizeof("Hello, kernel!\n") - 1);
    syscall(3, (int)name, (int)buffer, sizeof(buffer) - 1);
    syscall(1,0,0,0); // sys_exit
}

