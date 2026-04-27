#include <stdio.h>
#include <stdint.h>
#include "basic/structs_sample.h"

/* ====================================================================
 * Section 1: struct definition, initialization, member access (.)
 * ==================================================================== */

struct Student {
    char name[32];
    int32_t age;
    float score;
};

static void demo_definition_and_access(void)
{
    struct Student s1 = {"Alice", 20, 95.5f};

    struct Student s2 = {
        .name = "Bob",
        .age = 21,
        .score = 88.0f,
    };

    struct Student s3 = { .age = 19 };

    printf("  s1: %s, age=%d, score=%.1f\n",
           s1.name, (int)s1.age, (double)s1.score);
    printf("  s2: %s, age=%d, score=%.1f\n",
           s2.name, (int)s2.age, (double)s2.score);
    printf("  s3 (partial init): name=\"%s\", age=%d, score=%.1f\n",
           s3.name, (int)s3.age, (double)s3.score);
}

/* ====================================================================
 * Section 2: designated initializers
 * ==================================================================== */

struct Point {
    float x;
    float y;
};

static void demo_designated_initializers(void)
{
    struct Point p1 = { .x = 3.0f, .y = 4.0f };

    struct Point p2 = { .y = 7.0f };

    struct Point p3 = { 1.0f, 2.0f };

    printf("  p1 (both):    (%.1f, %.1f)\n", (double)p1.x, (double)p1.y);
    printf("  p2 (y only):  (%.1f, %.1f)\n", (double)p2.x, (double)p2.y);
    printf("  p3 (positional): (%.1f, %.1f)\n", (double)p3.x, (double)p3.y);
}

/* ====================================================================
 * Section 3: nested structs
 * ==================================================================== */

struct Date {
    int32_t year;
    int32_t month;
    int32_t day;
};

struct Person {
    char name[32];
    struct Date birthday;
};

static void demo_nested_structs(void)
{
    struct Person p = {
        .name = "Charlie",
        .birthday = { .year = 2000, .month = 6, .day = 15 },
    };

    printf("  姓名: %s\n", p.name);
    printf("  生日: %04d-%02d-%02d\n",
           (int)p.birthday.year,
           (int)p.birthday.month,
           (int)p.birthday.day);
}

/* ====================================================================
 * Section 4: passing struct to functions
 * ==================================================================== */

static float calc_average(const struct Student students[], int32_t count)
{
    float sum = 0.0f;
    for (int32_t i = 0; i < count; i++) {
        sum += students[i].score;
    }
    return count > 0 ? sum / (float)count : 0.0f;
}

static void demo_passing_struct(void)
{
    struct Student class[3] = {
        { .name = "张三", .score = 85.0f },
        { .name = "李四", .score = 92.0f },
        { .name = "王五", .score = 78.0f },
    };

    float avg = calc_average(class, 3);

    printf("  平均分: %.1f\n", (double)avg);
}

/* ====================================================================
 * Section 5: returning struct from function
 * ==================================================================== */

struct Circle {
    struct Point center;
    float radius;
};

static struct Circle make_circle(float cx, float cy, float r)
{
    struct Circle c;
    c.center.x = cx;
    c.center.y = cy;
    c.radius = r;
    return c;
}

static float circle_area(struct Circle c)
{
    return 3.14159f * c.radius * c.radius;
}

static void demo_returning_struct(void)
{
    struct Circle c1 = make_circle(0.0f, 0.0f, 5.0f);
    struct Circle c2 = make_circle(3.0f, 4.0f, 3.0f);

    printf("  圆1 面积: %.2f\n", (double)circle_area(c1));
    printf("  圆2 面积: %.2f\n", (double)circle_area(c2));
    printf("  面积差:   %.2f\n", (double)(circle_area(c1) - circle_area(c2)));
}

/* ====================================================================
 * Public coordinator
 * ==================================================================== */

int main_structs_sample(void)
{
    printf("-- struct 定义、初始化、成员访问 --\n");
    demo_definition_and_access();

    printf("-- designated initializers --\n");
    demo_designated_initializers();

    printf("-- 嵌套结构体 --\n");
    demo_nested_structs();

    printf("-- 函数参数传递 struct --\n");
    demo_passing_struct();

    printf("-- 函数返回 struct --\n");
    demo_returning_struct();

    return 0;
}
