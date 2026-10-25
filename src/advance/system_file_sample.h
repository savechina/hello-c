#ifndef SYSTEM_FILE_SAMPLE_H
#define SYSTEM_FILE_SAMPLE_H

/**
 * @brief File and Directory Operations — 文件与目录操作
 *
 * Demonstrates low-level file I/O using POSIX system calls:
 *   1. open(), read(), write(), close() — file descriptor operations
 *   2. stat() — file metadata inspection
 *   3. opendir(), readdir() — directory scanning
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_system_file_sample(void);

#endif /* SYSTEM_FILE_SAMPLE_H */
