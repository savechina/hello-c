#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include "basic/pointers_and_arrays_sample.h"

/*
 * 指针与数组 — 数组-指针对等、多维数组
 * arr[i] ≡ *(arr + i) 是 C 语言的底层真理。
 */

static void demo_array_pointer_equivalence(void)
{
    int32_t a[4] = {7, 14, 21, 28};

    printf("=== 数组名 ≡ 首元素地址 ===\n");
    printf("  a[0] = %" PRId32 ",  *(a+0) = %" PRId32 "\n", a[0], *(a + 0));
    printf("  a[1] = %" PRId32 ",  *(a+1) = %" PRId32 "\n", a[1], *(a + 1));
    printf("  a[2] = %" PRId32 ",  *(a+2) = %" PRId32 "\n", a[2], *(a + 2));
    printf("  a[3] = %" PRId32 ",  *(a+3) = %" PRId32 "\n", a[3], *(a + 3));
    printf("\n");
    printf("  &a[1] = %p   a+1 = %p  → 地址完全相同\n\n",
           (void *)&a[1], (void *)(a + 1));
}

static void demo_pointer_walk_array(void)
{
    int32_t nums[5] = {10, 20, 30, 40, 50};

    printf("=== 用指针遍历数组 ===\n");

    int32_t *p = nums;            /* p → nums[0] */
    int32_t *end = nums + 5;      /* end → nums[5] (合法的一格越界) */

    printf("  索引法:  ");
    for (int32_t i = 0; i < 5; i++) printf("%" PRId32 " ", nums[i]);
    printf("\n");

    printf("  指针法:  ");
    for (int32_t *q = p; q < end; q++) printf("%" PRId32 " ", *q);
    printf("\n\n");
}

static void demo_2d_array_layout(void)
{
    int32_t matrix[2][3] = {
        {1, 2, 3},
        {4, 5, 6}
    };

    printf("=== 二维数组——连续内存，行优先 ===\n");

    printf("  matrix[row][col]   *( *(matrix+row) + col )\n");
    printf("  ────────────────────┬────────────────────────\n");
    for (int32_t r = 0; r < 2; r++) {
        for (int32_t c = 0; c < 3; c++) {
            printf("  [%ld][%ld] = %-11" PRIi32 "  = %-11" PRIi32 "\n",
                   (long)r, (long)c,
                   matrix[r][c],
                   *(*(matrix + r) + c));
        }
    }

    printf("\n  内存布局 (行优先):\n");
    printf("  [1][2][3][4][5][6]\n");
    printf("   ↑ 一个 2×3 的二维数组在内存中就是一行 6 个 int32_t\n\n");
}

static void demo_pointer_syntax_confusion(void)
{
    printf("=== 指针语法易混淆点 ===\n\n");

    int32_t vals[] = {42, 99, 88};

    printf("  arr[2]    = %" PRId32 "\n", vals[2]);
    printf("  *(arr+2)  = %" PRId32 "\n", *(vals + 2));
    printf("  *(2+arr)  = %" PRId32 "  ← 可交换! 甚至 arr[2] == 2[arr]!\n",
           *(2 + vals));

    int32_t *p = vals;
    printf("\n  int32_t *p = arr;\n");
    printf("  p[1]       = %" PRId32 "  (指针也支持 [] 语法糖)\n", p[1]);
    printf("  *(p+1)     = %" PRId32 "\n\n", *(p + 1));
}

int main_pointers_and_arrays_sample(void)
{
    printf("========================================\n");
    printf("  指针与数组\n");
    printf("  「arr[i] 就是 *(arr+i) 的语法糖。」\n");
    printf("========================================\n\n");

    demo_array_pointer_equivalence();
    demo_pointer_walk_array();
    demo_2d_array_layout();
    demo_pointer_syntax_confusion();

    printf("指针与数组演示完毕。\n");
    return 0;
}
