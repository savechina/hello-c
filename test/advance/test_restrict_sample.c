/**
 * @file test_restrict_sample.c
 * @brief Unit tests for the restrict chapter (restrict_sample.c).
 *
 * Guards the invariants the chapter promises:
 * - the full demo entry terminates deterministically and returns 0
 * - add_arrays computes element-wise sums on disjoint buffers
 * - add_arrays handles the zero-length edge case without touching dst
 */

#include "unity.h"
#include "advance/restrict_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/**
 * @brief The whole chapter demo must run to completion and return success.
 */
void test_restrict_sample_entry_returns_zero(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_restrict_sample());
}

/**
 * @brief add_arrays must produce exact element-wise sums on disjoint buffers.
 */
void test_add_arrays_sums_elementwise(void)
{
    const double a[4] = {1.0, 2.0, 3.0, 4.0};
    const double b[4] = {10.0, 20.0, 30.0, 40.0};
    double dst[4] = {0.0, 0.0, 0.0, 0.0};

    add_arrays(dst, a, b, 4);

    TEST_ASSERT_EQUAL_FLOAT(11.0, dst[0]);
    TEST_ASSERT_EQUAL_FLOAT(22.0, dst[1]);
    TEST_ASSERT_EQUAL_FLOAT(33.0, dst[2]);
    TEST_ASSERT_EQUAL_FLOAT(44.0, dst[3]);
}

/**
 * @brief n == 0 must not write dst at all (pre-filled sentinel survives).
 *
 * Uses binary-exact fractions (0.5/0.25) so the equality asserts are
 * representable without any floating-point tolerance slack.
 */
void test_add_arrays_zero_length_writes_nothing(void)
{
    const double a[2] = {0.5, 0.25};
    const double b[2] = {0.25, 0.5};
    double dst[2] = {123.0, 456.0};

    add_arrays(dst, a, b, 0);

    TEST_ASSERT_EQUAL_FLOAT(123.0, dst[0]);
    TEST_ASSERT_EQUAL_FLOAT(456.0, dst[1]);

    add_arrays(dst, a, b, 2);
    TEST_ASSERT_EQUAL_FLOAT(0.75, dst[0]);
    TEST_ASSERT_EQUAL_FLOAT(0.75, dst[1]);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_restrict_sample_entry_returns_zero);
    RUN_TEST(test_add_arrays_sums_elementwise);
    RUN_TEST(test_add_arrays_zero_length_writes_nothing);
    return UNITY_END();
}
