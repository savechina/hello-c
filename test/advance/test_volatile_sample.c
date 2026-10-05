/**
 * @file test_volatile_sample.c
 * @brief Unit tests for the volatile chapter (volatile_sample.c).
 *
 * Guards the two invariants the chapter promises:
 * - the full demo entry terminates deterministically and returns 0
 * - the signal()/raise() demo always leaves the volatile flag set,
 *   and is repeatable (handler state is restored between calls)
 */

#include "unity.h"
#include "advance/volatile_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/**
 * @brief The whole chapter demo must run to completion and return success.
 */
void test_volatile_sample_entry_returns_zero(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_volatile_sample());
}

/**
 * @brief signal()/raise() demo must deterministically set the flag.
 *
 * Repeated calls must keep returning 1: each call reinstalls the handler
 * and restores SIG_DFL afterwards, so no state leaks between runs.
 */
void test_volatile_demo_poll_sets_flag(void)
{
    TEST_ASSERT_EQUAL_INT(1, volatile_demo_poll());
    TEST_ASSERT_EQUAL_INT(1, volatile_demo_poll());
    TEST_ASSERT_EQUAL_INT(1, volatile_demo_poll());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_volatile_sample_entry_returns_zero);
    RUN_TEST(test_volatile_demo_poll_sets_flag);
    return UNITY_END();
}
