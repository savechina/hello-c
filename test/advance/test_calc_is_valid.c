// test/advance/test_calc_is_valid.c
// Unity tests for calc_is_valid()

#include "unity.h"
#include "advance/calc.h"

void setUp(void) {}
void tearDown(void) {}

void test_calc_is_valid_in_range(void) {
    TEST_ASSERT_EQUAL_INT(1, calc_is_valid(0));
    TEST_ASSERT_EQUAL_INT(1, calc_is_valid(5000));
    TEST_ASSERT_EQUAL_INT(1, calc_is_valid(10000));
}

void test_calc_is_valid_out_of_range(void) {
    TEST_ASSERT_EQUAL_INT(0, calc_is_valid(-1));
    TEST_ASSERT_EQUAL_INT(0, calc_is_valid(10001));
    TEST_ASSERT_EQUAL_INT(0, calc_is_valid(50000));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_calc_is_valid_in_range);
    RUN_TEST(test_calc_is_valid_out_of_range);
    return UNITY_END();
}
