#include "syscalls/syscalls.h"
#include <stdint.h>

extern char buffer[12];  // Define it, don't extern it

void load_user_program() {  // Standard entry point name
    const char* msg  = "Hello, kernel!\n";
    const char* name = "hello";
    
    syscall_file(2, name, msg, 15);  // sizeof returns pointer size, use literal
    syscall_file(3, name, buffer, 12);
    
    syscall_exit(1);  // Exit cleanly instead of infinite loop
}
