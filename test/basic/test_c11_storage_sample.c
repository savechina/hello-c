/**
 * @file test_c11_storage_sample.c
 * @brief Gate for the C11 storage-class chapter: computed-value asserts,
 *        not return-0 smoke.
 *
 *   - c11s_cache_line_mod64() == 0   → _Alignas(64) alignment honored
 *   - c11s_alignof_int32() >= 4      → _Alignof queries real values
 *   - c11s_tls_thread_sum(3) == 1+3+6 → per-thread _Thread_local sums add up
 *   - c11s_noreturn_thread_sum(5) == 15 → _Noreturn worker's pthread_exit value
 *
 * `make test-asan` / `make tsan` additionally prove the pthread worker is
 * race-free (TLS writes only) and the demos are UB-free.
 */

#include "basic/c11_storage_sample.h"
#include "unity.h"

void setUp(void) {}

void tearDown(void) {}

void test_c11_storage_sample_entry_returns_zero(void) {
  TEST_ASSERT_EQUAL_INT(0, main_c11_storage_sample());
}

void test_alignas_buffer_is_64_byte_aligned(void) {
  TEST_ASSERT_EQUAL_INT(0, c11s_cache_line_mod64());
}

void test_alignof_returns_real_alignment(void) {
  TEST_ASSERT_GREATER_OR_EQUAL_INT(4, c11s_alignof_int32());
  TEST_ASSERT_GREATER_OR_EQUAL_INT(8, c11s_alignof_int64());
  TEST_ASSERT_EQUAL_INT(2 * c11s_alignof_int32(), c11s_alignof_int64());
}

void test_thread_local_sums_accumulate_per_thread(void) {
  /* worker k computes sum(1..k) in its own TLS copy:
   * k=1 → 1, k=2 → 3, k=3 → 6; total = 10 */
  /* worker k 返回 sum(1..k), total = Σ k(k+1)/2 = n(n+1)(n+2)/6 */
  TEST_ASSERT_EQUAL_INT(10, c11s_tls_thread_sum(3));
  TEST_ASSERT_EQUAL_INT(1, c11s_tls_thread_sum(1));
  TEST_ASSERT_EQUAL_INT(56, c11s_tls_thread_sum(6));
}

void test_noreturn_worker_returns_sum_via_pthread_exit(void) {
  TEST_ASSERT_EQUAL_INT(15, c11s_noreturn_thread_sum(5));
  TEST_ASSERT_EQUAL_INT(1, c11s_noreturn_thread_sum(1));
  TEST_ASSERT_EQUAL_INT(55, c11s_noreturn_thread_sum(10));
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_c11_storage_sample_entry_returns_zero);
  RUN_TEST(test_alignas_buffer_is_64_byte_aligned);
  RUN_TEST(test_alignof_returns_real_alignment);
  RUN_TEST(test_thread_local_sums_accumulate_per_thread);
  RUN_TEST(test_noreturn_worker_returns_sum_via_pthread_exit);
  return UNITY_END();
}
