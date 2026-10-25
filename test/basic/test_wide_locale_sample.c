/**
 * @file test_wide_locale_sample.c
 * @brief Gate for the wide-char/locale chapter: computed-value asserts on
 *        the mbrtowc/wcrtomb round-trip, not return-0 smoke.
 *
 *   - wl_mbtowc_n("hello") == 5 and dst[5] == L'\0' → ASCII converts and
 *     is NUL-terminated even in the default "C" locale
 *   - wl_roundtrip_ok("hello") == 1 → mbrtowc → wcrtomb reproduces the
 *     bytes exactly (including the terminating NUL)
 *   - under a UTF-8 locale, byte 0xFF must be rejected with (size_t)-1
 *     and "café" must round-trip (skipped if no UTF-8 locale installed)
 *
 * The ASCII asserts run in the default "C" locale and are fully
 * deterministic; only the 0xFF negative probe needs a UTF-8 locale.
 */

#include "basic/wide_locale_sample.h"
#include "unity.h"
#include <locale.h>
#include <stdint.h>

void setUp(void) {}

void tearDown(void) {}

void test_wl_mbtowc_n_counts_ascii_and_terminates(void) {
  wchar_t wide[16];

  TEST_ASSERT_EQUAL_INT(5, (int)wl_mbtowc_n("hello", wide, 16));
  TEST_ASSERT_EQUAL_INT(L'\0', (int)wide[5]);
  TEST_ASSERT_EQUAL_INT(L'h', (int)wide[0]);
  TEST_ASSERT_EQUAL_INT(L'o', (int)wide[4]);
}

void test_wl_roundtrip_ok_reproduces_ascii_bytes(void) {
  TEST_ASSERT_EQUAL_INT(1, wl_roundtrip_ok("hello"));
  TEST_ASSERT_EQUAL_INT(1, wl_roundtrip_ok(""));
}

void test_wl_mbtowc_n_rejects_invalid_byte_under_utf8(void) {
  /* 在 "C" locale 下 mbrtowc 把全部 256 个单字节值都当合法字符,
   * 0xFF 不会被拒绝 — 只有切到 UTF-8 locale 0xFF 才是非法首字节。
   * 切不过去(机器没装 UTF-8 locale)就跳过, 保持跨平台确定性。 */
  wchar_t wide[4];

  if (setlocale(LC_ALL, "C.UTF-8") == NULL &&
      setlocale(LC_ALL, "en_US.UTF-8") == NULL &&
      setlocale(LC_ALL, "") == NULL) {
    printf("SKIP: no UTF-8 locale available\n");
    return;
  }

  TEST_ASSERT_EQUAL_INT((int)(size_t)-1, (int)wl_mbtowc_n("\xff", wide, 4));
  TEST_ASSERT_EQUAL_INT(1, wl_roundtrip_ok("caf\xc3\xa9"));

  setlocale(LC_ALL, "C");
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_wl_mbtowc_n_counts_ascii_and_terminates);
  RUN_TEST(test_wl_roundtrip_ok_reproduces_ascii_bytes);
  RUN_TEST(test_wl_mbtowc_n_rejects_invalid_byte_under_utf8);
  return UNITY_END();
}
