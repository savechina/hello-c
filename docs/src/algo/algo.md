# 算法示例 (Algorithms)

```admonish note
**本章讲什么** — 仓库自带的算法演示模块：冒泡排序 (Bubble Sort)、二分查找 (Binary Search)、斐波那契的两种实现对比（`unsigned long long` 版 vs 大数数组版）。
**对应源码** — [`src/algo/algo.c`](https://github.com/savechina/hello-c/blob/main/src/algo/algo.c)
```

## 开篇故事

排序和查找是算法课的第一课，也是 C 语言最好的练手场：没有现成的库替你兜底，每个下标、每次交换都由你负责。本章把仓库 `src/algo/algo.c` 里的演示逐段拆开——代码全部来自真实源文件，输出全部来自真实运行。

模块入口是 `main_algo_sample()`，一次演示三件事：把一个乱序数组排好、在有序数组里找到目标值、以及用两种方式算斐波那契并对比结果。

## 本章适合谁

- 学完 [数组基础](../basic/arrays.md) 和 [循环](../basic/loops.md)，想看"真实项目里"的算法长什么样
- 准备面试、需要手写排序和查找的人
- 好奇 `unsigned long long` 为什么算不动 F(93) 以后的斐波那契的人
- 想了解用数组模拟大数运算 (Big Number) 思路的人

## 你会学到什么

1. 冒泡排序 (Bubble Sort) 的交换逻辑与 O(n²) 复杂度
2. 二分查找 (Binary Search) 的防溢出写法 `mid = lo + (hi - lo) / 2`
3. 递推式斐波那契的整型上限问题（`unsigned long long` 只够用到 F(92) 左右）
4. 大数运算：用 `int` 数组逐位存十进制数字，突破整型宽度限制
5. 模块入口 `main_algo_sample()` 如何组织多个算法演示

## 前置要求

- 已掌握 [数组基础](../basic/arrays.md)、[循环](../basic/loops.md)、[函数](../basic/functions.md)
- （选学）[递归函数](../basic/recursion.md) —— 理解为什么这里算斐波那契改用迭代

## 第一个例子：冒泡排序

源文件 `src/algo/algo.c` 中的第一个算法：

```c
/* ---------- 冒泡排序 (Bubble Sort) ---------- */
static void bubble_sort(int arr[], int n)
{
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }
}
```

配套的打印函数：

```c
static void print_array(const int arr[], int n)
{
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}
```

运行演示（`main_algo_sample()` 第一段）：

```
[1] Bubble Sort Example
  Before: 64 34 25 12 22 11 90
  After:  11 12 22 25 34 64 90
```

这段代码做了两件事：

- 外层 `i` 控制轮数：每一轮把当前未排序部分的最大值"冒"到末尾
- 内层 `j` 只扫到 `n - i - 1`：末尾已经排好的元素不必再比较

## 原理解析

### 1. 冒泡排序：为什么叫"冒泡"

相邻两个元素逆序就交换，最大值像气泡一样逐轮浮到数组末尾。第 `i` 轮结束后，最后 `i` 个元素已经就位，所以内层循环长度递减。

| 项目 | 值 |
|------|-----|
| 时间复杂度（平均/最坏） | O(n²) |
| 时间复杂度（最好，已有序） | O(n²)（本实现没有提前退出优化） |
| 空间复杂度 | O(1)，只用一个 `tmp` |
| 稳定性 | 稳定（`>` 保证相等不交换） |

> 教学取舍：冒泡排序不是工程首选，但它是理解"比较—交换"范式最直观的例子。工程排序用 [`qsort`](../basic/callbacks.md)（回调比较器）。

### 2. 二分查找：有序是前提

```c
/* ---------- 二分查找 (Binary Search) ---------- */
static int binary_search(const int arr[], int n, int target)
{
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (arr[mid] == target)
            return mid;
        else if (arr[mid] < target)
            lo = mid + 1;
        else
            hi = mid - 1;
    }
    return -1;
}
```

三个关键点：

- **`mid = lo + (hi - lo) / 2`** 而不是 `(lo + hi) / 2` —— 后者在 `lo + hi` 超过 `INT_MAX` 时溢出，这是二分查找最经典的坑
- **`while (lo <= hi)`** —— 区间是闭区间 `[lo, hi]`，`lo == hi` 时还剩一个元素要查
- **找不到返回 `-1`** —— 调用方必须检查返回值

演示用的有序数组和目标值：

```c
int sorted[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
int target = 23;
int result = binary_search(sorted, 10, target);
```

```
[2] Binary Search Example
  Found 23 at index 5
```

时间复杂度 O(log n)：10 个元素最多比较 4 次，100 万个元素最多约 20 次——这就是对数级查找的威力。

### 3. 斐波那契：`unsigned long long` 版

```c
/* ---------- 斐波那契动态规划 (unsigned long long, 上限 F(92)) ---------- */
static unsigned long long fibonacci_ull(int n)
{
    if (n <= 0) return 0;
    if (n == 1) return 1;

    unsigned long long a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        unsigned long long c = a + b;
        a = b;
        b = c;
    }
    return b;
}
```

与 [递归章节](../basic/recursion.md) 里指数级重复计算的写法不同，这里用**迭代递推**：只保留前两项 `a`、`b`，每轮滚动更新，时间 O(n)、空间 O(1)。

但整型宽度是硬约束——`unsigned long long` 最大 2⁶⁴ − 1 ≈ 1.8 × 10¹⁹，源码注释标注**上限 F(92)**，演示输出也提醒 F(93) 起开始溢出：

```
[3] Fibonacci Comparison (ull vs big number)
  --- unsigned long long 版本 (上限 F(92)) ---
  F(0) = 0
  F(1) = 1
  F(10) = 55
  F(50) = 12586269025
  F(90) = 2880067194370816120
  (注意: F(93) 开始 unsigned long long 溢出
```

超过上限怎么办？不是换更大的整数类型，而是**换一种表示法**。

### 4. 斐波那契：大数数组版

源码用一个 `int` 数组模拟大数，**每个元素存一位十进制数字，低位在前**：

```c
/* ---------- 斐波那契大数版 (数组模拟，支持 F(200)+) ---------- */
/* 用 int 数组存储大数，每个元素存一位十进制数字(低位在前)
 * 最多支持 MAX_DIGITS 位，F(200) 约 42 位，128 位足够 */
#define MAX_DIGITS 128
```

逐位加法，进位逐位传递：

```c
static void big_add(int a[], int b[], int result[])
{
    int carry = 0;
    for (int i = 0; i < MAX_DIGITS; i++) {
        int sum = a[i] + b[i] + carry;
        result[i] = sum % 10;
        carry = sum / 10;
    }
}
```

打印时跳过前导零（从最高位向前找第一个非零位）：

```c
static void big_print(const int num[])
{
    /* 跳过前导零 */
    int i = MAX_DIGITS - 1;
    while (i > 0 && num[i] == 0) {
        i--;
    }
    for (; i >= 0; i--) {
        printf("%d", num[i]);
    }
}
```

主循环和 `unsigned long long` 版结构相同，只是把标量加法换成 `big_add`，再用 `big_copy` 滚动两项：

```c
static void fibonacci_big(int n, int result[])
{
    for (int i = 0; i < MAX_DIGITS; i++) result[i] = 0;

    if (n <= 0) {
        result[0] = 0;
        return;
    }
    if (n == 1) {
        result[0] = 1;
        return;
    }

    int a[MAX_DIGITS] = {0}, b[MAX_DIGITS] = {0};
    a[0] = 0;  /* F(0) = 0 */
    b[0] = 1;  /* F(1) = 1 */

    for (int i = 2; i <= n; i++) {
        big_add(a, b, result);
        big_copy(a, b);
        big_copy(b, result);
    }
}
```

大数版输出（演示序列 `0, 1, 10, 50, 90, 100, 200`）：

```
  --- Big Number 版本 (数组模拟，支持 F(200)+) ---
  F(0) = 0
  F(1) = 1
  F(10) = 55
  F(50) = 12586269025
  F(90) = 2880067194370816120
  F(100) = 354224848179261915075
  F(200) = 280571172992510140037611932413038677189525
```

`F(200)` 是 42 位十进制数，早已超出任何原生整型——数组模拟让它算得下去。

## 模块入口

三个演示由统一入口按序调度，`main.c` 收到 `algo` 子命令时调用：

```c
int main_algo_sample(void)
{
    printf("=== Algo Module: Common Algorithms ===\n\n");

    /* 1. 冒泡排序演示 */
    printf("[1] Bubble Sort Example");
    ...

    /* 2. 二分查找演示 (需要有序数组) */
    printf("[2] Binary Search Example");
    ...

    /* 3. 斐波那契 — 两种实现对比 */
    printf("[3] Fibonacci Comparison (ull vs big number)");
    ...

    printf("=== Algo Module Complete ===\n");
    return 0;
}
```

运行方式：

```bash
make build
./build/bin/hello algo
```

完整输出以 `=== Algo Module: Common Algorithms ===` 开始、`=== Algo Module Complete ===` 结束，中间依次是上面三段演示。

## 小结

本章走完了仓库的算法演示模块：

- **冒泡排序** — 比较交换的最小范式，O(n²)，理解排序的起点
- **二分查找** — 有序前提 + 防溢出 `mid` 写法，O(log n)
- **斐波那契 `unsigned long long` 版** — 迭代递推 O(n)，但受整型上限约束（约 F(92)）
- **斐波那契大数版** — `int` 数组逐位存十进制，`big_add` 逐位进位，突破整型宽度，支持 F(200)+
- **模块组织** — 每个算法一个 `static` 函数，入口 `main_algo_sample()` 统一演示，`main.c` 按子命令分发

**核心术语**：Bubble Sort / Binary Search / Big Number Simulation / Iterative Recurrence / Dispatch

> 「算法在 C 里没有魔法：排序就是比较与交换的循环，查找就是区间的收缩，大数就是数组里的一位数字。看懂了这三个例子，剩下的算法书你都能读。」

## 术语表

| 英文 | 中文 |
|------|------|
| Bubble Sort | 冒泡排序 |
| Binary Search | 二分查找 |
| Stable Sort | 稳定排序 |
| Time Complexity | 时间复杂度 |
| Iterative Recurrence | 迭代递推 |
| Big Number (Bignum) | 大数运算 |
| Carry | 进位 |
| Overflow | 溢出 |

## 继续学习

- [递归函数](../basic/recursion.md)：斐波那契的递归写法与它为什么慢
- [回调函数与多态](../basic/callbacks.md)：用 `qsort` 和比较器回调做工程级排序
- [工具链](../advance/tools.md)：用 `make asan` / `make tsan` 检验你的算法实现有没有越界和数据竞争

> 💡 **提示**：把 `bubble_sort` 改成"提前退出"优化（某轮没有交换就停止），再用 `./build/bin/hello algo` 验证输出不变——这是最简单的算法改进练习。
