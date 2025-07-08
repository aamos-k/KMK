#include "syscalls.h"

void load_user_program() {
    const char* msg  = "Hello, kernel!\n";
    const char* name = "hello";
    syscall(2, (int)name, (int)msg, sizeof("Hello, kernel!\n") - 1);
    syscall(1,0,0,0); // sys_exit
}

