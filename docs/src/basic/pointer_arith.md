# 指针运算 (Pointer Arithmetic)

在上一章，我知道了指针就像一个"遥控器"——它存储了内存地址，通过 `*` 可以读/写那个地址上的值。但指针真正强大的地方还在于：它**可以移动**。

```c
int32_t arr[5] = {1, 2, 3, 4, 5};
int32_t *p = arr;   // p 指向 arr[0]
p++;                // p 现在指向 arr[1]！
*p;                 // 值是 2
```

这种能力称为**指针算术 (Pointer Arithmetic)**——它是 C 语言高效操作的秘密武器，也是初学者的头号陷阱。

## 开篇故事

想象你走进一条走廊，两边是一扇扇编号连续的门。你站在第一扇门前，往前走一步就到了第二扇门，再走一步是第三扇。每一步的大小取决于门本身的宽度——窄门一步就跨过去，宽门需要多花一点力气。

指针算术就是在这条走廊里「走路」。指针加 1 不是地址加 1，而是移动「一个元素」的距离。`int32_t*` 走一步跳过 4 字节，`int64_t*` 走一步跳过 8 字节。步长由类型决定。

```c
int32_t arr[4] = {10, 20, 30, 40};
int32_t *p = arr;    // 站在第一扇门前 (arr[0])
p++;                 // 往前走一步，现在站在 arr[1] 门前
*p;                  // 打开门一看，值是 20
```

指针算术的危险之处在于：走廊走到尽头之后还有空间，但那已经不属于这排房间了。越过数组边界继续走，你读到的就不再是有效数据，而是走廊尽头的杂物间——编译器不会阻止你，后果自己承担。

> "越界的指针不会报错，只会给你一段随机内存。这也是 C 语言的信任哲学。"

## 本章适合谁

- 已经了解指针基础（`&` 取地址、`*` 解引用）
- 刚学完数组，好奇"数组名是不是指针"
- 被段错误（Segmentation Fault）折磨过的程序员
- 想知道 C 为什么比 Python 快的底层原因

## 你会学到什么

- 指针的 `++`、`--`、`+n`、`-n` 运算及其步长规则
- 数组与指针的等价性：`arr[i]` ≡ `*(arr + i)`
- `sizeof` 在数组和指针上的区别（最易踩坑）
- 指针相减：计算两个元素之间的距离
- 指针比较：`>`、`<`、`==`、`!=` 的含义
- 常见陷阱：越界指针、无符号字节指针

## 前置要求

完成 [指针基础 (Pointers)](./pointers.md) 和 [数组 (Arrays)](./arrays.md) 章节。

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t nums[5] = {10, 20, 30, 40, 50};
    int32_t *p = nums;  // p 指向 nums[0]

    printf("%d\n", *p);     // 10
    p++;                    // p 前移 1 个 int32_t
    printf("%d\n", *p);     // 20
    p = p + 2;              // p 再前移 2 个 int32_t
    printf("%d\n", *p);     // 50
    return 0;
}
```

输出：
```
10
20
50
```

### 分步解析

1. `int32_t *p = nums`：`nums` 是数组名，代表数组首地址，赋值给指针 `p`
2. `*p`：解引用，得到 `nums[0]` 的值（10）
3. `p++`：指针自增，移动 `sizeof(int32_t)` = 4 字节，指向 `nums[1]`
4. `p = p + 2`：指针加 2，再前进 2 个 `int32_t` 位置（8 字节），指向 `nums[3]`

## 原理解析

### 指针加 1，究竟移动多少？

这是指针算术最核心的规则：**指针 +1 不是地址 +1，而是移动一个「元素」的距离**。

```
假设 int32_t* p 起始地址 = 0x1000

     操作      地址变化              指向
   ────────────────────────────────────────────
     p         0x1000         →  nums[0]
     p+1       0x1000 + 4     →  nums[1]  (移动 4 字节)
     p+2       0x1000 + 8     →  nums[2]
     p+n       0x1000 + n×4   →  nums[n]
```

移动的距离 = `元素的 sizeof`。

| 指针类型 | `p + 1` 移动 | `p + 3` 移动 |
|----------|-------------|-------------|
| `int8_t *` | 1 字节 | 3 字节 |
| `int32_t *` | 4 字节 | 12 字节 |
| `int64_t *` | 8 字节 | 24 字节 |
| `double *` | 8 字节 | 24 字节 |
| `char *` | 1 字节 | 3 字节 |

> 💡 编译器在编译时自动根据指针类型计算偏移量，你不需要手动算。

### 数组与指针的等价性

C 语言中，数组访问在底层**就是**指针算术：

```c
int32_t a[4] = {7, 14, 21, 28};

a[2]      // ≡  *(a + 2)     完全等价！
&a[1]     // ≡   a + 1        地址相同！
```

```
内存布局 (每个元素 4 字节):
┌──────┬──────┬──────┬──────┐
│  07  │  0E  │  15  │  1C  │  ← 十六进制值
└──────┴──────┴──────┴──────┘
  ↑        ↑        ↑        ↑
a+0      a+1      a+2      a+3
&a[0]    &a[1]    &a[2]    &a[3]
```

这意味着：用 `a[i]` 编写的代码，编译器内部会翻译为 `*(a + i)`。

### sizeof：数组 vs 指针

这是我学到指针时最容易混淆的地方。`sizeof` 作用于数组名和指针，结果完全不同：

```c
int32_t arr[8] = {0};
int32_t *ptr = arr;

sizeof(arr)     // = 32  (8 个元素 × 4 字节)
sizeof(ptr)     // = 8   (指针本身的大小)

sizeof(arr[0])  // = 4   (单个 int32_t)
sizeof(*ptr)    // = 4   (解引用后是单个元素)

// 计算元素个数：只对数组有效
sizeof(arr) / sizeof(arr[0])  // = 32 / 4 = 8 ✓
sizeof(ptr) / sizeof(*ptr)    // = 8 / 4 = 2 ✗ （错误！）
```

**关键规则**：一旦数组被赋值给指针变量（或作为函数参数传递），`sizeof` 就再也无法知道数组的实际大小。

### 指针相减：计算距离

两个同类型指针相减，得到它们之间的**元素个数**（不是字节数）：

```c
int32_t nums[6] = {10, 20, 30, 40, 50, 60};
int32_t *start = &nums[0];
int32_t *end   = &nums[5];

ptrdiff_t dist = end - start;  // dist = 5（元素数）
                               // 字节数 = 5 × 4 = 20
```

结果类型是 `ptrdiff_t`（定义在 `<stddef.h>`），它是一个有符号整数，保证能表示任何合法指针差值。

### 指针比较

同一数组内的指针可以比较大小：

```c
int32_t vals[5] = {50, 40, 30, 20, 10};
int32_t *p1 = &vals[0];
int32_t *p2 = &vals[4];

p1 < p2   // true，p1 在内存中更"靠前"
p1 == p2  // false
p1 == p2  // false（不同的元素地址不同）
```

比较指针大小等价于比较它们指向的元素在数组中的位置。`p1 < p2` 意味着 p1 指向的元素比 p2 更早出现在数组中。

## 常见错误

### ❌ 错误 1：指针越界——最危险的陷阱

```c
int32_t arr[4] = {10, 20, 30, 40};
int32_t *p = arr;

p = p + 4;   // p 已经越过了整个数组
*p = 999;    // ❌ 向未知内存写入！可能崩溃，可能 corrupt 数据
```

**修复**：始终用 `<` 控制指针范围：

```c
int32_t *p = arr;
int32_t *end = arr + 4;

for (; p < end; p++) {
    printf("%d ", *p);  // 安全遍历
}
```

> 注意：`p = arr + 4`（指向最后一个元素之后）是合法的，但不能解引用它。`p = arr + 5` 则是未定义行为。

### ❌ 错误 2：用 sizeof 获取指针指向的数组长度

```c
int32_t *ptr = arr;
size_t count = sizeof(ptr) / sizeof(*ptr);  // ✗ 得到 1 或 2，不是 8！
```

**修复**——在调用处计算好再传指针：

```c
void process(int32_t *data, size_t count) {
    // count 由调用者传入
    for (size_t i = 0; i < count; i++) {
        printf("%d\n", data[i]);
    }
}

int main(void) {
    int32_t arr[8] = {0};
    process(arr, sizeof(arr) / sizeof(arr[0]));
}
```

### ❌ 错误 3：比较不同数组的指针

```c
int32_t a[5];
int32_t b[5];
int32_t *p1 = a;
int32_t *p2 = b;

if (p1 < p2) { }  // ❌ 未定义行为！不同数组之间的地址比较无意义
```

**修复**：只比较同一数组内的指针。

## 动手练习

<details>
<summary>🟢 入门: 用指针遍历数组</summary>

用指针（不用 `[]`）打印 `{1, 2, 3, 4, 5}` 的所有元素。

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t arr[5] = {1, 2, 3, 4, 5};
    int32_t *p = arr;

    for (int32_t i = 0; i < 5; i++) {
        printf("%d ", *(p + i));
    }
    printf("\n");
    return 0;
}
```

**输出**: `1 2 3 4 5`

</details>

<details>
<summary>🟡 中级: 用指针相减求长度</summary>

给定两个指针指向同一数组的两端，用减法计算元素个数。

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

int main(void) {
    int32_t data[7] = {10, 20, 30, 40, 50, 60, 70};
    int32_t *head = &data[0];
    int32_t *tail = &data[6];

    ptrdiff_t count = tail - head;
    printf("距离 = %td 个元素\n", count);
    return 0;
}
```

**输出**: `距离 = 6 个元素`

</details>

<details>
<summary>🔴 挑战: 指针实现二分查找</summary>

用指针实现二分查找，不用 `[]` 索引。

```c
#include <stdio.h>
#include <stdint.h>

int32_t *binary_search(int32_t *first, int32_t *last, int32_t target) {
    while (first <= last) {
        int32_t *mid = first + (last - first) / 2;
        if (*mid == target) return mid;
        else if (*mid < target) first = mid + 1;
        else last = mid - 1;
    }
    return NULL;  // 未找到
}

int main(void) {
    int32_t arr[7] = {2, 5, 8, 12, 16, 23, 38};
    int32_t n = 7;
    int32_t *found = binary_search(arr, arr + n - 1, 16);
    if (found) {
        printf("找到! 索引 = %td\n", found - arr);
    }
    return 0;
}
```

**输出**: `找到! 索引 = 4`

</details>

## 故障排查 (FAQ)

<details>
<summary>Q: 为什么 `p++` 只前进 4（或 8）字节，而不是 1 字节？</summary>

A: C 的指针算术是**类型感知**的。`int32_t*` 的 "1" 代表"1 个 int32_t 元素"，即 4 字节。如果你需要逐字节移动，使用 `int8_t*` 或 `uint8_t*`。

</details>

<details>
<summary>Q: 数组名和指针到底有什么不同？</summary>

A: 数组名是一个**不可修改的地址常量**，它始终指向数组首元素。指针是一个**变量**，可以重新赋值。

```c
int32_t arr[5] = {0};
int32_t *p = arr;

arr = p;     // ❌ 编译错误：数组名不能赋值
p = arr + 2; // ✅ 指针可以重新赋值
```

但在大多数表达式中（除 `sizeof` 和 `&` 外），数组名会**退化为指针**。

</details>

<details>
<summary>Q: Python 的列表索引和 C 的指针有什么区别？</summary>

A: Python 的 `list[i]` 做了大量边界检查（越界抛 `IndexError`），C 的 `*(arr + i)` 不做任何检查，越界 = 未定义行为。

| Python | C |
|--------|---|
| `a[i]` 自动检查 `0 ≤ i < len(a)` | `*(a+i)` 零检查，越界 = UB |
| `slice` 安全 | 指针范围需要手动维护 |
| 列表有长度属性 | 数组传参后丢失长度信息 |

C 更快但需要你自己负责安全。

</details>

## 知识扩展 (选学)

### 指针 vs 索引：谁更快？

```c
// 索引方式
for (int i = 0; i < n; i++) sum += arr[i];

// 指针方式
for (int32_t *p = arr; p < end; p++) sum += *p;
```

现代编译器（GCC/Clang）优化后两种方式生成的汇编通常是**完全相同**的。指针写法的"更快"优势在 20 年前可能成立，现在更多是风格偏好。

### void* 指针：无类型的指针

```c
int32_t x = 42;
void *vp = &x;  // 可以指向任何类型
int32_t *ip = vp;  // 需要显式转回去
int val = *ip;  // 解引用必须转回具体类型
```

`void*` 不能直接 `*vp` 或 `vp++`（因为不知道元素大小），必须先转换为具体类型指针。

## 小结

这一章我发现：

- 指针 +1 移动的是**一个元素的大小**，不是 1 字节——由指针类型决定
- `arr[i]` 和 `*(arr + i)` 在编译器层面**完全等价**
- `sizeof` 在数组上得到总大小，在指针上只得到指针本身的大小——**最易混淆**
- 指针相减得到**元素个数**，不是字节数（用 `ptrdiff_t` 类型接收）
- 只有**同一数组内**的指针才能比较大小
- 越界指针不报错，**自己负责边界**

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 指针算术 | Pointer Arithmetic | 对指针进行 ++、--、+n、-n 等运算 |
| 步长 | Stride / Step Size | 指针每次 +1 移动的字节数 |
| 解引用 | Dereference | 通过 `*` 获取指针指向的值 |
| 退化 | Decay | 数组名自动转换为指针的现象 |
| sizeof 陷阱 | sizeof Pitfall | sizeof(指针) 得到指针大小而非数组大小 |
| 越界访问 | Out-of-Bounds Access | 指针指向合法范围之外 |
| 指针差值 | Pointer Difference | 两个指针相减得到元素个数 |
| 同一数组 | Same Array | 指针比较的前提条件 |
| void 指针 | Void Pointer | 无类型指针，不能直接解引用 |
| 类型感知 | Type-Aware | 指针算术自动考虑元素类型的大小 |

## 延伸阅读

- [cppreference - Pointer arithmetic](https://en.cppreference.com/w/c/language/pointer_expression) — 标准指针运算规则
- `ptrdiff_t` / `size_t` —— `<stddef.h>` 中的标准类型
- 下一章：[void* 类型擦除](./void_pointers.md) —— 万能指针与字节级操作

## 继续学习

| 下一步 | 方向 |
|--------|------|
| 下一章 → | [void* 类型擦除](./void_pointers.md) — 万能指针与类型擦除 |
| 复习 ← | [指针基础](./pointers.md) |
| 深入 → | [多维数组与指针](./arrays.md) — 指针的指针 |
