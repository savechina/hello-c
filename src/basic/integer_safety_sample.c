#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include "basic/integer_safety_sample.h"

/*
 * 整数安全 (CERT INT30-C / INT31-C / INT33-C 的入门版)
 * 三条铁律: 有符号溢出是 UB (绝不执行), 无符号回绕是良定义的,
 * 移位宽度必须先检查, 窄化必须先验证再转换。
 */

int checked_add_int(int a, int b, int *out)
{
    if (out == NULL) {
        return 0;
    }
    /* 检查本身不能溢出: b > 0 时用 INT_MAX - b (合法), b < 0 时用 INT_MIN - b
     * (减负数 = 加正量, 不会越过边界) — 这就是不能只写 a > INT_MAX - b 的原因 */
    if (b > 0 && a > INT_MAX - b) {
        return 0;
    }
    if (b < 0 && a < INT_MIN - b) {
        return 0;
    }
    *out = a + b;
    return 1;
}

int checked_shl_uint(uint32_t value, unsigned shift, uint32_t *out)
{
    if (out == NULL) {
        return 0;
    }
    /* 宽度检查必须发生在移位之前: shift >= 32 的移位是 UB (C17 6.5.7p3) */
    if (shift >= 32u) {
        return 0;
    }
    *out = value << shift;
    return 1;
}

/* 先在宽类型里验证范围, 再显式转换 — 顺序不能反过来 (CERT INT31-C) */
static int narrow_to_int(long long v, int *out)
{
    if (v < (long long)INT_MIN || v > (long long)INT_MAX) {
        return 0;
    }
    *out = (int)v;
    return 1;
}

static void demo_unsigned_wraparound(void)
{
    uint32_t u = UINT32_MAX;
    int32_t s = -42;

    printf("=== 无符号回绕是良定义的 ===\n");
    u = u + 1;
    printf("  UINT32_MAX + 1 = %" PRIu32 "  ← 0, 标准规定按模 2^32 回绕 (C17 6.5p5)\n", u);
    printf("  对比: 有符号 INT_MAX + 1 是 signed overflow → UB\n");
    printf("  本章绝不执行有符号溢出、绝不移位 >= 位宽、绝不计算 INT_MIN / -1\n");
    printf("  PRId32 打印 int32_t: s = %" PRId32 " (平台无关的格式宏)\n\n", s);
}

static void demo_checked_add(void)
{
    int out = 0;

    printf("=== 检查后再加 (checked_add_int) ===\n");
    if (checked_add_int(2000000000, 107483647, &out)) {
        printf("  checked_add_int(2000000000, 107483647) = %d\n", out);
    } else {
        printf("  2000000000 + 107483647 被拒绝 (会溢出)\n");
    }
    if (checked_add_int(INT_MAX, 1, &out)) {
        printf("  checked_add_int(INT_MAX, 1) = %d ← 不应到达\n", out);
    } else {
        printf("  checked_add_int(INT_MAX, 1)  → 拒绝, 溢出从未发生\n");
    }
    if (checked_add_int(INT_MAX, -1, &out)) {
        printf("  checked_add_int(INT_MAX, -1) = %d ← 边界内正常运算\n", out);
    }
    printf("\n");
}

static void demo_safe_narrowing(void)
{
    int out = 0;

    printf("=== 安全窄化: 先验证, 再显式转换 ===\n");
    if (narrow_to_int(123456789LL, &out)) {
        printf("  123456789LL → (int) %d ✓\n", out);
    }
    if (!narrow_to_int(5000000000LL, &out)) {
        printf("  5000000000LL → 拒绝: 超出 int 范围, 不截断不回绕\n");
    }
    /* 反面教材: 跳过验证的显式转换不报错也不 UB, 但结果已被截断 (实现定义, C17 6.3.1.3p3) */
    printf("  若跳过验证: (int)5000000000LL = %d ← 低 32 位被截断, 调用方毫不知情\n\n",
           (int)5000000000LL);
}

static void demo_shift_guard(void)
{
    uint32_t out = 0;

    printf("=== 移位前检查宽度 (checked_shl_uint) ===\n");
    if (checked_shl_uint(1u, 28, &out)) {
        printf("  1u << 28 = 0x%08" PRIX32 "\n", out);
    }
    if (checked_shl_uint(1u, 31, &out)) {
        printf("  1u << 31 = 0x%08" PRIX32 "\n", out);
    }
    if (!checked_shl_uint(1u, 32, &out)) {
        printf("  shift = 32 → 拒绝: 无符号数移位 >= 位宽是 UB (C17 6.5.7p3)\n");
    }
    printf("\n");
}

int main_integer_safety_sample(void)
{
    printf("--- 整数安全 (Integer Safety) ---\n");
    printf("CERT INT30/31/33-C: 有符号溢出绝不执行, 回绕只用无符号, 移位先查宽度.\n\n");

    demo_unsigned_wraparound();
    demo_checked_add();
    demo_safe_narrowing();
    demo_shift_guard();

    printf("整数安全演示完毕.\n");
    return 0;
}
