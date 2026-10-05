#include <stdio.h>
#include <string.h>
#include "basic/type_generic_sample.h"

/*
 * _Generic 类型泛型选择 (C11 §6.5.1.1)
 * 控制表达式只参与类型匹配、永不求值; 只有被选中的分支会被求值。
 * C11 没有 typeof, 每个分支必须写显式类型; C23 引入 typeof 后可大幅简化。
 */

/* ❌ 错例宏: 纯文本替换, 实参 a 在条件与真分支里各出现一次 */
#define NAIVE_MAX(a, b) ((a) > (b) ? (a) : (b))

int tg_max_int(int a, int b)
{
    return a > b ? a : b;
}

int tg_min_int(int a, int b)
{
    return a < b ? a : b;
}

static double tg_max_double(double a, double b)
{
    return a > b ? a : b;
}

static double tg_min_double(double a, double b)
{
    return a < b ? a : b;
}

/* 字符串按 strcmp 语义比较: 用关系运算符直接比较不相关对象的指针是 UB */
static const char *tg_max_cstr(const char *a, const char *b)
{
    return strcmp(a, b) >= 0 ? a : b;
}

static const char *tg_min_cstr(const char *a, const char *b)
{
    return strcmp(a, b) <= 0 ? a : b;
}

static void tg_show_int(int v)
{
    printf("  int    → %d\n", v);
}

static void tg_show_double(double v)
{
    printf("  double → %.2f\n", v);
}

static void tg_show_str(const char *s)
{
    printf("  str    → \"%s\" (len=%zu)\n", s, strlen(s));
}

/* 分支里的函数名必须先声明: 未选中的分支虽不求值, 但必须是合法表达式。
 * 故意不写 default — 未知类型宁可编译失败也不静默选错函数。
 * 字符串字面量经 array-to-pointer 衰变匹配 char* 分支 (§6.5.1.1p3)。 */
#define TG_MAX(a, b) _Generic((a),                      \
    int: tg_max_int,                                    \
    double: tg_max_double,                              \
    char *: tg_max_cstr,                                \
    const char *: tg_max_cstr                           \
    )((a), (b))

#define TG_MIN(a, b) _Generic((a),                      \
    int: tg_min_int,                                    \
    double: tg_min_double,                              \
    char *: tg_min_cstr,                                \
    const char *: tg_min_cstr                           \
    )((a), (b))

#define TG_SHOW(x) _Generic((x),                        \
    int: tg_show_int,                                   \
    double: tg_show_double,                             \
    char *: tg_show_str,                                \
    const char *: tg_show_str                           \
    )((x))

static void demo_naive_macro_bug(void)
{
    int i = 3;
    int got = NAIVE_MAX(i++, 0);

    printf("=== ❌ 错例: 朴素 max 宏的双重求值 ===\n");
    printf("  int i = 3; int got = NAIVE_MAX(i++, 0);\n");
    printf("  期望: got = 3, i = 4   (i++ 只该执行一次)\n");
    printf("  实际: got = %d, i = %d ← i++ 被执行了两次\n", got, i);
    printf("  原因: 宏是文本替换, (a) 在比较和真分支各展开一次\n");
    printf("  这是「良定义的错误结果」— 比 UB 更隐蔽, 编译器和 UBSan 都不报警\n\n");
}

static void demo_generic_max_min(void)
{
    int ia = 7;
    int ib = 3;
    double da = 2.5;
    double db = 9.75;
    const char *sa = "banana";
    const char *sb = "apple";
    int i = 3;
    int r = TG_MAX(i++, 0);

    printf("=== ✅ _Generic 安全 max/min (每种类型分派到真函数) ===\n");
    printf("  TG_MAX(%d, %d)            = %d\n", ia, ib, TG_MAX(ia, ib));
    printf("  TG_MIN(%d, %d)            = %d\n", ia, ib, TG_MIN(ia, ib));
    printf("  TG_MAX(%.2f, %.2f)      = %.2f\n", da, db, TG_MAX(da, db));
    printf("  TG_MIN(%.2f, %.2f)      = %.2f\n", da, db, TG_MIN(da, db));
    printf("  TG_MAX(\"%s\", \"%s\")   = %s\n", sa, sb, TG_MAX(sa, sb));
    printf("  TG_MIN(\"%s\", \"%s\")   = %s\n", sa, sb, TG_MIN(sa, sb));
    printf("  TG_MAX(i++, 0) = %d, i = %d ← 控制表达式不求值, i++ 只执行一次\n\n", r, i);
}

static void demo_show_dispatch(void)
{
    const char *name = "hello-c";

    printf("=== TG_SHOW: 类型安全的格式分派 ===\n");
    TG_SHOW(42);
    TG_SHOW(3.14);
    TG_SHOW("literal");
    TG_SHOW(name);
    printf("\n");
}

static void demo_type_safety(void)
{
    printf("=== 编译期类型安全 (错例只能在编译器里现形) ===\n");
    printf("  ❌ int *p; TG_MAX(p, p);\n");
    printf("     → 约束违规: int* 不匹配任何分支, 且没有 default\n");
    printf("  ❌ TG_MAX(\"apple\", 1);\n");
    printf("     → 编译错误: int 无法传给所选分支的 const char * 参数\n");
    printf("  ✅ 不写 default 是特性: 新类型接入前先在编译期拦下\n\n");
}

static void demo_typeof_note(void)
{
    printf("=== C11 没有 typeof / C23 typeof 的改进 ===\n");
    printf("  C11: 分支必须枚举显式类型 (int, double, char*, const char*...)\n");
    printf("       漏掉一种 → 编译错误, 类型系统不许静默转换\n");
    printf("  C23: typeof(a) 可在 default 分支按实参类型构造结果, 新类型零成本接入;\n");
    printf("       typeof 还支持「先存局部副本再比较」的宏, 从根上消除双重求值\n\n");
}

int main_type_generic_sample(void)
{
    printf("--- 类型泛型 (_Generic) ---\n");
    printf("C11 §6.5.1.1: 控制表达式不求值, 编译期按类型选择分支.\n\n");

    demo_naive_macro_bug();
    demo_generic_max_min();
    demo_show_dispatch();
    demo_type_safety();
    demo_typeof_note();

    printf("_Generic 演示完毕.\n");
    return 0;
}
