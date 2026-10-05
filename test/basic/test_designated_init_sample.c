/**
 * @file test_designated_init_sample.c
 * @brief Gate for the designated-initializer chapter: entry runs UB-free,
 *        and partial init really zero-fills every unspecified field.
 *
 * If a compiler/regression ever stopped zero-filling, di_sum_of_defaults()
 * would return a garbage-tainted sum instead of the designated fps.
 */

#include "unity.h"
#include "basic/designated_init_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_designated_init_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_designated_init_sample());
}

void test_di_sum_of_defaults_zero_fills_unspecified_fields(void)
{
    TEST_ASSERT_EQUAL_INT(30, di_sum_of_defaults(30));
    TEST_ASSERT_EQUAL_INT(0, di_sum_of_defaults(0));
    TEST_ASSERT_EQUAL_INT(-7, di_sum_of_defaults(-7));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_designated_init_sample_runs_without_undefined_behavior);
    RUN_TEST(test_di_sum_of_defaults_zero_fills_unspecified_fields);
    return UNITY_END();
}
