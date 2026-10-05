/**
 * @file test_pointers_and_functions_sample.c
 * @brief Entry smoke + pass-by-value vs pass-by-pointer contract assertions.
 */

#include "unity.h"
#include "basic/pointers_and_functions_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_pointers_and_functions_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_pointers_and_functions_sample());
}

void test_swap_by_pointer_exchanges_caller_values(void)
{
    int32_t x = 10;
    int32_t y = 20;

    swap_by_pointer(&x, &y);

    TEST_ASSERT_EQUAL_INT32(20, x);
    TEST_ASSERT_EQUAL_INT32(10, y);
}

void test_swap_by_value_leaves_caller_values_unchanged(void)
{
    int32_t x = 10;
    int32_t y = 20;

    swap_by_value(x, y);

    TEST_ASSERT_EQUAL_INT32(10, x);
    TEST_ASSERT_EQUAL_INT32(20, y);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_pointers_and_functions_sample_runs_without_undefined_behavior);
    RUN_TEST(test_swap_by_pointer_exchanges_caller_values);
    RUN_TEST(test_swap_by_value_leaves_caller_values_unchanged);
    return UNITY_END();
}
