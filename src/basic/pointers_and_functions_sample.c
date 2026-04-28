#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "basic/pointers_and_functions_sample.h"

/*
 * 指针与函数 — 指针做参数、传递引用、返回指针
 * 用指针穿过函数的墙，让被调函数修改调用者的变量。
 */

static void swap_by_value(int32_t a, int32_t b)
{
    int32_t tmp = a;
    a = b;
    b = tmp;
}

static void swap_by_pointer(int32_t *a, int32_t *b)
{
    int32_t tmp = *a;
    *a = *b;
    *b = tmp;
}

static void demo_pass_by_value_vs_reference(void)
{
    printf("=== 值传递 vs 传递引用 ===\n\n");

    int32_t x = 10, y = 20;
    printf("  调用 swap_by_value(%" PRId32 ", %" PRId32 ")\n", x, y);
    swap_by_value(x, y);
    printf("  结果: x = %" PRId32 ", y = %" PRId32 "  ← 未改变!\n\n", x, y);

    x = 10; y = 20;
    printf("  调用 swap_by_pointer(&x, &y)\n");
    swap_by_pointer(&x, &y);
    printf("  结果: x = %" PRId32 ", y = %" PRId32 "  ← 已交换!\n\n", x, y);

    printf("  值传递: 函数收到副本，修改不影响外部\n");
    printf("  指针传递: 函数收到地址，通过 * 修改真实数据\n\n");
}

static void demo_return_address_of_local(void)
{
    printf("=== 经典陷阱: 返回局部变量地址 ===\n\n");

    printf("  ❌ 危险模式:\n");
    printf("    int32_t *func(void) {\n");
    printf("        int32_t temp = 42;  // 栈上局部变量\n");
    printf("        return &temp;        // 函数返回后 temp 已失效!\n");
    printf("    }\n\n");
    printf("  函数退出后栈帧销毁，返回的地址变成野指针。\n");
    printf("  解引用 → 未定义行为 (UB)。\n\n");

    printf("  ✅ 正确模式 1: 通过参数返回\n");
    printf("    void calc(int32_t *out) { *out = 42; }\n\n");
    printf("  ✅ 正确模式 2: 返回堆分配内存\n");
    printf("    int32_t *make(int32_t v) {\n");
    printf("        int32_t *p = malloc(sizeof *p);\n");
    printf("        *p = v; return p;  // 堆上数据函数返回后仍有效\n");
    printf("    }\n\n");
}

static void demo_find_max_demo(void)
{
    int32_t data[] = {3, 17, -5, 42, 8};
    int32_t n = 5;

    printf("=== 实战: 指向最大元素的指针 ===\n");

    int32_t *max = &data[0];
    for (int32_t i = 1; i < n; i++) {
        if (data[i] > *max) {
            max = &data[i];
        }
    }

    printf("  数组: ");
    for (int32_t i = 0; i < n; i++) printf("%" PRId32 " ", data[i]);
    printf("\n");
    printf("  最大值: %" PRId32 " (地址 %p, 索引 %" PRId32 ")\n\n",
           *max, (void *)max, (int32_t)(max - data));
}

static void demo_modify_array_via_pointer(void)
{
    int32_t scores[5] = {80, 85, 90, 75, 95};
    int32_t n = 5;

    printf("=== 函数内修改数组内容 ===\n");

    printf("  原始: ");
    for (int32_t i = 0; i < n; i++) printf("%" PRId32 " ", scores[i]);
    printf("\n");

    /* 模拟 in-place 翻倍操作 */
    for (int32_t *p = scores; p < scores + n; p++) {
        *p *= 2;
    }

    printf("  翻倍: ");
    for (int32_t i = 0; i < n; i++) printf("%" PRId32 " ", scores[i]);
    printf("\n\n");
}

int main_pointers_and_functions_sample(void)
{
    printf("========================================\n");
    printf("  指针与函数\n");
    printf("  「指针是穿过函数墙壁的手段。」\n");
    printf("========================================\n\n");

    demo_pass_by_value_vs_reference();
    demo_return_address_of_local();
    demo_find_max_demo();
    demo_modify_array_via_pointer();

    printf("指针与函数演示完毕。\n");
    return 0;
}
