#ifndef WEB_CONCURRENT_SAMPLE_H
#define WEB_CONCURRENT_SAMPLE_H

/**
 * @brief Web Concurrent chapter — concurrent server models demo
 *
 * Demonstrates three server models:
 *   1. fork per connection (prefork model)
 *   2. thread per connection (pthread model)
 *   3. select/poll I/O multiplexing (single-threaded)
 *
 * Actual server binding is guarded by DEMO_ACTUAL_SERVER —
 * the tutorial demonstrates structure and explains without
 * occupying real ports.
 *
 * Platform: POSIX (fork, pthread, select)
 * Called from main_advance() → hello.c → main.c
 */
int main_web_concurrent_sample(void);

#endif /* WEB_CONCURRENT_SAMPLE_H */
