#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "basic/const_correctness_sample.h"

/*
 * const 正确性 — const 指针、指向 const 的指针
 * const 是你对编译器的承诺：这块数据不应被修改。
 */

static void demo_pointer_to_const(void)
{
    const int32_t pi = 314;
    const int32_t *ptr = &pi;

    printf("=== int32_t const *p — 指向 const 的指针 ===\n");
    printf("  *ptr = %" PRId32 "\n", *ptr);

    /* *ptr = 999;    ← ❌ 编译错误: 不能通过 ptr 修改 pi */
    printf("  ❌ *ptr = 999 → 编译错误 (指向 const, 不能修改所指向的值)\n");

    int32_t other = 42;
    ptr = &other;   /* ✅ ptr 本身不是 const, 可以重新指向 */
    printf("  ✅ ptr = &other → 指针可以改指向 (ptr 本身可变)\n");
    printf("  新目标: *ptr = %" PRId32 "\n\n", *ptr);
}

static void demo_const_pointer(void)
{
    int32_t vals[2] = {10, 20};
    int32_t *const ptr = &vals[0];

    printf("=== int32_t *const p — const 指针 ===\n");
    printf("  *ptr = %" PRId32 "\n", *ptr);

    *ptr = 99;      /* ✅ 可以修改所指向的值 */
    printf("  *ptr = 99 → vals[0] = %" PRId32 " (可以修改值)\n", vals[0]);

    /* ptr = &vals[1];  ← ❌ 编译错误: const 指针不能改指向 */
    printf("  ❌ ptr = &vals[1] → 编译错误 (指针是 const, 不能改指向)\n\n");
}

static void demo_const_pointer_to_const(void)
{
    const int32_t secret = 777;
    const int32_t *const ptr = &secret;

    printf("=== int32_t const *const p — 双重 const ===\n");
    printf("  *ptr = %" PRId32 "\n", *ptr);

    /* *ptr = 1;   ← ❌ 不能修改值 */
    printf("  ❌ *ptr = 1    → 编译错误\n");

    /* int32_t other = 0; ptr = &other;  ← ❌ 不能改指向 */
    printf("  ❌ ptr = &other → 编译错误\n");
    printf("  值不可改 + 指向不可改 = 完全只读\n\n");
}

static void demo_const_in_functions(void)
{
    printf("=== const 在函数签名中的意义 ===\n\n");

    printf("  void print_string(const char *s);\n");
    printf("    → 告诉调用者: 我不会修改你传入的字符串\n\n");

    printf("  void copy_str(char *dest, const char *src);\n");
    printf("    → dest 可写, src 只读\n\n");

    /* 实际演示: 安全读取 */
    const char msg[] = "hello const";
    size_t len = strlen(msg);
    printf("  strlen(\"%s\") = %zu\n\n", msg, len);

    printf("  Python 对照:\n");
    printf("    const char * → 类似 Tuple (不可变序列)\n");
    printf("    char *       → 类似 List  (可变序列)\n\n");
}

int main_const_correctness_sample(void)
{
    printf("========================================\n");
    printf("  const 正确性\n");
    printf("  「const 是你对编译器的承诺。」\n");
    printf("========================================\n\n");

    demo_pointer_to_const();
    demo_const_pointer();
    demo_const_pointer_to_const();
    demo_const_in_functions();

    printf("const 正确性演示完毕。\n");
    return 0;
}
