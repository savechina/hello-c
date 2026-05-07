# 指针与数组 (Pointers and Arrays)

## 开篇故事

想象一条笔直的街道，两旁是一排完全相同的 houses。每栋房子有一个门牌号（从 0 开始）。你知道第一栋的地址后，任何一栋的位置都可以通过简单加法算出来。

这正是 C 语言中**数组与指针的关系**。数组是连续的内存格子，指针是进入这条街道的入口。一旦你站在入口处，`arr[i]` 本质上就是「从入口前进 i 步后开门看里面的内容」——在 C 的底层，这等同于 `*(arr + i)`。

```c
int32_t arr[4] = {10, 20, 30, 40};
int32_t *p = arr;    // p 站在街道上 (arr[0] 的位置)

arr[2]      // 开门牌号 2 的房子 → 30
*(p + 2)    // 从 p 前进 2 步，开门 → 30 (完全等价！)
```

理解了这个等价性，你就打通了 C 语言最核心的任督二脉。

## 本章适合谁

- 已掌握指针基础（`&`、`*`、NULL）
- 刚学完数组，好奇「数组名是不是指针」
- 被数组越界和 `sizeof` 陷阱坑过的程序员
- 想理解 C 为什么比 Python 快的底层原因

## 你会学到什么

1. `arr[i]` 与 `*(arr + i)` 的等价关系——C 语言底层真相
2. 数组名退化为指针的规则和边界
3. `sizeof` 在数组 vs 指针上的致命差异（最频繁踩坑）
4. 用指针遍历数组（`for (p = arr; p < end; p++)`）
5. 二维数组的行优先内存布局
6. `i[arr] == arr[i]`——指针算术的可交换性
7. Python 列表索引 vs C 指针遍历对比

## 前置要求

- 已完成 [指针基础](./pointer_basics.md) 和 [数组](./arrays.md) 章节
- 理解数组是**连续内存**，索引从 0 开始
- 理解 `sizeof(array) / sizeof(element)` 计算元素个数

## ⚠️ 知识陷阱预警：数组名不是指针！

很多人第一次学 C 时会听到"数组名就是指针"这句话。这是**不准确的**——很多 C 教程都会犯这个错。

- **数组名**代表整个数组。`sizeof(arr)` 返回整个数组大小（4 个 int = 16 字节）。
- **指针**只是一个存储地址的变量。`sizeof(p)` 永远返回指针自身大小（64 位系统上 = 8 字节）。
- 当数组作为函数参数传递、或赋值给指针时，数组名**退化**为指针。这只发生在特定上下文中。

```c
int32_t arr[4] = {10, 20, 30, 40};
int32_t *p = arr;

sizeof(arr);  // = 16 (4 × 4) — ✅ 编译器知道整个数组
sizeof(p);    // = 8      — ❌ 只知道指针自身大小

/* ✅ 正确：在数组定义处计算长度 */
size_t len = sizeof(arr) / sizeof(arr[0]);  // = 4

/* ❌ 错误：传给函数后数组名退化为指针，sizeof 失效 */
void process(int32_t *data) {
    size_t n = sizeof(data) / sizeof(data[0]);  // = 2 (错！应该是 4)
}
```

**记住：数组名在大多数表达式中"退化为指针"，但 `sizeof` 是唯一能区分它们的上下文。**

另外，C99 引入了变长数组 (VLA)，`sizeof(vla)` 在运行时计算——这与普通数组在编译期计算不同。VLA 不能初始化，也不能在结构体中使用。

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t arr[4] = {10, 20, 30, 40};

    /* 索引写法 */
    printf("arr[2]     = %d\n", arr[2]);

    /* 指针写法——完全等价 */
    printf("*(arr + 2) = %d\n", *(arr + 2));

    /* 地址也相同 */
    printf("&arr[1]    = %p\n", (void *)&arr[1]);
    printf("arr + 1    = %p\n", (void *)(arr + 1));

    return 0;
}
```

输出：

```
arr[2]     = 30
*(arr + 2) = 30
&arr[1]    = 0x7ffee4c4a3a4
arr + 1    = 0x7ffee4c4a3a4
```

核心发现：`arr[i]` 和 `*(arr + i)` 产生的机器码**完全一样**。编译器在背后做了同一件事。

## 原理解析

### 1. `arr[i] ≡ *(arr + i)` —— 底层等价

C 标准明确规定：`a[e]` 等价于 `(*((a) + (e)))`。

```
  int32_t arr[4] = {7, 14, 21, 28}

  内存布局 (每个元素 4 字节):
  ┌────┬────┬────┬────┐
  │ 07 │ 0E │ 15 │ 1C │  ← 十六进制值
  └────┴────┴────┴────┘
    ↑     ↑     ↑     ↑
  arr+0 arr+1 arr+2 arr+3
  &a[0]  &a[1]  &a[2]  &a[3]

  arr[2]     → *(arr + 2) → 找到第三个元素 → 21 (十进制 33, 十六进制 0x15)
  *(2 + arr) → 同样的计算，结果相同
  2[arr]     → 居然合法！因为加法可交换
```

### 2. 数组名 vs 指针——关键区别

数组名是**不可修改的地址常量**，指针是**可重赋值的变量**：

```c
int32_t arr[5] = {0};
int32_t *p = arr;

/* 指针可以重新赋值 */
p = arr + 2;   /* ✅ p 现在指向 arr[2] */

/* 数组名不能赋值 */
arr = p;       /* ❌ 编译错误: 数组名是常量，不可修改 */
```

| 特性 | 数组名 `arr` | 指针变量 `*p` |
|------|------------|--------------|
| 本质 | 地址常量 | 变量 |
| 可重赋值 | ❌ | ✅ |
| `sizeof(arr)` | 整个数组大小 | 指针大小 (8 字节) |
| `sizeof(*p)` | N/A | 单个元素大小 |

### 3. `sizeof` 陷阱——数组 vs 指针

这是学到指针时**最多人踩的坑**：

```c
int32_t data[8] = {0};
int32_t *ptr = data;

sizeof(data)    // = 32  (8 × 4 字节)
sizeof(ptr)     // = 8   (指针自身大小，不是数组!)

sizeof(data[0]) // = 4
sizeof(*ptr)    // = 4

// 计算元素个数
sizeof(data) / sizeof(data[0])  // = 32/4 = 8 ✅
sizeof(ptr) / sizeof(*ptr)      // = 8/4 = 2  ❌ 错误!
```

**黄金法则**：一旦数组被赋值给指针（或传给函数），`sizeof` 再也无法知道原始数组大小。

### 4. 用指针遍历数组

```c
int32_t nums[5] = {10, 20, 30, 40, 50};
int32_t *p = nums;
int32_t *end = nums + 5;    /* end 指向数组末尾之后——合法! */

/* 索引法 */
for (int32_t i = 0; i < 5; i++) {
    printf("%d ", nums[i]);
}

/* 指针法 */
for (int32_t *q = p; q < end; q++) {
    printf("%d ", *q);
}
```

两种写法在现代编译器优化后生成**相同的汇编代码**。指针写法是风格偏好，不是性能优势。

### 5. 二维数组——行优先连续内存

```c
int32_t matrix[2][3] = {
    {1, 2, 3},
    {4, 5, 6}
};
```

C 的二维数组在内存中是**一行接一行**的连续块：

```
  内存布局:
  [1][2][3][4][5][6]
   ← row 0 →← row 1 →

  matrix[row][col] 与 *( *(matrix + row) + col ) 完全等价
```

二维数组名 `matrix` 的类型是 `int32_t (*)[3]`（指向包含 3 个元素的数组的指针），不是 `int32_t **`。

**常见误区**：

```c
int32_t matrix[2][3];
int32_t **pp = matrix;   /* ❌ 类型不匹配! */
int32_t (*pa)[3] = matrix; /* ✅ 正确: pa 指向一个长度为 3 的数组 */
```

### 6. Python 列表 vs C 数组指针对比

| 特性 | Python `list` | C 数组/指针 |
|------|--------------|------------|
| 索引 | `a[i]` 自动边界检查 | `*(a+i)` 无检查，越界 = UB |
| 长度 | `len(a)` O(1) | `sizeof(arr)/sizeof(arr[0])`（仅数组本身） |
| 底层 | 对象包装 + 动态 | 裸内存地址 + 算数 |
| 赋值 | `b = a` 共享引用 | `p = arr` 指针复制地址 |
| slice | `a[1:3]` 安全 | 需要手动 `p+1` 到 `p+3` |

Python 替你做好了所有安全检查，C 把选择权（和责任）交给你。

## 常见错误

### ❌ 错误 1：`sizeof` 数组退化后求长度

```c
void process(int32_t *data) {
    /* ❌ 在函数内 sizeof 得到的是指针大小 */
    size_t len = sizeof(data) / sizeof(data[0]);  // = 8/4 = 2
}

int32_t arr[10] = {0};
process(arr);  /* arr 退化为 int32_t* */
```

```c
/* ✅ 修复: 把长度作为参数传入 */
void process(int32_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        /* 安全遍历 */
    }
}
```

### ❌ 错误 2：混淆二维数组和指针的指针

```c
int32_t matrix[3][3];
int32_t **p = matrix;  /* ❌ matrix 不是 int32_t** */
```

```c
/* ✅ 正确 */
int32_t (*p)[3] = matrix;   /* p 指向「包含 3 个 int32_t 的数组」 */
/* 或 */
int32_t *flat = &matrix[0][0];  /* 展平为一维指针 */
```

### ❌ 错误 3：越界指针解引用

```c
int32_t arr[4] = {1, 2, 3, 4};
int32_t *p = arr;

p = p + 5;    /* 越界 beyond (arr+4 是合法的，arr+5 不是) */
*p = 99;      /* ❌ 未定义行为 */
```

```c
/* ✅ 始终用 end 指针控制范围 */
int32_t *end = arr + 4;
for (int32_t *p = arr; p < end; p++) {
    *p = 0;   /* 安全 */
}
```

## 动手练习

### 🟢 入门：索引 vs 指针对照

用两种方式打印 `{10, 20, 30, 40, 50}`：先用 `arr[i]`，再用 `*(arr + i)`，验证结果相同。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int32_t arr[5] = {10, 20, 30, 40, 50};
    for (int32_t i = 0; i < 5; i++) {
        printf("arr[%-2d] = %2" PRId32 "  ==  *(arr+%2d) = %2" PRId32 "\n",
               i, arr[i], i, *(arr + i));
    }
    return 0;
}
```

</details>

### 🟡 中级：指针遍历 + 求和

用**指针**（不用 `[]`）遍历 `{3, 1, 4, 1, 5, 9, 2, 6}` 并计算总和。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t data[] = {3, 1, 4, 1, 5, 9, 2, 6};
    int32_t n = (int32_t)(sizeof(data) / sizeof(data[0]));

    int64_t sum = 0;
    int32_t *end = data + n;
    for (int32_t *p = data; p < end; p++) {
        sum += *p;
    }
    printf("sum = %ld\n", (long)sum);  /* 31 */
    return 0;
}
```

</details>

### 🔴 挑战：二维数组指针遍历

用指针（不用 `[]`）遍历 `int32_t m[2][3] = {{1,2,3},{4,5,6}}`，按行打印每个元素。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int32_t m[2][3] = {{1, 2, 3}, {4, 5, 6}};

    for (int32_t r = 0; r < 2; r++) {
        for (int32_t c = 0; c < 3; c++) {
            /* *(*(m + r) + c) */
            printf("%2" PRId32 " ", *(*(m + r) + c));
        }
        printf("\n");
    }
    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q：`sizeof` 为什么在函数里对参数数组不生效？**

A：因为函数参数 `int32_t arr[]` **等同于** `int32_t *arr`——数组名已经退化为指针。`sizeof` 只能看到指针本身的大小（8 字节），不是整个数组。

**Q：`arr[2]` 和 `2[arr]` 真的一样？**

A：一样。因为 `arr[2]` → `*(arr + 2)` → `*(2 + arr)` → `2[arr]`。C 标准定义了这种对称性。虽然合法，但请不要在代码里写 `2[arr]` 😅

**Q：二维数组名 `int32_t m[2][3]` 是一维指针吗？**

A：不是。它的类型是 `int32_t (*)[3]`（指向含 3 个元素的数组的指针）。要展平为一维，用 `&m[0][0]`。

## 知识扩展

### 指针 vs 索引：性能真相

```c
/* 索引方式 */
for (int32_t i = 0; i < n; i++) sum += arr[i];

/* 指针方式 */
for (int32_t *p = arr; p < end; p++) sum += *p;
```

在 x86-64 + `-O2` 下，GCC 和 Clang 生成**完全相同的优化汇编**。指针的「更快」优势是 20 年前的历史，现在纯属风格选择。

### 指针减法

同数组的两个指针相减，得到**元素个数**（不是字节数）：

```c
int32_t data[7] = {10, 20, 30, 40, 50, 60, 70};
int32_t *head = &data[0];
int32_t *tail = &data[6];

ptrdiff_t dist = tail - head;  /* 6 个元素 */
```

使用 `ptrdiff_t` 类型（定义在 `<stddef.h>`），保证是足够大的带符号整数。

## 小结

- `arr[i]` 与 `*(arr + i)` **完全等价**——编译器生成相同代码
- 数组名是**地址常量**，指针是**可赋值变量**
- `sizeof` 只对**数组本身**有效——传给函数后退化为指针
- 二维数组 = 连续行优先内存，类型是 `T(*)[N]` 不是 `T**`
- 越界指针不报错——自己负责边界

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 数组名衰减 | Array decay | 数组名自动转为指针的现象 |
| 指针算术 | Pointer arithmetic | 指针 + 整数 = 前进若干元素 |
| sizeof 陷阱 | sizeof pitfall | sizeof(指针) ≠ sizeof(数组) |
| 行优先 | Row-major | 二维数组按行连续存储 |
| 指针减法 | Pointer subtraction | 两指针相减 = 元素间距 |

## 延伸阅读

- [cppreference - Pointer arithmetic](https://en.cppreference.com/w/c/language/pointer_expression)
- [C11 标准 §6.5.2.1 — 数组下标](https://en.cppreference.com/w/c/language/array_initialization)

## 继续学习

- [上一章](./void_pointers.md)：void* 指针——万能指针与类型擦除
- [下一章](./pointers_and_functions.md)：指针与函数——传递引用、返回指针

---

> 本章代码位于 [`src/basic/pointers_and_arrays_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/pointers_and_arrays_sample.c)。
