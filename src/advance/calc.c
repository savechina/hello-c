// calc.c - Extracted from testing_sample.c for Unity testability

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

/**
 * @brief Adds two integers
 * @param a First operand
 * @param b Second operand
 * @return Result of a + b
 */
int calc_add(int a, int b)
{
    return a + b;
}

/**
 * @brief Multiplies two integers
 * @param a First operand
 * @param b Second operand
 * @return Result of a * b
 */
int calc_multiply(int a, int b)
{
    return a * b;
}

/**
 * @brief Validates result is within [0, 10000] range
 * @param result The value to check
 * @return 1 if 0 <= result <= 10000, 0 otherwise
 */
int calc_is_valid(int result)
{
    return (result >= 0 && result <= 10000) ? 1 : 0;
}
