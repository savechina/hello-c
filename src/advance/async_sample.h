#ifndef ASYNC_SAMPLE_H
#define ASYNC_SAMPLE_H

/**
 * @brief Async & POSIX Threads chapter for the advance tutorial.
 *
 * Demonstrates:
 *   1. pthread_create / pthread_join — thread lifecycle
 *   2. Race condition on shared counter (error-first)
 *   3. Mutex (pthread_mutex_t) — fixing the race condition
 *   4. Condition variables — producer/consumer pattern
 *   5. Thread-local storage (TLS) — per-thread data
 *
 * Called from main_advance() → hello.c → main.c
 * Compile with: -pthread
 */
int main_async_sample(void);

#endif /* ASYNC_SAMPLE_H */
