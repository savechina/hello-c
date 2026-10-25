#ifndef SYSTEM_MMAP_SAMPLE_H
#define SYSTEM_MMAP_SAMPLE_H

/**
 * @brief Memory-Mapped I/O (mmap / munmap) — 内存映射 I/O
 *
 * Demonstrates:
 *   1. mmap read — map file and read as memory
 *   2. mmap write — map file and write via pointer
 *   3. mmap performance comparison concept
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_system_mmap_sample(void);

#endif /* SYSTEM_MMAP_SAMPLE_H */
