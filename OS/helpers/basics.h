#ifndef BASICS_H
#define BASICS_H

#include <stdint.h>
#include <stddef.h>

extern int col;
extern int row;
extern volatile char* const VIDEO_MEMORY;

int strcmp(const char *a, const char *b);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
int strlen(const char *str);
int strncmp(const char *s1, const char *s2, int n);
int simple_atoi(const char* str);
static char *trim(char *s);
void char_to_string(char c, char *out);
void int_to_hex(uint8_t val, char* out);
void int_to_chars(int num, char *buf, int buf_size);
void scroll();
void putc(char c);
void lputc(char c);
void bputc(char c);
void print_logo(const char* str);
void print_bear(const char* str);
void print(const char* str);
void print_buffer_n(const char *buffer, int len);
void print_buffer(const char *buffer);
void panic(const char *msg);

#endif
