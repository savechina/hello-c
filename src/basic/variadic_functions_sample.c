#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "basic/variadic_functions_sample.h"

/* ============================================================
 * Section 1: variadic_sum_sample — 用 count 参数控制
 * ============================================================ */

static int32_t variadic_sum(int32_t count, ...)
{
    va_list args;
    va_start(args, count);

    int32_t total = 0;
    for (int32_t i = 0; i < count; i++) {
        total += va_arg(args, int);
    }

    va_end(args);
    return total;
}

static void variadic_sum_sample(void)
{
    printf("==== [1] variadic_sum — count 参数控制 ====\n\n");

    int32_t s1 = variadic_sum(3, 10, 20, 30);
    printf("  variadic_sum(3, 10, 20, 30) = %d\n", s1);

    int32_t s2 = variadic_sum(5, 1, 2, 3, 4, 5);
    printf("  variadic_sum(5, 1, 2, 3, 4, 5) = %d\n", s2);

    int32_t s3 = variadic_sum(1, 42);
    printf("  variadic_sum(1, 42) = %d\n", s3);

    printf("\n");
    printf("  💡 关键: count 参数告诉函数有多少个参数，va_arg 按次数提取\n\n");
}

/* ============================================================
 * Section 2: variadic_print_ints_sample — sentinel value 模式
 * ============================================================ */

static void print_ints_sentinel(int first, ...)
{
    if (first == -1) {
        printf("(no arguments)\n");
        return;
    }

    printf("%d", first);

    va_list args;
    va_start(args, first);

    int val;
    while ((val = va_arg(args, int)) != -1) {
        printf(", %d", val);
    }

    va_end(args);
    printf("\n");
}

static void variadic_print_ints_sample(void)
{
    printf("==== [2] variadic_print_ints — sentinel value 模式 ====\n\n");

    printf("  print_ints_sentinel(1, 2, 3, -1) → ");
    print_ints_sentinel(1, 2, 3, -1);

    printf("  print_ints_sentinel(42, -1) → ");
    print_ints_sentinel(42, -1);

    printf("  print_ints_sentinel(-1) → ");
    print_ints_sentinel(-1);

    printf("\n");
    printf("  💡 sentinel value: 用 -1 标记结束，无需 count 参数\n");
    printf("     ⚠️ 缺陷: 如果数据本身包含 -1，就会截断\n\n");
}

/* ============================================================
 * Section 3: variadic_log_sample — 模拟 printf 内部机制
 * ============================================================ */

static void simple_log(const char *level, const char *fmt, ...)
{
    fprintf(stdout, "[%s] ", level);

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);
    va_end(args);

    fprintf(stdout, "\n");
}

static void variadic_log_sample(void)
{
    printf("==== [3] variadic_log — 模拟 printf 内部机制 ====\n\n");

    simple_log("INFO", "Server started on port %d", 8080);
    simple_log("WARN", "Disk usage at %.1f%%", 85.5);
    simple_log("ERROR", "Failed to open file: %s", "/tmp/config.ini");

    printf("\n");
    printf("  💡 原理: simple_log 接收 ... → va_start → vfprintf → va_end\n");
    printf("     printf 内部也是这样工作的！\n\n");
}

/* ============================================================
 * Section 4: variadic_safety_sample — 常见错误演示
 * ============================================================ */

static void variadic_safety_sample(void)
{
    printf("==== [4] variadic safety — 常见错误演示 ====\n\n");

    /* ── ❌ 错误 1: 类型不匹配 (UB) ── */
    printf("  ❌ 错误 1: va_arg 类型不匹配\n");
    printf("     variadic_sum(2, 3.14, 2.72);  // 传入 double，但当作 int 读取\n");
    printf("     → va_arg(args, int) 读取 4 字节，但 double 是 8 字节 → 垃圾值!\n");
    printf("     → 这是 undefined behavior，可能崩溃\n\n");

    printf("     ✅ 修复: va_arg 的类型必须与传入的实际类型一致\n");
    printf("        variadic_sum(2, (int)3, (int)2);  // 传入 int\n\n");

    /* ── ❌ 错误 2: count > 实际参数数 ── */
    printf("  ❌ 错误 2: count > 实际参数\n");
    printf("     variadic_sum(5, 1, 2);  // 说 5 个，只传了 2 个\n");
    printf("     → va_arg 读取不存在的参数 → 栈上垃圾内存!\n\n");

    printf("     ✅ 修复: 永远确保 count <= 实际传入的参数数量\n\n");

    /* ── ❌ 错误 3: 忘记 va_end ── */
    printf("  ❌ 错误 3: 忘记 va_end\n");
    printf("     void broken(const char *fmt, ...) {\n");
    printf("         va_list args;\n");
    printf("         va_start(args, fmt);\n");
    printf("         vprintf(fmt, args);\n");
    printf("         // 没有 va_end(args); → 未定义行为!\n");
    printf("     }\n\n");

    printf("     ✅ 修复: 每次 va_start 必须配对 va_end\n\n");

    /* ── ✅ 安全模式总结 ── */
    printf("  ✅ 安全模式总结:\n");
    printf("     1. 始终使用 count 或 sentinel 明确参数边界\n");
    printf("     2. va_arg 类型必须与实际传入一致\n");
    printf("     3. 每次 va_start 必须配对 va_end\n");
    printf("     4. 如需多次遍历参数，使用 va_copy\n\n");
}

/* ============================================================
 * Section 5: va_copy 正确用法演示
 * ============================================================ */

static void demo_va_copy(int32_t count, ...)
{
    va_list args1, args2;
    va_start(args1, count);
    va_copy(args2, args1);

    int32_t sum1 = 0, sum2 = 0;
    for (int32_t i = 0; i < count; i++) {
        sum1 += va_arg(args1, int);
    }
    for (int32_t i = 0; i < count; i++) {
        sum2 += va_arg(args2, int);
    }

    printf("  第一次遍历 = %d, 第二次遍历 = %d\n", sum1, sum2);

    va_end(args2);
    va_end(args1);
}

/* ============================================================
 * 公共入口
 * ============================================================ */

int main_variadic_functions_sample(void)
{
    printf("==== 可变参数函数 (Variadic Functions) ====\n\n");

    variadic_sum_sample();
    variadic_print_ints_sample();
    variadic_log_sample();
    variadic_safety_sample();

    printf("  ── va_copy 示例 (两次遍历同一组参数) ──\n");
    demo_va_copy(4, 10, 20, 30, 40);
    printf("\n");

    printf("variadic functions sample done.\n");
    return 0;
}
