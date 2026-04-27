# 数组基础 (Arrays)

想象一栋公寓楼的外墙，上面整齐排列着几十个信箱。每个信箱大小相同，门牌号从 0 开始依次编号。邮递员只看门牌号投递——`mailbox[0]`、`mailbox[1]`、`mailbox[2]`，依此类推。所有信箱格式一样，装的都是信件，不会混放包裹。

这就是 C 语言数组的形态。一块连续的内存空间，分成若干大小相同的格子，用整数索引编号。格子大小由你声明的类型决定——`int32_t scores[5]` 就是 5 个同样大小的 `int32_t` 格子。这种设计让读写非常高效，计算索引地址只需要简单的乘法和加法。

但公寓管理员有一个特殊规矩：他不盯着你按门牌号办事。你想开 `mailbox[5]` 这扇门，他不会拦你——但这扇门后面可能是走廊、邻居的墙，或者什么都没有。C 语言不检查数组越界，它信任你。这份信任换来的是速度和灵活，也意味着你必须自己管好边界。

> C 语言把选择权交给程序员，也把责任交给你。

## 本章适合谁

- 已经了解 C 语言基本变量（`int32_t`、`double` 等）
- 想理解"一组相同类型的数据"如何存储和操作
- 被数组越界 bug 折磨过的程序员（我当初就是 😅）

## 你会学到什么

- 一维数组的声明和初始化
- `{...}` 初始化语法（全部初始化、部分初始化、省略长度）
- 数组索引（0-based）与边界
- 遍历数组的循环模式
- `sizeof(array) / sizeof(element)` 计算元素个数
- 常见错误：越界访问、混淆大小写索引

## 前置要求

完成 [变量](./variables/variables.md) 和 [数据类型](./variables/datatype.md) 章节。

## 第一个例子

让我先展示一个完整的数组程序：

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    // 声明并初始化
    int32_t scores[5] = {85, 92, 78, 96, 88};

    // 计算元素个数
    int32_t count = (int32_t)(sizeof(scores) / sizeof(scores[0]));

    // 遍历打印
    for (int32_t i = 0; i < count; i++) {
        printf("学生 %d: %d 分\n", i + 1, scores[i]);
    }

    return 0;
}
```

输出：
```
学生 1: 85 分
学生 2: 92 分
学生 3: 78 分
学生 4: 96 分
学生 5: 88 分
```

### 分步解析

1. `int32_t scores[5]`：声明一个能存 5 个 `int32_t` 的数组
2. `{85, 92, 78, 96, 88}`：初始值列表
3. `sizeof(scores) / sizeof(scores[0])`：总字节数 ÷ 单个元素字节数 = 元素个数
4. `scores[i]`：通过索引访问第 `i` 个元素

## 原理解析

### 内存布局

数组在内存中是**连续存储**的：

```
scores: [85] [92] [78] [96] [88]
         ↑    ↑    ↑    ↑    ↑
       [0]  [1]  [2]  [3]  [4]
```

每个元素占 4 字节（`int32_t`）。`scores[0]` 的地址是数组的起始地址，`scores[1]` 的地址则是 `起始地址 + 4 字节`。

### 三种初始化方式

```c
// 方式 1: 全部初始化（指定长度）
int32_t a[5] = {1, 2, 3, 4, 5};

// 方式 2: 省略长度（编译器根据初始值推导）
int32_t b[] = {1, 2, 3, 4, 5};  // 长度 = 5

// 方式 3: 部分初始化（其余自动为 0）
int32_t c[5] = {1, 2};         // c = {1, 2, 0, 0, 0}
```

### sizeof 技巧

这是我最常用的数组技巧：

```c
int32_t data[] = {1, 2, 3, 4, 5, 6, 7, 8};
size_t count = sizeof(data) / sizeof(data[0]);
// sizeof(data) = 32 (8 个 × 4 字节)
// sizeof(data[0]) = 4
// count = 32 / 4 = 8
```

> ⚠️ 注意: 这个技巧**只对数组本身**有效。当数组传给函数后，它会**退化为指针**，`sizeof` 得到的是指针的大小（8 字节在 64 位机器上），不再是整个数组。

## 常见错误

### ❌ 错误 1: 数组越界

```c
int32_t arr[3] = {10, 20, 30};

// ❌ 越界访问：索引 3 超出范围
printf("%d\n", arr[3]);   // 未定义行为！可能崩溃，可能打印垃圾值
```

编译器通常不会拦截这个错误。修复方式：

```c
int32_t arr[3] = {10, 20, 30};
// ✅ 合法索引: 0 ~ 2
for (int32_t i = 0; i < 3; i++) {
    printf("arr[%d] = %d\n", i, arr[i]);
}
```

### ❌ 错误 2: 初始化时长度不匹配

```c
// ❌ 编译器可能警告，但不一定报错
int32_t arr[2] = {1, 2, 3, 4};  // 4 个值塞进 2 个容量！
```

修复：

```c
// ✅ 让编译器推导长度，或者给够长度
int32_t arr[] = {1, 2, 3, 4};   // 长度 = 4
// 显式指定
int32_t arr2[4] = {1, 2, 3, 4};
```

### ❌ 错误 3: 在函数中用 sizeof 获取数组长度

```c
void print_size(int32_t arr[]) {
    // ❌ arr 在这里退化成了指针！
    size_t count = sizeof(arr) / sizeof(arr[0]);  // 8 / 4 = 2（错误！）
    printf("%zu\n", count);
}
```

修复方案——**把长度当参数传进来**：

```c
void print_size(int32_t arr[], int32_t count) {
    // ✅ 从参数获取长度
    for (int32_t i = 0; i < count; i++) {
        printf("arr[%d] = %d\n", i, arr[i]);
    }
}

int main(void) {
    int32_t data[] = {1, 2, 3, 4, 5};
    int32_t count = (int32_t)(sizeof(data) / sizeof(data[0]));
    print_size(data, count);
    return 0;
}
```

## 动手练习

<details>
<summary>🟢 入门: 求和</summary>

创建一个包含 10 个元素的数组 `{1, 2, 3, ..., 10}`，计算它们的总和。

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t nums[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int64_t sum = 0;
    for (int32_t i = 0; i < 10; i++) {
        sum += nums[i];
    }
    printf("总和: %lld\n", (long long)sum);
    return 0;
}
```

**输出**: `总和: 55`

</details>

<details>
<summary>🟡 中级: 反转数组</summary>

将数组 `{1, 2, 3, 4, 5}` 原地反转为 `{5, 4, 3, 2, 1}`。

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t arr[] = {1, 2, 3, 4, 5};
    int32_t n = (int32_t)(sizeof(arr) / sizeof(arr[0]));

    for (int32_t i = 0; i < n / 2; i++) {
        int32_t temp = arr[i];
        arr[i] = arr[n - 1 - i];
        arr[n - 1 - i] = temp;
    }

    for (int32_t i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    return 0;
}
```

**输出**: `5 4 3 2 1`

</details>

<details>
<summary>🔴 挑战: 找第二大元素</summary>

在不排序的前提下，找到数组中的第二大元素。

```c
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

int main(void) {
    int32_t arr[] = {12, 35, 1, 10, 35, 1};
    int32_t n = (int32_t)(sizeof(arr) / sizeof(arr[0]));

    int32_t max = INT32_MIN;
    int32_t second = INT32_MIN;

    for (int32_t i = 0; i < n; i++) {
        if (arr[i] > max) {
            second = max;
            max = arr[i];
        } else if (arr[i] > second && arr[i] != max) {
            second = arr[i];
        }
    }

    if (second == INT32_MIN) {
        printf("不存在第二大元素（所有元素相同）\n");
    } else {
        printf("第二大元素: %d\n", second);
    }
    return 0;
}
```

**输出**: `第二大元素: 12`

</details>

## 故障排查 (FAQ)

<details>
<summary>Q: 为什么我的 `arr[10]` 没有报错但输出不对？</summary>

A: C 语言**不会**自动做越界检查。当数组长度为 10，合法索引是 `0~9`。访问 `arr[10]` 是**未定义行为** (Undefined Behavior)——可能读到其他变量的值，也可能直接崩溃。

解决方案：始终用 `sizeof(arr) / sizeof(arr[0])` 计算长度，循环条件用 `< length` 而非 `<= length`。

</details>

<details>
<summary>Q: 数组声明时可以不初始化吗？</summary>

A: 可以。但**局部数组**（函数内声明）中的值是**不确定的**，使用前必须赋值。

```c
int32_t arr[5];  // ❌ 每个元素的值是垃圾值
printf("%d\n", arr[0]);  // 随机数！
```

如果希望全部归零：
```c
int32_t arr[5] = {0};  // ✅ 所有元素都是 0
// 或者
int32_t arr[5] = {0, 0, 0, 0, 0};
```

</details>

<details>
<summary>Q: 我能直接用 `==` 比较两个数组的内容吗？</summary>

A: **不能**。C 语言中数组名本质上是个地址。`arr1 == arr2` 比较的是**地址**而非内容。

```c
int32_t a[] = {1, 2, 3};
int32_t b[] = {1, 2, 3};
// a == b 比较地址！结果是 false。
```

正确做法——逐元素比较或使用 `<string.h>` 中的 `memcmp`：
```c
#include <string.h>
#include <stdbool.h>

int same = (memcmp(a, b, sizeof(a)) == 0) ? 1 : 0;
// 1 表示内容相同
```

</details>

## 知识扩展 (选学)

### 变长数组 (VLA) — C99 引入

```c
int32_t n = 10;
int32_t arr[n];  // 运行时确定大小 (C99+)
```

> 注意: C11 开始 VLA 变为**可选特性**，部分编译器（如 MSVC）不支持。不推荐在生产代码中使用。

### 多维数组

C 支持二维及更高维度的数组：

```c
int32_t matrix[3][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};

printf("%d\n", matrix[1][2]);  // 输出 6 (第 2 行，第 3 列)
```

## 小结

这一章我发现：

- 数组是**连续内存**，索引从 0 开始
- 声明时可以用 `{...}` 初始化，支持部分初始化和省略长度
- `sizeof(array) / sizeof(array[0])` 是计算元素个数的标准做法
- **越界访问是未定义行为**——编译器不负责拦截，要靠自己小心
- 数组传给函数后会**退化为指针**，需要单独传长度

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 数组 | Array | 一组同类型、连续存储的数据 |
| 索引 | Index | 访问数组元素的位置编号，从 0 开始 |
| 越界 | Out of bounds | 访问超出数组合法范围的位置 |
| 未定义行为 | Undefined Behavior (UB) | C 标准不规定结果的行为，程序可能崩溃或产生随机结果 |
| sizeof 运算符 | sizeof operator | 计算类型或变量所占字节数 |
| 退化 | Decay | 数组传给函数时退化为指针的现象 |
| VLA | Variable Length Array | 运行时确定大小的数组 |
| 部分初始化 | Partial initialization | 初始化列表中只给部分值，其余自动为 0 |

## 延伸阅读

- [C99 标准 §6.7.8 — 数组初始化](https://en.cppreference.com/w/c/language/array_initialization)
- `memcpy` / `memcmp` / `memmove` —— `<string.h>` 中的内存操作函数
- 下一章: [控制流](./variables/control-flow.md) — if/else 分支与 switch 语句

## 继续学习

| 下一步 | 方向 |
|--------|------|
| 下一章 → | [控制流：if/else/switch](./variables/control-flow.md) |
| 复习 ← | [数据类型](./variables/datatype.md) |
| 深入 → | [循环](./loops/loops.md) — for/while/do-while |
