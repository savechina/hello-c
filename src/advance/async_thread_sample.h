#ifndef ASYNC_THREAD_SAMPLE_H
#define ASYNC_THREAD_SAMPLE_H

/**
 * @brief Sub-chapter 1: Thread Creation & Lifecycle
 *
 * Demonstrates:
 *   1. pthread_create + pthread_join — basic thread lifecycle
 *   2. Passing struct data via void*
 *   3. Returning results via pthread_exit + pthread_join retval
 *   4. Detached threads (pthread_detach) for fire-and-forget
 *
 * Called from main_advance() → advance.c → hello.c → main.c
 * Compile with: -pthread
 */
int main_async_thread_sample(void);

#endif /* ASYNC_THREAD_SAMPLE_H */
