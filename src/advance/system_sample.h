#ifndef SYSTEM_CALLS_SAMPLE_H
#define SYSTEM_CALLS_SAMPLE_H

/**
 * @brief System Calls chapter — 系统调用（POSIX Signals, mmap, Process）
 *
 * Demonstrates:
 *   1. POSIX signals (sigaction, signal handlers)
 *   2. Memory-mapped files (mmap/munmap) with fd
 *   3. File descriptors (open, read, write, close)
 *   4. Process management (fork, wait, exec) — platform guarded
 *   5. Pipe IPC — platform guarded
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_system_sample(void);

#endif /* SYSTEM_CALLS_SAMPLE_H */
