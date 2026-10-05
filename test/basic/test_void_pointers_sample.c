/**
 * @file test_void_pointers_sample.c
 * @brief Entry smoke + assertions on the exposed cmp_int32() qsort comparator.
 */

#include <stdlib.h>

#include "unity.h"
#include "basic/void_pointers_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_void_pointers_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_void_pointers_sample());
}

void test_cmp_int32_returns_signed_ordering(void)
{
    int32_t a = 7;
    int32_t b = 3;

    TEST_ASSERT_EQUAL_INT(1, cmp_int32(&a, &b));
    TEST_ASSERT_EQUAL_INT(-1, cmp_int32(&b, &a));
    TEST_ASSERT_EQUAL_INT(0, cmp_int32(&a, &a));
}

void test_cmp_int32_sorts_ascending_via_qsort(void)
{
    int32_t nums[] = {33, 10, 75, 42, 5};
    int32_t expected[] = {5, 10, 33, 42, 75};
    size_t count = sizeof(nums) / sizeof(nums[0]);

    qsort(nums, count, sizeof(nums[0]), cmp_int32);

    TEST_ASSERT_EQUAL_INT_ARRAY(expected, nums, (int)count);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_void_pointers_sample_runs_without_undefined_behavior);
    RUN_TEST(test_cmp_int32_returns_signed_ordering);
    RUN_TEST(test_cmp_int32_sorts_ascending_via_qsort);
    return UNITY_END();
}
