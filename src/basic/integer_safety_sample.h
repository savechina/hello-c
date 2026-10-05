#ifndef INTEGER_SAFETY_SAMPLE_H
#define INTEGER_SAFETY_SAMPLE_H

#include <stdint.h>

/**
 * @brief Entry of the integer-safety chapter (CERT INT30/31/33-C flavor).
 */
int main_integer_safety_sample(void);

/**
 * @brief Add a + b without ever executing signed overflow (CERT INT30-C).
 *
 * Guard arithmetic itself is overflow-free by construction: the check for
 * b > 0 uses INT_MAX - b, the check for b < 0 uses INT_MIN - b.
 *
 * @return 1 on success (*out = a + b), 0 if the sum would overflow or
 *         out is NULL (*out untouched).
 */
int checked_add_int(int a, int b, int *out);

/**
 * @brief Left-shift a uint32_t after verifying shift < width (CERT INT33-C).
 *
 * @return 1 on success (*out = value << shift), 0 if shift >= 32 or
 *         out is NULL (*out untouched).
 */
int checked_shl_uint(uint32_t value, unsigned shift, uint32_t *out);

#endif /* INTEGER_SAFETY_SAMPLE_H */
