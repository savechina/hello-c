#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include "basic/unions_sample.h"

/* ---- Section 1: union basics — shared memory ---- */

union Data {
    int32_t i;
    float   f;
    char    str[8];
};

static void demo_union_basic(void)
{
    union Data d;

    d.i = 42;
    printf("  Write int 42:  d.i = %d\n", (int)d.i);
    printf("  Read as float:  d.f = %f  ← garbage (union stores last write)\n",
           (double)d.f);

    d.f = 3.14f;
    printf("  Write float 3.14:  d.f = %f\n", (double)d.f);
    printf("  Read as int:  d.i = %d  ← garbage (bit reinterpretation)\n",
           (int)d.i);

    strncpy(d.str, "Hi", sizeof(d.str));
    printf("  Write string \"Hi\":  d.str = \"%s\"\n", d.str);
    printf("  sizeof(union Data) = %zu (max member = 8 bytes for str)\n",
           sizeof(union Data));
}

/* ---- Section 2: union size and alignment ---- */

union Small {
    int8_t  a;
    int16_t b;
};

union Big {
    int64_t x;
    char    pad[3];
};

static void demo_union_size(void)
{
    printf("  union Small: sizeof=%zu (max of 1 and 2, aligned to 2)\n",
           sizeof(union Small));
    printf("  union Big:   sizeof=%zu (max of 8 and 3 → 8)\n",
           sizeof(union Big));
    printf("  <-- union size = largest member, aligned to member with strictest alignment\n");
}

/* ---- Section 3: tagged union — safe type discrimination ---- */

typedef enum ValueType {
    VAL_INT,
    VAL_FLOAT,
    VAL_STRING
} ValueType;

struct TaggedValue {
    ValueType tag;
    union {
        int32_t   i;
        float     f;
        char      str[16];
    } data;
};

static struct TaggedValue make_int(int32_t v)
{
    struct TaggedValue t = { .tag = VAL_INT, .data.i = v };
    return t;
}

static struct TaggedValue make_float(float v)
{
    struct TaggedValue t = { .tag = VAL_FLOAT, .data.f = v };
    return t;
}

static struct TaggedValue make_string(const char *v)
{
    struct TaggedValue t;
    t.tag = VAL_STRING;
    strncpy(t.data.str, v, sizeof(t.data.str) - 1);
    t.data.str[sizeof(t.data.str) - 1] = '\0';
    return t;
}

static void print_tagged(const struct TaggedValue *tv)
{
    switch (tv->tag) {
    case VAL_INT:
        printf("    INT:    %" PRId32 "\n", tv->data.i);
        break;
    case VAL_FLOAT:
        printf("    FLOAT:  %.2f\n", (double)tv->data.f);
        break;
    case VAL_STRING:
        printf("    STRING: \"%s\"\n", tv->data.str);
        break;
    }
}

static void demo_tagged_union(void)
{
    struct TaggedValue a = make_int(42);
    struct TaggedValue b = make_float(3.14f);
    struct TaggedValue c = make_string("hello");

    printf("  a: "); print_tagged(&a);
    printf("  b: "); print_tagged(&b);
    printf("  c: "); print_tagged(&c);

    printf("  sizeof(struct TaggedValue) = %zu\n", sizeof(struct TaggedValue));
    printf("  <-- tag (enum) + union = safe variant type, like Rust enum\n");
}

/* ---- Section 4: union for protocol parsing ---- */

struct Header {
    uint8_t  version;
    uint16_t length;
};

union Payload {
    uint8_t  bytes[8];
    uint16_t words[4];
    uint32_t dwords[2];
};

struct Packet {
    struct Header hdr;
    union Payload pay;
};

static void demo_protocol_union(void)
{
    struct Packet pkt;
    pkt.hdr.version = 1;
    pkt.hdr.length  = 8;
    pkt.pay.dwords[0] = 0x12345678U;
    pkt.pay.dwords[1] = 0xAABBCCDDU;

    /* Inspect same data via different views */
    printf("  Packet as dword[0]: 0x%08X\n", (unsigned)pkt.pay.dwords[0]);
    printf("  Packet as word[0,1]: 0x%04X 0x%04X\n",
           (unsigned)pkt.pay.words[0], (unsigned)pkt.pay.words[1]);
    printf("  Packet as byte[0..3]: 0x%02X 0x%02X 0x%02X 0x%02X\n",
           pkt.pay.bytes[0], pkt.pay.bytes[1],
           pkt.pay.bytes[2], pkt.pay.bytes[3]);
    printf("  <-- union lets you view the same memory as bytes, words, or dwords\n");
}

/* ---- Public entry ---- */

int main_unions_sample(void)
{
    printf("-- union basics (shared memory) --\n");
    demo_union_basic();

    printf("-- union size and alignment --\n");
    demo_union_size();

    printf("-- tagged union (safe variant) --\n");
    demo_tagged_union();

    printf("-- union for protocol parsing --\n");
    demo_protocol_union();

    return 0;
}
