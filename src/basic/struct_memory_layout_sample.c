#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include "basic/struct_memory_layout_sample.h"

/* ---- Section 1: padding caused by field order ---- */

struct PaddedBad {
    char   a;    /* 1 byte */
    int32_t b;   /* 4 bytes — needs 4-byte alignment → 3 padding bytes */
    char   c;    /* 1 byte + 3 trailing padding to reach multiple of 4 */
};

struct PaddedGood {
    int32_t b;   /* 4 bytes */
    char   a;    /* 1 byte */
    char   c;    /* 1 byte + 2 trailing padding */
};

static void demo_field_order(void)
{
    printf("  struct PaddedBad:\n");
    printf("    field sizes: 1 + 4 + 1 = 6 (actual data)\n");
    printf("    sizeof      = %zu (has internal + trailing padding)\n",
           sizeof(struct PaddedBad));
    printf("    offsetof(a) = %zu, offsetof(b) = %zu, offsetof(c) = %zu\n",
           offsetof(struct PaddedBad, a),
           offsetof(struct PaddedBad, b),
           offsetof(struct PaddedBad, c));

    printf("  struct PaddedGood (sorted biggest-first):\n");
    printf("    sizeof      = %zu (less padding)\n",
           sizeof(struct PaddedGood));
    printf("    offsetof(b) = %zu, offsetof(a) = %zu, offsetof(c) = %zu\n",
           offsetof(struct PaddedGood, b),
           offsetof(struct PaddedGood, a),
           offsetof(struct PaddedGood, c));
}

/* ---- Section 2: 64-bit type alignment ---- */

struct WithInt64 {
    char   tag;     /* 1 byte */
    int64_t value;  /* 8 bytes — needs 8-byte alignment → 7 padding */
};

static void demo_64bit_alignment(void)
{
    printf("  struct WithInt64:\n");
    printf("    data: 1 + 8 = 9 bytes\n");
    printf("    sizeof = %zu\n", sizeof(struct WithInt64));
    printf("    offsetof(tag)   = %zu\n", offsetof(struct WithInt64, tag));
    printf("    offsetof(value) = %zu (7 padding bytes between tag and value)\n",
           offsetof(struct WithInt64, value));
    printf("    sizeof = 1 + 7(pad) + 8 = 16 (aligned to 8)\n");
}

/* ---- Section 3: packed struct via __attribute__ ---- */

struct __attribute__((packed)) Packed {
    char   a;
    int32_t b;
    char   c;
};

static void demo_packed(void)
{
    struct Packed p = { .a = 'X', .b = 12345, .c = 'Y' };

    printf("  struct Packed (no padding):\n");
    printf("    sizeof      = %zu (bare minimum: 1+4+1)\n",
           sizeof(struct Packed));
    printf("    offsetof(a) = %zu, offsetof(b) = %zu, offsetof(c) = %zu\n",
           offsetof(struct Packed, a),
           offsetof(struct Packed, b),
           offsetof(struct Packed, c));
    printf("    data: a='%c', b=%" PRId32 ", c='%c'\n",
           p.a, p.b, p.c);
    printf("  <-- packed removes padding; may degrade performance on some CPUs\n");
}

/* ---- Section 4: alignment of fundamental types ---- */

static void demo_type_alignment(void)
{
    printf("  _Alignof(int8_t)  = %zu\n", _Alignof(int8_t));
    printf("  _Alignof(int16_t) = %zu\n", _Alignof(int16_t));
    printf("  _Alignof(int32_t) = %zu\n", _Alignof(int32_t));
    printf("  _Alignof(int64_t) = %zu\n", _Alignof(int64_t));
    printf("  _Alignof(float)   = %zu\n", _Alignof(float));
    printf("  _Alignof(double)  = %zu\n", _Alignof(double));
    printf("  _Alignof(void *)  = %zu\n", _Alignof(void *));
}

/* ---- Section 5: nested struct memory layout ---- */

struct Inner {
    int32_t a;
    char   b;
};

struct Outer {
    char   x;
    struct Inner inner;
    int64_t y;
};

static void demo_nested_layout(void)
{
    printf("  struct Inner:\n");
    printf("    sizeof = %zu  offsetof(a)=%zu, offsetof(b)=%zu\n",
           sizeof(struct Inner),
           offsetof(struct Inner, a),
           offsetof(struct Inner, b));

    printf("  struct Outer:\n");
    printf("    sizeof = %zu\n", sizeof(struct Outer));
    printf("    offsetof(x)    = %zu\n", offsetof(struct Outer, x));
    printf("    offsetof(inner)= %zu (3 padding after x)\n",
           offsetof(struct Outer, inner));
    printf("    offsetof(y)    = %zu (padding before y for 8-alignment)\n",
           offsetof(struct Outer, y));
}

/* ---- Public entry ---- */

int main_struct_memory_layout_sample(void)
{
    printf("-- field order affects padding --\n");
    demo_field_order();

    printf("-- 64-bit type alignment --\n");
    demo_64bit_alignment();

    printf("-- packed struct (no padding) --\n");
    demo_packed();

    printf("-- type alignment values --\n");
    demo_type_alignment();

    printf("-- nested struct layout --\n");
    demo_nested_layout();

    return 0;
}
