#include <stdio.h>
#include "basic/preprocessor_sample.h"

/* ---- #define 常量替换样本 ---- */

void preprocessor_define_sample(void) {
    printf("---- preprocessor: #define 常量 ----\n");

#define BUFFER_SIZE 256
#define PI 3.14159265358979
#define GREETING "Hello, Preprocessor!"

    printf("  BUFFER_SIZE = %d\n", BUFFER_SIZE);
    printf("  PI          = %.10f\n", PI);
    printf("  GREETING    = %s\n", GREETING);

    /* 计算圆面积 */
    double radius = 5.0;
    double area = PI * radius * radius;
    printf("  半径 %.1f 的圆面积 = %.4f\n", radius, area);

#undef BUFFER_SIZE
#undef PI
#undef GREETING

    printf("---- preprocessor: #define 样本完毕 ----\n\n");
}

/* ---- 条件编译样本 ---- */

void preprocessor_ifdef_sample(void) {
    printf("---- preprocessor: #ifdef/#ifndef 样本 ----\n");

#ifdef __STDC__
    printf("  __STDC__ 已定义: 符合 C 标准。\n");
#endif

#ifndef CUSTOM_FLAG
    printf("  CUSTOM_FLAG 未定义: 使用默认配置。\n");
#define CUSTOM_FLAG 1
#endif

#if CUSTOM_FLAG == 1
    printf("  CUSTOM_FLAG = 1: 启用自定义功能。\n");
#endif

    printf("---- preprocessor: #ifdef 样本完毕 ----\n\n");
}

/* ---- 宏函数样本 ---- */

void preprocessor_macro_func_sample(void) {
    printf("---- preprocessor: 宏函数 ----\n");

#define SWAP(a, b) do { int t = (a); (a) = (b); (b) = t; } while (0)
#define SQUARE(x) ((x) * (x))
#define IS_EVEN(n) ((n) % 2 == 0)

    int m = 10, n = 20;
    printf("  交换前: m=%d, n=%d\n", m, n);
    SWAP(m, n);
    printf("  交换后: m=%d, n=%d\n", m, n);

    printf("  SQUARE(7) = %d\n", SQUARE(7));
    printf("  SQUARE(3+4) = %d (展开正确: ((3+4)*(3+4)))\n", SQUARE(3 + 4));

    for (int i = 1; i <= 6; i++) {
        printf("  %d 是%s偶数。\n", i, IS_EVEN(i) ? "" : "不");
    }

    printf("---- preprocessor: 宏函数样本完毕 ----\n");
}

/* ---- 字符串化与 token pasting 样本 ---- */

void preprocessor_advanced_sample(void) {
    printf("---- preprocessor: 高级宏 ----\n");

#define TO_STR(x) #x
#define PASTE(a, b) a##b

    printf("  TO_STR(hello world) = %s\n", TO_STR(hello world));

    int data1 = 100;
    int data2 = 200;

    printf("  PASTE(data, 1) = %d\n", PASTE(data, 1));
    printf("  PASTE(data, 2) = %d\n", PASTE(data, 2));

    printf("---- preprocessor: 高级宏样本完毕 ----\n");
}

void main_preprocessor_sample(void) {
    preprocessor_define_sample();
    preprocessor_ifdef_sample();
    preprocessor_macro_func_sample();
}
