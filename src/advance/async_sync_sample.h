#ifndef ASYNC_SYNC_SAMPLE_H
#define ASYNC_SYNC_SAMPLE_H

/**
 * @brief Sub-chapter 2: Synchronization Primitives
 *
 * Demonstrates:
 *   1. Race condition — shared counter without mutex
 *   2. Mutex fix — pthread_mutex_t protects the shared counter
 *   3. Condition variable — producer/consumer with pthread_cond_t
 *   4. Atomic comparison — C11 atomic_int vs mutex performance
 *
 * Called from main_advance() → advance.c → hello.c → main.c
 * Compile with: -pthread
 */
int main_async_sync_sample(void);

#endif /* ASYNC_SYNC_SAMPLE_H */
