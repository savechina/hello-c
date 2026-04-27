#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "basic/struct_fields_sample.h"

/* ====================================================================
 * Section 1: sizeof with different struct layouts (padding)
 * ==================================================================== */

/* char + int32_t + char — scattered layout with more padding */
struct Gappy {
    char   a;
    int32_t b;
    char   c;
};

/* Two char + one int32_t — natural alignment, minimal padding */
static void demo_compact_layout(void)
{
    struct Compact {
        char  a;
        char  b;
        int32_t c;
    };

    printf("  sizeof(struct Compact)       = %zu\n", sizeof(struct Compact));
    printf("    offset of a: %zu  (expected 0)\n", offsetof(struct Compact, a));
    printf("    offset of b: %zu  (expected 1)\n", offsetof(struct Compact, b));
    printf("    offset of c: %zu  (expected 4, has 2 bytes padding between b and c)\n",
           offsetof(struct Compact, c));
}

/* char + int32_t + char — scattered layout with more padding */
static void demo_gappy_layout(void)
{
    printf("  sizeof(struct Gappy)         = %zu\n", sizeof(struct Gappy));
    printf("    offset of a: %zu  (expected 0)\n", offsetof(struct Gappy, a));
    printf("    offset of b: %zu  (expected 4, has 3 bytes padding between a and b)\n",
           offsetof(struct Gappy, b));
    printf("    offset of c: %zu  (expected 8)\n", offsetof(struct Gappy, c));
}

/* Same fields, different order — smaller due to reordering */
static void demo_reordered_layout(void)
{
    struct Reordered {
        int32_t b;
        char   a;
        char   c;
    };

    printf("  sizeof(struct Reordered)     = %zu\n", sizeof(struct Reordered));
    printf("    offset of b: %zu  (expected 0)\n", offsetof(struct Reordered, b));
    printf("    offset of a: %zu  (expected 4)\n", offsetof(struct Reordered, a));
    printf("    offset of c: %zu  (expected 5)\n", offsetof(struct Reordered, c));
}

/* ====================================================================
 * Section 2: __attribute__((packed)) verification
 * ==================================================================== */

struct __attribute__((packed)) PackedGappy {
    char  a;
    int32_t b;
    char  c;
};

static void demo_packed_layout(void)
{
    printf("  sizeof(struct Gappy)       = %zu  (has padding)\n",
           (size_t)sizeof(struct Gappy));
    printf("  sizeof(struct PackedGappy) = %zu  (no padding)\n",
           sizeof(struct PackedGappy));

    struct PackedGappy pg = {.a = 'X', .b = 42, .c = 'Y'};
    printf("  PackedGappy: a='%c', b=%d, c='%c'\n", pg.a, (int)pg.b, pg.c);
}

/* ====================================================================
 * Section 3: nested struct field access with sizeof
 * ==================================================================== */

struct Date {
    int32_t year;
    int32_t month;
    int32_t day;
};

struct Employee {
    char name[32];
    struct Date hire_date;
    int32_t id;
};

static void demo_nested_struct_layout(void)
{
    printf("  sizeof(struct Date)         = %zu\n", sizeof(struct Date));
    printf("  sizeof(struct Employee)     = %zu\n", sizeof(struct Employee));
    printf("    offsetof(name):       %zu\n", offsetof(struct Employee, name));
    printf("    offsetof(hire_date):  %zu\n", offsetof(struct Employee, hire_date));
    printf("    offsetof(id):         %zu\n", offsetof(struct Employee, id));

    struct Employee emp = {
        .name = "Alice",
        .hire_date = {.year = 2023, .month = 6, .day = 1},
        .id = 1001,
    };

    printf("  Employee: %s, hired %04d-%02d-%02d, id=%d\n",
           emp.name,
           (int)emp.hire_date.year,
           (int)emp.hire_date.month,
           (int)emp.hire_date.day,
           (int)emp.id);
}

/* ====================================================================
 * Section 4: bit fields
 * ==================================================================== */

struct Permissions {
    unsigned int read    : 1;
    unsigned int write   : 1;
    unsigned int execute : 1;
    unsigned int reserved: 5;
};

static void demo_bit_fields(void)
{
    printf("  sizeof(struct Permissions) = %zu\n", sizeof(struct Permissions));

    struct Permissions perms = {
        .read = 1,
        .write = 1,
        .execute = 0,
        .reserved = 0,
    };

    printf("  read=%u, write=%u, execute=%u\n",
           perms.read, perms.write, perms.execute);

    struct Permissions all_off = {0};
    printf("  all_zero: read=%u, write=%u, execute=%u\n",
           all_off.read, all_off.write, all_off.execute);
}

/* ====================================================================
 * Section 5: alignment of different types
 * ==================================================================== */

static void demo_alignment(void)
{
    printf("  alignof(int8_t)  = %zu\n", _Alignof(int8_t));
    printf("  alignof(int16_t) = %zu\n", _Alignof(int16_t));
    printf("  alignof(int32_t) = %zu\n", _Alignof(int32_t));
    printf("  alignof(int64_t) = %zu\n", _Alignof(int64_t));
    printf("  alignof(float)   = %zu\n", _Alignof(float));
    printf("  alignof(double)  = %zu\n", _Alignof(double));
}

/* ====================================================================
 * Public coordinator
 * ==================================================================== */

int main_struct_fields_sample(void)
{
    printf("-- sizeof with different struct layouts --\n");
    demo_compact_layout();
    demo_gappy_layout();
    demo_reordered_layout();

    printf("-- packed vs default layout --\n");
    demo_packed_layout();

    printf("-- nested struct layout and access --\n");
    demo_nested_struct_layout();

    printf("-- bit fields --\n");
    demo_bit_fields();

    printf("-- type alignment --\n");
    demo_alignment();

    return 0;
}
