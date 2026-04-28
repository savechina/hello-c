# void* 指针 (Void Pointers and Type Erasure)

## 开篇故事

想象一个万能充电适配器。它能插进美标、欧标、英标的插座——因为它的插头是通用的。但当你把数据线连到设备上时，你必须知道那台设备需要多少伏特。适配器不替你操心，你得自己确认。

`void*` 就是 C 的万能适配器。它可以指向任何类型的数据（`int`、`double`、`struct`……），但在读取之前你必须**把它转回正确的类型**。适配器能接任何插头，但接错了电压设备就烧了。`void*` 能存任何地址，但转错了类型数据就乱了。

灵活性很高，责任也在你。

## 本章适合谁

- 已掌握具体类型指针（`int*`、`char*`）的概念
- 好奇 C 如何实现「泛型」编程
- 用过 Python 动态类型，想了解 C 的「类型擦除」
- 正在读标准库函数（`qsort`、`bsearch`、`memset`）源码的人

## 你会学到什么

1. `void*` 的本质——无类型指针，可指向任何对象
2. 如何安全地将 `void*` 转回具体类型
3. 泛型函数：`void*` + `size_t size` 实现字节级操作
4. Type tag 模式——用枚举标签记录被擦除的类型
5. `qsort` 回调模式：`int (*cmp)(const void*, const void*)`
6. void* 的局限性与 Python/C++ 泛型对比

## 前置要求

- 已完成 [指针基础](./pointer_basics.md) 和 [指针运算](./pointer_arith.md)
- 理解 `<stdint.h>` 中的固定宽度类型
- 理解函数参数和回调函数的概念

## 第一个例子

最简单的 `void*` 演示——它能指向任何类型的变量：

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int32_t i = 42;
    double d = 3.14;
    char c = 'Z';

    void *vp1 = &i;  /* void* 接收 int32_t* */
    void *vp2 = &d;  /* void* 接收 double* */
    void *vp3 = &c;  /* void* 接收 char* */

    /* ⚠️ 要读取值，必须先转回正确类型 */
    printf("i = %" PRId32 "\n", *(int32_t *)vp1);
    printf("d = %.2f\n",   *(double *)vp2);
    printf("c = '%c'\n",   *(char *)vp3);

    return 0;
}
```

输出：

```
i = 42
d = 3.14
c = 'Z'
```

核心规则：
- `void*` **可以接收**任何类型地址（无需强制转换）
- `void*` **不能直接解引用**——必须先 `(type *)` 转回具体类型

## 原理解析

### 1. `void*` 的本质

`void` 表示「无类型」。`void*` 因此被称为**通用指针** (universal pointer)：

```c
void *vp;  /* vp 可以指向任何类型的对象 */
```

内存中的 `void*` 和 `int32_t*`、`char*` **完全一样**——都是一个 8 字节的地址。区别在于**编译器如何看待它**：

| 类型 | 解引用大小 | `p+1` 步长 | 能直接 `*p`？ |
|------|-----------|------------|--------------|
| `int32_t *` | 4 字节 | +4 字节 | ✅ |
| `double *` | 8 字节 | +8 字节 | ✅ |
| `char *` | 1 字节 | +1 字节 | ✅ |
| `void *` | **不确定** | **无法确定** | ❌ 编译错误 |

`void*` 没有大小信息，编译器不知道它背后是 1 字节还是 8 字节。

### 2. 转回具体类型

```c
int32_t value = 42;
void *vp = &value;

/* ✅ 正确: 转换为 int32_t* */
int32_t recovered = *(int32_t *)vp;

/* ❌ 错误: 直接解引用 void* */
/* int32_t bad = *vp; */
/* error: invalid use of undefined type 'void' */
```

**类比**：`void*` 像拆了标签的快递盒。你必须先「贴标签」（类型转换），才能正确打开它。

### 3. 泛型 swap——`void*` + `size` 模式

利用 `void*`，可以写出**适用于任何类型**的交换函数：

```c
void generic_swap(void *a, void *b, size_t size) {
    unsigned char temp[256];
    if (size > sizeof(temp)) return;
    memcpy(temp, a, size);   /* 按字节拷贝 */
    memcpy(a, b, size);
    memcpy(b, temp, size);
}

int32_t xi = 10, yi = 20;
generic_swap(&xi, &yi, sizeof(int32_t));
/* xi = 20, yi = 10 */

double xd = 1.5, yd = 9.9;
generic_swap(&xd, &yd, sizeof(double));
/* xd = 9.9, yd = 1.5 */
```

关键点：
- 函数不关心类型，只管按字节数拷贝
- 调用者必须传 `sizeof(类型)`
- 这就是 `qsort` 和 `bsearch` 的设计模式

### 4. Type tag 模式——找回被擦除的类型

`void*` 丢失类型信息后，需要额外手段追回：

```c
typedef enum { TYPE_INT32, TYPE_DOUBLE, TYPE_CHAR } VpTypeTag;

void typed_print(VpTypeTag tag, void *data) {
    switch (tag) {
    case TYPE_INT32:  printf("%" PRId32, *(int32_t*)data); break;
    case TYPE_DOUBLE: printf("%.3f",    *(double*)data);   break;
    case TYPE_CHAR:   printf("'%c'",    *(char*)data);     break;
    }
}
```

这类似于 Python 的 `isinstance()`——但 Python 自带类型标签，C 需要程序员手动传递。

### 5. `qsort` 回调模式

标准库 `qsort` 是 `void*` 最经典的用法：

```c
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
```

```c
int cmp_int32(const void *a, const void *b) {
    int32_t va = *(const int32_t *)a;
    int32_t vb = *(const int32_t *)b;
    return (va > vb) - (va < vb);
}

int32_t nums[] = {33, 10, 75, 42, 5};
qsort(nums, 5, sizeof(int32_t), cmp_int32);
/* nums 现在有序: {5, 10, 33, 42, 75} */
```

核心设计：
- `void*` 抹掉类型，让 `qsort` 能排序任何数据类型
- 回调函数负责「解释」内容——它知道如何比较两个元素

### 6. Python 动态类型 vs C void* 对比

| 特征 | Python | C `void*` |
|------|--------|-----------|
| 存储任意类型 | ✅ 原生 | ✅ 通过 `void*` |
| 运行时类型信息 | ✅ 对象自带 | ❌ 需要手动跟踪 |
| 类型安全检查 | ✅ 运行时 | ❌ 无（程序员负责） |
| 类型错误后果 | `TypeError` 异常 | 未定义行为 (UB) |

```python
# Python — 自动管理
x = 42        # x 是 int
x = "hello"   # x 变成 str — 无需声明
```

```c
// C — 手动管理
void *vp = &i;              // vp 指向 int
vp = &d;                    // vp 指向 double (类型已丢失!)
double val = *(double*)vp;  // ⚠️ 必须记得当前类型
```

**我的理解**：`void*` 是 C 的「类型擦除」(type erasure)——抹掉类型信息换取灵活性，代价是安全责任转移给程序员。

## 常见错误

### ❌ 错误 1：直接解引用 `void*`

```c
void *vp = &some_int;
int x = *vp;   /* ❌ 编译错误: invalid use of undefined type 'void' */
```

```c
/* ✅ 修正 */
int x = *(int32_t *)vp;  /* 显式转换 */
```

### ❌ 错误 2：转回错误类型——运行时 UB

```c
double d = 3.14;
void *vp = &d;
int x = *(int32_t *)vp;  /* ❌ 编译通过，数据完全错误! */
```

**这是最危险的错误**——编译器不警告，数据却全乱。

### ❌ 错误 3：忘记类型信息

```c
void *container[2];
container[0] = &int_val;
container[1] = &double_val;

/* 后来忘了 container[1] 的类型 */
int wrong = *(int32_t *)container[1];  /* 数据错乱! */
```

```c
/* ✅ 用 struct 包装类型信息 */
typedef struct { VpTypeTag tag; void *data; } TypedValue;
```

## 动手练习

### 🟢 入门：void* 赋值与转换

声明 `int32_t` 和 `double` 变量，用 `void*` 分别指向它们，正确转换回原类型并打印。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int32_t i = 100;
    double d = 2.718;
    void *vp;

    vp = &i;
    printf("int: %" PRId32 "\n", *(int32_t *)vp);

    vp = &d;
    printf("double: %.3f\n", *(double *)vp);

    return 0;
}
```

</details>

### 🟡 中级：泛型 max 函数

编写 `void *generic_max(void *arr, int count, size_t size, int (*cmp)(const void*, const void*))`，返回最大值元素的地址。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

void *generic_max(void *arr, int count, size_t size,
                  int (*cmp)(const void*, const void*))
{
    void *max_elem = arr;
    for (int j = 1; j < count; j++) {
        void *current = (unsigned char *)arr + (size_t)j * size;
        if (cmp(current, max_elem) > 0) {
            max_elem = current;
        }
    }
    return max_elem;
}

int cmp_int32(const void *a, const void *b) {
    int32_t va = *(const int32_t *)a;
    int32_t vb = *(const int32_t *)b;
    return (va > vb) - (va < vb);
}

int main(void) {
    int32_t nums[] = {3, 7, 2, 9, 4};
    int32_t *max = (int32_t *)generic_max(nums, 5, sizeof(int32_t), cmp_int32);
    printf("max: %d\n", *max);  /* 9 */
    return 0;
}
```

</details>

### 🔴 挑战：类型安全的泛型包装器

设计 `TypedValue` 结构体，包含 `VpTypeTag type` 和 `void *data`，实现 `print_typed_value(TypedValue tv)` 根据 tag 安全打印。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

typedef enum { TV_INT32, TV_DOUBLE, TV_CHAR } VpTypeTag;

typedef struct { VpTypeTag type; void *data; } TypedValue;

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

## 故障排查 (FAQ)

**Q：`void*` 能进行指针算术吗？**

A：标准 C 不允许。`vp + 1` 无法定义（编译器不知道 `void` 的大小）。GCC/Clang 作为扩展允许（按 1 字节计算），但可移植代码应先转成 `unsigned char*`：

```c
void *vp = some_data;
unsigned char *cp = (unsigned char *)vp;
cp++;   /* ✅ 前进 1 字节 */
```

**Q：为什么不用 `uintptr_t` 代替 `void*`？**

A：`uintptr_t` 是整数类型，不能直接解引用。`void*` 的价值是「可间接寻址 + 类型擦除」，你总需要在某个时刻把它转回具体类型来读写。

**Q：`void*` 能和 `NULL` 比较吗？**

A：可以。`void*` 支持所有指针比较运算：

```c
void *vp = NULL;
if (vp == NULL) { /* ... */ }  /* ✅ */
```

## 知识扩展

### `void*` 的 `const` 变体

```c
const void *cvp = &pi;   /* 指向 const 数据的通用指针 */
void *const cvp2 = &mutable;  /* const 通用指针（很少用） */
```

`qsort` 的比较函数用 `const void*`——保证不会修改被比较的元素。

### vs C++ 模板 / Rust 泛型

```
┌───────────┬──────────┬───────────┬──────────┐
│ 特性        │ C           │ C++         │ Rust      │
├───────────┼──────────┼───────────┼──────────┤
│ 类型安全     │ ❌ 手动      │ ✅ 编译期    │ ✅ 编译期   │
│ 运行时开销   │ 无          │ 无          │ 无         │
│ 编译期开销   │ 小          │ 大          │ 中等       │
│ 错误信息     │ 差(运行时)  │ 极好        │ 极好       │
└───────────┴──────────┴───────────┴──────────┘
```

C 选择 `void*` 是因为它零运行时开销——代价是安全责任全在程序员手中。

## 小结

- **`void*`** 是无类型指针，可接收任何对象地址
- **不能直接解引用 `void*`**——必须先转回具体类型
- **泛型函数**用 `void*` + `size` 实现字节级操作
- **类型信息丢失**是根本局限——用 enum tag 补充
- **`qsort` 回调模式**是 void* 最经典的 API 设计
- **C++/Rust 泛型**在编译期保证类型安全，C 需要手动管理

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| void pointer | void 指针 / 无类型指针 | 可指向任何类型的通用指针 |
| Type erasure | 类型擦除 | 抹掉类型信息换取灵活性 |
| Generic programming | 泛型编程 | 不依赖具体类型的编程 |
| Type tag | 类型标签 | 手动记录被擦除的类型 |
| Callback function | 回调函数 | 作为参数传递的函数指针 |

## 延伸阅读

- [cppreference - Void pointer](https://en.cppreference.com/w/c/language/pointer) — void* 操作参考
- [C11 `_Generic` selection](https://en.cppreference.com/w/c/language/generic) — 编译期泛型选择

## 继续学习

- [上一章](./pointers_and_functions.md)：指针与函数
- [下一章](./const_correctness.md)：const 正确性——const 指针与指向 const 的指针

---

> 本章代码位于 [`src/basic/void_pointers_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/void_pointers_sample.c)。
