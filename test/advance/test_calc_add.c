/**
 * @file test_calc_add.c
 * @brief Unit tests for calc_add() function using Unity test framework.
 *
 * Tests basic addition, edge cases (zero), and negative number scenarios.
 */

#include "unity.h"
#include "advance/calc.h"

/**
 * @brief Setup function called before each test.
 *
 * Currently unused — no test-specific setup required.
 */
void setUp(void)
{
}

/**
 * @brief Teardown function called after each test.
 *
 * Currently unused — no cleanup required.
 */
void tearDown(void)
{
}

/**
 * @brief Test basic calc_add functionality.
 *
 * Verifies:
 * - Positive addition: calc_add(2, 3) == 5
 * - Zero addition:  calc_add(0, 0) == 0
 * - Sign neutralization: calc_add(-1, 1) == 0
 */
void test_calc_add_basic(void)
{
    TEST_ASSERT_EQUAL_INT(5, calc_add(2, 3));
    TEST_ASSERT_EQUAL_INT(0, calc_add(0, 0));
    TEST_ASSERT_EQUAL_INT(0, calc_add(-1, 1));
}

/**
 * @brief Test calc_add with negative numbers.
 *
 * Verifies:
 * - Mixed sign: calc_add(-2, 1) == -1
 * - Both negative: calc_add(-2, -3) == -5
 */
void test_calc_add_negative(void)
{
    TEST_ASSERT_EQUAL_INT(-1, calc_add(-2, 1));
    TEST_ASSERT_EQUAL_INT(-5, calc_add(-2, -3));
}

/**
 * @brief Main entry point for the test runner.
 *
 * Initializes Unity, registers all test cases, and returns the result.
 *
 * @return int Unity test result (0 = all passed, non-zero = failures)
 */
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_calc_add_basic);
    RUN_TEST(test_calc_add_negative);
    return UNITY_END();
}
