#ifndef SYSTEM_SIGNAL_SAMPLE_H
#define SYSTEM_SIGNAL_SAMPLE_H

/**
 * @brief POSIX Signal Handling — POSIX 信号处理
 *
 * Demonstrates:
 *   1. sigaction() for signal handler registration
 *   2. sigprocmask() for blocking/unblocking signals
 *   3. SA_RESTART flag explanation
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_system_signal_sample(void);

#endif /* SYSTEM_SIGNAL_SAMPLE_H */
