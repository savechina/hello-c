#ifndef FUNCTION_POINTERS_SAMPLE_H
#define FUNCTION_POINTERS_SAMPLE_H

#include <stdint.h>

int main_function_pointers_sample(void);

int32_t add(int32_t a, int32_t b);
int32_t sub(int32_t a, int32_t b);
int32_t mul(int32_t a, int32_t b);

int32_t apply_op(int32_t a, int32_t b, int32_t (*op)(int32_t, int32_t));

int32_t square(int32_t x);
int32_t negate(int32_t x);
void apply_unary_to_array(int32_t arr[], int32_t len, int32_t (*op)(int32_t));

#endif /* FUNCTION_POINTERS_SAMPLE_H */
