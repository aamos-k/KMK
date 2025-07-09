#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>
#include <stddef.h>

static inline int syscall_exit(int num) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num)                       // input: eax = syscall number
    );
    return ret;
}
static inline int syscall_file(int num, const char*  arg1, const void* arg2, int arg3) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num),                      // input: eax = syscall number
          "b" (arg1),                     // ebx = 1st arg
          "c" (arg2),                     // ecx = 2nd arg
          "d" (arg3)                      // edx = 3rd arg
        : "memory"
    );
    return ret;
}
static inline int syscall_unlink(int num, const char*  arg1) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num),                      // input: eax = syscall number
          "b" (arg1)                      // ebx = 1st arg
    );
    return ret;
}
static inline int syscall_rename(int num, const char*  arg1, const char*  arg2) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num),                      // input: eax = syscall number
          "b" (arg1),                      // ebx = 1st arg
          "c" (arg2)                     // ecx = 2nd arg
    );
    return ret;
}
static inline int syscall_truncate(int num, const char*  arg1, int  arg2) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num),                      // input: eax = syscall number
          "b" (arg1),                      // ebx = 1st arg
          "c" (arg2)                     // ecx = 2nd arg
    );
    return ret;
}
static inline int syscall_chmod(int num, const char*  arg1, uint8_t arg2) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num),                      // input: eax = syscall number
          "b" (arg1)                      // ebx = 1st arg
    );
    return ret;
}
static inline int syscall_pipe(int num, int  arg1) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num)                        // input: eax = syscall number
    );
    return ret;
}
static inline int syscall_sched_yield(int num) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num) 
   );
    return ret;
}
static inline int syscall_getchar(int num) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num) 
    );
    return ret;
}
static inline int syscall_kill(int num, int arg1) {
    int ret;
    asm volatile (
        "int $0x80"
        : "=a" (ret)                      // output: eax return value
        : "a" (num)                      // input: eax = syscall number
    );
    return ret;
}

#endif
