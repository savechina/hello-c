/**
 * @file test_raii_safety.c
 * @brief Unit tests for the memory-safety primitives in raii_sample.c.
 *
 * These guard the invariants the abstractions exist to provide:
 * - arena: overflow-safe capacity checks, max_align_t alignment, single destroy,
 *          failed init zeroes the object instead of leaving indeterminate fields
 * - Option<T>: SOME/NONE discrimination, out-param not written when NONE
 * - Buf64:  bounds enforced against capacity, len never corrupted on rejection
 */

#include "unity.h"
#include <string.h>
#include "advance/raii_sample.h"

void setUp(void)
{
}

void tearDown(void)
{
}

/* --- Arena --- */

void test_arena_alloc_returns_distinct_regions(void)
{
    arena_t a;
    TEST_ASSERT_TRUE(raii_arena_init(&a, 256));

    void *p1 = raii_arena_alloc(&a, 16);
    void *p2 = raii_arena_alloc(&a, 16);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_TRUE(p1 != p2);

    raii_arena_destroy(&a);
}

void test_arena_alloc_aligns_to_max_align(void)
{
    arena_t a;
    TEST_ASSERT_TRUE(raii_arena_init(&a, 256));

    /* First allocation lands at offset 0, which is trivially aligned — asserting
     * only on that would be vacuous. Consume an odd number of bytes first so the
     * *next* allocation starts unaligned, then require it be realigned. */
    TEST_ASSERT_NOT_NULL(raii_arena_alloc(&a, 1));

    char *c = raii_arena_alloc(&a, 8);
    TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned)((uintptr_t)c % _Alignof(max_align_t)));

    raii_arena_destroy(&a);
}

/* A request larger than remaining capacity must fail, not wrap or overflow. */
void test_arena_rejects_overflow_request(void)
{
    arena_t a;
    TEST_ASSERT_TRUE(raii_arena_init(&a, 128));

    TEST_ASSERT_NULL(raii_arena_alloc(&a, SIZE_MAX));
    TEST_ASSERT_NOT_NULL(raii_arena_alloc(&a, 64));
    TEST_ASSERT_NULL(raii_arena_alloc(&a, SIZE_MAX));
    TEST_ASSERT_NOT_NULL(raii_arena_alloc(&a, 64));
    /* now full */
    TEST_ASSERT_NULL(raii_arena_alloc(&a, 1));

    raii_arena_destroy(&a);
}

void test_arena_destroy_resets_state(void)
{
    arena_t a;
    TEST_ASSERT_TRUE(raii_arena_init(&a, 128));
    TEST_ASSERT_NOT_NULL(raii_arena_alloc(&a, 32));

    raii_arena_destroy(&a);
    TEST_ASSERT_NULL(a.base);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned)a.used);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned)a.cap);
}

/* malloc(SIZE_MAX) fails deterministically: the failed init must hand back a
 * zeroed object, so a later raii_arena_alloc reads cap==0 instead of garbage. */
void test_arena_init_failure_zeroes_state(void)
{
    arena_t a;
    memset(&a, 0xAA, sizeof(a));

    TEST_ASSERT_FALSE(raii_arena_init(&a, SIZE_MAX));
    TEST_ASSERT_NULL(a.base);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned)a.cap);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned)a.used);
}

/* --- Option<T> --- */

void test_opt_some_yields_value(void)
{
    raii_opt_int_t o = raii_some(42);
    int out = 0;
    TEST_ASSERT_EQUAL_INT(RAII_SOME, o.tag);
    TEST_ASSERT_TRUE(raii_value(o, &out));
    TEST_ASSERT_EQUAL_INT(42, out);
}

void test_opt_none_reports_absence(void)
{
    raii_opt_int_t o = raii_none();
    int out = 12345;
    TEST_ASSERT_EQUAL_INT(RAII_NONE, o.tag);
    TEST_ASSERT_FALSE(raii_value(o, &out));
    /* out must be untouched so a caller that ignores the result can't read garbage */
    TEST_ASSERT_EQUAL_INT(12345, out);
}

void test_opt_none_handles_null_out(void)
{
    /* Must not dereference NULL; absence is still reported. */
    TEST_ASSERT_FALSE(raii_value(raii_none(), NULL));
    TEST_ASSERT_TRUE(raii_value(raii_some(1), &(int){0}));
}

/* --- Buf64 --- */

void test_buf_append_and_len(void)
{
    raii_buf_t b;
    raii_buf_init(&b);
    TEST_ASSERT_EQUAL_UINT(0u, (unsigned)raii_buf_len(&b));

    TEST_ASSERT_TRUE(raii_buf_append_str(&b, "Hello"));
    TEST_ASSERT_TRUE(raii_buf_append_str(&b, ", world"));
    TEST_ASSERT_EQUAL_UINT(12u, (unsigned)raii_buf_len(&b));
    TEST_ASSERT_EQUAL_STRING_LEN("Hello, world", (const char *)b.data, 12);

    memset(b.data + b.len, 0, 1);
}

void test_buf_append_exactly_full(void)
{
    raii_buf_t b;
    raii_buf_init(&b);

    /* Fill to exactly RAII_BUF_CAP in two steps. Filling in one shot would not
     * catch an off-by-one in the bounds check, because len==0 makes the
     * comparison trivially true; the boundary only matters once len>0. */
    TEST_ASSERT_TRUE(raii_buf_append(&b, "0123456789", 10));
    TEST_ASSERT_TRUE(raii_buf_append(&b, "0123456789012345678901234567890123456789"
                                    "012345678901234", RAII_BUF_CAP - 10));
    TEST_ASSERT_EQUAL_UINT(RAII_BUF_CAP, (unsigned)raii_buf_len(&b));

    /* now one more byte must be refused */
    TEST_ASSERT_FALSE(raii_buf_append(&b, "x", 1));
    TEST_ASSERT_EQUAL_UINT(RAII_BUF_CAP, (unsigned)raii_buf_len(&b));
}

void test_buf_rejects_overflow_and_preserves_len(void)
{
    raii_buf_t b;
    raii_buf_init(&b);
    TEST_ASSERT_TRUE(raii_buf_append_str(&b, "0123456789"));
    size_t before = raii_buf_len(&b);

    /* one byte past capacity must be rejected, len unchanged */
    TEST_ASSERT_FALSE(raii_buf_append(&b, "x", RAII_BUF_CAP));
    TEST_ASSERT_EQUAL_UINT((unsigned)before, (unsigned)raii_buf_len(&b));

    /* absurd request must not corrupt len either */
    TEST_ASSERT_FALSE(raii_buf_append(&b, "x", SIZE_MAX));
    TEST_ASSERT_EQUAL_UINT((unsigned)before, (unsigned)raii_buf_len(&b));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_arena_alloc_returns_distinct_regions);
    RUN_TEST(test_arena_alloc_aligns_to_max_align);
    RUN_TEST(test_arena_rejects_overflow_request);
    RUN_TEST(test_arena_destroy_resets_state);
    RUN_TEST(test_arena_init_failure_zeroes_state);
    RUN_TEST(test_opt_some_yields_value);
    RUN_TEST(test_opt_none_reports_absence);
    RUN_TEST(test_opt_none_handles_null_out);
    RUN_TEST(test_buf_append_and_len);
    RUN_TEST(test_buf_append_exactly_full);
    RUN_TEST(test_buf_rejects_overflow_and_preserves_len);
    return UNITY_END();
}
