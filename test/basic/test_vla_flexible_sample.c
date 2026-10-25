/**
 * @file test_vla_flexible_sample.c
 * @brief Gate for the VLA + flexible-array-member chapter: computed-value
 *        asserts, not return-0 smoke.
 *
 *   - vla_sum_n(n) == n*(n+1)/2       → runtime-sized VLA sums correctly
 *   - vla_sizeof_n(n) == n*sizeof(int) → runtime sizeof matches the clamped n
 *   - fam_sum_n(n) == n*(n+1)/2       → one-malloc FAM record sums correctly
 *
 * `make test-asan` additionally proves the FAM malloc is header+n*sizeof
 * (no heap overflow) and the VLA stack usage stays inside bounds.
 */

#include "basic/vla_flexible_sample.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_vla_flexible_sample_entry_returns_zero(void) {
  TEST_ASSERT_EQUAL_INT(0, main_vla_flexible_sample());
}

void test_vla_sum_matches_closed_form(void) {
  TEST_ASSERT_EQUAL_INT(1, vla_sum_n(1));
  TEST_ASSERT_EQUAL_INT(55, vla_sum_n(10));
  TEST_ASSERT_EQUAL_INT(2080, vla_sum_n(64));
}

void test_vla_sizeof_is_runtime_evaluated(void) {
  TEST_ASSERT_EQUAL_INT(1 * (int)sizeof(int), vla_sizeof_n(1));
  TEST_ASSERT_EQUAL_INT(10 * (int)sizeof(int), vla_sizeof_n(10));
  TEST_ASSERT_EQUAL_INT(64 * (int)sizeof(int), vla_sizeof_n(64));
}

void test_flexible_array_member_sum_matches_closed_form(void) {
  TEST_ASSERT_EQUAL_INT(1, fam_sum_n(1));
  TEST_ASSERT_EQUAL_INT(55, fam_sum_n(10));
  TEST_ASSERT_EQUAL_INT(2080, fam_sum_n(64));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_vla_flexible_sample_entry_returns_zero);
  RUN_TEST(test_vla_sum_matches_closed_form);
  RUN_TEST(test_vla_sizeof_is_runtime_evaluated);
  RUN_TEST(test_flexible_array_member_sum_matches_closed_form);
  return UNITY_END();
}
