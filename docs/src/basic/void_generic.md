# void* 泛型编程 (Generic Programming with void*)

## 开篇故事

想象一个万能充电适配器。它能插进美标、欧标、英标的插座——因为它的插头是通用的。但当你把数据线连到设备上时，你必须知道那台设备需要多少伏特。适配器不替你操心，你得自己确认。

`void*` 就是 C 的万能适配器。它可以指向任何类型的数据（`int`、`double`、`struct`……），但在读取之前你必须**把它转回正确的类型**。适配器能接任何插头，但接错了电压设备就烧了。`void*` 能存任何地址，但转错了类型数据就乱了。

灵活性很高，责任也在你。

## 本章适合谁

- 学过指针基础, 能理解 `int*`、`char*` 的概念
- 对"泛型"、"多态"这些词好奇的 C 初学者
- 用过 Python/JavaScript 等动态类型语言, 想了解 C 的"类型擦除"
- 正在读标准库函数（如 `qsort`、`bsearch`）源码的人

## 你会学到什么

1. `void*` 是什么 —— 无类型指针的核心语义
2. 如何安全地将 `void*` 转回具体类型
3. `void*` 在函数参数中的泛型用法
4. 基于宏的泛型容器技巧
5. C11 `_Generic` 类型选择表达式
6. `void*` 的局限性与 C++/Rust 泛型对比

## 前置要求

- 已掌握 [指针基础](./pointers.md) 和 [指针运算](./pointer-arith.md)
- 理解函数指针和回调函数的概念
- 熟悉 `stdint.h` 中的固定宽度类型

## 第一个例子

下面演示 `void*` 最基础的用法 —— 它可以直接指向任何类型的变量：

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t  i = 42;
    double   d = 3.14;
    char     c = 'A';

    void *vp1 = &i;  /* void* 可以指向 int */
    void *vp2 = &d;  /* void* 可以指向 double */
    void *vp3 = &c;  /* void* 可以指向 char */

    /* 但要读取值, 必须先转回正确类型 */
    printf("i = %" PRId32 "\n", *(int32_t *)vp1);
    printf("d = %.2f\n",   *(double *)vp2);
    printf("c = '%c'\n",   *(char *)vp3);

    return 0;
}
```

编译并运行：

```bash
gcc -Wall -Wextra -std=c17 -o demo demo.c
./demo
```

输出：

```
i = 42
d = 3.14
c = 'A'
```

这段代码揭示了两件事：
- `void*` **可以接收**任何类型的地址（无需强制转换）
- `void*` **不能直接解引用** —— 必须先转回具体类型

## 原理解析

### 1. `void*` 的本质

`void` 在 C 语言中表示"无类型" 或"类型未知"。`void*` 因此被称为**通用指针** (universal pointer)。

```c
void *vp;  /* vp 可以指向任何类型的对象 */
```

**为什么需要 `void*`？** 因为 C 语言没有泛型函数 —— 同一个函数要处理 `int`、`double`、`struct` 等多种类型时, 只能依赖 `void*` 抹掉类型信息。

**内存中的 `void*`**：和 `int*`、`char*` 完全一样 —— 都是存储一个地址。区别在于编译器如何看待它：

| 指针类型 | 解引用大小 | 指针加法步长 | 能否直接解引用 |
|----------|-----------|-------------|---------------|
| `int32_t*` | 4 字节 | +4 字节 | ✅ |
| `char*` | 1 字节 | +1 字节 | ✅ |
| `void*` | 无法确定 | 无法确定 | ❌ 编译错误 |

### 2. 将 `void*` 转回具体类型

`void*` 本身没有大小信息, 编译器不知道它指向 1 字节还是 8 字节。你必须显式地**告诉**编译器：

```c
int32_t value = 42;
void *vp = &value;

/* ✅ 正确: 转换为 int32_t* */
int32_t recovered = *(int32_t *)vp;
printf("%" PRId32 "\n", recovered);  /* 42 */

/* ❌ 错误: 直接解引用 void* */
/* int32_t bad = *vp; */
/* error: invalid use of undefined type 'void' */
```

**类比**：`void*` 就像是一个拆了标签的快递盒。你需要先"贴上标签"（类型转换）才能正确打开它。

### 3. `void*` 在函数参数中 —— 泛型 swap

利用 `void*`, 我们可以写出一个**适用于任何类型**的交换函数：

```c
#include <string.h>

void generic_swap(void *a, void *b, size_t size)
{
    unsigned char temp[256];
    if (size > sizeof(temp)) return;
    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);
}

int main(void) {
    int32_t x = 10, y = 20;
    generic_swap(&x, &y, sizeof(int32_t));
    /* x = 20, y = 10 */

    double a = 1.5, b = 9.9;
    generic_swap(&a, &b, sizeof(double));
    /* a = 9.9, b = 1.5 */
}
```

关键点：
- 函数不关心具体类型, 只按字节数复制
- 调用者必须传入 `sizeof(类型)` 告诉函数数据有多大
- 这就是标准库 `qsort` 和 `bsearch` 的设计思路

### 4. 泛型打印函数 —— 类型标签

没有类型信息的 `void*` 需要额外的"标签"来记录类型：

```c
typedef enum { TYPE_INT32, TYPE_DOUBLE, TYPE_CHAR } TypeTag;

void generic_print(TypeTag tag, void *data) {
    switch (tag) {
        case TYPE_INT32:  printf("%" PRId32, *(int32_t*)data);  break;
        case TYPE_DOUBLE: printf("%.3f",    *(double*)data);     break;
        case TYPE_CHAR:   printf("'%c'",    *(char*)data);      break;
    }
}
```

这类似于 Python 的 `isinstance()` 检查 —— 但 Python 的运行时自动携带类型, C 需要程序员手动传递标签。

### 5. Python 动态类型 vs C `void*` 对比

| 特征 | Python | C `void*` |
|------|--------|-----------|
| 存储任意类型 | ✅ 原生支持 | ✅ 通过 `void*` |
| 运行时类型信息 | ✅ 对象自带 | ❌ 必须手动跟踪 |
| 类型安全检查 | ✅ 运行时 | ❌ 无 (程序员负责) |
| 类型错误后果 | `TypeError` 异常 | 未定义行为 (崩溃/数据错乱) |

```python
# Python — 自动跟踪类型
x = 42        # x 是 int
x = "hello"   # x 变成 str
```

```c
// C — 手动管理类型
void *vp = &i;     // vp 指向 int32_t
vp = &d;           // vp 指向 double (类型信息已丢失!)
double val = *(double*)vp;  // ★ 必须记得当前指向 double
```

**我的理解**：`void*` 就是 C 语言的"类型擦除" (type erasure)——把类型信息抹掉, 换取灵活性, 代价是安全责任转移给程序员。

## 常见错误

### ❌ 错误 1：直接解引用 `void*`（编译错误）

```c
void *vp = &some_int;
int x = *vp;  /* ❌ error: invalid use of undefined type 'void' */
```

✅ **修正**：

```c
int x = *(int32_t *)vp;  /* ✅ 显式转换 */
```

### ❌ 错误 2：转回错误的类型（运行时 UB）

```c
double d = 3.14;
void *vp = &d;
int x = *(int32_t *)vp;  /* ❌ 错误类型! 数据完全不对 */
```

✅ **修正**：转回与原始类型一致的类型。

```c
double recovered = *(double *)vp;  /* ✅ */
```

**这是最危险的错误** —— 编译器**不会警告**你类型不匹配, 但读出的数据完全错误。

### ❌ 错误 3：忘记类型标签

```c
void* arr[2];
arr[0] = &int_value;
arr[1] = &double_value;

/* 后来你忘了 arr[1] 是 double —— 转成 int 就读出错误数据 */
int wrong = *(int32_t *)arr[1];  /* 崩溃或数据错乱 */
```

✅ **修正**：使用结构体包装类型信息，或确保类型信息不丢失。

## 动手练习

### 🟢 入门：`void*` 赋值与转换

声明一个 `int32_t` 变量和一个 `double` 变量, 用 `void*` 分别指向它们, 然后正确转换回原类型并打印。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t i = 100;
    double  d = 2.718;
    void *vp;

    vp = &i;
    printf("int: %" PRId32 "\n", *(int32_t *)vp);

    vp = &d;
    printf("double: %.3f\n", *(double *)vp);

    return 0;
}
```

</details>

### 🟡 中级：泛型求和

写一个函数 `double generic_sum(void *arr, int count, size_t elem_size, TypeTag tag)`, 根据 tag 计算 `int32_t` 或 `double` 数组的和。

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
```

</details>

### 🔴 挑战：类型安全包装器

设计一个 `TypedValue` 结构体, 包含 `TypeTag type` 和 `void *data`, 实现一个 `print_typed_value(TypedValue tv)` 函数, 根据 tag 安全打印。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

typedef enum {
    TV_INT32, TV_DOUBLE, TV_CHAR
} TypeTag;

typedef struct {
    TypeTag type;
    void *data;
} TypedValue;

void print_typed_value(TypedValue tv) {
    switch (tv.type) {
        case TV_INT32:
            printf("int32: %" PRId32 "\n", *(int32_t *)tv.data);
            break;
        case TV_DOUBLE:
            printf("double: %.3f\n", *(double *)tv.data);
            break;
        case TV_CHAR:
            printf("char: '%c'\n", *(char *)tv.data);
            break;
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

## 故障排查 (FAQ)

**Q：`void*` 能进行指针算术吗？**

A：标准 C 不允许。`vp + 1` 没有定义, 因为编译器不知道 `void` 的大小。GCC/Clang 允许作为扩展（按 1 字节计算）, 但可移植代码应该先转成 `unsigned char*`：

```c
void *vp = some_data;
unsigned char *cp = (unsigned char *)vp;
cp++;  /* ✅ 前进 1 字节 */
```

**Q：为什么不用 `uintptr_t` 代替 `void*`？**

A：`uintptr_t` 是整数类型, 不能直接解引用。`void*` 的主要价值是"可间接寻址 + 类型擦除", 你需要在某个时刻把它转回具体类型来读写数据。

**Q：`void*` 和 `NULL` 能比较吗？**

A：可以。`void*` 支持所有指针比较运算：

```c
void *vp = NULL;
if (vp == NULL) { /* ... */ }  /* ✅ */
```

## 知识扩展 (选学)

### C11 `_Generic` 类型选择表达式

虽然 C 没有模板, C11 引入了 `_Generic`, 允许在编译期根据类型选择表达式：

```c
#define TYPE_NAME(x) _Generic((x),      \
    int32_t:  "int32_t",                 \
    double:   "double",                  \
    char:     "char",                    \
    default:  "unknown"                  \
)

int32_t i = 42;
printf("%s\n", TYPE_NAME(i));  /* "int32_t" */
printf("%s\n", TYPE_NAME(3.14));  /* "double" */
```

### 宏泛型 (Generic Selection)

结合 `_Generic` 和宏, 可以实现类似模板的效果：

```c
#define MAX(a, b) _Generic((a),            \
    int32_t:    ((a) > (b) ? (a) : (b)),   \
    double:     ((a) > (b) ? (a) : (b))    \
)
```

### vs C++ 模板 / Rust 泛型

```
┌───────────┬──────────┬───────────┬──────────┐
│ 特性        │ C           │ C++         │ Rust      │
├───────────┼──────────┼───────────┼──────────┤
│ 类型安全     │ ❌ 手动      │ ✅ 编译期    │ ✅ 编译期   │
│ 运行时开销   │ 无          │ 无          │ 无         │
│ 编译期开销   │ 小          │ 大          │ 中等       │
│ 错误提示     │ 差          │ 极好        │ 极好       │
└───────────┴──────────┴───────────┴──────────┘
```

C 选择 `void*` 是因为它零运行时开销 —— 代价是安全责任全在程序员手中。

## 小结

本章的核心要点：

- **`void*`** 是无类型指针, 可以指向任何类型的对象
- **不能直接解引用 `void*`** —— 必须先转换为具体类型
- **泛型函数**用 `void*` 参数 + `size` 实现类型无关操作
- **类型信息丢失**是根本局限 —— 用 enum tag 或 `_Generic` 补充
- **宏可以模拟**部分泛型效果, 但缺少类型安全
- **C++ 模板 / Rust 泛型**在编译期保证类型安全, C 需要手动管理
- `const void*` 保证数据不被修改

## 术语表

| 英文 | 中文 |
|------|------|
| void pointer | void 指针 / 无类型指针 |
| Type erasure | 类型擦除 |
| Generic programming | 泛型编程 |
| Type tag / discriminant | 类型标签 |
| Type casting | 类型转换 |
| Callback function | 回调函数 |
| `_Generic` | C11 泛型选择表达式 |
| Type safety | 类型安全 |

## 延伸阅读

- [C17 标准 §6.2.5 — void 类型](https://en.cppreference.com/w/c/language/type) — 官方标准定义
- [cppreference - void pointer](https://en.cppreference.com/w/c/language/pointer) — void* 操作参考
- [C11 _Generic selection](https://en.cppreference.com/w/c/language/generic) — 泛型选择表达式

**选择建议**：先用 cppreference 理解 `void*` 基本概念, 再阅读 C11 `_Generic` 了解编译期泛型。

## 继续学习

你已经掌握了 C 语言最"自由"的指针类型 —— `void*`。它给你最大的灵活性, 也给你最大的责任。下一步, 我们将学习**位运算与内存操作** —— 如何操作单个 bit, 如何实现 bitmask 权限系统, 以及如何直接操作内存字节。

- [上一章](./scope.md)：作用域与生命周期
- [下一章](./bit-ops.md)：位运算与内存操作
