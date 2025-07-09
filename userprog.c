#include "syscalls.h"

extern char buffer[12];

void load_user_program() {
    const char* msg  = "Hello, kernel!\n";
    const char* name = "hello";
    syscall_file(2, name, msg, sizeof("Hello, kernel!\n") - 1);
    syscall_file(3, name, buffer, sizeof(buffer));
    syscall_exit(1); // sys_exit
    
}

