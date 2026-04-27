#ifndef SYSTEM_IPC_SAMPLE_H
#define SYSTEM_IPC_SAMPLE_H

/**
 * @brief Inter-Process Communication (pipe, socketpair) — 管道与进程间通信
 *
 * Demonstrates:
 *   1. Basic pipe — parent writes, child reads
 *   2. Bidirectional pipe — two pipes for two-way communication
 *   3. socketpair() — AF_UNIX full-duplex communication
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_system_ipc_sample(void);

#endif /* SYSTEM_IPC_SAMPLE_H */
