/**
 * @file test_inline_functions_sample.c
 * @brief Gate for the C99 inline-functions chapter: computed-value asserts,
 *        not return-0 smoke.
 *
 *   - inl_clamp_via_call(v,lo,hi) == expected → the header static-inline
 *     helper computes the clamped value
 *   - inl_same_result_via_address() == 1      → direct call and address-taken
 *     call agree (inline never changes semantics)
 *
 * The tautological "entry returns 0" smoke is intentionally omitted — the
 * whole point of this chapter is that inline is only an optimization hint.
 */

#include "unity.h"
#include "basic/inline_functions_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_inl_clamp_via_call_returns_computed_value(void)
{
    TEST_ASSERT_EQUAL_INT(5, inl_clamp_via_call(5, 0, 10));
    TEST_ASSERT_EQUAL_INT(0, inl_clamp_via_call(-3, 0, 10));
    TEST_ASSERT_EQUAL_INT(10, inl_clamp_via_call(99, 0, 10));
    TEST_ASSERT_EQUAL_INT(7, inl_clamp_via_call(7, 7, 7));
}

void test_inl_same_result_via_address_proves_inline_is_hint(void)
{
    TEST_ASSERT_EQUAL_INT(1, inl_same_result_via_address());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_inl_clamp_via_call_returns_computed_value);
    RUN_TEST(test_inl_same_result_via_address_proves_inline_is_hint);
    return UNITY_END();
}
