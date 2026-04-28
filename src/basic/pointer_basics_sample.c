#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "basic/pointer_basics_sample.h"

/*
 * 指针基础 — 开篇、&, *, NULL、指针初始化
 * 指针像 GPS 坐标——它不是目的地，而是告诉你目的地的方向。
 */

static void demo_gps_address(void)
{
    int32_t destination = 42;
    int32_t *gps = &destination;

    printf("=== GPS 类比：指针是地址，不是目的地 ===\n");
    printf("  destination = %" PRId32 "          ← 目的地本身\n", destination);
    printf("  gps         = %p          ← GPS 坐标（地址）\n", (void *)gps);
    printf("  *gps        = %" PRId32 "          ← 顺着 GPS 找到目的地\n\n", *gps);
}

static void demo_ampersand_star(void)
{
    int32_t x = 7;
    int32_t *p = &x;

    printf("=== & 取地址 vs * 解引用 ===\n");
    printf("  x   = %" PRId32 "\n", x);
    printf("  &x  = %p   (取 x 的地址)\n", (void *)&x);
    printf("  p   = %p   (p 存的就是 &x)\n", (void *)p);
    printf("  *p  = %" PRId32 "   (*p 解引用得到 x 的值)\n\n", *p);

    *p = 99;
    printf("  *p = 99 之后: x = %" PRId32 " (值已被修改)\n\n", x);
}

static void demo_null_safety(void)
{
    printf("=== NULL 指针与安全检查 ===\n");

    int32_t *ptr = NULL;
    printf("  初始: ptr = %p (NULL，不指向任何有效地址)\n", (void *)ptr);
    printf("  ❌ 若 *ptr → 段错误 (Segmentation Fault)\n");

    int32_t target = 123;
    ptr = &target;
    if (ptr != NULL) {
        printf("  ✅ ptr != NULL 检查通过: *ptr = %" PRId32 "\n", *ptr);
    }

    ptr = NULL;
    if (ptr == NULL) {
        printf("  ✅ ptr 已重置为 NULL，跳过解引用\n");
    }
    printf("\n");
}

static void demo_initialization_good_bad(void)
{
    printf("=== 指针初始化：拒绝野指针 ===\n");

    printf("  ❌ 野指针 — 未初始化，指向随机地址:\n");
    printf("     int32_t *bad;\n");
    printf("     *bad = 5;  // 写入随机内存 = 崩溃/数据损坏\n\n");

    int32_t val = 0;
    int32_t *good_a = &val;
    int32_t *good_b = NULL;

    printf("  ✅ 初始化为有效地址: int32_t *a = &val;\n");
    printf("  ✅ 初始化为 NULL:    int32_t *b = NULL;\n");
    printf("     good_a = %p\n", (void *)good_a);
    printf("     good_b = %p (NULL)\n\n", (void *)good_b);
}

static void demo_pointer_type_matters(void)
{
    int32_t iv = 0x01020304;

    printf("=== 指针类型决定步长 ===\n");

    uint8_t *bp = (uint8_t *)&iv;
    printf("  int32_t iv = 0x%08" PRIx32 "\n", (uint32_t)iv);
    for (int32_t i = 0; i < 4; i++) {
        printf("    ((uint8_t*)&iv)[%" PRId32 "] = 0x%02" PRIx8 "\n", i, bp[i]);
    }

    printf("\n  uint8_t*  每次移动 1 字节\n");
    printf("  int32_t*  每次移动 4 字节\n");
    printf("  指针类型 = 编译器如何看待所指向的内存\n\n");
}

int main_pointer_basics_sample(void)
{
    printf("========================================\n");
    printf("  指针基础 (&, *, NULL, 初始化)\n");
    printf("  「指针像 GPS 坐标——它不是目的地，\n");
    printf("     而是告诉你目的地的方向。」\n");
    printf("========================================\n\n");

    demo_gps_address();
    demo_ampersand_star();
    demo_null_safety();
    demo_initialization_good_bad();
    demo_pointer_type_matters();

    printf("指针基础演示完毕。\n");
    return 0;
}
