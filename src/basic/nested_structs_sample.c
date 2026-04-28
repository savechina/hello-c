#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "basic/nested_structs_sample.h"

/* ---- Section 1: nested struct definition and initialization ---- */

struct Date {
    int32_t year;
    int32_t month;
    int32_t day;
};

struct Address {
    char city[24];
    char street[48];
};

struct Person {
    char name[32];
    struct Date birthday;
    struct Address home;
};

static void demo_nested_init(void)
{
    struct Person p = {
        .name    = "Alice",
        .birthday = { .year = 2000, .month = 6, .day = 15 },
        .home    = { .city = "Beijing", .street = "ChaoYang Rd 100" },
    };

    printf("  Name: %s\n", p.name);
    printf("  Birthday: %04" PRId32 "-%02" PRId32 "-%02" PRId32 "\n",
           p.birthday.year, p.birthday.month, p.birthday.day);
    printf("  Address: %s, %s\n", p.home.city, p.home.street);
}

/* ---- Section 2: nested struct partial initialization ---- */

struct Inner {
    int32_t x;
    int32_t y;
};

struct Outer {
    struct Inner pos;
    int32_t id;
};

static void demo_partial_nested(void)
{
    /* Only initialize nested pos, id defaults to 0 */
    struct Outer o1 = { .pos = { .x = 10, .y = 20 } };

    /* Only initialize pos.x, rest zeroed */
    struct Outer o2 = { .pos.x = 5 };

    printf("  o1: pos=(%" PRId32 ", %" PRId32 "), id=%" PRId32 "\n",
           o1.pos.x, o1.pos.y, o1.id);
    printf("  o2: pos=(%" PRId32 ", %" PRId32 "), id=%" PRId32 "\n",
           o2.pos.x, o2.pos.y, o2.id);
    printf("  <-- unmentioned nested members are zero-initialized\n");
}

/* ---- Section 3: nested struct assignment and comparison ---- */

struct Rectangle {
    struct Point_top_left {
        float x;
        float y;
    } top_left;
    float width;
    float height;
};

static void demo_nested_operate(void)
{
    struct Rectangle r1 = {
        .top_left = { .x = 0.0f, .y = 10.0f },
        .width  = 100.0f,
        .height = 200.0f,
    };

    /* Copy struct (includes nested) */
    struct Rectangle r2 = r1;
    r2.top_left.x = 50.0f;

    printf("  r1 top_left: (%.1f, %.1f)\n",
           (double)r1.top_left.x, (double)r1.top_left.y);
    printf("  r2 top_left (after modification): (%.1f, %.1f)\n",
           (double)r2.top_left.x, (double)r2.top_left.y);

    /* Manual member-wise comparison */
    int32_t same_area = (r1.width == r2.width) && (r1.height == r2.height);
    printf("  same area? %s\n", same_area ? "yes" : "no");
}

/* ---- Section 4: deeply nested struct (3 levels) ---- */

struct Level3 {
    int32_t value;
};

struct Level2 {
    struct Level3 l3;
};

struct Level1 {
    struct Level2 l2;
};

static void demo_deeply_nested(void)
{
    struct Level1 root = {
        .l2.l3.value = 42,
    };

    printf("  root.l2.l3.value = %" PRId32 "\n", root.l2.l3.value);
    printf("  <-- nested access chains: . . . — each dot one level deeper\n");
}

/* ---- Public entry ---- */

int main_nested_structs_sample(void)
{
    printf("-- nested struct definition and init --\n");
    demo_nested_init();

    printf("-- partial nested initialization --\n");
    demo_partial_nested();

    printf("-- nested struct operations --\n");
    demo_nested_operate();

    printf("-- deeply nested (3 levels) --\n");
    demo_deeply_nested();

    return 0;
}
