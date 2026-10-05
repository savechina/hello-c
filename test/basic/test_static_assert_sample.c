/**
 * @file test_static_assert_sample.c
 * @brief Gate for the _Static_assert chapter: entry runs UB-free, and the
 *        clamp helper honours its [0, 255] contract on both boundaries.
 *
 * The real asserts already ran at compile time — a broken sizeof/layout
 * assumption turns `make test` red before any binary exists.
 */

#include "unity.h"
#include "basic/static_assert_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_static_assert_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_static_assert_sample());
}

void test_sa_clamp_u8_clamps_to_range(void)
{
    TEST_ASSERT_EQUAL_INT(0, sa_clamp_u8(-5));
    TEST_ASSERT_EQUAL_INT(255, sa_clamp_u8(999));
    TEST_ASSERT_EQUAL_INT(42, sa_clamp_u8(42));
    TEST_ASSERT_EQUAL_INT(0, sa_clamp_u8(0));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_static_assert_sample_runs_without_undefined_behavior);
    RUN_TEST(test_sa_clamp_u8_clamps_to_range);
    return UNITY_END();
}
