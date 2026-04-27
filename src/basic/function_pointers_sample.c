#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include "basic/function_pointers_sample.h"

/* ====================================================================
 * 函数指针演示 (Function Pointers)
 *
 * 本节演示 C 语言函数指针的核心概念：
 *  - 基本函数指针声明与调用
 *  - 函数指针数组（Dispatch Table）
 *  - 函数指针作为参数（高阶函数模式）
 *  - 结构体 + 函数指针（模拟 OOP 方法）
 * ==================================================================== */

/* ── 1. 基本函数指针声明与调用 ── */

static int32_t add(int32_t a, int32_t b)
{
    return a + b;
}

static int32_t sub(int32_t a, int32_t b)
{
    return a - b;
}

static int32_t mul(int32_t a, int32_t b)
{
    return a * b;
}

static void demo_basic_function_pointer(void)
{
    printf("=== 1. 基本函数指针声明与调用 ===\n");

    /* 声明函数指针: int32_t (*fp)(int32_t, int32_t) */
    int32_t (*fp)(int32_t, int32_t) = &add;

    /* 方式 A: 隐式解引用调用 fp(a, b) */
    int32_t r1 = fp(3, 5);
    printf("  fp(3, 5) = %" PRId32 "\n", r1);

    /* 方式 B: 显式解引用调用 (*fp)(a, b) */
    int32_t r2 = (*fp)(10, 20);
    printf("  (*fp)(10, 20) = %" PRId32 "\n", r2);

    /* 赋值: 函数名 decay 为指针（& 可省略） */
    fp = sub;
    int32_t r3 = fp(100, 37);
    printf("  fp = sub; fp(100, 37) = %" PRId32 "\n", r3);

    fp = mul;
    int32_t r4 = (*fp)(6, 7);
    printf("  fp = mul; (*fp)(6, 7) = %" PRId32 "\n", r4);

    printf("\n");
}

/* ── 2. 函数指针数组: Dispatch Table ── */

/* 用 typedef 简化函数指针类型 */
typedef int32_t (*binary_op_t)(int32_t, int32_t);

static void demo_dispatch_table(void)
{
    printf("=== 2. 函数指针数组: Dispatch Table ===\n");

    /* 函数指针数组 */
    const binary_op_t ops[3] = { add, sub, mul };
    const char *op_names[3] = { "+", "-", "*" };

    int32_t x = 10, y = 3;
    printf("  x = %" PRId32 ", y = %" PRId32 "\n", x, y);
    printf("  Dispatch table: [add, sub, mul]\n\n");

    for (int32_t i = 0; i < 3; i++) {
        int32_t result = ops[i](x, y);
        printf("  ops[%" PRId32 "] (%s): %" PRId32 " %s %" PRId32 " = %" PRId32 "\n",
               i, op_names[i], x, op_names[i], y, result);
    }
    printf("\n");
}

/* ── 3. 函数指针作为参数 (高阶函数) ── */

/* apply: 接受一个二元操作函数指针 */
static int32_t apply_op(int32_t a, int32_t b, binary_op_t op)
{
    return op(a, b);
}

/* map_op: 对数组每个元素应用一个一元操作函数指针 */
typedef int32_t (*unary_op_t)(int32_t);

static int32_t square(int32_t x)
{
    return x * x;
}

static int32_t negate(int32_t x)
{
    return -x;
}

static void apply_unary_to_array(int32_t arr[], int32_t len, unary_op_t op)
{
    for (int32_t i = 0; i < len; i++) {
        arr[i] = op(arr[i]);
    }
}

static void demo_higher_order_function(void)
{
    printf("=== 3. 函数指针作为参数 (高阶函数) ===\n");

    /* 二元操作: apply_op */
    printf("  apply_op(6, 7, mul) = %" PRId32 "\n", apply_op(6, 7, mul));
    printf("  apply_op(100, 37, sub) = %" PRId32 "\n", apply_op(100, 37, sub));

    /* 一元操作: apply_unary_to_array */
    int32_t nums[5] = { 1, 2, 3, 4, 5 };
    printf("  原数组: ");
    for (int32_t i = 0; i < 5; i++) printf("%" PRId32 " ", nums[i]);
    printf("\n");

    apply_unary_to_array(nums, 5, square);
    printf("  square 后: ");
    for (int32_t i = 0; i < 5; i++) printf("%" PRId32 " ", nums[i]);
    printf("\n");

    apply_unary_to_array(nums, 5, negate);
    printf("  negate 后: ");
    for (int32_t i = 0; i < 5; i++) printf("%" PRId32 " ", nums[i]);
    printf("\n");

    printf("\n");
}

/* ── 4. Struct + 函数指针: 模拟 OOP 方法 ── */

typedef struct Point2D_t {
    double x;
    double y;
    double (*length)(const struct Point2D_t *self);
} Point2D;

static double point2d_length(const Point2D *self)
{
    return sqrt(self->x * self->x + self->y * self->y);
}

static void point2d_init(Point2D *p, double px, double py)
{
    p->x = px;
    p->y = py;
    p->length = point2d_length;
}

/* 带名称的 Shape 接口模拟 */
typedef struct Shape_t {
    double (*area)(const struct Shape_t *self);
    double (*perimeter)(const struct Shape_t *self);
} Shape;

/* Circle */
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

static void circle_init(Circle *c, double r)
{
    c->radius = r;
    c->base.area = circle_area;
    c->base.perimeter = circle_perimeter;
}

/* Rectangle */
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

static void rect_init(Rectangle *r, double w, double h)
{
    r->width = w;
    r->height = h;
    r->base.area = rect_area;
    r->base.perimeter = rect_perimeter;
}

static void demo_struct_function_pointers(void)
{
    printf("=== 4. Struct + 函数指针: 模拟 OOP 方法 ===\n");

    /* Point2D */
    Point2D pt;
    point2d_init(&pt, 3.0, 4.0);
    printf("  Point2D(3.0, 4.0).length() = %.1f\n", pt.length(&pt));

    /* Circle */
    Circle circ;
    circle_init(&circ, 5.0);
    printf("  Circle(radius=5.0).area() = %.4f\n", circ.base.area((Shape *)&circ));
    printf("  Circle(radius=5.0).perimeter() = %.4f\n", circ.base.perimeter((Shape *)&circ));

    /* Rectangle */
    Rectangle rect;
    rect_init(&rect, 4.0, 6.0);
    printf("  Rectangle(4.0 x 6.0).area() = %.4f\n", rect.base.area((Shape *)&rect));
    printf("  Rectangle(4.0 x 6.0).perimeter() = %.4f\n", rect.base.perimeter((Shape *)&rect));

    /* 多态: 通过 Shape* 统一调用 */
    Shape *shapes[2];
    shapes[0] = (Shape *)&circ;
    shapes[1] = (Shape *)&rect;

    printf("\n  多态调用 (通过 Shape*):\n");
    for (int32_t i = 0; i < 2; i++) {
        printf("    shapes[%" PRId32 "].area() = %.4f\n", i, shapes[i]->area(shapes[i]));
        printf("    shapes[%" PRId32 "].perimeter() = %.4f\n", i, shapes[i]->perimeter(shapes[i]));
    }
    printf("\n");
}

/* ── Coordinator entry ── */

int main_function_pointers_sample(void)
{
    printf("========================================\n");
    printf("  函数指针 (Function Pointers)\n");
    printf("========================================\n\n");

    demo_basic_function_pointer();
    demo_dispatch_table();
    demo_higher_order_function();
    demo_struct_function_pointers();

    printf("函数指针演示完毕。\n");
    return 0;
}
