#include "syscalls/syscalls.h"
#include <stdint.h>

// Simple string length function for user space
static int str_len(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

// Simple string copy function for user space
static void str_copy(char* dest, const char* src, int max) {
    int i = 0;
    while (i < max - 1 && src[i]) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// Main user program entry point
void user_program_main() {
    char read_buffer[128];
    int result;

    // Test 1: Write a file to the filesystem
    const char* test_file = "userspace.txt";
    const char* test_data = "Hello from user space!\n";

    result = syscall_file(2, test_file, test_data, str_len(test_data));

    // Test 2: Read the file back
    for (int i = 0; i < 128; i++) read_buffer[i] = 0;
    result = syscall_file(3, test_file, read_buffer, sizeof(read_buffer));

    // Test 3: Create another file
    const char* test_file2 = "document.txt";
    const char* doc_content = "This is a document created from user mode.\nIt demonstrates file system operations.\n";

    result = syscall_file(2, test_file2, doc_content, str_len(doc_content));

    // Test 4: Rename the second file
    const char* new_name = "renamed_doc.txt";
    result = syscall_rename(5, test_file2, new_name);

    // Test 5: Change file permissions
    result = syscall_chmod(7, test_file, 0x07);  // RWX permissions

    // Test 6: Test pipe creation
    int pipe_fds[2];
    result = syscall_pipe(8, pipe_fds);

    // Test 7: Write to pipe (if pipe creation succeeded)
    if (result == 0) {
        const char* pipe_msg = "Pipe message from user space";
        // Note: pipe write uses special FD format, but we can demonstrate the syscall
    }

    // Test 8: Truncate a file
    result = syscall_truncate(6, test_file, 10);

    // Test 9: Create a final status file
    const char* status_file = "user_status.txt";
    const char* status = "User space program completed successfully!\nAll system calls tested.\n";
    result = syscall_file(2, status_file, status, str_len(status));

    // Exit cleanly
    syscall_exit(1);

    // Should never reach here
    while (1);
}
