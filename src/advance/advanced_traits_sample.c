#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "advance/advanced_traits_sample.h"

/* ====================================================================
 * 高级多态演示 (Advanced Traits / VTable Polymorphism)
 *
 * 本节目演 C 语言如何通过函数指针表实现运行时多态：
 *  - Function pointer dispatch table
 *  - VTable-like struct pattern（模拟 OO 虚函数表）
 *  - Interface pattern: struct of fp + void* data = "virtual class"
 *  - Dynamic dispatch: Shape → Circle, Rectangle, Triangle
 *  - Error-first: NULL function pointer guard
 * ==================================================================== */

/* ── 1. Function Pointer Dispatch Table ── */

typedef int32_t (*op_func_t)(int32_t, int32_t);

static int32_t op_add(int32_t a, int32_t b) { return a + b; }
static int32_t op_sub(int32_t a, int32_t b) { return a - b; }
static int32_t op_mul(int32_t a, int32_t b) { return a * b; }
static int32_t op_div(int32_t a, int32_t b)
{
    if (b == 0) {
        printf("    [Error] Division by zero!\n");
        return 0;
    }
    return a / b;
}
static int32_t op_mod(int32_t a, int32_t b)
{
    if (b == 0) {
        printf("    [Error] Modulo by zero!\n");
        return 0;
    }
    return a % b;
}

static void demo_dispatch_table(void)
{
    printf("=== 1. Function Pointer Dispatch Table ===\n");

    static const op_func_t dispatch[] = {
        op_add, op_sub, op_mul, op_div, op_mod
    };
    static const char *names[] = { "add", "sub", "mul", "div", "mod" };
    int32_t num_ops = (int32_t)(sizeof(dispatch) / sizeof(dispatch[0]));

    int32_t x = 20, y = 7;
    printf("  x = %" PRId32 ", y = %" PRId32 "\n", x, y);

    for (int32_t i = 0; i < num_ops; i++) {
        int32_t result = dispatch[i](x, y);
        printf("  dispatch[%2" PRId32 "] (%-3s): %" PRId32 " -> %" PRId32 "\n",
               i, names[i], result, result);
    }

    /* 安全：NULL guard */
    printf("  NULL 检查: dispatch[0] = %s\n", dispatch[0] ? "非空 ✓" : "NULL ✗");

    printf("\n");
}

/* ── 2. VTable-like Struct for Polymorphism ── */

/* Shape VTable — 每个 Shape 实例通过指针共享同一份 vtable */

typedef double (*shape_area_fn)(const void *self);
typedef double (*shape_perimeter_fn)(const void *self);
typedef void (*shape_destroy_fn)(void *self);
typedef void (*shape_print_fn)(const void *self);

typedef struct {
    shape_area_fn     area;
    shape_perimeter_fn perimeter;
    shape_destroy_fn  destroy;
    shape_print_fn    print;
} ShapeVTable;

/* Shape 基类：vtable 指针 + 通用数据指针 */

typedef struct {
    ShapeVTable *vtable;
    void *data;
} Shape;

/* Circle 实现 */

typedef struct {
    double radius;
} CircleData;

static double circle_area(const void *self)
{
    const CircleData *d = (const CircleData *)self;
    return 3.14159265358979323846 * d->radius * d->radius;
}

static double circle_perimeter(const void *self)
{
    const CircleData *d = (const CircleData *)self;
    return 2.0 * 3.14159265358979323846 * d->radius;
}

static void circle_destroy(void *self)
{
    (void)self;
    printf("    [Circle] destroyed\n");
}

static void circle_print(const void *self)
{
    const CircleData *d = (const CircleData *)self;
    printf("    Circle(r=%.2f)\n", d->radius);
}

static ShapeVTable circle_vtable = {
    .area     = circle_area,
    .perimeter = circle_perimeter,
    .destroy  = circle_destroy,
    .print    = circle_print
};

/* Rectangle 实现 */

typedef struct {
    double width;
    double height;
} RectData;

static double rect_area(const void *self)
{
    const RectData *d = (const RectData *)self;
    return d->width * d->height;
}

static double rect_perimeter(const void *self)
{
    const RectData *d = (const RectData *)self;
    return 2.0 * (d->width + d->height);
}

static void rect_destroy(void *self)
{
    (void)self;
    printf("    [Rectangle] destroyed\n");
}

static void rect_print(const void *self)
{
    const RectData *d = (const RectData *)self;
    printf("    Rectangle(%.2f x %.2f)\n", d->width, d->height);
}

static ShapeVTable rect_vtable = {
    .area     = rect_area,
    .perimeter = rect_perimeter,
    .destroy  = rect_destroy,
    .print    = rect_print
};

/* Triangle 实现 (Heron's formula) */

typedef struct {
    double a;
    double b;
    double c;
} TriData;

static double tri_area(const void *self)
{
    const TriData *d = (const TriData *)self;
    double s = (d->a + d->b + d->c) / 2.0;
    double inner = s * (s - d->a) * (s - d->b) * (s - d->c);
    if (inner <= 0.0) {
        return 0.0;  /* 非法三角形 */
    }
    return sqrt(inner);  /* Heron's formula: sqrt(s*(s-a)*(s-b)*(s-c)) */
}

static double tri_perimeter(const void *self)
{
    const TriData *d = (const TriData *)self;
    return d->a + d->b + d->c;
}

static void tri_destroy(void *self)
{
    (void)self;
    printf("    [Triangle] destroyed\n");
}

static void tri_print(const void *self)
{
    const TriData *d = (const TriData *)self;
    printf("    Triangle(%.2f, %.2f, %.2f)\n", d->a, d->b, d->c);
}

static ShapeVTable tri_vtable = {
    .area     = tri_area,
    .perimeter = tri_perimeter,
    .destroy  = tri_destroy,
    .print    = tri_print
};

/* 通过 Shape 接口 dispatch — 动态多态 */

static double shape_area(Shape *s)
{
    if (s == NULL || s->vtable == NULL || s->vtable->area == NULL) {
        printf("    [Error] NULL shape or NULL vtable->area!\n");
        return 0.0;
    }
    return s->vtable->area(s->data);
}

static double shape_perimeter(Shape *s)
{
    if (s == NULL || s->vtable == NULL || s->vtable->perimeter == NULL) {
        printf("    [Error] NULL shape or NULL vtable->perimeter!\n");
        return 0.0;
    }
    return s->vtable->perimeter(s->data);
}

static void shape_print(Shape *s)
{
    if (s == NULL || s->vtable == NULL || s->vtable->print == NULL) {
        printf("    [Error] NULL shape or NULL vtable->print!\n");
        return;
    }
    s->vtable->print(s->data);
}

__attribute__((unused)) static void shape_destroy(Shape *s)
{
    if (s == NULL || s->vtable == NULL || s->vtable->destroy == NULL) {
        return;
    }
    s->vtable->destroy(s->data);
}

static void demo_vtable_polymorphism(void)
{
    printf("=== 2. VTable-like Struct — Runtime Polymorphism ===\n");

    /* 初始化各种 Shape */
    CircleData circ_data  = { .radius = 5.0 };
    RectData   rect_data  = { .width = 4.0, .height = 6.0 };
    TriData    tri_data   = { .a = 3.0, .b = 4.0, .c = 5.0 };

    Shape shapes[3];
    shapes[0] = (Shape){ .vtable = &circle_vtable, .data = &circ_data };
    shapes[1] = (Shape){ .vtable = &rect_vtable,   .data = &rect_data };
    shapes[2] = (Shape){ .vtable = &tri_vtable,    .data = &tri_data };

    printf("\n  多态遍历 (Shape* 统一接口):\n");
    for (int32_t i = 0; i < 3; i++) {
        printf("  Shape[%" PRId32 "]: ", i);
        shape_print(&shapes[i]);
        printf("    area = %.4f, perimeter = %.4f\n",
               shape_area(&shapes[i]),
               shape_perimeter(&shapes[i]));
    }

    /* NULL guard 演示 */
    printf("\n  NULL guard 测试:\n");
    Shape null_shape = { .vtable = NULL, .data = NULL };
    shape_area(&null_shape);    /* 安全: 打印错误, 返回 0 */
    shape_perimeter(&null_shape); /* 安全: 打印错误, 返回 0 */
    shape_print(&null_shape);      /* 安全: 打印错误 */

    printf("\n");
}

/* ── 3. Interface Pattern: struct { fp* ... }; + void* data ── */

/*
 * 这是 C 语言实现 "virtual class" 的标准模式：
 *
 *   - vtable 是静态共享的（所有 Circle 实例共享 circle_vtable）
 *   - data 指向实例数据（可 heap 分配、可栈上、可 embedded）
 *   - 接口函数 (shape_area 等) 做 NULL 检查后调用 vtable 的函数指针
 *
 * 类比: Python __call__, C++ virtual, Rust trait — 都是同一件事
 */

typedef struct {
    void (*setup)(void *data);
    double (*dispatch)(void *data, double input);
    void   (*cleanup)(void *data);
} TransformInterface;

typedef struct {
    double scale;
    double offset;
} ScaleOffsetData;

static void so_setup(void *data)
{
    ScaleOffsetData *d = (ScaleOffsetData *)data;
    d->scale = 2.0;
    d->offset = 10.0;
    printf("    [ScaleOffset] setup: scale=%.1f, offset=%.1f\n", d->scale, d->offset);
}

static double so_dispatch(void *data, double input)
{
    ScaleOffsetData *d = (ScaleOffsetData *)data;
    return d->scale * input + d->offset;
}

static void so_cleanup(void *data)
{
    (void)data;
    printf("    [ScaleOffset] cleanup\n");
}

typedef struct {
    double multiplier;
} MultiplyData;

static void mul_setup(void *data)
{
    MultiplyData *d = (MultiplyData *)data;
    d->multiplier = 3.0;
    printf("    [Multiply] setup: multiplier=%.1f\n", d->multiplier);
}

static double mul_dispatch(void *data, double input)
{
    MultiplyData *d = (MultiplyData *)data;
    return d->multiplier * input;
}

static void mul_cleanup(void *data)
{
    (void)data;
    printf("    [Multiply] cleanup\n");
}

static void demo_interface_pattern(void)
{
    printf("=== 3. Interface Pattern (struct fp + void* data) ===\n");

    ScaleOffsetData so_data = { 0 };
    MultiplyData mul_data = { 0 };

    TransformInterface iface1;
    iface1.setup    = so_setup;
    iface1.dispatch = so_dispatch;
    iface1.cleanup  = so_cleanup;

    TransformInterface iface2;
    iface2.setup    = mul_setup;
    iface2.dispatch = mul_dispatch;
    iface2.cleanup  = mul_cleanup;

    /* 统一接口调用: "universal remote control" 类比 */
    TransformInterface *ifaces[2] = { &iface1, &iface2 };
    void *data_arr[2] = { &so_data, &mul_data };
    const char *names[2] = { "ScaleOffset", "Multiply" };

    printf("\n  Universal remote control (同一接口, 不同设备):\n");
    for (int32_t i = 0; i < 2; i++) {
        printf("  [Device: %s]\n", names[i]);
        ifaces[i]->setup(data_arr[i]);
        double result = ifaces[i]->dispatch(data_arr[i], 5.0);
        printf("    dispatch(5.0) = %.2f\n", result);
        ifaces[i]->cleanup(data_arr[i]);
    }

    printf("\n");
}

/* ── 4. Dynamic Dispatch + NULL Error Guard ── */

typedef struct {
    int32_t (*invoke)(const char *label, int32_t value);
} Callable;

static int32_t safe_invoke(Callable *c, const char *label, int32_t value)
{
    /* Error-first: NULL function pointer → segfault 的防御 */
    if (c == NULL) {
        printf("    [Error] NULL Callable pointer!\n");
        return -1;
    }
    if (c->invoke == NULL) {
        printf("    [Error] Callable->invoke is NULL! (label=\"%s\")\n", label);
        return -1;
    }
    return c->invoke(label, value);
}

static int32_t logger_func(const char *label, int32_t value)
{
    printf("    [Logger] %s = %" PRId32 "\n", label, value);
    return value;
}

static void demo_null_guard(void)
{
    printf("=== 4. Dynamic Dispatch + NULL Error Guard ===\n");

    Callable good = { .invoke = logger_func };
    Callable bad  = { .invoke = NULL };
    Callable *null_ptr = NULL;

    printf("  [Good] 调用正常函数:\n");
    safe_invoke(&good, "status", 42);

    printf("\n  [Bad]   调用 NULL invoke:\n");
    safe_invoke(&bad, "broken", 99);

    printf("\n  [NULL]  调用 NULL Callable 指针:\n");
    safe_invoke(null_ptr, "null", 0);

    printf("\n");
}

/* ── 5. VTable 共享 vs 嵌入 ── */

/*
 * 两种 VTable 存储策略:
 *
 * 策略 A: 共享静态 VTable (上面演示的)
 *   + 内存高效 (N 个实例共享 1 个 vtable)
 *   + 不能运行时更换行为
 *
 * 策略 B: 每个实例嵌入 VTable (heap 分配)
 *   + 可运行时更换 vtable (swap behavior)
 *   - 内存开销: M 个实例就需要 M 个 vtable 副本
 */

/* 策略 B 演示: 运行时可切换的 VTable */

typedef struct {
    const char *label;
    double (*evaluate)(double);
} DynamicFunc;

static double linear_eval(double x) { return 2.0 * x + 1.0; }
static double quadratic_eval(double x) { return x * x; }
static double const_eval(double x) { (void)x; return 42.0; }

__attribute__((unused)) static ShapeVTable dummy_vtable = {
    .area = NULL, .perimeter = NULL, .destroy = NULL, .print = NULL
};

static void demo_dynamic_dispatch(void)
{
    printf("=== 5. Dynamic Dispatch — Runtime VTable Swap ===\n");

    DynamicFunc funcs[3];
    funcs[0] = (DynamicFunc){ "linear", linear_eval };
    funcs[1] = (DynamicFunc){ "quadratic", quadratic_eval };
    funcs[2] = (DynamicFunc){ "const42", const_eval };

    double x = 3.0;
    printf("  x = %.1f\n\n", x);

    /* 运行时选择函数 */
    for (int32_t i = 0; i < 3; i++) {
        if (funcs[i].evaluate != NULL) {
            printf("  funcs[%" PRId32 "].%s(%.1f) = %.2f\n",
                   i, funcs[i].label, x, funcs[i].evaluate(x));
        } else {
            printf("  funcs[%" PRId32 "].evaluate is NULL!\n", i);
        }
    }

    /* 运行时切换: 把 linear 换成 const42 */
    printf("\n  运行时切换: funcs[0] 从 linear → const42\n");
    funcs[0] = funcs[2];  /* 拷贝 vtable */
    printf("  funcs[0].%s(%.1f) = %.2f (已切换!)\n",
           funcs[0].label, x, funcs[0].evaluate(x));

    printf("\n");
}

/* ── Coordinator entry ── */

int main_advanced_traits_sample(void)
{
    printf("========================================\n");
    printf("  高级多态 (Advanced Traits / VTable)\n");
    printf("========================================\n\n");

    demo_dispatch_table();
    demo_vtable_polymorphism();
    demo_interface_pattern();
    demo_null_guard();
    demo_dynamic_dispatch();

    printf("高级多态演示完毕。\n");
    return 0;
}
