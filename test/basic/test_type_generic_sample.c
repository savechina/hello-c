/**
 * @file test_type_generic_sample.c
 * @brief Gate for the _Generic chapter: entry runs UB-free, and the
 *        non-static helpers behind the macros return correct values.
 *
 * `make test`      → smoke only. `make test-asan` → UBSan proves the
 *                    double-evaluation demo is a *wrong result*, not UB.
 */

#include "unity.h"
#include "basic/type_generic_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_type_generic_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_type_generic_sample());
}

void test_tg_max_int_returns_larger_operand(void)
{
    TEST_ASSERT_EQUAL_INT(7, tg_max_int(3, 7));
    TEST_ASSERT_EQUAL_INT(-2, tg_max_int(-2, -9));
    TEST_ASSERT_EQUAL_INT(5, tg_max_int(5, 5));
}

void test_tg_min_int_returns_smaller_operand(void)
{
    TEST_ASSERT_EQUAL_INT(3, tg_min_int(3, 7));
    TEST_ASSERT_EQUAL_INT(-9, tg_min_int(-2, -9));
    TEST_ASSERT_EQUAL_INT(5, tg_min_int(5, 5));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_type_generic_sample_runs_without_undefined_behavior);
    RUN_TEST(test_tg_max_int_returns_larger_operand);
    RUN_TEST(test_tg_min_int_returns_smaller_operand);
    return UNITY_END();
}
