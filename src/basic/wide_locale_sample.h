#ifndef WIDE_LOCALE_SAMPLE_H
#define WIDE_LOCALE_SAMPLE_H

#include <stddef.h>
#include <wchar.h>

/**
 * @brief Entry of the wide-character / locale chapter
 *        (wchar_t, setlocale, mbrtowc/wcrtomb).
 */
int main_wide_locale_sample(void);

/* Non-static on purpose (repo convention, see src/advance/calc.h):
 * Unity tests assert these directly. */

/* Converts a NUL-terminated multibyte string to wchar_t via mbrtowc.
 * Returns the number of wide chars written (NUL not counted), or
 * (size_t)-1 on invalid sequence / insufficient capacity. dst is always
 * NUL-terminated on success. */
size_t wl_mbtowc_n(const char* src, wchar_t* dst, size_t cap);

/* Round-trips src through mbrtowc → wcrtomb and compares byte-for-byte
 * (including the terminating NUL). Returns 1 on identical round-trip,
 * 0 on mismatch, (size_t)-1 on invalid multibyte input. */
int wl_roundtrip_ok(const char* src);

#endif /* WIDE_LOCALE_SAMPLE_H */
