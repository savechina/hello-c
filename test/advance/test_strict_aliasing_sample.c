/**
 * @file test_strict_aliasing_sample.c
 * @brief Unit tests for the strict-aliasing chapter (strict_aliasing_sample.c).
 *
 * Guards the invariants the chapter promises:
 * - the full demo entry terminates deterministically and returns 0
 * - pun_u32_to_bytes/pun_bytes_to_u32 round-trip losslessly
 * - byte layout matches the host's endianness (detected at runtime,
 *   never hardcoded — the helpers copy the native representation)
 */

#include "unity.h"
#include <stdint.h>
#include "advance/strict_aliasing_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/**
 * @brief The whole chapter demo must run to completion and return success.
 */
void test_strict_aliasing_sample_entry_returns_zero(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_strict_aliasing_sample());
}

/**
 * @brief memcpy-based punning must be lossless in both directions.
 */
void test_pun_roundtrip_is_lossless(void)
{
    uint8_t buf[4];

    pun_u32_to_bytes(0xDEADBEEFu, buf);
    TEST_ASSERT_EQUAL_HEX32(0xDEADBEEFu, pun_bytes_to_u32(buf));

    pun_u32_to_bytes(0u, buf);
    TEST_ASSERT_EQUAL_HEX32(0u, pun_bytes_to_u32(buf));
}

/**
 * @brief Byte order check: bytes must match the runtime-detected host endianness.
 *
 * The helpers copy the native in-memory representation, so asserting a fixed
 * layout would silently break on a big-endian host. Detect endianness with a
 * char-type read (always legal per C11 §6.5p7) and assert accordingly.
 */
void test_pun_u32_to_bytes_follows_host_endianness(void)
{
    uint8_t buf[4];
    const uint32_t probe = 1u;
    const int little_endian = (*(const unsigned char *)&probe == 1u);

    pun_u32_to_bytes(0x01020304u, buf);

    if (little_endian) {
        TEST_ASSERT_EQUAL_HEX8(0x04u, buf[0]);
        TEST_ASSERT_EQUAL_HEX8(0x03u, buf[1]);
        TEST_ASSERT_EQUAL_HEX8(0x02u, buf[2]);
        TEST_ASSERT_EQUAL_HEX8(0x01u, buf[3]);
    } else {
        TEST_ASSERT_EQUAL_HEX8(0x01u, buf[0]);
        TEST_ASSERT_EQUAL_HEX8(0x02u, buf[1]);
        TEST_ASSERT_EQUAL_HEX8(0x03u, buf[2]);
        TEST_ASSERT_EQUAL_HEX8(0x04u, buf[3]);
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_strict_aliasing_sample_entry_returns_zero);
    RUN_TEST(test_pun_roundtrip_is_lossless);
    RUN_TEST(test_pun_u32_to_bytes_follows_host_endianness);
    return UNITY_END();
}
