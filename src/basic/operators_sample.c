#include <stdio.h>
#include "basic/operators_sample.h"
#include <stdint.h>

/* ---- Arithmetic operators sample ---- */

static void operators_arithmetic_sample(void) {
    printf("----basic ... operators_arithmetic_sample.....start ----\n");
    int a = 17, b = 5;
    printf("  加法: %d + %d = %d\n", a, b, a + b);
    printf("  减法: %d - %d = %d\n", a, b, a - b);
    printf("  乘法: %d * %d = %d\n", a, b, a * b);
    printf("  整数除法: %d / %d = %d (截断小数)\n", a, b, a / b);
    printf("  浮点除法: %.1f / %d = %.2f\n",
           (double)a, b, (double)a / b);
    printf("  取余: %d %% %d = %d\n", a, b, a % b);
    printf("  负数取余: -7 %% 3 = %d\n", -7 % 3);
    printf("----basic ... operators_arithmetic_sample.....done ----\n");
}

/* ---- Relational operators sample ---- */

static void operators_relational_sample(void) {
    printf("----basic ... operators_relational_sample.....start ----\n");
    int x = 10, y = 20;
    printf("  x = %d, y = %d\n", x, y);
    printf("  x == y: %d\n", x == y);
    printf("  x != y: %d\n", x != y);
    printf("  x < y:  %d\n", x < y);
    printf("  x > y:  %d\n", x > y);
    printf("  x <= y: %d\n", x <= y);
    printf("  x >= y: %d\n", x >= y);
    printf("----basic ... operators_relational_sample.....done ----\n");
}

/* ---- Logical operators & short-circuit sample ---- */

static int trace_call(const char *label, int value) {
    printf("    %s (返回 %d)\n", label, value);
    return value;
}

static void operators_logical_sample(void) {
    printf("----basic ... operators_logical_sample.....start ----\n");
    int a = 1, b = 0;
    printf("  a && b (逻辑与): %d\n", a && b);
    printf("  a || b (逻辑或): %d\n", a || b);
    printf("  !a (逻辑非): %d\n", !a);

    /* 德摩根定律 */
    int p = 0, q = 1;
    int d1 = !(p && q), d2 = (!p) || (!q);
    printf("  德摩根定律: !(0 && 1) = %d, (!0) || (!1) = %d\n", d1, d2);

    printf("\n  && 短路: 0 && ...\n");
    if (trace_call("左边: 0", 0) && trace_call("右边: 1", 1)) {
        printf("  不会执行\n");
    }
    printf("  || 短路: 1 || ...\n");
    if (trace_call("左边: 1", 1) || trace_call("右边: 0", 0)) {
        printf("  条件为真 (|| 短路跳过右边)\n");
    }
    printf("----basic ... operators_logical_sample.....done ----\n");
}

/* ---- Bitwise operators sample ---- */

static void print_bits(uint8_t val) {
    for (int i = 7; i >= 0; i--)
        printf("%d", (val >> i) & 1);
}

static void operators_bitwise_sample(void) {
    printf("----basic ... operators_bitwise_sample.....start ----\n");
    uint8_t x = 5, y = 3;
    printf("  x = "); print_bits(x); printf(" (%u)\n", x);
    printf("  y = "); print_bits(y); printf(" (%u)\n", y);
    printf("  x & y  = "); print_bits((uint8_t)(x & y)); printf(" (%u) 按位与\n", x & y);
    printf("  x | y  = "); print_bits((uint8_t)(x | y)); printf(" (%u) 按位或\n", x | y);
    printf("  x ^ y  = "); print_bits((uint8_t)(x ^ y)); printf(" (%u) 按位异或\n", x ^ y);
    printf("  x << 1 = "); print_bits((uint8_t)(x << 1)); printf(" (%u) 左移\n", x << 1);
    printf("  x >> 1 = "); print_bits((uint8_t)(x >> 1)); printf(" (%u) 右移\n", x >> 1);

    /* Flag manipulation */
    uint8_t flags = 0;
    printf("\n  位域操作:\n");
    printf("  初始: "); print_bits(flags); printf("\n");
    flags |= (1 << 2); printf("  设第2位 "); print_bits(flags); printf(" (添加 READ)\n");
    flags &= ~(1 << 2); printf("  清第2位 "); print_bits(flags); printf(" (移除 READ)\n");
    flags ^= (1 << 4); printf("  翻转4位 "); print_bits(flags); printf(" (切换 EXEC)\n");
    printf("----basic ... operators_bitwise_sample.....done ----\n");
}

/* ---- Compound assignment & precedence sample ---- */

static void operators_precedence_sample(void) {
    printf("----basic ... operators_precedence_sample.....start ----\n");
    int a = 2, b = 3, c = 4;
    printf("  a=%d, b=%d, c=%d\n", a, b, c);
    printf("  a + b * c        = %d (乘法先算: 2+12=14)\n", a + b * c);
    printf("  (a + b) * c      = %d\n", (a + b) * c);
    printf("  a < b == 0       = %d (等价 (a<b)==0)\n", (a < b) == 0);

    int x = 5;
    printf("  x=%d, ++x -> x=%d (前缀先加)\n", 5, ++x);
    x = 5;
    int post = x++;
    printf("  x=%d, x++ -> 返回=%d, 后 x=%d (后缀后加)\n", 5, post, x);

    int val = 10;
    printf("\n  复合赋值: val=%d\n", val);
    val += 5;  printf("  val += 5  -> %d\n", val);
    val -= 3;  printf("  val -= 3  -> %d\n", val);
    val *= 2;  printf("  val *= 2  -> %d\n", val);
    val /= 4;  printf("  val /= 4  -> %d\n", val);
    val %= 3;  printf("  val %%= 3 -> %d\n", val);
    printf("----basic ... operators_precedence_sample.....done ----\n");
}

/* ---- Ternary & sizeof sample ---- */

static void operators_misc_sample(void) {
    printf("----basic ... operators_misc_sample.....start ----\n");
    int x = 10, y = 20;
    int max = (x > y) ? x : y;
    printf("  三元运算符: max(%d, %d) = %d\n", x, y, max);
    printf("  sizeof: char=%zu, int=%zu, long=%zu, float=%zu, double=%zu\n",
           sizeof(char), sizeof(int), sizeof(long), sizeof(float), sizeof(double));
    printf("----basic ... operators_misc_sample.....done ----\n");
}

/* ---- Coordinator entry point ---- */

int main_operators_sample(void) {
    printf("=== 运算符与表达式（Operators & Expressions）演示 ===\n\n");
    operators_arithmetic_sample();
    printf("\n");
    operators_relational_sample();
    printf("\n");
    operators_logical_sample();
    printf("\n");
    operators_bitwise_sample();
    printf("\n");
    operators_precedence_sample();
    printf("\n");
    operators_misc_sample();
    printf("\n");
    return 0;
}
