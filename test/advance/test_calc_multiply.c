// test/advance/test_calc_multiply.c
// Unity tests for calc_multiply()

#include "unity.h"
#include "advance/calc.h"

void setUp(void) {}
void tearDown(void) {}

void test_calc_multiply_basic(void) {
    TEST_ASSERT_EQUAL_INT(6, calc_multiply(2, 3));
    TEST_ASSERT_EQUAL_INT(0, calc_multiply(0, 5));
    TEST_ASSERT_EQUAL_INT(0, calc_multiply(5, 0));
}

void test_calc_multiply_negative(void) {
    TEST_ASSERT_EQUAL_INT(-6, calc_multiply(-2, 3));
    TEST_ASSERT_EQUAL_INT(-6, calc_multiply(2, -3));
    TEST_ASSERT_EQUAL_INT(6, calc_multiply(-2, -3));
}

void test_calc_multiply_overflow(void) {
    // 100 * 100 = 10000 (within range)
    TEST_ASSERT_EQUAL_INT(10000, calc_multiply(100, 100));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_calc_multiply_basic);
    RUN_TEST(test_calc_multiply_negative);
    RUN_TEST(test_calc_multiply_overflow);
    return UNITY_END();
}
