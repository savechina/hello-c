/**
 * @file test_safe_strings_sample.c
 * @brief Entry smoke: bounded string-op demos are static void with no
 *        assertable helper, so coverage is the entry + sanitizer run.
 */

#include "unity.h"
#include "basic/safe_strings_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_safe_strings_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_safe_strings_sample());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_safe_strings_sample_runs_without_undefined_behavior);
    return UNITY_END();
}
