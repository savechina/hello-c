#ifndef SYSTEM_PROCESS_SAMPLE_H
#define SYSTEM_PROCESS_SAMPLE_H

/**
 * @brief Process Management — 进程管理
 *
 * Demonstrates:
 *   1. fork() — create child process
 *   2. exec() — replace process image
 *   3. wait() — collect child process
 *   4. Zombie process concept
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_system_process_sample(void);

#endif /* SYSTEM_PROCESS_SAMPLE_H */
