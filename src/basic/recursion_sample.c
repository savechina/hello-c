#include <stdio.h>
#include <stdint.h>
#include "basic/recursion_sample.h"

static long long recursion_factorial(int n)
{
    if (n <= 1) {
        return 1;
    }
    return (long long)n * recursion_factorial(n - 1);
}

static void recursion_factorial_sample(void)
{
    printf("==== [sample] 递归阶乘 ====\n");

    for (int i = 0; i <= 10; i++) {
        printf("  %2d! = %lld\n", i, recursion_factorial(i));
    }
}

static long long recursion_fibonacci(int n)
{
    if (n <= 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }
    return recursion_fibonacci(n - 1) + recursion_fibonacci(n - 2);
}

static void recursion_fibonacci_sample(void)
{
    printf("\n==== [sample] 递归 Fibonacci ====\n");

    for (int i = 0; i <= 10; i++) {
        printf("  fib(%d) = %lld\n", i, recursion_fibonacci(i));
    }
    printf("  提示: fib(40) 需要约 20 亿次调用，不要用递归！\n");
}

static void recursion_no_base_case(int n, int step)
{
    if (step >= 5) {
        printf("  └─ step %d: 已到达安全上限，停止演示\n", step);
        return;
    }
    printf("  │ step %d: n = %d（若没有基线条件，会无限执行…）\n", step, n);
    recursion_no_base_case(n - 1, step + 1);
}

static void recursion_no_base_case_sample(void)
{
    printf("\n==== [sample] 缺少基线条件的后果 ====\n");
    printf("  （实际运行中，没有基线条件会导致栈溢出 → 段错误）\n");
    printf("  这里用 step 上限做安全演示：\n");

    recursion_no_base_case(10, 0);

    printf("  如果去掉 step 检查，程序会崩溃：\n");
    printf("    %s\n", "Segmentation fault: 11");
    printf("  修复方式: 始终先写基线条件!\n");
}

static void recursion_stack_depth(int n, int depth)
{
    printf("  ↓ 压入: depth = %d\n", depth);

    if (n <= 0) {
        printf("  ★ 基线条件触发! depth = %d\n", depth);
        return;
    }

    recursion_stack_depth(n - 1, depth + 1);

    printf("  ↑ 弹出: depth = %d\n", depth);
}

static void recursion_stack_depth_sample(void)
{
    printf("\n==== [sample] 调用栈与栈展开 ====\n");
    printf("  演示 count_down(3): 压栈 → 基线 → 弹栈\n");

    recursion_stack_depth(3, 1);
}

int main_recursion_sample(void)
{
    recursion_factorial_sample();
    recursion_fibonacci_sample();
    recursion_no_base_case_sample();
    recursion_stack_depth_sample();

    printf("\nrecursion sample done.\n");
    return 0;
}
