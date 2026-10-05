/**
 * @file test_string_basics_sample.c
 * @brief Entry smoke: all logic lives in static void demo_* printf functions,
 *        so coverage is the entry return code + ASan/UBSan during the run.
 */

#include "unity.h"
#include "basic/string_basics_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_string_basics_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_string_basics_sample());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_string_basics_sample_runs_without_undefined_behavior);
    return UNITY_END();
}
