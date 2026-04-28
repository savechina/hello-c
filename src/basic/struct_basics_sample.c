#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "basic/struct_basics_sample.h"

/* ---- Section 1: struct definition and initialization ---- */

struct Student {
    char name[32];
    int32_t age;
    float score;
};

static void demo_definition_and_init(void)
{
    /* Positional initialization */
    struct Student s1 = {"Alice", 20, 95.5f};

    /* Designated initializer (C99, recommended) */
    struct Student s2 = {
        .name = "Bob",
        .age  = 21,
        .score = 88.0f,
    };

    /* Partial initialization — remaining members zeroed */
    struct Student s3 = { .age = 19 };

    printf("  s1: %s, age=%" PRId32 ", score=%.1f\n",
           s1.name, s1.age, (double)s1.score);
    printf("  s2: %s, age=%" PRId32 ", score=%.1f\n",
           s2.name, s2.age, (double)s2.score);
    printf("  s3 (partial): name=\"%s\", age=%" PRId32 ", score=%.1f\n",
           s3.name, s3.age, (double)s3.score);
}

/* ---- Section 2: member access (. operator) ---- */

struct Point {
    float x;
    float y;
};

static void demo_member_access(void)
{
    struct Point origin = { 0.0f, 0.0f };

    /* Read members */
    printf("  origin: (%.1f, %.1f)\n",
           (double)origin.x, (double)origin.y);

    /* Modify members */
    origin.x = 3.0f;
    origin.y = 4.0f;
    printf("  moved:  (%.1f, %.1f)\n",
           (double)origin.x, (double)origin.y);

    /* Member in expressions */
    float distance_sq = origin.x * origin.x + origin.y * origin.y;
    printf("  distance^2: %.1f\n", (double)distance_sq);
}

/* ---- Section 3: array of structs and struct copy ---- */

struct ScoreEntry {
    char subject[16];
    int32_t points;
};

static void demo_struct_array_and_copy(void)
{
    /* Array of structs */
    struct ScoreEntry report[3] = {
        { "Math",    95 },
        { "English", 88 },
        { "Science", 92 },
    };

    for (int32_t i = 0; i < 3; i++) {
        printf("  %-8s: %" PRId32 "\n", report[i].subject, report[i].points);
    }

    /* Struct copy (member-wise shallow copy) */
    struct ScoreEntry backup = report[0];
    report[0].points = 100;

    printf("  After modifying report[0]:\n");
    printf("    report[0].points = %" PRId32 ", backup.points = %" PRId32 "\n",
           report[0].points, backup.points);
    printf("  <-- struct copy is a shallow clone, not a reference\n");
}

/* ---- Section 4: designated initializer edge cases ---- */

static void demo_designated_edges(void)
{
    /* Default all members to zero — common pattern */
    struct Student zeroed = { 0 };
    printf("  zeroed student: name=\"%s\", age=%" PRId32 ", score=%.1f\n",
           zeroed.name, zeroed.age, (double)zeroed.score);

    /* Demonstrate: designated initializer with a single assignment */
    struct Student clean = {
        .age   = 25,
        .name  = "Charlie",
    };
    printf("  designated age: %" PRId32 ", name=%s\n",
           clean.age, clean.name);
    printf("  <-- last assignment wins if same member re-designated\n");
}

/* ---- Public entry ---- */

int main_struct_basics_sample(void)
{
    printf("-- struct definition and initialization --\n");
    demo_definition_and_init();

    printf("-- member access (.) operator --\n");
    demo_member_access();

    printf("-- struct array and copy --\n");
    demo_struct_array_and_copy();

    printf("-- designated initializer edge cases --\n");
    demo_designated_edges();

    return 0;
}
