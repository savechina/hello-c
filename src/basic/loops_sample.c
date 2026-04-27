#include <stdio.h>
#include <limits.h>
#include "basic/loops_sample.h"

/* ---- for 循环样本 ---- */

void loops_for_sample(void) {
    printf("---- loops: for 循环样本 ----\n");

    /* 遍历数组 */
    int scores[] = {85, 92, 78, 96, 88};
    int n = (int)(sizeof(scores) / sizeof(scores[0]));

    printf("  成绩数组: ");
    for (int i = 0; i < n; i++) {
        printf("%d", scores[i]);
        if (i < n - 1) {
            printf(", ");
        }
    }
    putchar('\n');

    /* 计算平均值 */
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += scores[i];
    }
    printf("  总分: %d, 平均分: %.1f\n", sum, (double)sum / n);

    printf("---- loops: for 样本完毕 ----\n\n");
}

/* ---- while 循环样本 ---- */

void loops_while_sample(void) {
    printf("---- loops: while 循环样本 ----\n");

    /* 欧几里得算法求最大公约数 */
    int a = 48, b = 18;
    int orig_a = a, orig_b = b;

    printf("  求 GCD(%d, %d):\n", orig_a, orig_b);
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    printf("  GCD(%d, %d) = %d\n", orig_a, orig_b, a);

    printf("---- loops: while 样本完毕 ----\n\n");
}

/* ---- do-while 循环样本 ---- */

void loops_do_while_sample(void) {
    printf("---- loops: do-while 循环样本 ----\n");

    /* 数字反转 */
    int num = 12345;
    int reversed = 0;
    int temp = num;

    printf("  原始数字: %d\n", num);
    do {
        int digit = temp % 10;
        reversed = reversed * 10 + digit;
        temp /= 10;
    } while (temp > 0);

    printf("  反转结果: %d\n", reversed);
    printf("---- loops: do-while 样本完毕 ----\n\n");
}

/* ---- break/continue 样本 ---- */

void loops_control_sample(void) {
    printf("---- loops: break/continue 样本 ----\n");

    /* 找素数 (2..20) */
    printf("  2..20 之间的素数: ");
    for (int i = 2; i <= 20; i++) {
        int is_prime = 1;
        for (int j = 2; j * j <= i; j++) {
            if (i % j == 0) {
                is_prime = 0;
                break;
            }
        }
        if (is_prime) {
            printf("%d ", i);
        }
    }
    putchar('\n');

    /* 跳过某些值 */
    printf("  1..10 中跳过 3 的倍数: ");
    for (int i = 1; i <= 10; i++) {
        if (i % 3 == 0) {
            continue;
        }
        printf("%d ", i);
    }
    putchar('\n');

    printf("---- loops: break/continue 样本完毕 ----\n");
}

void main_loops_sample(void) {
    loops_for_sample();
    loops_while_sample();
    loops_do_while_sample();
}
