#ifndef ERROR_HANDLING_SAMPLE_H
#define ERROR_HANDLING_SAMPLE_H

/**
 * @brief Error handling chapter for the advance tutorial.
 *
 * Demonstrates:
 *   1. errno pattern + errno.h
 *   2. perror / strerror
 *   3. setjmp / longjmp for error recovery
 *   4. Error callback chains
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_error_handling_sample(void);

#endif /* ERROR_HANDLING_SAMPLE_H */
