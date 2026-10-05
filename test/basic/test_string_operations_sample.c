/**
 * @file test_string_operations_sample.c
 * @brief Entry smoke + real assertions on the exposed my_strlen() helper.
 */

#include <string.h>

#include "unity.h"
#include "basic/string_operations_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_string_operations_sample_runs_without_undefined_behavior(void)
{
    TEST_ASSERT_EQUAL_INT(0, main_string_operations_sample());
}

void test_my_strlen_matches_known_lengths(void)
{
    TEST_ASSERT_EQUAL_size_t(0, my_strlen(""));
    TEST_ASSERT_EQUAL_size_t(5, my_strlen("Hello"));
    TEST_ASSERT_EQUAL_size_t(13, my_strlen("C Programming"));
    TEST_ASSERT_EQUAL_size_t(4, my_strlen("🌏"));
}

void test_my_strlen_agrees_with_libc(void)
{
    const char *cases[] = {
        "", "a", "Hello", "C Programming", "🌏", "null\tterminated\n"
    };
    size_t count = sizeof(cases) / sizeof(cases[0]);

    for (size_t i = 0; i < count; i++) {
        TEST_ASSERT_EQUAL_size_t(strlen(cases[i]), my_strlen(cases[i]));
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_string_operations_sample_runs_without_undefined_behavior);
    RUN_TEST(test_my_strlen_matches_known_lengths);
    RUN_TEST(test_my_strlen_agrees_with_libc);
    return UNITY_END();
}
