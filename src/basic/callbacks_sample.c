#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "basic/callbacks_sample.h"

/* ====================================================================
 * 回调函数与多态演示 (Callbacks & Polymorphism)
 *
 * 本节演示 C 语言回调模式的核心应用：
 *  - qsort 自定义比较回调
 *  - void* 泛型数据传递给回调
 *  - 事件驱动回调模拟（发布-订阅）
 *  - 多态通过结构体函数指针
 * ==================================================================== */

/* ── 1. qsort 回调: 自定义比较函数 ── */

/* 安全的三态比较: (a > b) - (a < b) 避免溢出 */
static int cmp_int_for_qsort(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

/* 降序比较 */
static int cmp_int_desc(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ib > ia) - (ib < ia);
}

/* 字符串比较回调 (字符串数组排序) */
static int cmp_str_for_qsort(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

/* 结构体字段比较: 按 age 排序 Person */
typedef struct {
    char name[32];
    int32_t age;
    float score;
} Person;

static int cmp_person_by_age(const void *a, const void *b)
{
    const Person *pa = (const Person *)a;
    const Person *pb = (const Person *)b;
    return (pa->age > pb->age) - (pa->age < pb->age);
}

static int cmp_person_by_score(const void *a, const void *b)
{
    const Person *pa = (const Person *)a;
    const Person *pb = (const Person *)b;
    /* float 比较: 用 sign 模式 */
    float diff = pa->score - pb->score;
    if (diff < 0.0f) return -1;
    if (diff > 0.0f) return 1;
    return 0;
}

static void demo_qsort_callbacks(void)
{
    printf("=== 1. qsort 自定义比较回调 ===\n");

    /* 整数升序 */
    int nums[] = { 5, 2, 9, 1, 7, 3, 8 };
    int32_t n = (int32_t)(sizeof(nums) / sizeof(nums[0]));

    printf("  原数组: ");
    for (int32_t i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n");

    qsort(nums, (size_t)n, sizeof(int), cmp_int_for_qsort);
    printf("  qsort 升序: ");
    for (int32_t i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n");

    qsort(nums, (size_t)n, sizeof(int), cmp_int_desc);
    printf("  qsort 降序: ");
    for (int32_t i = 0; i < n; i++) printf("%d ", nums[i]);
    printf("\n");

    /* 字符串排序 */
    const char *words[] = { "banana", "apple", "cherry", "date" };
    int32_t wn = (int32_t)(sizeof(words) / sizeof(words[0]));

    qsort(words, (size_t)wn, sizeof(const char *), cmp_str_for_qsort);
    printf("  字符串排序: ");
    for (int32_t i = 0; i < wn; i++) printf("%s ", words[i]);
    printf("\n");

    /* 结构体排序: 按 age */
    Person class[4] = {
        { "Alice", 22, 92.5f },
        { "Bob", 19, 85.0f },
        { "Charlie", 21, 95.0f },
        { "Diana", 20, 88.5f },
    };
    int32_t pn = (int32_t)(sizeof(class) / sizeof(class[0]));

    qsort(class, (size_t)pn, sizeof(Person), cmp_person_by_age);
    printf("  Person 按 age 排序:\n");
    for (int32_t i = 0; i < pn; i++) {
        printf("    %s (age=%" PRId32 ", score=%.1f)\n",
               class[i].name, class[i].age, (double)class[i].score);
    }

    qsort(class, (size_t)pn, sizeof(Person), cmp_person_by_score);
    printf("  Person 按 score 排序:\n");
    for (int32_t i = 0; i < pn; i++) {
        printf("    %s (age=%" PRId32 ", score=%.1f)\n",
               class[i].name, class[i].age, (double)class[i].score);
    }

    printf("\n");
}

/* ── 2. void* 泛型数据传递: 泛型 forEach ── */

typedef void (*element_visitor_t)(const void *elem, void *user_data);

/* 泛型 forEach: 遍历任意类型数组 */
static void foreach_array(const void *arr, int32_t len, size_t elem_size,
                          element_visitor_t visit, void *user_data)
{
    const char *base = (const char *)arr;
    for (int32_t i = 0; i < len; i++) {
        visit(base + i * elem_size, user_data);
    }
}

/* 回调: 打印 int */
static void print_int_elem(const void *elem, void *user_data)
{
    (void)user_data;
    printf("%d ", *(const int *)elem);
}

/* 回调: 累加求和 */
static void sum_int_elem(const void *elem, void *user_data)
{
    int32_t *sum = (int32_t *)user_data;
    *sum += *(const int *)elem;
}

/* 回调: 统计满足条件的元素个数 (void* 传入阈值) */
typedef struct {
    int threshold;
    int count;
} ThresholdCtx;

static void count_above_threshold(const void *elem, void *user_data)
{
    int val = *(const int *)elem;
    ThresholdCtx *ctx = (ThresholdCtx *)user_data;
    if (val > ctx->threshold) {
        ctx->count++;
    }
}

static void demo_void_star_callback(void)
{
    printf("=== 2. void* 泛型数据传递 --- forEach + 闭包模拟 ===\n");

    int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    int32_t n = (int32_t)(sizeof(arr) / sizeof(arr[0]));

    printf("  数组: ");
    foreach_array(arr, n, sizeof(int), print_int_elem, NULL);
    printf("\n");

    /* 求和 */
    int32_t sum = 0;
    foreach_array(arr, n, sizeof(int), sum_int_elem, &sum);
    printf("  用 void* 求和 = %d\n", sum);

    /* 统计 > 5 的元素 */
    ThresholdCtx ctx = { .threshold = 5, .count = 0 };
    foreach_array(arr, n, sizeof(int), count_above_threshold, &ctx);
    printf("  用 void* 传阈值, >5 的元素个数 = %d\n", ctx.count);

    printf("\n");
}

/* ── 3. 事件驱动回调模拟 (发布-订阅) ── */

#define MAX_OBSERVERS 8

typedef void (*observer_func_t)(int32_t event_code, void *user_data);

typedef struct {
    observer_func_t func;
    void *user_data;
} Observer;

typedef struct {
    Observer observers[MAX_OBSERVERS];
    int32_t count;
} EventManager;

static void em_init(EventManager *em)
{
    em->count = 0;
}

static int32_t em_register(EventManager *em, observer_func_t func,
                           void *user_data)
{
    if (em->count >= MAX_OBSERVERS) {
        return -1;  /* 已达上限 */
    }
    em->observers[em->count].func = func;
    em->observers[em->count].user_data = user_data;
    em->count++;
    return 0;
}

static void em_notify(EventManager *em, int32_t event_code)
{
    for (int32_t i = 0; i < em->count; i++) {
        em->observers[i].func(event_code, em->observers[i].user_data);
    }
}

/* 观测者函数 */
static void on_click(int32_t code, void *user_data)
{
    (void)user_data;
    printf("    [ClickHandler] received event code %" PRId32 "\n", code);
}

static void on_log(int32_t code, void *user_data)
{
    const char *prefix = (const char *)user_data;
    printf("    [%sLogger] event %" PRId32 " logged\n",
           prefix ? prefix : "", code);
}

static void demo_event_manager(void)
{
    printf("=== 3. 事件驱动回调模拟 (发布-订阅) ===\n");

    EventManager em;
    em_init(&em);

    /* 注册两个观测者 */
    em_register(&em, on_click, NULL);
    em_register(&em, on_log, (void *)"Game");

    printf("  触发 event code 100 (CLICK):\n");
    em_notify(&em, 100);

    printf("  触发 event code 200 (HOVER):\n");
    em_notify(&em, 200);

    printf("\n");
}

/* ── 4. 多态: 函数指针结构体 ── */

/* Shape 接口 */
typedef struct Shape_t {
    double (*area)(const struct Shape_t *self);
    double (*perimeter)(const struct Shape_t *self);
} Shape;

/* Circle 实现 */
typedef struct {
    Shape base;
    double radius;
} Circle;

static double circle_area(const Shape *self)
{
    const Circle *c = (const Circle *)self;
    return 3.14159265358979 * c->radius * c->radius;
}

static double circle_perimeter(const Shape *self)
{
    const Circle *c = (const Circle *)self;
    return 2.0 * 3.14159265358979 * c->radius;
}

/* Rectangle 实现 */
typedef struct {
    Shape base;
    double width;
    double height;
} Rectangle;

static double rect_area(const Shape *self)
{
    const Rectangle *r = (const Rectangle *)self;
    return r->width * r->height;
}

static double rect_perimeter(const Shape *self)
{
    const Rectangle *r = (const Rectangle *)self;
    return 2.0 * (r->width + r->height);
}

static void demo_polymorphism(void)
{
    printf("=== 4. 多态: 函数指针实现 OO 风格 ===\n");

    /* 初始化 Circle */
    Circle circ;
    circ.radius = 5.0;
    circ.base.area = circle_area;
    circ.base.perimeter = circle_perimeter;

    /* 初始化 Rectangle */
    Rectangle rect;
    rect.width = 4.0;
    rect.height = 6.0;
    rect.base.area = rect_area;
    rect.base.perimeter = rect_perimeter;

    /* 多态数组: Shape* 统一接口 */
    Shape *shapes[2];
    shapes[0] = (Shape *)&circ;
    shapes[1] = (Shape *)&rect;

    printf("  多态遍历 Shape 数组:\n");
    for (int32_t i = 0; i < 2; i++) {
        printf("    shapes[%" PRId32 "].area() = %.4f\n",
               i, shapes[i]->area(shapes[i]));
        printf("    shapes[%" PRId32 "].perimeter() = %.4f\n",
               i, shapes[i]->perimeter(shapes[i]));
    }

    printf("\n");
}

/* ── Coordinator entry ── */

int main_callbacks_sample(void)
{
    printf("========================================\n");
    printf("  回调函数与多态 (Callbacks & Polymorphism)\n");
    printf("========================================\n\n");

    demo_qsort_callbacks();
    demo_void_star_callback();
    demo_event_manager();
    demo_polymorphism();

    printf("回调函数演示完毕。\n");
    return 0;
}
