# void* 泛型编程 (Generic Programming with void*)

> **前置回顾**: 你已经在上一章掌握了 `void*` 指针的基本用法——它可以接收任何类型的地址，但读取前必须转回具体类型。本章直接进入泛型设计模式。

## 开篇故事

一家快递公司需要运送各种货物：信件、包裹、冷藏食品。他们不关心货物种类，只关心两个信息：**有多大**（需要多大的箱子）、**送到哪**（地址）。货物类型由收件人自己判断。

`void*` 泛型函数就是这样工作的：函数不关心数据类型，只管按字节搬运。调用者需要告诉函数「数据有多大」（`sizeof`），以及「如何解读数据」（回调或类型标签）。

## 本章适合谁

- 已掌握 void* 指针的基本用法
- 想理解 `qsort` 等标准库泛型 API 的设计原理
- 好奇 C 语言如何实现"泛型编程"
- 用过 C++ Templates 或 Rust 泛型，想了解 C 的等价方案

## 你会学到什么

1. `void* + size_t` 模式——泛型函数的基石
2. Type Tag 模式——手动记录被擦除的类型
3. `qsort` 回调模式——标准库泛型设计
4. `memcpy` 与字节级操作
5. C11 `_Generic` 类型选择表达式
6. 宏泛型——编译期类型选择
7. C void* vs C++ Templates vs Rust 泛型

## 前置要求

- 已完成 [void* 指针](./void_pointers.md)
- 理解函数参数和回调函数概念

## 第一个例子：泛型 swap

利用 `void*`, 可以写出**适用于任何类型**的交换函数：

```c
#include <string.h>
#include <stdio.h>
#include <stdint.h>

void generic_swap(void *a, void *b, size_t size) {
    unsigned char temp[256];
    if (size > sizeof(temp)) return;
    memcpy(temp, a, size);   /* 按字节拷贝 */
    memcpy(a,   b, size);
    memcpy(b, temp, size);
}

int main(void) {
    int32_t xi = 10, yi = 20;
    generic_swap(&xi, &yi, sizeof(int32_t));
    /* xi = 20, yi = 10 */

    double xd = 1.5, yd = 9.9;
    generic_swap(&xd, &yd, sizeof(double));
    /* xd = 9.9, yd = 1.5 */

    printf("int:    %d, %d\n", xi, yi);
    printf("double: %.1f, %.1f\n", xd, yd);
    return 0;
}
```

关键点：
- 函数不关心类型，只管按字节数拷贝
- 调用者必须传 `sizeof(类型)`
- 这就是 `qsort` 和 `bsearch` 的设计模式

## 原理解析

### 1. `void* + size_t` 模式——泛型的基石

`void*` 抹掉了类型信息，但带来了灵活性。核心公式：

```
泛型函数 = void* (数据地址) + size_t (数据大小) + [可选回调]
```

```c
/* 泛型打印函数: void* + size + type tag */
typedef enum { TYPE_INT, TYPE_DOUBLE, TYPE_CHAR } TypeTag;

void generic_print(void *data, TypeTag tag) {
    switch (tag) {
    case TYPE_INT:    printf("%d",   *(int32_t*)data);  break;
    case TYPE_DOUBLE: printf("%.3f", *(double*)data);   break;
    case TYPE_CHAR:   printf("'%c'", *(char*)data);     break;
    }
}
```

Type Tag 是 C 语言找回被擦除类型信息的手段——类似于 Python 的 `isinstance()`，但需要手动维护。

### 2. `qsort` 设计模式

标准库 `qsort` 是 C 泛型设计最经典的案例：

```c
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
```

```c
int cmp_int32(const void *a, const void *b) {
    int32_t va = *(const int32_t *)a;
    int32_t vb = *(const int32_t *)b;
    return (va > vb) - (va < vb);  /* 安全三态比较 */
}

int32_t nums[] = {33, 10, 75, 42, 5};
qsort(nums, 5, sizeof(int32_t), cmp_int32);
/* nums 有序: {5, 10, 33, 42, 75} */
```

`qsort` 的巧妙之处：
- `void* base`——抹掉类型，可以排序任何数组
- `size`——告诉函数每个元素有多大
- `compar`——回调函数负责"如何比较"，排序算法不关心类型

### 3. `memcpy` 与字节级操作

`memcpy` 是 C 标准库最通用的函数之一：

```c
void *memcpy(void *dest, const void *src, size_t n);
```

```c
int32_t src[3] = {1, 2, 3};
int32_t dst[3];
memcpy(dst, src, sizeof(src));  /* 整个数组拷贝 */
```

**`memcpy` vs `memmove` 的区别**：

| 函数     | 源和目标重叠时            | 适用场景       |
| -------- | ----------------------- | -------------- |
| `memcpy` | 未定义行为 (UB)          | 不重叠的快速拷贝 |
| `memmove`| 安全处理重叠             | 可能重叠的拷贝   |

```c
/* 重叠场景：必须用 memmove */
char buf[] = "hello world";
memmove(buf, buf + 6, 6);  /* buf 现在是 "world" */
/* 如果用 memcpy，结果可能是错的 */
```

### 4. C11 `_Generic` 类型选择表达式

虽然 C 没有泛型模板，C11 引入了 `_Generic`，允许在**编译期**根据表达式类型选择不同的分支：

```c
#define TYPE_NAME(x) _Generic((x),      \
    int32_t:  "int32_t",                 \
    double:   "double",                  \
    char:     "char",                    \
    default:  "unknown"                  \
)

int32_t i = 42;
printf("%s\n", TYPE_NAME(i));    /* "int32_t" */
printf("%s\n", TYPE_NAME(3.14)); /* "double" */
```

`_Generic` 是**编译期类型选择**，不是运行时判断——比 switch 的 Type Tag 模式更安全（编译器会检查类型是否存在），但它只能用于宏/表达式层面。

### 5. 宏泛型 (Generic Macros)

结合 `_Generic` 和宏，可以实现类似 C++ `std::max` 的效果：

```c
#define GENERIC_MAX(a, b) _Generic((a),            \
    int32_t:    max_int32,                          \
    double:     max_double,                         \
    char:       max_char                            \
)((a), (b))

/* 需要为每种类型定义对应的函数 */
static inline int32_t max_int32(int32_t a, int32_t b) { return a > b ? a : b; }
static inline double max_double(double a, double b) { return a > b ? a : b; }
static inline char   max_char(char a, char b)   { return a > b ? a : b; }

int32_t x = GENERIC_MAX(10, 20);     /* 调用 max_int32 */
double d = GENERIC_MAX(1.5, 9.9);    /* 调用 max_double */
```

更简洁的直接内联写法：

```c
#define MAX(a, b) _Generic((a),            \
    int32_t:    ((a) > (b) ? (a) : (b)),   \
    double:     ((a) > (b) ? (a) : (b))    \
)

int32_t x = MAX(10, 20);     /* 宏展开为比较表达式 */
double d = MAX(1.5, 9.9);
```

### 6. Python 动态类型 vs C `void*` 泛型

| 特征         | Python          | C `void*` 泛型         |
| ------------ | --------------- | -------------------- |
| 存储任意类型  | ✅ 原生          | ✅ 通过 `void*`       |
| 运行时类型信息 | ✅ 对象自带      | ❌ 需要手动跟踪 (Type Tag) |
| 类型安全检查  | ✅ 运行时        | ❌ 无（程序员负责）    |
| 类型错误后果  | `TypeError` 异常 | 未定义行为 (UB)       |
| 泛型函数      | 自动             | void* + size + 回调   |

```python
# Python — 自动管理
def swap(a, b):
    return b, a  # 任何类型都能用

x, y = 10, "hello"
x, y = swap(x, y)  # OK
```

```c
// C — 手动管理
int32_t xi = 10, yi = 20;
generic_swap(&xi, &yi, sizeof(int32_t));  // 必须传入 sizeof
```

**核心概念**：`void*` 是 C 的「类型擦除」(type erasure)——抹掉类型信息换取灵活性。Python 帮你做了这一切，C 把它交给了你。

### 7. vs C++ 模板 / Rust 泛型

```
┌───────────┬──────────┬───────────┬──────────┐
│ 特性        │ C void*  │ C++ 模板  │ Rust 泛型 │
├───────────┼──────────┼───────────┼──────────┤
│ 类型安全     │ ❌ 手动    │ ✅ 编译期  │ ✅ 编译期  │
│ 运行时开销   │ 无        │ 无        │ 无         │
│ 编译期开销   │ 小        │ 大        │ 中等       │
│ 错误提示     │ 差(UB)   │ 极好      │ 极好       │
│ 代码膨胀     │ 无        │ 有        │ 无         │
└───────────┴──────────┴───────────┴──────────┘
```

C 选择 `void*` 是因为它零运行时开销——代价是安全责任全在程序员手中。C++ 用模板在编译期保证类型安全，但会产生代码膨胀。Rust 的 monomorphization 介于两者之间。

## 常见错误

### ❌ 错误 1：转回错误类型（运行时 UB）

```c
double d = 3.14;
void *vp = &d;
int n = *(int32_t *)vp;  /* ❌ 编译通过，数据全错！*/
```

**最危险的 void* 错误**——编译器不警告，但读出的数据完全错误。必须确保类型转换与原类型一致。

### ❌ 错误 2：忘记类型信息

```c
void *container[2];
container[0] = &int_val;
container[1] = &double_val;

/* 后来忘了 container[1] 是 double */
int wrong = *(int32_t *)container[1];  /* 数据错乱! */
```

```c
/* ✅ 用 struct 包装类型信息 */
typedef struct { TypeTag tag; void *data; } TypedValue;
```

## 动手练习

### 🟡 中级：泛型求和

编写 `double generic_sum(void *arr, int count, size_t elem_size, TypeTag tag)`，根据 tag 计算 `int32_t` 或 `double` 数组的和。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

typedef enum { SUM_INT, SUM_DOUBLE } TypeTag;

double generic_sum(void *arr, int count, size_t elem_size, TypeTag tag)
{
    double sum = 0.0;
    for (int j = 0; j < count; j++) {
        void *item = (unsigned char *)arr + (size_t)j * elem_size;
        if (tag == SUM_INT) {
            sum += (double)*(int32_t *)item;
        } else {
            sum += *(double *)item;
        }
    }
    return sum;
}

int main(void) {
    int32_t ints[] = {1, 2, 3, 4, 5};
    double sum_i = generic_sum(ints, 5, sizeof(int32_t), SUM_INT);
    printf("int sum: %.0f\n", sum_i);  /* 15 */

    double doubles[] = {1.1, 2.2, 3.3};
    double sum_d = generic_sum(doubles, 3, sizeof(double), SUM_DOUBLE);
    printf("double sum: %.1f\n", sum_d);  /* 6.6 */
    return 0;
}
```

</details>

### 🔴 挑战：类型安全包装器

设计 `TypedValue` 结构体, 包含 `TypeTag type` 和 `void *data`, 实现 `print_typed_value(TypedValue tv)` 根据 tag 安全打印。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

typedef enum { TV_INT32, TV_DOUBLE, TV_CHAR } TypeTag;

typedef struct { TypeTag type; void *data; } TypedValue;

void print_typed_value(TypedValue tv) {
    switch (tv.type) {
    case TV_INT32:  printf("int32:  %" PRId32 "\n", *(int32_t *)tv.data); break;
    case TV_DOUBLE: printf("double: %.3f\n",    *(double *)tv.data);    break;
    case TV_CHAR:   printf("char:   '%c'\n",    *(char *)tv.data);      break;
    }
}

int main(void) {
    int32_t i = 42;
    double d = 3.14;
    char c = 'X';

    print_typed_value((TypedValue){TV_INT32, &i});
    print_typed_value((TypedValue){TV_DOUBLE, &d});
    print_typed_value((TypedValue){TV_CHAR, &c});
    return 0;
}
```

</details>

## 知识扩展：C11 `_Generic` 类型选择

`_Generic` 是 C11 引入的编译期特性，它根据**表达式的类型**选择不同的分支：

```c
#define ABS(x) _Generic((x),                  \
    int:      abs_int,                         \
    float:    fabsf,                           \
    double:   fabs                             \
)(x)

printf("%d\n", ABS(-5));     /* 调用 abs_int */
printf("%.1f\n", ABS(-3.14)); /* 调用 fabs */
```

与 Type Tag 模式相比：
- ✅ 更安全——编译器检查类型是否存在
- ❌ 更限制——只能用于编译期已知的类型
- ✅ 零运行时开销——编译期就选定了分支

## 小结

- **泛型函数** = `void*` (地址) + `size_t` (大小) + [可选回调/Type Tag]
- **qsort 模式**——`void*` 抹掉类型，回调函数负责解读
- **Type Tag**——手动记录被擦除的类型，是 C 的 "运行时反射"
- **`memcpy` vs `memmove`**——重叠场景必须用 `memmove`
- **C11 `_Generic`**——编译期类型选择，比 Type Tag 更安全但更受限
- **C void* vs C++ Templates vs Rust**——零运行时开销 vs 编译期安全 vs 两全

## 术语表

| 英文 | 中文 | 解释 |
|------|------|------|
| Generic programming | 泛型编程 | 不依赖具体类型的编程 |
| Type erasure | 类型擦除 | 抹掉类型信息换取灵活性 |
| Type tag | 类型标签 | 手动记录被擦除的类型 |
| `_Generic` | C11 泛型选择表达式 | 编译期根据类型选择表达式 |
| Callback function | 回调函数 | 作为参数传递的函数指针 |
| Memory copy | 内存拷贝 | 按字节拷贝内存 |

## 延伸阅读

- [C11 `_Generic` selection](https://en.cppreference.com/w/c/language/generic) — 泛型选择表达式
- [cppreference - qsort](https://en.cppreference.com/w/c/algorithm/qsort) — qsort 完整参考
- [cppreference - memcpy / memmove](https://en.cppreference.com/w/c/string/byte/memcpy) — 内存拷贝函数

## 继续学习

- [上一章](./callbacks.md)：回调函数与多态
- [下一章](./variadic_functions.md)：可变参数函数

---

> 本章代码位于 [`src/basic/void_generic_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/void_generic_sample.c)。
