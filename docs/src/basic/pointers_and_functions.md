# 指针与函数 (Pointers and Functions)

## 开篇故事

想象你有一间房子，你的朋友想修改你家客厅的家具。你有两个选择：

1. **拍照给朋友**——他看到客厅的样子，可以在自己的纸上做笔记，但改不了你家客厅。（**值传递**）
2. **给他一把钥匙**——他可以直接打开你家门，移动家具。（**指针传递**）

指针让函数能够「穿过调用边界」修改调用者的数据。没有指针，C 函数的参数传递永远是副本——修改只在函数内部有效。有了指针，函数就能直接操作原始内存。

```c
void swap_by_copy(int32_t a, int32_t b) {
    int32_t tmp = a; a = b; b = tmp;  /* 只修改副本 */
}

void swap_by_pointer(int32_t *a, int32_t *b) {
    int32_t tmp = *a; *a = *b; *b = tmp;  /* 修改真实数据 */
}
```

指针是 C 函数与外部世界沟通的桥梁。

## 本章适合谁

- 已掌握指针基础（`&`、`*`、NULL）
- 理解函数声明和调用
- 好奇「C 是不是只能值传递？」
- 被「函数改了数据但外部没变」困扰过的初学者

## 你会学到什么

1. 值传递 (pass-by-value) 的本质——函数收到副本
2. 用指针实现「传递引用」(pass-by-reference)
3. 经典陷阱：返回局部变量的地址
4. 正确模式 1：通过输出参数返回结果
5. 正确模式 2：堆分配 + 返回指针
6. 函数内修改数组内容（数组名退化为指针）
7. C 值传递 vs Python 参数传递对比

## 前置要求

- 已完成 [指针基础](./pointer_basics.md) 和 [函数](./functions.md) 章节
- 理解函数的参数、返回值、调用过程
- 理解栈帧 (stack frame) 的基本概念

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

void add_one_value(int32_t x) {
    x = x + 1;   /* 只修改副本 */
}

void add_one_pointer(int32_t *x) {
    *x = *x + 1;  /* 修改原始数据 */
}

int main(void) {
    int32_t a = 10;
    printf("before: a = %d\n", a);

    add_one_value(a);
    printf("pass-by-value: a = %d  (没变)\n", a);

    add_one_pointer(&a);
    printf("pass-by-pointer: a = %d  (变了!)\n", a);

    return 0;
}
```

输出：

```
before: a = 10
pass-by-value: a = 10  (没变)
pass-by-pointer: a = 11  (变了!)
```

关键理解：`add_one_value(10)` 收到的是 `10` 的副本，函数退出后副本销毁。`add_one_pointer(&a)` 收到的是 `a` 的地址，`*x` 操作直接修改 `a` 本身。

> 📌 **回顾之前学的**: C 是值传递（Pass by Value），函数参数的修改不会影响调用者。要修改调用者变量，必须传指针（& 取地址）。详见 [函数](./functions.md#参数传递)。

## 原理解析

### 1. 值传递——函数收到的是副本

```c
void foo(int32_t x) {
    x = 999;   /* 修改的是 foo 的栈帧内的副本 */
}

int32_t a = 10;
foo(a);        /* 传递 a 的值 (10) */
printf("%d\n", a);  /* 10 — 未改变 */
```

**内存视角**：

```
  main 的栈帧:
  a = 10  (地址 0x7ff…a0)

  foo 的栈帧 (调用时创建):
  x = 10  (地址 0x7ff…b0)  ← 副本！

  foo 返回后:
  foo 的栈帧销毁，x 的修改丢失
```

C 语言**所有**参数都是值传递——没有例外。想要修改外部变量，必须传递地址。

### 2. 传递引用——指针作为参数

```c
void swap(int32_t *a, int32_t *b) {
    int32_t tmp = *a;
    *a = *b;
    *b = tmp;
}

int32_t x = 10, y = 20;
swap(&x, &y);
/* x = 20, y = 10 */
```

```
  调用 swap(&x, &y) 时:

  main 栈帧:        swap 栈帧:
  x = 10            a = 0x7ff…a0  (x 的地址)
  y = 20            b = 0x7ff…a8  (y 的地址)

  *a = *b  →  把 b 指向的值 (20) 写入 a 指向的位置 (x)
  *b = tmp →  把 tmp 的值写入 b 指向的位置 (y)

  结果: x 和 y 真的被交换了
```

```text
┌─────── pass-by-value (值传递) ────────┐
│  swap_by_copy(x, y):                  │
│  main 栈帧       swap 栈帧 (副本)      │
│  ┌────────┐      ┌────────┐            │
│  │ x = 10 │      │ a = 10 │← x 副本    │
│  ├────────┤      ├────────┤            │
│  │ y = 20 │      │ b = 20 │← y 副本    │
│  └────────┘      └────────┘            │
│       ↓ 交换副本 → a=20, b=10          │
│  返回后: x=10, y=20  ❌ 没变            │
│                                        │
├────── pass-by-pointer (指针传递) ──────┤
│  swap_by_pointer(&x, &y):             │
│  main 栈帧       swap 栈帧 (地址)      │
│  ┌────────┐      ┌──────────┐          │
│  │ x = 10 │◄─────│ a = &x   │          │
│  ├────────┤      ├──────────┤          │
│  │ y = 20 │◄─────│ b = &y   │          │
│  └────────┘      └──────────┘          │
│       ↓ *a=20, *b=10                   │
│  x = 20, y = 10  ← 直接修改 main 变量  │
│  返回后: x=20, y=10  ✅ 交换成功        │
└────────────────────────────────────────┘
```

### 3. 经典陷阱：返回局部变量地址

```c
int32_t *bad_func(void) {
    int32_t temp = 42;   /* 在 bad_func 的栈帧中 */
    return &temp;         /* ❌ 函数返回后栈帧销毁，地址失效 */
}

int32_t *p = bad_func();
printf("%d\n", *p);     /* ❌ 未定义行为! (野指针) */
```

```
  bad_func 栈帧:
  temp = 42   (地址 0x7ff…c0)

  函数返回后:
  [栈帧销毁] — 0x7ff…c0 变成「垃圾区」
  p 指向无效地址 → 解引用 = UB
```

**永远不要返回局部变量的地址**。局部变量存放在栈上，函数退出时自动回收。

### 4. 正确模式 1：输出参数

```c
/* ✅ 通过输出参数返回结果 */
void compute_sum(int32_t a, int32_t b, int32_t *out) {
    *out = a + b;
}

int32_t result;
compute_sum(3, 4, &result);
printf("sum = %d\n", result);  /* 7 */
```

调用者提供存储空间，函数负责填充。返回值是 `void`，通过指针参数「输出」结果。这是 C 标准库的常见模式（如 `scanf`）。

### 5. 正确模式 2：堆分配返回指针

```c
#include <stdlib.h>

int32_t *make_value(int32_t v) {
    int32_t *ptr = malloc(sizeof(int32_t));
    if (ptr == NULL) return NULL;   /* malloc 可能失败 */
    *ptr = v;
    return ptr;   /* ✅ 堆上数据函数返回后仍有效 */
}

int32_t *p = make_value(42);
printf("%d\n", *p);   /* 42 */
free(p);              /* ⚠️ 不要忘记释放 */
```

堆 (heap) 内存不随函数栈帧销毁，需要手动 `free()`。这是 `strdup()` 等标准库函数的原理。

### 6. 数组传参——退化为指针

```c
void print_array(int32_t *data, size_t n) {
    /* data 是退化后的指针，不是数组 */
    /* sizeof(data) = 8 (指针大小), 不是数组大小 */
    for (size_t i = 0; i < n; i++) {
        printf("%d ", data[i]);
    }
}

int32_t arr[5] = {1, 2, 3, 4, 5};
print_array(arr, 5);   /* arr 退化为 int32_t* */
```

```c
/* 以下三种函数声明完全等价 */
void foo(int32_t *data);
void foo(int32_t data[]);
void foo(int32_t data[5]);   /* 5 被忽略! 实际仍是 int32_t* */
```

数组传给函数时自动退化为指向首元素的指针。**调用者必须单独传递长度**。

### 7. C 值传递 vs Python 参数传递对比

| 特性 | C | Python |
|------|---|--------|
| 基础类型 | 值传递，不可修改外部 | 一切皆对象，传引用（但不可变对象无法修改） |
| 数组/列表 | 退化为指针，需传长度 | 传对象引用，自带长度 |
| 多返回值 | 输出参数 / struct | `return a, b` (tuple) |
| 可变性 | 指针指向的数据可改 | 可变对象可改，不可变对象不可改 |

```python
# Python — 一切传引用
def modify(lst):
    lst.append(99)   # 修改原列表

a = [1, 2, 3]
modify(a)
print(a)  # [1, 2, 3, 99] — 变了!
```

```c
// C — 基础类型值传递，需要指针
void modify(int32_t *arr, size_t n) {
    arr[n] = 99;
}

int32_t a[4] = {1, 2, 3};
modify(a, 3);  // 数组退化为指针 — 能修改
```

## 常见错误

### ❌ 错误 1：返回局部变量地址

```c
char *get_string(void) {
    char msg[] = "hello";
    return msg;   /* ❌ msg 在栈上，返回后失效 */
}
```

```c
/* ✅ 用 static 或堆分配 */
char *get_string(void) {
    static char msg[] = "hello";  /* static 数据在数据段，不随栈帧销毁 */
    return msg;
}
```

### ❌ 错误 2：忘记 NULL 检查输出参数

```c
void compute(int32_t input, int32_t *out) {
    *out = input * 2;  /* ❌ 如果 out 是 NULL? 崩溃! */
}
```

```c
/* ✅ 防御性检查 */
void compute(int32_t input, int32_t *out) {
    if (out == NULL) return;
    *out = input * 2;
}
```

### ❌ 错误 3：函数内用 sizeof 获取数组长度

```c
void foo(int32_t arr[]) {
    size_t n = sizeof(arr) / sizeof(arr[0]);  /* ❌ arr 已是指针! */
}
```

```c
/* ✅ 把长度作为参数传入 */
void foo(int32_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) { /* ... */ }
}
```

## 动手练习

### 🟢 入门：用指针翻转两个变量

编写 `void flip(int32_t *a, int32_t *b)`，交换 `*a` 和 `*b` 的值。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

void flip(int32_t *a, int32_t *b) {
    int32_t tmp = *a;
    *a = *b;
    *b = tmp;
}

int main(void) {
    int32_t x = 3, y = 7;
    flip(&x, &y);
    printf("x = %" PRId32 ", y = %" PRId32 "\n", x, y);
    return 0;
}
```

</details>

### 🟡 中级：查找并返回指针

编写 `int32_t *find(int32_t *arr, size_t n, int32_t target)`，找到返回元素地址，未找到返回 NULL。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

int32_t *find(int32_t *arr, size_t n, int32_t target) {
    int32_t *end = arr + (int32_t)n;
    for (int32_t *p = arr; p < end; p++) {
        if (*p == target) return p;
    }
    return NULL;
}

int main(void) {
    int32_t data[] = {3, 1, 4, 1, 5, 9};
    int32_t *found = find(data, 6, 5);
    if (found) {
        printf("found at index %" PRIdPTR "\n", found - data);
    }
    return 0;
}
```

</details>

### 🔴 挑战：实现 in-place 数组翻倍

编写 `void double_inplace(int32_t *data, size_t n)`，原地将每个元素翻倍。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

void double_inplace(int32_t *data, size_t n) {
    int32_t *end = data + (int32_t)n;
    for (int32_t *p = data; p < end; p++) {
        *p *= 2;
    }
}

int main(void) {
    int32_t arr[] = {1, 2, 3, 4, 5};
    double_inplace(arr, 5);
    for (int32_t i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");  /* 2 4 6 8 10 */
    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q：C 真的不能传引用吗？**

A：C 语言标准中**没有**「引用」这个概念。所有参数都是值传递。想要修改外部数据，必须显式传递指针。C++ 的 `&` 引用语法（`void foo(int &x)`）在 C 中不可用。

**Q：为什么传递数组时需要单独传长度？**

A：因为数组传给函数时退化为 `T*`，编译器只知道它是指针，不知道原来数组有多少元素。`sizeof` 只能得到指针大小（8 字节）。

**Q：函数返回 `static` 局部变量安全吗？**

A：线程安全角度：**不安全**（多个线程共享同一块数据）。但在单线程程序中，`static` 数据存储在数据段而非栈上，函数返回后仍然有效。

## 知识扩展

### const 输出参数——输入/输出语义

```c
/* src 是输入（只读），dst 是输出（只写） */
void copy_string(const char *src, char *dst) {
    while (*dst++ = *src++);
}
```

用 `const` 标注输入参数，用裸指针标注输出参数，是 C API 设计的经典约定。

### 多维数组传参

```c
/* 二维数组传参——必须指定列数 */
void process_matrix(int32_t matrix[][3], size_t rows) {
    /* matrix 退化为 int32_t (*)[3] */
    for (size_t r = 0; r < rows; r++) {
        for (int32_t c = 0; c < 3; c++) {
            printf("%d ", matrix[r][c]);
        }
    }
}
```

## 小结

- C 的**所有**参数都是值传递——函数收到的是副本
- 用指针可实现「传递引用」——修改调用者的数据
- **永远不要返回局部变量地址**——栈帧销毁后变成野指针
- 正确模式：输出参数 (`void foo(int *out)`) 或堆分配 (`malloc`)
- 数组传参退化为指针，需要单独传长度

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 值传递 | Pass-by-value | 函数收到参数的副本 |
| 传递引用 | Pass-by-reference | 通过指针间接修改外部变量 |
| 栈帧 | Stack frame | 函数调用的局部内存区域 |
| 输出参数 | Output parameter | 通过指针参数返回结果 |
| 野指针 | Dangling pointer | 指向已销毁数据的指针 |
| 未定义行为 | Undefined behavior | 标准不规定结果的行为 |

## 延伸阅读

- [K&R《The C Programming Language》§5.3 — 指针与数组参数](https://en.wikipedia.org/wiki/The_C_Programming_Language)
- [cppreference - Function parameters](https://en.cppreference.com/w/c/language/function)

## 继续学习

- [上一章](./pointers_and_arrays.md)：指针与数组
- [下一章](./const_correctness.md)：const 正确性

---

> 本章代码位于 [`src/basic/pointers_and_functions_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/pointers_and_functions_sample.c)。
