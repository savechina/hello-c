#include <stdio.h>
#include "basic/functions_sample.h"

/* ---- Basic function: add two numbers ---- */

static int add(int a, int b) {
    return a + b;
}

void functions_basic_sample(void) {
    printf("----basic ... functions_basic_sample.....start ----\n");

    int x = 3, y = 5;
    int result = add(x, y);
    printf("  add(%d, %d) = %d\n", x, y, result);

    printf("----basic ... functions_basic_sample.....done ----\n");
}

/* ---- Forward declaration demo ---- */

static double divide_safe(double x, double y);

void functions_forward_sample(void) {
    printf("----basic ... functions_forward_sample.....start ----\n");

    double numerator = 10.0;
    double denominator = 3.0;
    double quotient = divide_safe(numerator, denominator);
    printf("  divide_safe(%.1f, %.1f) = %.4f\n",
           numerator, denominator, quotient);

    printf("----basic ... functions_forward_sample.....done ----\n");
}

static double divide_safe(double x, double y) {
    if (y != 0.0) {
        return x / y;
    }
    printf("  警告: 除数为 0，返回 0.0\n");
    return 0.0;
}

/* ---- void function: print separator line ---- */

static void print_line(int length) {
    for (int i = 0; i < length; i++) {
        putchar('-');
    }
    putchar('\n');
}

void functions_void_sample(void) {
    printf("----basic ... functions_void_sample.....start ----\n");

    printf("  长度为 10 的分隔线:  ");
    print_line(10);

    printf("  长度为 30 的分隔线:  ");
    print_line(30);

    printf("----basic ... functions_void_sample.....done ----\n");
}

/* ---- Pass by value demo ---- */

static void try_to_modify(int val) {
    val = 999;
    printf("  函数内部: val = %d\n", val);
}

void functions_value_pass_sample(void) {
    printf("----basic ... functions_value_pass_sample.....start ----\n");

    int num = 42;
    printf("  调用前: num = %d\n", num);
    try_to_modify(num);
    printf("  调用后: num = %d (值传递不修改原值)\n", num);

    printf("----basic ... functions_value_pass_sample.....done ----\n");
}

/* ---- Recursive function: factorial ---- */

static long long fact(int n) {
    if (n <= 1) {
        return 1;
    }
    return (long long)n * fact(n - 1);
}

void functions_recursive_sample(void) {
    printf("----basic ... functions_recursive_sample.....start ----\n");

    for (int i = 0; i <= 10; i++) {
        printf("  %2d! = %lld\n", i, fact(i));
    }

    printf("----basic ... functions_recursive_sample.....done ----\n");
}

/* ---- static function visibility demo ---- */

static int is_even(int n) {
    return (n % 2) == 0;
}

void functions_static_sample(void) {
    printf("----basic ... functions_static_sample.....start ----\n");

    printf("  static 函数演示: is_even 只在当前文件可见\n");
    for (int i = 1; i <= 5; i++) {
        printf("  %d 是%s偶数\n",
               i, is_even(i) ? "" : "不");
    }

    printf("----basic ... functions_static_sample.....done ----\n");
}

int main_functions_sample(void) {
    functions_basic_sample();
    functions_forward_sample();
    functions_void_sample();
    return 0;
}
