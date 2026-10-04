/**
 * @file test_async_thread_lifecycle.c
 * @brief Smoke test: runs all four thread demos under the sanitizer build.
 *
 * Mutation-verified: the detached-thread `&stack_local` bug is caught by
 * `make asan` (3/3), NOT by this test (0/3 — different frame reuse).
 * `make asan` is the gate for that bug; this test only catches crashes here.
 */

#include "unity.h"
#include "advance/async_thread_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_async_thread_sample_runs_without_memory_errors(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_async_thread_sample());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_async_thread_sample_runs_without_memory_errors);
    return UNITY_END();
}
