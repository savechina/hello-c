/**
 * @file test_string_processing_sample.c
 * @brief Entry smoke: tokenizer/trim logic is inlined inside static void
 *        demo functions (extraction would be a refactor), so coverage is
 *        the entry + sanitizer run.
 */

#include "unity.h"
#include "basic/string_processing_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_string_processing_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_string_processing_sample());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_string_processing_sample_runs_without_undefined_behavior);
    return UNITY_END();
}
