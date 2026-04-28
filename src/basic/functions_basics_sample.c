#include <stdio.h>
#include "basic/functions_basics_sample.h"

/* ---- 函数声明（前向声明） ---- */

static int add(int a, int b);

/* ---- 函数定义 ---- */

static int add(int a, int b) {
    return a + b;
}

static int demo_return_int(int a, int b) {
    return (a > b) ? a : b;
}

static float demo_return_float(float n) {
    return n / 2.0f;
}

static char demo_return_char(int score) {
    if (score >= 90) return 'A';
    if (score >= 80) return 'B';
    if (score >= 60) return 'C';
    return 'F';
}

static void demo_return_void(void) {
    printf("  这是 void 函数 — 不返回任何值，只执行操作。\n");
}

static int demo_single_param(int n) {
    return (n < 0) ? -n : n;
}

static int demo_multi_param(int a, int b, int c) {
    int m = a;
    if (b > m) m = b;
    if (c > m) m = c;
    return m;
}

static void demo_call_by_value(int x) {
    x = x * 2;
    printf("    函数内部: x = %d (只改了副本)\n", x);
}

/* ---- demo: 声明 vs 定义 ---- */

static void demo_declaration_vs_definition(void) {
    printf("==== [sample] Functions Basics: Declaration vs Definition ====\n");

    int x = 3, y = 5;
    int result = add(x, y);
    printf("  add(%d, %d) = %d\n", x, y, result);
    printf("  声明在前，定义在后，编译器先看到签名，调用才不出错。\n");
}

/* ---- demo: 返回类型 ---- */

static void demo_return_types(void) {
    printf("\n==== [sample] Functions Basics: Return Types ====\n");

    printf("  int return:   max(5, 8)     = %d\n", demo_return_int(5, 8));
    printf("  float return: half(9.0)     = %.1f\n", demo_return_float(9.0f));
    printf("  char return:  grade(85)     = '%c'\n", demo_return_char(85));
    printf("  void return:  (see next)\n");
    demo_return_void();
}

/* ---- demo: 参数 ---- */

static void demo_parameters(void) {
    printf("\n==== [sample] Functions Basics: Parameters ====\n");

    printf("  单参数: abs(-42)   = %d\n", demo_single_param(-42));
    printf("多参数: max(10,20,30) = %d\n", demo_multi_param(10, 20, 30));
    printf("多参数: add(7, 8)    = %d\n", add(7, 8));
}

/* ---- demo: 值传递 ---- */

static void demo_call_by_value_wrapper(void) {
    printf("\n==== [sample] Functions Basics: Call by Value ====\n");

    int num = 21;
    printf("  调用前: num = %d\n", num);
    demo_call_by_value(num);
    printf("  调用后: num = %d (值传递，原变量不变)\n", num);
}

/* ---- 公共入口 ---- */

int main_functions_basics_sample(void) {
    demo_declaration_vs_definition();
    demo_return_types();
    demo_parameters();
    demo_call_by_value_wrapper();

    printf("\nfunctions_basics sample done.\n");
    return 0;
}
