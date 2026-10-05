/**
 * @file test_pointers_and_arrays_sample.c
 * @brief Entry smoke: array/pointer walking demos are static void with no
 *        assertable helper, so coverage is the entry + sanitizer run.
 */

#include "unity.h"
#include "basic/pointers_and_arrays_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_pointers_and_arrays_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_pointers_and_arrays_sample());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_pointers_and_arrays_sample_runs_without_undefined_behavior);
    return UNITY_END();
}
