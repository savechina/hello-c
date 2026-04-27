# 指针基础 (Pointer Basics)

## 开篇故事

想象你拿到一张酒店房卡。房卡上印的不是房间本身，而是一个房间号。你需要拿着这个号码走到对应的门前，刷卡，才能进入房间。

指针就是 C 语言里的「房卡」。它不存储数据本身，而是存储数据所在的地址。`&a` 是在问「a 住在哪个地址」，`*p` 是拿着地址 p 走到门前，开门看看里面是什么。

```c
int a = 10;
int *p = &a;   // p 拿着 a 的地址，像房卡指向房门
*p = 20;       // 顺着地址找到 a，修改门里的值
```

很多人第一次看到 `*` 和 `&` 就觉得玄乎。其实它们做的事情很朴素：一个告诉你「去哪找」，一个帮你「找到以后打开看」。理解了这一点，指针就不再是神秘的咒语，而是 C 语言给你的一把手术刀——锋利，但握对了就不怕受伤。

> "指针的本质不是数据，而是数据的地址。" —— 每一个 C 程序员迟早会明白的事

## 本章适合谁

- 已经掌握 C 语言变量和数据类型基础
- 听说过「指针」但总觉得神秘、怕踩坑
- 用过 Python/JavaScript 等高级语言，想了解 C 的内存控制能力
- 被「段错误 (Segmentation Fault)」折磨过的初学者

## 你会学到什么

1. 指针的声明方法：`int *p` 到底是什么意思
2. `&` 取地址运算符和 `*` 解引用运算符的用法
3. 如何在内存中用指针「定位」和「修改」数据
4. NULL 指针的含义及安全检查模式
5. 指针类型（`int*` vs `char*`）对解范围的影响
6. 如何正确初始化指针，避免野指针

## 前置要求

已完成 [变量](./variables.md) 和 [数据类型](./datatype.md) 章节。你需要理解：
- 变量是内存中的命名存储空间
- 不同类型（`int`、`double`、`char`）占据不同大小的内存
- `<stdint.h>` 中的固定宽度类型（`int32_t`、`int64_t` 等）

## 第一个例子

下面是最简短的指针演示程序。它声明一个变量和一个指向它的指针，然后打印它们的值和地址：

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t  num = 42;     /* 普通变量 */
    int32_t *ptr = &num;   /* 指针：存储 num 的地址 */

    printf("num 的值  = %d\n", num);
    printf("num 的地址 = %p\n", (void *)&num);
    printf("ptr 的值  = %p  (和 num 的地址相同)\n", (void *)ptr);
    printf("*ptr 的值 = %d  (解引用得到 num 的值)\n", *ptr);

    *ptr = 100;            /* 通过指针修改 num */
    printf("修改 *ptr = 100 后, num = %d\n", num);

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
num 的值  = 42
num 的地址 = 0x7ffee4c4a3ac
ptr 的值  = 0x7ffee4c4a3ac  (和 num 的地址相同)
*ptr 的值 = 42  (解引用得到 num 的值)
修改 *ptr = 100 后, num = 100
```

这段代码做了几件事：
- 声明了一个 `int32_t` 变量 `num`，值为 42
- 声明了一个指针 `ptr`，用 `&num` 把 `num` 的地址赋给它
- `*ptr` 解引用——顺着地址找到 `num`，读取它的值
- 通过 `*ptr = 100` 修改指针指向的内容，等价于修改 `num` 本身

## 原理解析

### 指针语法：`&` 取地址 和 `*` 解引用

C 语言中有两个专门的指针运算符：

| 运算符 | 名称 | 作用 | 示例 |
|--------|------|------|------|
| `&` | 取地址 (Address-of) | 获取变量在内存中的地址 | `&num` 返回 num 的地址 |
| `*` | 解引用 (Dereference) | 通过地址找到变量本身 | `*ptr` 访问 ptr 指向的值 |

```c
int32_t num = 42;
int32_t *ptr = &num;

/* num 是变量名, 值是 42 */
/* ptr 是指针, 值是 &num (num 的地址) */
/* *ptr 是解引用, 等价于 num */

printf("%d\n", num);   /* 42 */
printf("%d\n", *ptr);  /* 42, 通过指针读 */
*ptr = 99;
printf("%d\n", num);   /* 99, num 被改变了! */
```

### 内存布局：ASCII 示意图

理解指针最直观的方法是看内存图。假设变量 `x` 和指针 `px` 都在栈上：

```
  符号        地址           值
  ------+---------------+--------------
  x     | 0x7ff...b0    |  42
  px    | 0x7ff...b8    |  0x7ff...b0  ← px 里存的是 x 的地址
                                  ↑
                           *px → 0x7ff...b0 → 42
```

- `x` 在地址 `0x7ff...b0` 处，存放数值 42
- `px` 在地址 `0x7ff...b8` 处，存放的是 `0x7ff...b0`（即 `&x`）
- `*px` 的意思是：取出 `px` 中的值 (`0x7ff...b0`)，然后到那个地址取值，得到 42

### NULL 指针

`NULL` 是一个特殊地址值（通常是 `0`），表示「不指向任何有效数据」。

**永远不要解引用 NULL 指针**——会导致段错误 (Segmentation Fault)：

```c
int32_t *ptr = NULL;
printf("%d\n", *ptr);  /* ❌ Segmentation fault! 程序崩溃 */
```

正确做法是使用前检查：

```c
int32_t *ptr = NULL;
/* ... 某个函数可能给 ptr 赋值 ... */
if (ptr != NULL) {
    printf("%d\n", *ptr);  /* ✅ 安全 */
} else {
    printf("ptr is NULL\n");
}
```

### 指针初始化

**野指针 (Dangling Pointer)** 是最常见的指针错误之一——声明了指针但没有初始化，它指向一个随机内存地址。

```c
int32_t *p;   /* ❌ 未初始化! p 指向随机地址 */
*p = 42;      /* ❌ 向随机地址写入 = 崩溃 或 数据损坏 */
```

两种安全的初始化方式：

```c
int32_t val = 42;
int32_t *p1 = &val;  /* ✅ 指向有效变量 */
int32_t *p2 = NULL;  /* ✅ 明确指向空 */
```

### 指针类型

指针的**类型**决定了编译器如何解释它指向的内存：

```c
int32_t  num = 0x01020304;
int32_t *pi = &num;   /* pi 每次移动 4 字节 */
uint8_t *pb = (uint8_t *)&num;  /* pb 每次移动 1 字节 */

printf("%d\n", *pi);  /* 读取整个 int32_t (4 字节) */
printf("%d\n", *pb);  /* 只读取第一个字节 */
```

指针类型的重要性：
- `int32_t*`：解引用时读取 4 字节，指针加法 `p+1` 前进 4 字节
- `uint8_t*`：解引用时读取 1 字节，指针加法 `p+1` 前进 1 字节
- 类型不匹配时，编译器会给出警告

## 常见错误

### 错误 1：解引用野指针

```c
/* ❌ 错误代码 */
int32_t *p;      /* 未初始化 */
*p = 42;         /* 写入随机内存地址 */
```

编译器可能不会报错（或未初始化警告），但运行时会**段错误**或产生难以调试的数据损坏：

```
Segmentation fault (core dumped)
```

```c
/* ✅ 修复：初始化指针 */
int32_t  val = 0;
int32_t *p = &val;   /* 指向有效变量 */
*p = 42;              /* 安全 */
```

### 错误 2：解引用 NULL 指针

```c
/* ❌ 错误代码 */
int32_t *p = NULL;
printf("%d\n", *p);   /* 解引用 NULL = 崩溃 */
```

运行时报错：

```
Segmentation fault (core dumped)
```

```c
/* ✅ 修复：检查 NULL */
int32_t *p = NULL;
int32_t target = 42;
p = &target;
if (p != NULL) {
    printf("%d\n", *p);   /* 安全检查后使用 */
}
```

### 错误 3：混淆 `*p = value` 与 `p = &value`

```c
/* ❌ 错误代码 */
int32_t a = 10;
int32_t b = 20;
int32_t *p = &a;
p = &b;   /* 这个操作是"让 p 改指向 b"，不是"修改 a 为 20" */
printf("%d\n", *p);  /* 输出 20 */
printf("%d\n", a);   /* a 还是 10! 没被修改 */
```

```c
/* ✅ 如果你想通过指针修改 a 的值 */
int32_t *p = &a;
*p = 20;   /* *p 解引用 = a，现在 a = 20 */
printf("%d\n", a);   /* 输出 20 */
```

## 动手练习

### 🟢 入门：交换两个变量

声明 `int32_t a = 3` 和 `int32_t b = 7`，用指针修改它们的值，使得 `a` 变成 7、`b` 变成 3。

<details><summary>点击查看答案</summary>

```c
int32_t a = 3, b = 7;
int32_t *pa = &a;
int32_t *pb = &b;

int32_t temp = *pa;
*pa = *pb;
*pb = temp;

printf("a = %d, b = %d\n", (int)a, (int)b);
/* 输出: a = 7, b = 3 */
```

</details>

### 🟡 中级：指针遍历

声明一个 `int32_t` 数组 `{10, 20, 30, 40, 50}`，用**指针**（不是数组索引 `[]`）遍历并打印所有元素。

<details><summary>点击查看答案</summary>

```c
int32_t arr[] = {10, 20, 30, 40, 50};
int32_t n = (int32_t)(sizeof(arr) / sizeof(arr[0]));
int32_t *p = arr;  /* 数组名 = 首元素地址 */

for (int32_t i = 0; i < n; i++) {
    printf("%d ", *(p + i));  /* *(p + i) 等价于 p[i] 等价于 arr[i] */
}
printf("\n");
/* 输出: 10 20 30 40 50 */
```

</details>

### 🔴 挑战：NULL 防御

写一个函数 `safe_divide(int32_t a, int32_t b, int32_t *result)`，当 `b == 0` 时返回失败（`result` 保持 NULL 不解引用），否则执行 `*result = a / b`。调用方必须在使用 `result` 前检查是否为 NULL。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

int safe_divide(int32_t a, int32_t b, int32_t *result)
{
    if (b == 0) {
        return 0;  /* 除数不能为 0 */
    }
    if (result != NULL) {
        *result = a / b;
    }
    return 1;  /* 成功 */
}

int main(void)
{
    int32_t res = 0;
    if (safe_divide(10, 3, &res)) {
        printf("10 / 3 = %d\n", (int)res);  /* 输出: 3 */
    }
    if (!safe_divide(10, 0, &res)) {
        /* 除零失败, res 保持原值 */
        printf("除零错误, res = %d (未修改)\n", (int)res);
    }
    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q：`*` 在声明里和在使用里含义不一样？**

A：对！这是 C 语言最容易混淆的地方之一：
- `int *p;` **声明**：`*` 表示 `p` 是一个「指向 int 的指针」类型
- `*p = 10;` **使用**：`*` 是解引用运算符，「找到 p 指向的那个变量，赋值 10」
- 一个是类型标记，一个是运行时操作。

**Q：什么是指针的「类型」？为什么不能所有指针都用 `void*`？**

A：指针类型告诉编译器两个重要信息：
1. **解引用范围**：`int32_t*` 解引用读 4 字节，`char*` 读 1 字节
2. **指针算术步长**：`p+1` 在 `int32_t*` 里前进 4 字节，在 `char*` 里前进 1 字节

`void*`（无类型指针）确实存在，但你不能直接 `*vp` 解引用——必须先转换为具体类型。

**Q：`int *p` 的 `*` 应该贴紧谁？`int *p` 还是 `int* p`？**

A：两种风格都可以，但要注意：**`*` 属于变量名，不是类型名**。

```c
int* a, b;   /* a 是 int*, b 是 int —— 不是两个指针！ */
int *a, *b;  /* 这才是两个 int* */
```

很多 C 程序员偏好 `int *p`（星号贴变量）来提醒自己 `*` 是变量修饰符。

**Q：指针和索引 `[]` 有什么关系？**

A：`arr[i]` 在 C 语言中本质上就是 `*(arr + i)` 的语法糖！

```c
int32_t arr[3] = {10, 20, 30};
/* 以下两种写法完全等价 */
printf("%d\n", arr[1]);    /* 20 */
printf("%d\n", *(arr + 1)); /* 20 */
```

## 知识扩展 (选学)

### 指向指针的指针 (Pointer to Pointer)

指针本身也是变量，也有地址。所以你可以声明一个指针指向另一个指针：

```c
int32_t  value = 42;
int32_t *ptr = &value;      /* ptr → value */
int32_t **pptr = &ptr;       /* pptr → ptr → value */

printf("%d\n", value);      /* 42 */
printf("%d\n", *ptr);       /* 42 */
printf("%d\n", **pptr);     /* 42 (两次解引用) */
```

这在以下场景非常有用：
- **函数内修改指针本身**（不是修改指针指向的值，而是修改指针的指向）
- **动态二维数组**（`char** argv` 就是命令行参数数组）
- **链表头节点的修改**

```c
/* 函数内修改指针指向的例子 */
void set_to_null(int32_t **pp)
{
    *pp = NULL;   /* 修改 pp 自身，不是它指向的值 */
}

int32_t *p = &some_value;
set_to_null(&p);  /* 现在 p 变成 NULL 了 */
```

### Python 变量 vs C 指针对比表

| 特性 | Python | C 语言指针 |
|------|--------|-----------|
| 赋值 | `b = a` 复制值 | `b = a` 复制值（相同） |
| 引用同一对象 | `b = a` 指向同一对象 | `*pb = *pa` 共享同一内存 |
| 地址可见性 | 不可见（解释器管理） | `&a` 显式地址 |
| 空引用 | `b = None` | `p = NULL` |
| 空检查 | `if b is not None` | `if (p != NULL)` |
| 类型系统 | 动态 | 静态，指针类型决定解范围 |

## 小结

本章的核心要点：

- **指针**是存储内存地址的变量，类型为 `T*`（`T` 是指向的类型）
- **`&`** 取地址运算符，`&var` 返回变量的地址
- **`*`** 解引用运算符，`*ptr` 访问指针指向的值
- **NULL 指针**表示不指向任何数据，使用前必须检查
- **野指针**（未初始化的指针）是危险的——始终初始化为有效地址或 NULL
- **指针类型**决定了解引用范围和指针算术步长
- `arr[i]` 本质上是 `*(arr + i)` 的语法糖

## 术语表

| 英文 | 中文 |
|------|------|
| Pointer | 指针 |
| Address | 地址 |
| Dereference | 解引用 |
| Address-of (`&`) | 取地址 |
| NULL pointer | NULL 指针 |
| Dangling pointer | 野指针 |
| Segmentation fault | 段错误 |
| Pointer type | 指针类型 |
| Indirection | 间接引用 |
| Memory layout | 内存布局 |
| Pointer arithmetic | 指针算术 |

## 延伸阅读

- [C17 标准 §6.5.3.2 — 解引用运算符](https://en.cppreference.com/w/c/language/operator_member_access) — 官方标准中的指针定义
- [cppreference - Pointers](https://en.cppreference.com/w/c/language/pointer) — 指针类型和操作的完整参考
- [Kernighan & Ritchie《The C Programming Language》§5.1-5.5](https://en.wikipedia.org/wiki/The_C_Programming_Language) — 经典指针章节

**选择建议**：初学者建议先阅读 cppreference 的指针章节理解基本概念；有一定基础后再看 K&R 经典教材的指针章节加深理解。

## 继续学习

本章你已经理解了 C 语言最核心的概念——指针。它是 C 语言强大但也容易出错的原因。下一步，我们将学习指针算术——如何用指针遍历数组、实现字符串操作、以及指针与数组的等价关系。

- [上一章](./datatype.md)：数据类型
- [下一章](./pointer-arith.md)：指针算术
