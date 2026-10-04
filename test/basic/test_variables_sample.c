/**
 * @file test_variables_sample.c
 * @brief Regression gate for the integer-overflow demo in variables_sample.c.
 *
 * `make test`      → smoke only (no instrumentation).
 * `make test-asan` → UBSan aborts on signed overflow, so reverting the demo
 *                    to `int32_t max + 1` turns this red instead of teaching UB.
 */

#include "unity.h"
#include "basic/variables_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_variables_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_variables_sample());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_variables_sample_runs_without_undefined_behavior);
    return UNITY_END();
}
