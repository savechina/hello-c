/**
 * @file test_function_pointers_sample.c
 * @brief Entry smoke + assertions on exposed op helpers, dispatch and mapping.
 */

#include "unity.h"
#include "basic/function_pointers_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_function_pointers_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_function_pointers_sample());
}

void test_binary_op_helpers_compute_correctly(void)
{
    TEST_ASSERT_EQUAL_INT32(5, add(2, 3));
    TEST_ASSERT_EQUAL_INT32(6, sub(10, 4));
    TEST_ASSERT_EQUAL_INT32(21, mul(3, 7));
    TEST_ASSERT_EQUAL_INT32(0, sub(42, 42));
    TEST_ASSERT_EQUAL_INT32(-6, mul(2, -3));
}

void test_apply_op_dispatches_through_function_pointer(void)
{
    TEST_ASSERT_EQUAL_INT32(42, apply_op(6, 7, mul));
    TEST_ASSERT_EQUAL_INT32(7, apply_op(10, 3, sub));
    TEST_ASSERT_EQUAL_INT32(9, apply_op(4, 5, add));
}

void test_unary_ops_and_apply_unary_to_array(void)
{
    TEST_ASSERT_EQUAL_INT32(25, square(5));
    TEST_ASSERT_EQUAL_INT32(-7, negate(7));
    TEST_ASSERT_EQUAL_INT32(0, square(0));

    int32_t nums[4] = {1, 2, 3, 4};
    int32_t squared[4] = {1, 4, 9, 16};
    int32_t negated[4] = {-1, -4, -9, -16};

    apply_unary_to_array(nums, 4, square);
    TEST_ASSERT_EQUAL_INT_ARRAY(squared, nums, 4);

    apply_unary_to_array(nums, 4, negate);
    TEST_ASSERT_EQUAL_INT_ARRAY(negated, nums, 4);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_function_pointers_sample_runs_without_undefined_behavior);
    RUN_TEST(test_binary_op_helpers_compute_correctly);
    RUN_TEST(test_apply_op_dispatches_through_function_pointer);
    RUN_TEST(test_unary_ops_and_apply_unary_to_array);
    return UNITY_END();
}
