#ifndef CALC_H
#define CALC_H

/**
 * @brief Calculator utility functions for the advance tutorial.
 *
 * Provides basic arithmetic operations and a validation helper.
 * Called from advance.c examples demonstrating modular code organization.
 */

/**
 * @brief Add two integers.
 *
 * @param a First operand
 * @param b Second operand
 * @return int Sum of a and b
 */
int calc_add(int a, int b);

/**
 * @brief Multiply two integers.
 *
 * @param a First operand
 * @param b Second operand
 * @return int Product of a and b
 */
int calc_multiply(int a, int b);

/**
 * @brief Validate a result value.
 *
 * Returns 1 if result is positive (greater than 0), 0 otherwise.
 *
 * @param result The value to validate
 * @return int 1 if valid (positive), 0 otherwise
 */
int calc_is_valid(int result);

#endif /* CALC_H */
