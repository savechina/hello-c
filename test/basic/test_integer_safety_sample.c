/**
 * @file test_integer_safety_sample.c
 * @brief Gate for the integer-safety chapter.
 *
 * The entry runs under `make test-asan`: UBSan would abort on any signed
 * overflow or shift >= width, so these tests prove the checked helpers
 * *reject* dangerous inputs instead of executing them.
 */

#include <limits.h>
#include <stdint.h>

#include "unity.h"
#include "basic/integer_safety_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_integer_safety_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_integer_safety_sample());
}

void test_checked_add_int_sums_within_range(void)
{
    int out = -1;

    TEST_ASSERT_TRUE(checked_add_int(2, 3, &out));
    TEST_ASSERT_EQUAL_INT(5, out);

    TEST_ASSERT_TRUE(checked_add_int(-7, 4, &out));
    TEST_ASSERT_EQUAL_INT(-3, out);

    /* INT_MAX + INT_MIN == -1: guards must not false-positive at the edges */
    TEST_ASSERT_TRUE(checked_add_int(INT_MAX, INT_MIN, &out));
    TEST_ASSERT_EQUAL_INT(-1, out);
}

void test_checked_add_int_rejects_overflow_without_executing_it(void)
{
    int out = 12345;

    TEST_ASSERT_FALSE(checked_add_int(INT_MAX, 1, &out));
    TEST_ASSERT_FALSE(checked_add_int(INT_MIN, -1, &out));
    TEST_ASSERT_FALSE(checked_add_int(INT_MAX, INT_MAX, &out));
    TEST_ASSERT_EQUAL_INT(12345, out);
}

void test_checked_shl_uint_guards_shift_width(void)
{
    uint32_t out = 0;

    TEST_ASSERT_TRUE(checked_shl_uint(1u, 31, &out));
    TEST_ASSERT_EQUAL_UINT32(0x80000000u, out);

    TEST_ASSERT_TRUE(checked_shl_uint(0x1234u, 8, &out));
    TEST_ASSERT_EQUAL_UINT32(0x123400u, out);

    TEST_ASSERT_FALSE(checked_shl_uint(1u, 32, &out));
    TEST_ASSERT_FALSE(checked_shl_uint(1u, 64, &out));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_integer_safety_sample_runs_without_undefined_behavior);
    RUN_TEST(test_checked_add_int_sums_within_range);
    RUN_TEST(test_checked_add_int_rejects_overflow_without_executing_it);
    RUN_TEST(test_checked_shl_uint_guards_shift_width);
    return UNITY_END();
}
