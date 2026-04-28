# 指针基础 (&, *, NULL, 初始化)

## 开篇故事

指针像 GPS 坐标——它不是目的地，而是告诉你目的地的方向。

想象你在一个陌生的城市旅行。你不需要亲自走到每一条街上，你只需要一个坐标——经度和纬度。输入坐标，导航就会带你到达。指针在 C 语言中做的事情完全一样：它不存储数据本身，它存储的是数据在内存中的"坐标"。

```c
int32_t destination = 42;      // 目的地本身
int32_t *gps = &destination;   // 指向目的地的坐标
int32_t value = *gps;          // 顺着坐标找到目的地，取出值 = 42
```

很多人第一次看到 `*` 和 `&` 就头皮发麻。其实它们做的事情很朴素：一个告诉你「去哪找」，一个帮你「找到以后打开看」。理解了这一点，指针就不再是神秘的咒语，而是 C 语言给你的一把手术刀——锋利，但握对了就不怕受伤。

## 本章适合谁

- 已经了解 C 变量和数据类型（`int32_t`、`double` 等）
- 听说过「指针」但总觉得神秘、怕踩坑
- 用过 Python/JavaScript 等高级语言，想了解 C 的内存控制能力
- 被「段错误 (Segmentation Fault)」折磨过的初学者

## 你会学到什么

1. `&` 取地址运算符和 `*` 解引用运算符的本质含义
2. 指针声明语法：`int32_t *p` 中 `*` 属于变量而非类型
3. NULL 指针的含义、 dangers 以及安全检查模式
4. 如何正确初始化指针，彻底杜绝野指针 (Dangling Pointer)
5. 指针类型如何决定编译器解释内存的方式
6. Python 变量赋值 vs C 指针赋值的认知对照

## 前置要求

- 已完成 [变量](./variables.md) 和 [数据类型](./datatype.md) 章节
- 理解变量是内存中的命名存储空间
- 理解 `<stdint.h>` 中的固定宽度类型（`int32_t`、`int64_t` 等）

## 第一个例子

最简短的指针演示程序——声明一个变量和指向它的指针：

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t  num = 42;
    int32_t *ptr = &num;   // ptr 存储 num 的「GPS 坐标」

    printf("num 的值  = %d\n", num);
    printf("num 的地址 = %p\n", (void *)&num);
    printf("ptr 的值  = %p  (和 num 的地址相同)\n", (void *)ptr);
    printf("*ptr 的值 = %d  (解引用得到 num 的值)\n", *ptr);

    *ptr = 100;            // 顺着 GPS 坐标修改目的地
    printf("修改 *ptr 后, num = %d\n", num);

    return 0;
}
```

编译运行：

```bash
gcc -Wall -Wextra -Werror -std=c17 -o demo demo.c
./demo
```

输出：

```
num 的值  = 42
num 的地址 = 0x7ffee4c4a3ac
ptr 的值  = 0x7ffee4c4a3ac  (和 num 的地址相同)
*ptr 的值 = 42  (解引用得到 num 的值)
修改 *ptr 后, num = 100
```

这段代码揭示了 C 指针的核心模式：
- `&num` → 获取 num 的地址（GPS 坐标）
- `int32_t *ptr` → 声明一个「指向 int32_t」的指针变量
- `*ptr` → 解引用：顺着坐标找到 num，读取或修改它的值

## 原理解析

### 1. `&` 取地址：获取变量的 GPS 坐标

每一个变量在内存中都有一个确定的地址。`&` 运算符返回这个地址：

```c
int32_t x = 42;
printf("%p\n", (void *)&x);  // 打印 x 在内存中的地址
```

`(void *)` 转换是因为 `%p` 格式说明符要求 `void*` 类型参数。

### 2. `*` 解引用：顺着坐标找到数据

`*` 运算符「跟随」指针中的地址，访问那个地址上的数据：

```c
int32_t x = 42;
int32_t *p = &x;

printf("%d\n", *p);   // 42 — 顺着 p 找到 x
*p = 99;              // 修改 p 指向的数据 → x 变成 99
```

### 3. 内存布局 ASCII 图

理解指针最直观的方式是看内存图：

```
  栈内存 (Stack Memory)

  ┌──────────────────────────────┐
  │ 符号    │ 地址       │ 值      │
  ├──────────────────────────────┤
  │ x       │ 0x7ff…b0   │ 42      │  ← 实际数据
  │ p       │ 0x7ff…b8   │ 0x7ff…b0 │  ← 指针存 x 的地址
  └──────────────────────────────┘
                         │
                    *p → ┘  解引用: 取出 p 的值 (0x7ff…b0)，
                         再到那个地址取值 → 42
```

**关键认知**：
- `x` 在 `0x7ff…b0`，存放数值 42
- `p` 在 `0x7ff…b8`，存放的是 `0x7ff…b0`（`&x`）
- `*p` = 取出 `p` 中的地址 → 到那个地址取值 = 42
- `&p` = p 自己的地址 = `0x7ff…b8`（指针本身也有地址！）

### 4. NULL 指针与安全检查

`NULL` 是一个特殊地址值（通常是 `0`），表示「不指向任何有效数据」。

**永远不要解引用 NULL 指针**——会导致段错误：

```c
int32_t *ptr = NULL;
printf("%d\n", *ptr);  /* ❌ Segmentation fault! 程序崩溃 */
```

**正确模式**：使用前检查

```c
int32_t target = 42;
int32_t *ptr = NULL;

/* 某个操作可能给 ptr 赋值 */
ptr = &target;

if (ptr != NULL) {
    printf("%d\n", *ptr);  /* ✅ 安全 */
} else {
    printf("ptr 尚未初始化\n");
}
```

### 5. 指针初始化——拒绝野指针

**野指针 (Dangling Pointer)** 是最常见的指针错误：

```c
int32_t *p;    /* ❌ 未初始化！p 指向随机地址 */
*p = 42;       /* ❌ 向随机内存写入 = 崩溃 或 数据损坏 */
```

**两种安全的初始化方式**：

```c
int32_t val = 42;
int32_t *p1 = &val;  /* ✅ 指向有效变量 */
int32_t *p2 = NULL;  /* ✅ 明确指向空 */
```

黄金法则：**声明指针时必须初始化**——要么指向确定的地址，要么初始化为 NULL。

### 6. 指针类型决定步长

指针的**类型**告诉编译器两件事：解引用时读多少字节，指针加法时前进多少字节。

```c
int32_t  iv = 0x01020304;
int32_t *pi = &iv;    /* pi 每次 +1 前进 4 字节 */
uint8_t *pb = (uint8_t *)&iv;  /* pb 每次 +1 前进 1 字节 */
```

| 指针类型 | `p + 1` 移动 | 解引用大小 | 适用场景 |
|----------|-------------|-----------|---------|
| `int8_t *` | 1 字节 | 1 字节 | 逐字节操作 |
| `int32_t *` | 4 字节 | 4 字节 | 整数数组 |
| `int64_t *` | 8 字节 | 8 字节 | 长整数 |
| `double *` | 8 字节 | 8 字节 | 浮点数组 |
| `char *` | 1 字节 | 1 字节 | 字符串 |

## 常见错误

### ❌ 错误 1：解引用未初始化的指针

```c
int32_t *p;      /* 未初始化 */
*p = 42;         /* ❌ 写入随机内存! */
```

编译器可能报 `uninitialized` 警告，但运行时会**段错误**或产生隐蔽的数据损坏。

```c
/* ✅ 修复：初始化指针 */
int32_t  val = 0;
int32_t *p = &val;
*p = 42;         /* ✅ 安全 */
```

### ❌ 错误 2：解引用 NULL

```c
int32_t *p = NULL;
printf("%d\n", *p);   /* ❌ 段错误! */
```

```c
/* ✅ 修复：检查 NULL */
if (p != NULL) {
    printf("%d\n", *p);
}
```

### ❌ 错误 3：混淆 `*p = value` 与 `p = &value`

```c
int32_t a = 10;
int32_t b = 20;
int32_t *p = &a;

p = &b;   /* ❌ 这是让 p 改指向 b，不是修改 a */
printf("%d\n", a);   /* a 还是 10! 没被修改 */
```

```c
/* ✅ 如果想通过指针修改 a 的值 */
int32_t *p = &a;
*p = 20;   /* *p = a，现在 a = 20 */
```

## 动手练习

### 🟢 入门：GPS 导航——用指针读取和修改

声明 `int32_t x = 100`，创建指针 `p` 指向它，用 `*p` 把值改为 200，打印验证 `x` 已被修改。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t x = 100;
    int32_t *p = &x;

    printf("修改前: x = %" PRId32 ", *p = %" PRId32 "\n", x, *p);
    *p = 200;
    printf("修改后: x = %" PRId32 ", *p = %" PRId32 "\n", x, *p);
    return 0;
}
```

**输出**:
```
修改前: x = 100, *p = 100
修改后: x = 200, *p = 200
```

</details>

### 🟡 中级：NULL 防御式编程

编写一个函数 `int safe_read(int32_t *p, int32_t *out)`，当 `p` 为 NULL 时返回 0，否则将 `*p` 复制到 `out` 并返回 1。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int safe_read(int32_t *p, int32_t *out)
{
    if (p == NULL) return 0;
    if (out == NULL) return 0;
    *out = *p;
    return 1;
}

int main(void) {
    int32_t val = 42;
    int32_t result = 0;

    if (safe_read(&val, &result)) {
        printf("读取成功: %" PRId32 "\n", result);
    }
    if (!safe_read(NULL, &result)) {
        printf("NULL 指针安全拦截\n");
    }
    return 0;
}
```

</details>

### 🔴 挑战：追踪指针轨迹

声明三个变量 `a = 1, b = 2, c = 3`，用指针 `p` 依次指向它们，每次指向后用 `*p` 打印值。最后用二级指针 `pp` 指向 `p`，通过 `**pp` 获取值。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int32_t a = 1, b = 2, c = 3;
    int32_t *p = &a;

    printf("*p (指向 a) = %" PRId32 "\n", *p);
    p = &b;
    printf("*p (指向 b) = %" PRId32 "\n", *p);
    p = &c;
    printf("*p (指向 c) = %" PRId32 "\n", *p);

    int32_t **pp = &p;
    printf("**pp = %" PRId32 " (通过二级指针间接获取)\n", **pp);
    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q：`*` 在声明里和在使用里含义不一样？**

A：对！这是 C 最著名的混淆点：
- `int *p;`**声明**：`*` 表示 `p` 是「指向 int 的指针」类型
- `*p = 10;`**使用**：`*` 是解引用运算符——找到 p 指向的变量，赋值 10

一个是类型标记，一个是运行时操作。

**Q：`int *p` 的 `*` 应该贴紧谁？`int *p` 还是 `int* p`？**

A：两种风格都可以，但记住：**`*` 属于变量名，不是类型名**。

```c
int* a, b;   /* a 是 int*, b 是 int —— 不是两个指针！ */
int *a, *b;  /* ✅ 这才是两个 int* */
```

C 程序员普遍偏好 `int *p`（星号贴变量）来提醒自己 `*` 是变量修饰符。

**Q：指针的「类型」为什么不能都用 `void*`？**

A：指针类型决定了：
1. **解引用大小**：`int32_t*` 读 4 字节，`char*` 读 1 字节
2. **指针算术步长**：`p+1` 在 `int32_t*` 前进 4 字节，在 `char*` 前进 1 字节

`void*` 没有类型信息，不能直接 `*vp` 解引用。

## 知识扩展

### 指向指针的指针 (二级指针)

指针本身也是变量，也有地址。所以可以声明一个指针指向另一个指针：

```c
int32_t  value = 42;
int32_t *ptr   = &value;
int32_t **pp   = &ptr;     /* pp → ptr → value */

printf("%d\n", **pp);      /* 42 (两次解引用) */
```

常见场景：
- **函数返回多个值**（如 `scanf("%d", &x)` 里的 `&x`）
- **动态二维数组**（`char** argv` 命令行参数）
- **修改指针本身**（而非它指向的值）

### 内存地址的可视化

```
  堆栈布局 (64 位系统)

  高地址 ┌──────────────┐
        │  main 帧        │
        │  value = 42     │  0x7ff...a0
        │  ptr = 0x7ff…a0 │  0x7ff...a8  ← 8 字节
        │  pp  = 0x7ff…a8 │  0x7ff...b0  ← 8 字节
  低地址 └──────────────┘

  sizeof(int32_t*) = 8  (64 位指针)
  sizeof(int32_t)  = 4
```

## 小结

- **指针**是存储内存地址的变量，类型为 `T*`
- **`&`** 取地址运算符，返回变量地址
- **`*`** 解引用运算符，通过地址访问值
- **NULL 指针**表示无效地址，使用前必须检查
- **野指针**（未初始化）是危险的——始终初始化为有效地址或 NULL
- **指针类型**决定解引用范围和步长

## 术语表

| 英文 | 中文 |
|------|------|
| Pointer | 指针 |
| Address-of (`&`) | 取地址 |
| Dereference (`*`) | 解引用 |
| NULL pointer | NULL 指针 |
| Dangling pointer | 野指针 |
| Segmentation fault | 段错误 |
| Pointer type | 指针类型 |
| Indirection | 间接引用 |
| Memory layout | 内存布局 |

## 延伸阅读

- [cppreference - Pointers](https://en.cppreference.com/w/c/language/pointer) — 指针类型和操作参考
- [C17 标准 §6.5.3.2 — 解引用运算符](https://en.cppreference.com/w/c/language/operator_member_access)

## 继续学习

- [上一章](./datatype.md)：数据类型
- [下一章](./pointer_arith.md)：指针运算——数组等价性、指针算术

---

> 本章代码位于 [`src/basic/pointer_basics_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/pointer_basics_sample.c)。
