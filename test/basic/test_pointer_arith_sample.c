/**
 * @file test_pointer_arith_sample.c
 * @brief Entry smoke: pointer-arithmetic demos are static void with no
 *        assertable helper, so coverage is the entry + sanitizer run
 *        (catches one-past-end mistakes at runtime).
 */

#include "unity.h"
#include "basic/pointer_arith_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_pointer_arith_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_pointer_arith_sample());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_pointer_arith_sample_runs_without_undefined_behavior);
    return UNITY_END();
}
