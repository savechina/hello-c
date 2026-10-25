#ifndef ASYNC_POOL_SAMPLE_H
#define ASYNC_POOL_SAMPLE_H

/**
 * @brief Sub-chapter 3: Thread Pool Pattern
 *
 * Demonstrates:
 *   1. ThreadPool creation — fixed workers + ring-buffer queue
 *   2. Task submission — enqueue work items
 *   3. Graceful shutdown — flush queue, join workers
 *
 * Called from main_advance() → advance.c → hello.c → main.c
 * Compile with: -pthread
 */
int main_async_pool_sample(void);

#endif /* ASYNC_POOL_SAMPLE_H */
