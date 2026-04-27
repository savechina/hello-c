#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "basic/void_generic_sample.h"

/*
 * void* 泛型编程演示 (Generic Programming with void*)
 *
 * 本节演示 C 语言中 void*（无类型指针）的常见用法：
 *  - void* 作为通用指针
 *  - 安全地将 void* 转回具体类型
 *  - 泛型函数参数
 *  - 基于宏的泛型容器
 *  - C 泛型 vs C++ 模板 / Rust 泛型的对比
 */

/* ================================================================
 * Section 1: void* 基础 —— 可以指向任何类型
 * ================================================================ */

static void demo_void_pointer_basic(void)
{
    int32_t  i = 42;
    double   d = 3.14159;
    char     c = 'A';

    /* void* 可以指向任意类型 */
    void *vp1 = &i;
    void *vp2 = &d;
    void *vp3 = &c;

    printf("=== void* 基础 ===\n");
    printf("  vp1 (int*)   = %p, 指向 int32_t: %" PRId32 "\n",
           vp1, *(int32_t *)vp1);
    printf("  vp2 (double*) = %p, 指向 double: %.5f\n",
           vp2, *(double *)vp2);
    printf("  vp3 (char*)  = %p, 指向 char: '%c'\n",
           vp3, *(char *)vp3);
    printf("\n");
}

/* ================================================================
 * Section 2: 将 void* 转回具体类型 —— 必须显式转换
 * ================================================================ */

static void demo_void_cast_back(void)
{
    int32_t value = 12345;
    void *vp = &value;

    printf("=== void* 转回具体类型 ===\n");

    /* ✅ 正确: 显式转换为 int32_t* */
    int32_t recovered = *(int32_t *)vp;
    printf("  正确: *(int32_t*)vp = %" PRId32 "\n", recovered);

    /* ❌ 错误示范: 直接解引用 void* (编译错误) */
    /*
     *  int32_t bad = *vp;
     *  error: invalid use of undefined type 'void'
     */
    printf("  ❌ `int32_t bad = *vp;` —— 编译错误!\n");
    printf("     void* 没有大小信息, 不能直接解引用\n");
    printf("\n");
}

/* ================================================================
 * Section 3: void* 在函数参数中 —— 泛型 swap
 * ================================================================ */

static void generic_swap(void *a, void *b, size_t size)
{
    unsigned char temp[256];
    if (size > sizeof(temp)) {
        fprintf(stderr, "generic_swap: size %zu exceeds buffer\n", size);
        return;
    }
    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);
}

static void demo_generic_swap(void)
{
    int32_t x = 10, y = 20;

    printf("=== 泛型 swap (void*) ===\n");
    printf("  before swap: x = %" PRId32 ", y = %" PRId32 "\n", x, y);

    generic_swap(&x, &y, sizeof(int32_t));

    printf("  after swap:  x = %" PRId32 ", y = %" PRId32 "\n", x, y);

    /* 交换 double */
    double a = 1.5, b = 2.7;
    printf("  before swap: a = %.1f, b = %.1f\n", a, b);

    generic_swap(&a, &b, sizeof(double));

    printf("  after swap:  a = %.1f, b = %.1f\n", a, b);
    printf("\n");
}

/* ================================================================
 * Section 4: void* 泛型打印函数 —— 类型标签分发
 * ================================================================ */

typedef enum {
    TYPE_INT32 = 1,
    TYPE_DOUBLE = 2,
    TYPE_CHAR = 3,
    TYPE_STRING = 4,
} TypeTag;

static void generic_print(TypeTag tag, void *data)
{
    switch (tag) {
        case TYPE_INT32:
            printf("int32: %" PRId32, *(int32_t *)data);
            break;
        case TYPE_DOUBLE:
            printf("double: %.3f", *(double *)data);
            break;
        case TYPE_CHAR:
            printf("char: '%c'", *(char *)data);
            break;
        case TYPE_STRING:
            printf("string: \"%s\"", (const char *)data);
            break;
        default:
            printf("unknown type (tag=%d)", tag);
            break;
    }
}

static void demo_generic_print(void)
{
    int32_t i = 42;
    double  d = 3.14;
    char    c = 'Z';
    const char *s = "hello void*";

    printf("=== 泛型打印 (void* + type tag) ===\n");
    printf("  "); generic_print(TYPE_INT32, &i);   printf("\n");
    printf("  "); generic_print(TYPE_DOUBLE, &d);  printf("\n");
    printf("  "); generic_print(TYPE_CHAR, &c);    printf("\n");
    printf("  "); generic_print(TYPE_STRING, (void *)s); printf("\n");
    printf("\n");
}

/* ================================================================
 * Section 5: 比较函数 —— qsort 风格回调
 * ================================================================ */

static int cmp_int32(const void *a, const void *b)
{
    int32_t va = *(const int32_t *)a;
    int32_t vb = *(const int32_t *)b;
    return (va > vb) - (va < vb);
}

static int cmp_double(const void *a, const void *b)
{
    double va = *(const double *)a;
    double vb = *(const double *)b;
    return (va > vb) - (va < vb);
}

static void demo_qsort_callback(void)
{
    int32_t ints[] = {33, 10, 75, 42, 5};
    double  dbls[] = {3.3, 1.0, 7.5, 4.2, 0.5};
    int32_t n = 5;

    printf("=== qsort 风格回调 (void* 比较函数) ===\n");

    printf("  before (int):  ");
    for (int32_t k = 0; k < n; k++) printf("%" PRId32 " ", ints[k]);
    printf("\n");

    qsort(ints, (size_t)n, sizeof(int32_t), cmp_int32);

    printf("  after  (int):  ");
    for (int32_t k = 0; k < n; k++) printf("%" PRId32 " ", ints[k]);
    printf("\n");

    qsort(dbls, (size_t)n, sizeof(double), cmp_double);

    printf("  after  (double):");
    for (int32_t k = 0; k < n; k++) printf(" %.1f", dbls[k]);
    printf("\n\n");
}

/* ================================================================
 * Section 6: void** 泛型 "数组" —— 异质容器
 * ================================================================ */

static void demo_voidptr_array(void)
{
    int32_t  vals_i[] = {10, 20, 30};
    double   vals_d[] = {1.1, 2.2, 3.3};
    char     vals_c[] = {'X', 'Y', 'Z'};

    /* void** 可以指向不同类型的数组 */
    void *container[3];
    container[0] = vals_i;
    container[1] = vals_d;
    container[2] = vals_c;

    printf("=== void** 泛型数组 (异质容器) ===\n");

    for (int32_t j = 0; j < 3; j++) {
        printf("  container[%" PRId32 "]", j);
        if (j == 0) {
            printf(" → int32_t[3]: ");
            for (int32_t k = 0; k < 3; k++) {
                printf("%" PRId32 " ", ((int32_t *)container[j])[k]);
            }
        } else if (j == 1) {
            printf(" → double[3]: ");
            for (int32_t k = 0; k < 3; k++) {
                printf("%.1f ", ((double *)container[j])[k]);
            }
        } else {
            printf(" → char[3]: ");
            for (int32_t k = 0; k < 3; k++) {
                printf("'%c' ", ((char *)container[j])[k]);
            }
        }
        printf("\n");
    }
    printf("\n");

    printf("  ⚠️  注意: void* 数组丢失了类型信息!\n");
    printf("  你必须在别处记录每个元素的类型, 才能安全转回.\n");
    printf("  这就是 C 泛型的根本局限 —— 类型擦除.\n\n");
}

/* ================================================================
 * Section 7: 泛型 vs Python 动态类型 (认知对照)
 * ================================================================ */

static void demo_vs_python_dynamic_typing(void)
{
    printf("=== void* vs Python 动态类型 (认知对照) ===\n");
    printf("\n");
    printf("  Python:\n");
    printf("    x = 42        # x 是 int\n");
    printf("    x = \"hello\"   # x 变成 str —— 无需声明\n");
    printf("    # Python 在运行时自动跟踪类型\n");
    printf("\n");
    printf("  C with void*:\n");
    printf("    void *vp = &i;   // vp 指向 int\n");
    printf("    vp = &d;         // vp 指向 double\n");
    printf("    // C 不会自动跟踪! 你必须自己记住类型\n");
    printf("    double val = *(double*)vp;  // ★ 必须显式转换\n");
    printf("\n");
    printf("  关键区别:\n");
    printf("   - Python: 类型由解释器管理, 运行时安全\n");
    printf("   - C void*: 类型靠程序员管理, 编译期不检查\n");
    printf("   - void* 是「类型擦除」(type erasure): 类型信息被抹掉\n");
    printf("   - 转换错误 = 未定义行为, 没有运行时保护\n");
    printf("\n");
}

/* ================================================================
 * Section 8: 基于宏的泛型容器 —— MIN/MAX 宏
 * ================================================================ */

#define GENERIC_MIN(a, b)                             \
    ((a) < (b) ? (a) : (b))

#define GENERIC_SWAP宏(a, b)                          \
    do {                                              \
        __typeof__(a) _tmp = (a);                     \
        (a) = (b);                                    \
        (b) = _tmp;                                   \
    } while (0)

static void demo_macro_generic(void)
{
    printf("=== 基于宏的泛型 ===\n");
    printf("  GENERIC_MIN(3, 7) = %d\n", GENERIC_MIN(3, 7));
    printf("  GENERIC_MIN(3.14, 2.72) = %.2f\n",
           GENERIC_MIN(3.14, 2.72));
    printf("  GENERIC_MIN('a', 'z') = '%c'\n",
           GENERIC_MIN('a', 'z'));

    /* __typeof__ 泛型 swap (GCC/Clang 扩展) */
    int32_t mi = 10, ni = 20;
    GENERIC_SWAP宏(mi, ni);
    printf("  GENERIC_SWAP(10, 20) → mi=%" PRId32 ", ni=%" PRId32 "\n", mi, ni);

    double md = 1.5, nd = 9.9;
    GENERIC_SWAP宏(md, nd);
    printf("  GENERIC_SWAP(1.5, 9.9) → md=%.1f, nd=%.1f\n", md, nd);

    printf("\n  ⚠️  __typeof__ 是 GCC/Clang 扩展, 非标准 C\n");
    printf("  标准 C 没有真正的泛型函数, 宏是最接近的替代品\n\n");
}

/* ================================================================
 * Section 9: C11 _Generic —— 类型选择表达式
 * ================================================================ */

#define TYPE_NAME(x) _Generic((x),                \
    int32_t:  "int32_t",                          \
    double:   "double",                            \
    char:     "char",                              \
    char *:   "char*",                             \
    default:  "unknown"                            \
)

static void demo_c11_generic(void)
{
    int32_t ival = 42;
    double  dval = 3.14;
    char    cval = 'X';
    char   *sval = "hello";

    printf("=== C11 _Generic 类型选择 ===\n");
    printf("  TYPE_NAME(%" PRId32 ") → %s\n",  ival, TYPE_NAME(ival));
    printf("  TYPE_NAME(%.2f)    → %s\n",     dval, TYPE_NAME(dval));
    printf("  TYPE_NAME('%c')    → %s\n",     cval, TYPE_NAME(cval));
    printf("  TYPE_NAME(\"...\")    → %s\n",   TYPE_NAME(sval));
    printf("  TYPE_NAME(100UL)    → %s\n",     TYPE_NAME(100UL));
    printf("\n");

    printf("  _Generic 在编译期根据表达式的类型选择结果.\n");
    printf("  类似 Python 的 isinstance(), 但是在编译期完成.\n\n");
}

/* ================================================================
 * Section 10: 实用: 泛型 max 回调函数
 * ================================================================ */

typedef int (*CompareFn)(const void *a, const void *b);

static void *generic_max(void *arr, int32_t count, size_t elem_size, CompareFn cmp)
{
    void *max_elem = arr;
    for (int32_t j = 1; j < count; j++) {
        void *current = (unsigned char *)arr + (size_t)j * elem_size;
        if (cmp(current, max_elem) > 0) {
            max_elem = current;
        }
    }
    return max_elem;
}

static void demo_generic_max(void)
{
    int32_t  nums[] = {3, 7, 2, 9, 4};
    double   scores[] = {85.5, 92.3, 78.1, 95.0, 88.7};

    printf("=== 泛型 max (回调函数) ===\n");

    int32_t *max_i = (int32_t *)generic_max(nums, 5, sizeof(int32_t), cmp_int32);
    printf("  int32_t 最大值: %" PRId32 "\n", *max_i);

    double *max_d = (double *)generic_max(scores, 5, sizeof(double), cmp_double);
    printf("  double 最大值: %.1f\n", *max_d);
    printf("\n");
}

/* ================================================================
 * Section 11: void* 的常见错误 —— 误用类型转换
 * ================================================================ */

static void demo_void_cast_errors(void)
{
    int32_t value = 0x01020304;
    void *vp = &value;

    printf("=== void* 常见错误: 类型不匹配转换 ===\n");
    printf("  原始: int32_t value = 0x%08" PRIx32 "\n", (uint32_t)value);

    /* ✅ 正确: 转回正确的类型 */
    int32_t correct = *(int32_t *)vp;
    printf("  ✅ *(int32_t*)vp = 0x%08" PRIx32 " (正确)\n", (uint32_t)correct);

    /* ❌ 错误: 转换成错误的类型 (编译不报错, 运行时数据错乱) */
    char wrong = *(char *)vp;
    printf("  ❌ *(char*)vp    = 0x%02" PRIx8 " (只读了 1 字节!)\n",
           (uint8_t)wrong);

    printf("\n  ⚠️  void* 的危险: 转换成错误类型不会触发编译警告!\n");
    printf("  程序员必须确保转换类型与原始类型一致.\n\n");
}

/* ================================================================
 * Section 12: void* 与 const 正确性
 * ================================================================ */

static void demo_void_const(void)
{
    const int32_t pi = 314;
    const void *cvp = &pi;
    (void)cvp;

    printf("=== void* 与 const 正确性 ===\n");
    printf("  const void *cvp = &pi;  // ✅ 常量地址 → const void*\n");
    printf("  *(int32_t*)cvp = 100;   // ❌ 编译错误: 丢弃了 const\n");
    /*
     * 上述赋值会导致: error: read-only variable is not assignable
     * 修复: 不修改通过 const void* 看到的数据
     */

    int32_t mutable = 99;
    void *mv = &mutable;
    *(int32_t *)mv = 200;  /* ✅ mutable 不是 const, 可以修改 */
    printf("  void *mv = &mutable; *(int32_t*)mv = 200; ✅\n");
    printf("  mutable = %" PRId32 "\n", mutable);
    printf("\n");
}

/* ================================================================
 * Section 13: void* 在回调 API 中的用户数据模式
 * ================================================================ */

typedef void (*VisitFn)(void *item, void *user_data);

static void visit_items(void *arr, int32_t count, size_t size,
                        VisitFn visit, void *user_data)
{
    for (int32_t j = 0; j < count; j++) {
        void *item = (unsigned char *)arr + (size_t)j * size;
        visit(item, user_data);
    }
}

static void print_and_accumulate(void *item, void *user_data)
{
    int32_t value = *(int32_t *)item;
    int32_t *sum = (int32_t *)user_data;
    *sum += value;
    printf("    item = %" PRId32 ", running_sum = %" PRId32 "\n",
           value, *sum);
}

static void demo_visitor_pattern(void)
{
    int32_t data[] = {10, 20, 30, 40};
    int32_t sum = 0;

    printf("=== 回调 API 中的 void* user_data ===\n");
    printf("  遍历数组并累加:\n");
    visit_items(data, 4, sizeof(int32_t),
                print_and_accumulate, &sum);
    printf("  最终累加和: %" PRId32 "\n", sum);
    printf("\n");
}

/* ================================================================
 * Section 14: 泛型 vs C++ 模板 / Rust 泛型 对比
 * ================================================================ */

static void demo_vs_cpp_templates(void)
{
    printf("=== C void* vs C++ 模板 / Rust 泛型 ===\n");
    printf("\n");
    printf("  C++ 模板:\n");
    printf("    template<typename T>\n");
    printf("    T max(T a, T b) { return a > b ? a : b; }\n");
    printf("    // 编译期生成类型安全的 int 版本和 double 版本\n");
    printf("\n");
    printf("  Rust 泛型:\n");
    printf("    fn max<T: PartialOrd>(a: T, b: T) -> T { ... }\n");
    printf("    // 编译期单态化, trait 约束保证类型安全\n");
    printf("\n");
    printf("  C (void* + 宏):\n");
    printf("    #define MAX(a, b) ((a) > (b) ? (a) : (b))\n");
    printf("    // 文本替换, 无类型检查; 或 void* + 手动转换\n");
    printf("\n");
    printf("  对比表:\n");
    printf("  ┌───────────┬──────────┬───────────┬──────────┐\n");
    printf("  │ 特性        │ C           │ C++         │ Rust      │\n");
    printf("  ├───────────┼──────────┼───────────┼──────────┤\n");
    printf("  │ 类型安全     │ ❌ 手动      │ ✅ 编译期    │ ✅ 编译期   │\n");
    printf("  │ 运行时开销   │ 无          │ 无          │ 无         │\n");
    printf("  │ 编译期开销   │ 小          │ 大          │ 中等       │\n");
    printf("  │ 错误信息     │ 差(运行时)  │ 极好        │ 极好       │\n");
    printf("  └───────────┴──────────┴───────────┴──────────┘\n");
    printf("\n");
}

/* ================================================================
 * Section 15: 小结 + 实用建议
 * ================================================================ */

static void demo_summary(void)
{
    printf("=== void* 泛型编程小结 ===\n");
    printf("  1. void* 可以指向任何类型, 但不能直接解引用\n");
    printf("  2. 必须显式转换为具体类型后再解引用\n");
    printf("  3. 泛型函数用 void* 参数 + size 实现\n");
    printf("  4. 类型信息丢失 → 程序员必须自行跟踪类型\n");
    printf("  5. _Generic (C11) 提供编译期类型选择\n");
    printf("  6. 宏可以模拟简单泛型, 但缺少类型安全\n");
    printf("  7. void** 可实现异质容器, 但代价是高复杂度\n");
    printf("  8. const void* 保证不会修改所指向的数据\n");
    printf("\n");
}

/* ================================================================
 * Coordinator entry
 * ================================================================ */

int main_void_generic_sample(void)
{
    printf("========================================\n");
    printf("  void* 泛型编程 (Generic Programming)\n");
    printf("========================================\n\n");

    demo_void_pointer_basic();
    demo_void_cast_back();
    demo_generic_swap();
    demo_generic_print();
    demo_qsort_callback();
    demo_voidptr_array();
    demo_vs_python_dynamic_typing();
    demo_macro_generic();
    demo_c11_generic();
    demo_generic_max();
    demo_void_cast_errors();
    demo_void_const();
    demo_visitor_pattern();
    demo_vs_cpp_templates();
    demo_summary();

    printf("void* 泛型编程演示完毕。\n");
    return 0;
}
