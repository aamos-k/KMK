#include <stdint.h>
#include <stddef.h>

#define LOGO_WIDTH 31
#define LOGO_HEIGHT 25
#define BEAR_WIDTH 13
#define BEAR_HEIGHT 33
#define VIDEO_WIDTH 80
#define VIDEO_HEIGHT 25

volatile char* const VIDEO_MEMORY = (volatile char*)0xB8000;
int col = 0;
int row = 0;

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return *(unsigned char*)a - *(unsigned char*)b;
}

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest;
    const char *s = src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dest;
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    for (size_t i = 0; i < n; i++) p[i] = (unsigned char)c;
    return s;
}



char *strncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

// Correct strlen signature
int strlen(const char *str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

// Correct strncmp signature
int strncmp(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == '\0' || s2[i] == '\0') {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
    }
    return 0;
}

int simple_atoi(const char* str) {
    int result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

static char *trim(char *s) {
    while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
        ++s;

    char *end = s + strlen(s);
    while (end > s &&
          (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r' || end[-1] == '\n'))
        --end;
    *end = '\0';
    return s;
}

void char_to_string(char c, char *out) {
    out[0] = c;
    out[1] = '\0';
}

void int_to_hex(uint8_t val, char* out) {
    const char* hexchars = "0123456789ABCDEF";
    out[0] = hexchars[(val >> 4) & 0xF];
    out[1] = hexchars[val & 0xF];
    out[2] = '\0';
}

void int_to_chars(int num, char *buf, int buf_size) {
    if (buf_size == 0) return;

    int i = buf_size - 1;
    buf[i] = '\0'; // Null terminate

    if (num == 0) {
        if (i > 0) buf[--i] = '0';
    } else {
        int is_negative = 0;
        if (num < 0) {
            is_negative = 1;
            num = -num;
        }

        while (num > 0 && i > 0) {
            buf[--i] = '0' + (num % 10);
            num /= 10;
        }

        if (is_negative && i > 0) {
            buf[--i] = '-';
        }
    }

    // Move result to start of buffer
    int start = i, j = 0;
    while (buf[start] != '\0') {
        buf[j++] = buf[start++];
    }
    buf[j] = '\0';
}

void scroll() {
    if (row < VIDEO_HEIGHT) return;  // no scroll needed

    // Copy each row (starting from 1) to previous row
    for (int r = 1; r < VIDEO_HEIGHT; r++) {
        for (int c = 0; c < VIDEO_WIDTH; c++) {
            int src = 2 * (r * VIDEO_WIDTH + c);
            int dest = 2 * ((r - 1) * VIDEO_WIDTH + c);
            VIDEO_MEMORY[dest] = VIDEO_MEMORY[src];
            VIDEO_MEMORY[dest + 1] = VIDEO_MEMORY[src + 1];
        }
    }
    // Clear the last line
    int last_row = VIDEO_HEIGHT - 1;
    for (int c = 0; c < VIDEO_WIDTH; c++) {
        int pos = 2 * (last_row * VIDEO_WIDTH + c);
        VIDEO_MEMORY[pos] = ' ';
        VIDEO_MEMORY[pos + 1] = 0x07; // Light gray on black
    }

    row = VIDEO_HEIGHT - 1;
}

void putc(char c) {
    if (c == '\n') {
        col = 0;
        row++;
    } else {
        int index = 2 * (row * VIDEO_WIDTH + col);
        VIDEO_MEMORY[index] = c;
        VIDEO_MEMORY[index + 1] = 0x07; // color attribute
        col++;
        if (col >= VIDEO_WIDTH) {
            col = 0;
            row++;
        }
    }

    scroll();  // Scroll if needed
}

void print(const char* str) {
    while (*str) {
        char c = *str++;
        putc(c);
    }
}

void print_buffer_n(const char *buffer, int len) {
    for (int i = 0; i < len; i++) {
        putc(buffer[i]);
    }
}

void print_buffer(const char *buffer) {
    while (*buffer != '\0') {
        char c = *buffer++;
        putc(c);
    }
}


void panic(const char *msg) {
    print(msg);
    while (1) { __asm__("hlt"); }
}

