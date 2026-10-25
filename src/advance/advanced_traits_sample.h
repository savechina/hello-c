#ifndef ADVANCED_TRAITS_SAMPLE_H
#define ADVANCED_TRAITS_SAMPLE_H

/**
 * @brief Advanced Traits chapter — 高级多态（Function Pointer VTable）
 *
 * Demonstrates:
 *   1. Function pointer tables (dispatch tables)
 *   2. VTable-like struct for runtime polymorphism
 *   3. Interface pattern: struct of function pointers + void* data
 *   4. Dynamic dispatch via Shape interface (Circle, Rectangle)
 *   5. Error-first: NULL function pointer guards
 *
 * Called from main_advance() → hello.c → main.c
 */
int main_advanced_traits_sample(void);

#endif /* ADVANCED_TRAITS_SAMPLE_H */
