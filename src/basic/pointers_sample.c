#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "basic/pointers_sample.h"

/*
 * 指针基础演示 (Pointer Basics)
 *
 * 本节演示 C 语言指针的核心概念：
 *  - 指针声明与初始化
 *  - & 取地址运算符
 *  - * 解引用运算符
 *  - NULL 指针与安全检查
 *  - 指针类型的重要性
 *  - 常见指针错误
 */

/* ── 1. 指针声明与基本操作 ── */
static void pointers_basic_demo(void)
{
    int32_t value = 42;
    int32_t *ptr = &value;   /* ptr 存储 value 的地址 */

    printf("=== 1. 指针声明与基本操作 ===\n");
    printf("  value = %" PRId32 "\n", value);
    printf("  &value = %p\n", (void *)&value);
    printf("  ptr    = %p\n", (void *)ptr);
    printf("  *ptr   = %" PRId32 "\n", *ptr);
    printf("  &ptr   = %p\n", (void *)&ptr);
    printf("\n");

    /* 通过指针修改值 */
    *ptr = 100;
    printf("  通过 *ptr = 100 修改后:\n");
    printf("  value = %" PRId32 " (已改变!)\n", value);
    printf("\n");
}

/* ── 2. & 取地址 和 * 解引用 ── */
static void pointers_address_of_deref_demo(void)
{
    int32_t x = 7;
    int32_t *p = &x;

    printf("=== 2. & 取地址 vs * 解引用 ===\n");
    printf("  x = %" PRId32 "\n", x);
    printf("  &x = %p   (取 x 的地址)\n", (void *)&x);
    printf("  p  = %p   (p 存的就是 &x)\n", (void *)p);
    printf("  *p = %" PRId32 "   (*p 解引用得到 x 的值)\n", *p);
    printf("\n");

    /* 双重间接: ** */
    int32_t **pp = &p;
    printf("  **pp = %" PRId32 "  (pp -> p -> x, 间接取值)\n", **pp);
    printf("\n");
}

/* ── 3. 内存布局 ASCII 示意图演示 ── */
static void pointers_memory_layout_demo(void)
{
    printf("=== 3. 内存布局示意图 ===\n");
    printf("\n");
    printf("  符号       地址      值\n");
    printf("  ----+------+------+------\n");
    printf("  x   |0x7ff…00|   42    ← 实际变量\n");
    printf("  px  |0x7ff…08|0x7ff…00 ← 指针存 x 的地址\n");
    printf("\n");
    printf("  *px  → 跟随 px 中的地址, 找到 x 的值 = 42\n");
    printf("  &x   → x 本身的地址 = 0x7ff…00\n");
    printf("  &px  → px 自己的地址 = 0x7ff…08\n");
    printf("\n");

    int32_t x = 42;
    int32_t *px = &x;
    printf("  实际输出验证:\n");
    printf("  x     = %" PRId32 "\n", x);
    printf("  &x    = %p\n", (void *)&x);
    printf("  px    = %p\n", (void *)px);
    printf("  *px   = %" PRId32 "\n", *px);
    printf("  &px   = %p\n", (void *)&px);
    printf("\n");
}

/* ── 4. NULL 指针与安全检查 ── */
static void pointers_null_check_demo(void)
{
    printf("=== 4. NULL 指针安全检查 ===\n");
    printf("\n");

    int32_t *ptr = NULL;  /* 明确初始化为 NULL */
    printf("  ptr = %p (NULL, 不指向任何有效地址)\n", (void *)ptr);

    /* 错误示例: 直接解引用 NULL 会崩溃 */
    printf("  ❌ 不能 *ptr —— 会触发段错误 (Segmentation Fault)\n");

    /* 正确做法: 使用前检查 */
    int32_t target = 99;
    ptr = &target;
    if (ptr != NULL) {
        printf("  ✅ ptr != NULL 检查通过后, *ptr = %" PRId32 "\n", *ptr);
    }

    /* 重置为 NULL */
    ptr = NULL;
    if (ptr == NULL) {
        printf("  ✅ ptr 此时为 NULL, 跳过解引用\n");
    }
    printf("\n");
}

/* ── 5. 指针类型的重要性 ── */
static void pointers_type_demo(void)
{
    printf("=== 5. 指针类型很重要 ===\n");
    printf("\n");

    int32_t  iv = 0x01020304;
    uint8_t *bp = (uint8_t *)&iv;

    printf("  int32_t iv = 0x%08" PRIx32 "\n", (uint32_t)iv);
    printf("  按字节逐字节查看:\n");

    for (int32_t i = 0; i < 4; i++) {
        printf("    byte[%" PRId32 "] = 0x%02" PRIx8 "\n", i, bp[i]);
    }
    printf("\n");
    printf("  为什么用 uint8_t* 而不是 int32_t* ?\n");
    printf("  - uint8_t* 每次移动 1 字节\n");
    printf("  - int32_t* 每次移动 4 字节\n");
    printf("  - 指针类型决定了解引用的范围和步长\n");
    printf("\n");
}

/* ── 6. 指针初始化 —— 不要野指针 ── */
static void pointers_initialization_demo(void)
{
    printf("=== 6. 指针初始化: 拒绝野指针 ===\n");
    printf("\n");

    /* 野指针: 未初始化, 指向随机地址 */
    printf("  野指针 (未初始化) 的危险:\n");
    printf("    int32_t *bad;   // ❌ bad 指向随机地址\n");
    printf("    // *bad = 5;   // ❌ 写入随机内存 = 崩溃/数据损坏\n");
    printf("\n");

    /* 好的做法 */
    int32_t safe_value = 0;
    int32_t *good_ptr_a = &safe_value;   /* 初始化为有效地址 */
    int32_t *good_ptr_b = NULL;          /* 明确初始化为 NULL */

    printf("  好的做法:\n");
    printf("    int32_t *a = &safe_value;  // ✅ 指向有效变量\n");
    printf("    int32_t *b = NULL;         // ✅ 明确为空\n");
    printf("    a 的地址: %p\n", (void *)good_ptr_a);
    printf("    b 的地址: %p (NULL)\n", (void *)good_ptr_b);
    printf("\n");
}

/* ── 7. Python 变量赋值 vs C 指针赋值对比 ── */
static void pointers_vs_python_demo(void)
{
    printf("=== 7. Python 变量赋值 vs C 指针赋值 ===\n");
    printf("\n");

    printf("  Python:\n");
    printf("    a = 10    # a 是变量, 存储值 10\n");
    printf("    b = a     # b 得到 a 的值的副本\n");
    printf("    b = 20    # a 仍是 10, b 现在是 20 (独立)\n");
    printf("\n");

    printf("  C 语言 (值拷贝, 类似 Python):\n");
    int32_t a = 10;
    int32_t b = a;
    printf("    int32_t a = 10;\n");
    printf("    int32_t b = a;   // b 得到 a 的副本\n");
    printf("    a = %d, b = %d\n", (int)a, (int)b);
    b = 20;
    printf("    b = 20;  // a 不受影响\n");
    printf("    a = %d, b = %d\n", (int)a, (int)b);
    printf("\n");

    printf("  C 语言 (指针, Python 没有对应物):\n");
    int32_t x = 10;
    int32_t *px_a = &x;
    int32_t *px_b = px_a;
    printf("    int32_t *px_a = &x;  // px_a 指向 x\n");
    printf("    int32_t *px_b = px_a; // px_b 也指向 x (同一个地址!)\n");
    printf("    *px_a = %d, *px_b = %d\n", (int)*px_a, (int)*px_b);
    *px_b = 99;
    printf("    *px_b = 99;  // x 也被修改!\n");
    printf("    x = %d, *px_a = %d\n", (int)x, (int)*px_a);
    printf("\n");
    printf("  关键区别: C 指针让你直接操作内存, 多个指针可以指向同一位置\n");
    printf("\n");
}

/* ── Coordinator entry ── */
int main_pointers_sample(void)
{
    printf("========================================\n");
    printf("  指针基础 (Pointer Basics)\n");
    printf("========================================\n\n");

    pointers_basic_demo();
    pointers_address_of_deref_demo();
    pointers_memory_layout_demo();
    pointers_null_check_demo();
    pointers_type_demo();
    pointers_initialization_demo();
    pointers_vs_python_demo();

    printf("指针演示完毕。\n");
    return 0;
}
