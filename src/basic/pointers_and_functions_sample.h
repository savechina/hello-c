#ifndef POINTERS_AND_FUNCTIONS_SAMPLE_H
#define POINTERS_AND_FUNCTIONS_SAMPLE_H

#include <stdint.h>

int main_pointers_and_functions_sample(void);

/* 值传递副本交换 — 调用者变量不变 */
void swap_by_value(int32_t a, int32_t b);

/* 指针传递原地交换 — 调用者变量被修改 */
void swap_by_pointer(int32_t *a, int32_t *b);

#endif /* POINTERS_AND_FUNCTIONS_SAMPLE_H */
