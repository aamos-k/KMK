#ifndef SYSCALLS_H
#define SYSCALLS_H

static inline int syscall(int num, int arg1, int arg2, int arg3) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num),                     // input: eax = syscall number
          "b" (arg1),                    // ebx = 1st arg
          "c" (arg2),                    // ecx = 2nd arg
          "d" (arg3)                     // edx = 3rd arg
        : "memory"
    );
    return ret;
}

#endif
