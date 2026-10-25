#ifndef ASYNC_IOMUX_SAMPLE_H
#define ASYNC_IOMUX_SAMPLE_H

/**
 * @brief Sub-chapter 4: I/O Multiplexing
 *
 * Demonstrates:
 *   1. select() — monitor multiple file descriptors
 *   2. Pipe multiplexing — detect which pipe has data
 *   3. epoll (#ifdef __linux__) — Linux-only, fallback on macOS
 *
 * Called from main_advance() → advance.c → hello.c → main.c
 * Compile with: -pthread (not strictly needed for I/O mux but for consistency)
 */
int main_async_iomux_sample(void);

#endif /* ASYNC_IOMUX_SAMPLE_H */
