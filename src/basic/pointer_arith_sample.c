#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "basic/pointer_arith_sample.h"

/* ============================================================
 * 指针算术运算 (Pointer Arithmetic) 演示
 * ============================================================ */

static void demo_out_of_bounds(void) {
    printf("-- [错误演示] 指针越界 --\n");

    int32_t arr[4] = {10, 20, 30, 40};
    int32_t *p = arr;

    printf("  arr[0]=%d, *(p+0)=%d\n", arr[0], *p);

    p = p + 4;  /* 指向 arr 末尾之后一个位置 —— 已越界 */
    /* ⚠️  绝不要解引用越界指针！这里只打印地址说明问题 */
    printf("  越界后指针地址: %p (不应解引用)\n", (void *)p);

    p = arr;    /* 回到合法位置 */
    p = p + 3;  /* 回到 arr[3]，合法 */
    printf("  回到合法位置 *(p+3)=%d\n", *p);
    printf("  -- [错误演示结束] --\n");
}

static void demo_ptr_increment(void) {
    printf("-- 指针自增自减 (ptr++, ptr--) --\n");

    int32_t nums[5] = {1, 2, 3, 4, 5};
    int32_t *p = nums;

    printf("  初始: *p = %d\n", *p);
    p++;
    printf("  p++:  *p = %d\n", *p);
    p++;
    printf("  p++:  *p = %d\n", *p);
    p--;
    printf("  p--:  *p = %d\n", *p);

    printf("  sizeof(int32_t) = %zu 字节，每次 +1 地址移动 %zu 字节\n",
           sizeof(int32_t), sizeof(int32_t));
}

static void demo_ptr_add_sub(void) {
    printf("-- 指针加减整数 (ptr+n, ptr-n) --\n");

    int32_t data[6] = {100, 200, 300, 400, 500, 600};
    int32_t *p = data;

    printf("  基址 data[0] = %d\n", *p);
    printf("  p+2      = %d\n", *(p + 2));
    printf("  p+5      = %d\n", *(p + 5));

    int32_t *q = p + 5;
    printf("  q = p+5,  *q = %d\n", *q);
    printf("  q-3      = %d\n", *(q - 3));

    printf("  sizeof(int32_t) = %zu 字节\n", sizeof(int32_t));
}

static void demo_array_equivalence(void) {
    printf("-- 数组与指针的等价性 --\n");

    int32_t a[4] = {7, 14, 21, 28};

    printf("  a[0] = %d, *(a+0) = %d\n", a[0], *(a + 0));
    printf("  a[2] = %d, *(a+2) = %d\n", a[2], *(a + 2));
    printf("  a[3] = %d, *(a+3) = %d\n", a[3], *(a + 3));

    printf("  &a[1] = %p,  a+1  = %p  (地址相同)\n",
           (void *)&a[1], (void *)(a + 1));

    int32_t *p = a;
    printf("  用指针遍历: ");
    for (int32_t i = 0; i < 4; i++) {
        printf("%d ", *(p + i));
    }
    printf("\n");
}

static void demo_sizeof_ptr_vs_arr(void) {
    printf("-- sizeof: 数组 vs 指针 --\n");

    int32_t arr[8] = {0};

    printf("  sizeof(arr)     = %zu  (整个数组: 8 × %zu = %zu)\n",
           sizeof(arr), sizeof(int32_t), (size_t)(8 * sizeof(int32_t)));
    printf("  sizeof(arr[0])  = %zu\n", sizeof(arr[0]));
    printf("  元素个数        = %zu\n", sizeof(arr) / sizeof(arr[0]));

    int32_t *ptr = arr;
    printf("  sizeof(ptr)     = %zu  (指针本身，不是数组)\n", sizeof(ptr));
    printf("  sizeof(*ptr)    = %zu  (解引用: 一个 int32_t)\n", sizeof(*ptr));

    int32_t *q2 = ptr;
    /* 故意展示错误写法 —— 编译器会用 %zu 打印 1，而非 8 */
    size_t wrong_count = sizeof(*q2) / sizeof(*q2);
    (void)wrong_count;
    printf("  ⚠️  对指针用 sizeof 陷阱 → 结果不是 8！\n");
}

static void demo_ptr_subtraction(void) {
    printf("-- 指针相减：计算距离 --\n");

    int32_t nums[6] = {10, 20, 30, 40, 50, 60};
    int32_t *start = &nums[0];
    int32_t *end   = &nums[5];

    ptrdiff_t dist = end - start;
    printf("  &nums[0] 到 &nums[5] 的距离 = %td 个元素\n", dist);
    printf("  换算字节: %td 字节\n", (ptrdiff_t)(dist * (ptrdiff_t)sizeof(int32_t)));

    int32_t *mid = &nums[2];
    printf("  mid - start = %td\n", mid - start);
    printf("  end   - mid = %td\n", end   - mid);
}

static void demo_ptr_comparison(void) {
    printf("-- 指针比较 (> < == !=) --\n");

    int32_t vals[5] = {50, 40, 30, 20, 10};
    int32_t *p1 = &vals[0];
    int32_t *p2 = &vals[4];

    printf("  vals[0] 地址: %p\n", (void *)p1);
    printf("  vals[4] 地址: %p\n", (void *)p2);

    if (p1 < p2) {
        printf("  p1 < p2: true (p1 在 p2 前面)\n");
    }
    if (p2 > p1) {
        printf("  p2 > p1: true (p2 在 p1 后面)\n");
    }
    if (p1 != p2) {
        printf("  p1 != p2: true\n");
    }

    int32_t *p3 = p1;
    if (p3 == p1) {
        printf("  p3 == p1: true (指向同一位置)\n");
    }

    printf("  遍历方向验证: ");
    for (int32_t *q = p1; q <= p2; q++) {
        printf("%d ", *q);
    }
    printf("\n");
}

/* -- 入口 -- */

int main_pointer_arith_sample(void) {
    printf("========================================\n");
    printf("  指针算术 (Pointer Arithmetic)\n");
    printf("========================================\n");

    demo_out_of_bounds();
    printf("\n");
    demo_ptr_increment();
    printf("\n");
    demo_ptr_add_sub();
    printf("\n");
    demo_array_equivalence();
    printf("\n");
    demo_sizeof_ptr_vs_arr();
    printf("\n");
    demo_ptr_subtraction();
    printf("\n");
    demo_ptr_comparison();

    printf("\n========================================\n");
    printf("  指针算术演示完毕\n");
    printf("========================================\n");
    return 0;
}
