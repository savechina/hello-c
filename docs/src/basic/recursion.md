# 递归函数（Recursion）

> "递归就像照镜子——镜子中的镜子中的镜子……如果没有尽头，无限循环就出现了。递归也必须有一个'尽头'。" —— 我发现

## 开篇故事

递归就像照镜子——你站在两面相对的镜子中间，看到的影像无限延伸：镜子中有镜子，镜子中还有镜子……每一面镜子都反射出一个"自己"，但每一层"自己"都比上一层更模糊、更远。

如果两面镜子之间没有尽头，这个过程会永远进行下去。但在编程中，"永远"是一件危险的事——它会耗尽计算机的内存，最终导致程序崩溃。

递归（Recursion）就是函数调用自己的技术。它和镜子的比喻一样，每一层调用都在复制一个"自己"。关键的区别是：递归必须有一个**尽头**（基线条件），否则就是无限循环。

```
递归调用的镜像效果

main()
  └─ factorial(4)
       └─ factorial(3)
            └─ factorial(2)
                 └─ factorial(1) ← 基线条件！不再递归
                      ↑
                   开始返回结果（栈展开）
```

> "递归就像剥洋葱——每一层都要剥开，直到最后发现里面什么都没有。" —— 计算机科学家

## 本章适合谁

- 已经理解了函数调用和参数传递的人
- 对"函数能调用自己"感到好奇或困惑的人
- 遇到"栈溢出"（Stack Overflow）错误想搞清楚原因的人
- 想要理解算法书中递归写法的人

## 你会学到什么

- 基线条件（Base Case）——递归的"尽头"
- 递归步骤（Recursive Step）——如何把大问题拆成小问题
- 调用栈（Call Stack）——递归背后发生了什么
- 栈展开（Stack Unwinding）——结果如何层层返回
- 栈溢出（Stack Overflow）——为什么递归会耗尽内存
- 何时该用递归、何时不该用递归

## 前置要求

- 理解函数的定义与调用（[函数基础](./functions.md)）
- 了解控制流：`if` / `else`（[控制流](./control_flow.md)）
- 了解变量的作用域（[作用域与生命周期](./scope.md)）

## 第一个例子：阶乘（Factorial）

阶乘是递归最经典的例子。`n!`（n 的阶乘）的定义本身就是递归的：

| 数学定义 | 含义 |
|----------|------|
| `0! = 1` | 基线条件（最简单的情形） |
| `n! = n × (n-1)!` | 递归步骤（大问题拆成小问题） |

```c
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) {           // ← 基线条件：递归的尽头
        return 1;
    }
    return n * factorial(n - 1);  // ← 递归步骤：调用自己
}

int main(void) {
    printf("5! = %d\n", factorial(5));  // 输出: 5! = 120
    return 0;
}
```

运行结果：

```
5! = 120
```

让我展开 `factorial(5)` 的完整执行过程：

```
factorial(5)
  = 5 * factorial(4)
  = 5 * (4 * factorial(3))
  = 5 * (4 * (3 * factorial(2)))
  = 5 * (4 * (3 * (2 * factorial(1))))
  = 5 * (4 * (3 * (2 * 1)))     ← 基线条件触发，开始返回
  = 5 * (4 * (3 * 2))
  = 5 * (4 * 6)
  = 5 * 24
  = 120
```

**我的理解**：递归的过程就像搭积木——先一层层往上搭（递），搭到尽头后，再一层层拆下来算结果（归）。

## 原理解析

### 1. 递归的两个要素

任何正确的递归函数都**必须**有这两个部分：

| 要素 | 作用 | 缺一不可？ |
|------|------|-----------|
| **基线条件**（Base Case） | 递归的尽头，直接返回值，不再调用自己 | ✅ 必须 |
| **递归步骤**（Recursive Step） | 把问题缩小，调用自己处理更小的子问题 | ✅ 必须 |

```c
// ✅ 正确结构
int my_recursive(int n) {
    if (基线条件) {       // ← 先写这个！
        return 直接结果;
    }
    return ... my_recursive(缩小后的参数) ...;  // ← 再写这个
}

// ❌ 缺少基线条件 → 无限递归 → 段错误（段错误 = Segmentation Fault）
int broken_recursive(int n) {
    return n * broken_recursive(n - 1);  // 永远不停！
}
```

### 2. 调用栈 —— 递归背后的"记账本"

每次函数调用，计算机都会在"调用栈"（Call Stack）上压入一个新的**栈帧**（Stack Frame）。递归函数的每次自调用都会压入一个新栈帧。

```
调用栈（Call Stack）—— factorial(3) 的执行过程

内存地址 ↑

┌─────────────────────────────────┐
│ factorial(1)                    │ ← 最新压入：n=1, 返回 1
│   return 1                      │
├─────────────────────────────────┤
│ factorial(2)                    │ ← n=2, 等待 factorial(1) 返回
│   等待: 2 * factorial(1) = 2    │
├─────────────────────────────────┤
│ factorial(3)                    │ ← n=3, 等待 factorial(2) 返回
│   等待: 3 * factorial(3) = ?    │
├─────────────────────────────────┤
│ main()                          │ ← 压入 factorial(3)
│   调用: factorial(3)            │
└─────────────────────────────────┘

                ↓ 基线条件触发
         开始"栈展开"（Stack Unwinding）
                ↑

每个栈帧依次弹出，带着返回值往上传递：
  factorial(1) → 1
  factorial(2) → 2 * 1 = 2
  factorial(3) → 3 * 2 = 6
  main()       → 收到 6
```

**我的理解**：调用栈就像一个弹簧——每次递归调用把弹簧压下一层，基线条件触发后，弹簧开始弹回，每一层带着结果弹出。弹簧有弹性极限——压得太深（递归太深）就会断裂（栈溢出）。

### 3. 栈帧的内存消耗

每个栈帧占用多少内存？取决于函数的局部变量数量和参数数量。一个简单的阶乘函数，每个栈帧大约需要 16-32 字节。但如果是复杂的递归，每个栈帧可能消耗数百字节。

```
假设每个栈帧 32 字节：

递归深度 10    → 320 字节    ✅ 完全没问题
递归深度 100   → 3,200 字节  ✅ 完全没问题
递归深度 10,000  → 320,000 字节 ≈ 312 KB  ✅ 仍然安全
递归深度 100,000 → 3,200,000 字节 ≈ 3 MB  ⚠️ 接近默认栈大小限制
递归深度 500,000 → 16,000,000 字节 ≈ 15 MB  🔴 栈溢出！（Stack Overflow!）
```

**典型栈大小**：
- macOS: 8 MB（线程栈）
- Linux: 8 MB（默认）
- Windows: 1 MB（默认）

### 4. 递归 vs. 迭代

递归和循环（迭代）可以互相替代，但各有优劣：

| 对比维度 | 递归 | 迭代（循环） |
|----------|------|-------------|
| 代码简洁性 | 通常更简洁、更接近数学定义 | 需要手动管理循环变量 |
| 内存消耗 | 每个调用占用一个栈帧 | 只需要循环变量 |
| 执行速度 | 调用开销大 | 无额外开销 |
| 可读性 | 对熟悉递归的人更易读 | 对所有人易读 |
| 栈溢出风险 | 有（递归太深时） | 无 |

```c
/* 递归版：阶乘 */
int factorial_recursive(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial_recursive(n - 1);
}

/* 迭代版：阶乘 */
int factorial_iterative(int n) {
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}
```

**我的建议**：如果递归深度不大（< 1000），且递归写法更清晰，就选递归。如果有大深度风险或性能敏感，选迭代。

## 常见错误

### ❌ 错误 1：没有基线条件

```c
int infinite_recursion(int n) {
    return n + infinite_recursion(n - 1);  /* ❌ 没有 if 判断！ */
}

/* 结果：无限递归 → 栈溢出 → Segmentation Fault（段错误） */
```

✅ **修正**：始终写基线条件。

```c
int safe_recursion(int n) {
    if (n <= 0) {              /* ✅ 基线条件 */
        return 0;
    }
    return n + safe_recursion(n - 1);
}
```

### ❌ 错误 2：基线条件永远不会触发

```c
int almost_right(int n) {
    if (n == 0) {              /* 期望 n 最终变为 0 */
        return 1;
    }
    return n * almost_right(n - 2);  /* ❌ 如果 n 是奇数，永远到不了 0 */
    /* n: 5 → 3 → 1 → -1 → -3 → ... 永远不等于 0！ */
}
```

✅ **修正**：用 `<=` 代替 `==`。

```c
int fixed(int n) {
    if (n <= 0) {              /* ✅ 用 <= 兜底 */
        return 1;
    }
    return n * fixed(n - 2);
}
```

### ❌ 错误 3：递归深度过大 → 栈溢出

```c
/* ❌ 用递归计算 100000 的阶乘 → 栈溢出！ */
long long big_factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return (long long)n * big_factorial(n - 1);  /* 100000 层调用 → 溢出 */
}
```

✅ **修正**：对于大 N，用迭代。

```c
long long safe_factorial(int n) {
    long long result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}
```

### ❌ 错误 4：重复计算（Fibonacci 的经典问题）

```c
/* ❌ 朴素递归 Fibonacci —— 指数级重复计算 */
int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
    /* fibonacci(5) 会调用 fibonacci(4) 和 fibonacci(3)
       fibonacci(4) 又会调用 fibonacci(3) 和 fibonacci(2)
       → fibonacci(3) 被计算了 2 次！
       → n=40 时需要约 20 亿次调用！ */
}
```

✅ **修正 1**：迭代法（推荐）。

```c
int fibonacci_iterative(int n) {
    if (n <= 1) {
        return n;
    }
    int prev2 = 0;
    int prev1 = 1;
    for (int i = 2; i <= n; i++) {
        int current = prev1 + prev2;
        prev2 = prev1;
        prev1 = current;
    }
    return prev1;
}
```

✅ **修正 2**：记忆化递归（后续章节会讲——"动态规划"）。

## 动手练习

### 🟢 练习 1：写一个递归 Fibonacci 函数

```c
/* 写一个递归函数计算 Fibonacci 数列
   fib(0) = 0
   fib(1) = 1
   fib(n) = fib(n-1) + fib(n-2)
   
   测试: fib(10) = 55
*/
```

<details>
<summary>点击查看答案</summary>

```c
int fibonacci(int n) {
    if (n <= 0) {
        return 0;
    }
    if (n == 1) {
        return 1;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

/* 验证 */
printf("fib(10) = %d\n", fibonacci(10));  /* 55 */
```
</details>

### 🟡 练习 2：递归求数组之和

```c
/* 用递归计算数组前 n 个元素的和
   提示：sum(arr, n) = arr[n-1] + sum(arr, n-1)
   
   测试: sum([1, 2, 3, 4, 5], 5) = 15
*/
```

<details>
<summary>点击查看答案</summary>

```c
int array_sum(const int arr[], int n) {
    if (n <= 0) {
        return 0;
    }
    return arr[n - 1] + array_sum(arr, n - 1);
}

/* 验证 */
int data[] = {1, 2, 3, 4, 5};
printf("sum = %d\n", array_sum(data, 5));  /* 15 */
```
</details>

### 🔴 练习 3：递归实现二分查找（Binary Search）

```c
/* 用递归实现二分查找
   在有序数组 arr[left..right] 中查找 target
   找到返回索引，未找到返回 -1
   
   提示：比较中间元素，决定搜索左半还是右半
*/
```

<details>
<summary>点击查看答案</summary>

```c
int binary_search_recursive(const int arr[], int left, int right, int target) {
    if (left > right) {
        return -1;  /* 基线条件：搜索区间为空 */
    }

    int mid = left + (right - left) / 2;  /* 防溢出写法 */

    if (arr[mid] == target) {
        return mid;  /* 基线条件：找到了 */
    } else if (arr[mid] < target) {
        return binary_search_recursive(arr, mid + 1, right, target);
    } else {
        return binary_search_recursive(arr, left, mid - 1, target);
    }
}

/* 验证 */
int data[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
printf("找到 23: 索引 %d\n",
       binary_search_recursive(data, 0, 9, 23));   /* 5 */
printf("找到 7:  索引 %d\n",
       binary_search_recursive(data, 0, 9, 7));    /* -1 */
```
</details>

## 故障排查（FAQ）

### Q: 什么是"栈溢出"（Stack Overflow）？

每个程序有一个固定大小的栈空间（通常 8 MB）。每次函数调用都在栈上分配一个"栈帧"。如果递归太深，栈空间用完了，就会触发**栈溢出**（Stack Overflow），程序会立刻崩溃（Segmentation Fault）。

```
┌────────────────────────────┐ ← 栈顶
│ recursive()  n = -100      │
│ recursive()  n = -99       │
│ recursive()  n = -98       │
│         ...                │
│ recursive()  n = -1        │
│ recursive()  n = 0         │  ← 栈空间耗尽！
├────────────────────────────┤ ← 栈底
│ main()                     │
┝━━━━━━━━━━━━━━━━━━━━━━━━━━━━┥ ← Stack Overflow!
│          堆 (Heap)          │
└────────────────────────────┘
```

**如何避免**：
1. 始终确保基线条件能被触发
2. 不要用递归处理可能深度很大的问题（如 10 万次迭代）
3. 如果不确定递归深度，改用迭代

### Q: 递归和迭代有什么本质区别？

**本质区别在于"谁管理状态"**：
- **递归**：编译器帮你管理状态——每个递归调用的局部变量都存放在各自的栈帧中
- **迭代**：你自己管理状态——循环变量由你更新和维护

递归更简洁，但消耗更多内存。迭代更高效，但有时代码更复杂。

### Q: 递归函数可以没有返回值吗？

可以。`void` 函数也可以递归：

```c
void print_countdown(int n) {
    if (n <= 0) {          /* 基线条件 */
        printf("发射！🚀\n");
        return;
    }
    printf("%d...\n", n);
    print_countdown(n - 1);  /* 递归步骤 */
}
```

### Q: `main()` 可以递归调用自己吗？

C 标准允许 `main()` 被递归调用（C++ 不允许）。但**不推荐**——可读性差，且容易忘记基线条件。

## 知识扩展（选学）

### 尾递归优化（Tail Recursion Optimization, TCO）

**尾递归**是指：函数的最后一个操作就是递归调用自身，且返回值直接返回递归调用的结果（不做任何额外计算）。

```c
/* ❌ 不是尾递归：返回 "n * factorial(...)" —— 乘法是递归之后的操作 */
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

/* ✅ 是尾递归：最后一步直接 return helper(...) —— 没有额外计算 */
int factorial_tail_helper(int n, int acc) {
    if (n <= 1) {
        return acc;
    }
    return factorial_tail_helper(n - 1, n * acc);
}
int factorial_tail(int n) {
    return factorial_tail_helper(n, 1);
}
```

**优化原理**：编译器发现尾递归后，可以用**跳转向量化**代替压栈——不需要创建新栈帧，而是直接修改当前栈帧的参数并跳转（类似 `goto`）。这样就把 O(n) 的栈空间优化成 O(1)。

```
普通递归（需 n 个栈帧）：         尾递归优化后（仅 1 个栈帧）：

┌──────────┐                   ┌──────────┐
│ f(5)     │                   │ f(5,1)   │ ← 跳转: n=5,acc=1
├──────────┤                   │ f(4,5)   │ ← 跳转: n=4,acc=5
│ f(4)     │ ← 5 层栈帧!       │ f(3,20)  │ ← 跳转: n=3,acc=20
├──────────┤                   │ f(2,60)  │ ← 跳转: n=2,acc=60
│ f(3)     │                   │ f(1,120) │ ← 结果: 120
├──────────┤                   └──────────┘
│ f(2)     │
├──────────┤
│ f(1)     │
└──────────┘
```

**现代编译器**：GCC、Clang 在 `-O2` 及以上优化等级时会自动对尾递归进行优化。你可以用 `-foptimize-sibling-calls` 标志显式开启（GCC 默认开启）。

### 分治法（Divide and Conquer）

递归最常见的应用场景：**分治**。思路：
1. **分**：把大问题拆成若干个小问题
2. **治**：递归解决每个小问题
3. **合**：合并小问题的解为大问题的解

典型算法：归并排序（Merge Sort）、快速排序（Quick Sort）、二分查找。

## 小结

祝贺！你掌握了 C 语言的递归函数概念。总结——

- **递归** = 函数调用自己
- **必须有两个要素**：基线条件（尽头）+ 递归步骤（缩小问题）
- **调用栈** = 每次递归调用压入一个栈帧，基线条件触发后逐层弹出（栈展开）
- **栈溢出** = 递归太深导致栈空间耗尽 → 段错误 → 崩溃
- **何时用递归**：代码更简洁、递归深度可控（通常 < 1000）
- **何时不用递归**：深度不可控、性能敏感 → 改用迭代
- **尾递归优化**：编译器可以把特定的尾递归优化成循环，消除栈帧开销

> **我的理解**：递归的本质是"自我复制，直到触底，然后带着答案回来"。写递归之前，先问自己：尽头在哪？每一步够小吗？如果这两步回答清楚了，递归就永远不会出错。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 递归（Recursion） | 函数调用自身的技术 |
| 基线条件（Base Case） | 递归的终点，直接返回值，不再调用自己 |
| 递归步骤（Recursive Step） | 把问题缩小并调用自己的部分 |
| 调用栈（Call Stack） | 跟踪函数调用的栈式数据结构 |
| 栈帧（Stack Frame） | 每次函数调用在栈上分配的内存块 |
| 栈展开（Stack Unwinding） | 基线条件触发后，层层返回结果的过程 |
| 栈溢出（Stack Overflow） | 递归太深导致栈空间耗尽的崩溃 |
| 尾递归（Tail Recursion） | 函数的最后一个操作是递归调用 |
| 尾递归优化（TCO） | 编译器将尾递归优化为循环，消除栈帧 |
| 分治法（Divide and Conquer） | 将大问题拆成小问题，递归解决再合并的策略 |
| 段错误（Segmentation Fault） | 访问非法内存时的崩溃（常由栈溢出触发） |

## 延伸阅读

- [cppreference: Recursion in C](https://en.cppreference.com/w/c/language/function)
- [Beej's Guide to C: Recursion](https://beej.us/guide/bgc/)
- K&R《C 程序设计语言》第 4.2 节：递归
- 《算法导论》第 2 章：归并排序（分治法的经典例子）
- [Stack Overflow 百科（讽刺的是这个名字来源于此）](https://stackoverflow.com/questions/tagged/recursion)

## 继续学习

你已经理解了递归的精妙之处——函数通过"复制自己"来解决问题。下一章我们将学习**可变参数函数（Variadic Functions）**，了解 C 语言如何实现像 `printf` 这样接受任意数量参数的函数。

> 💡 **提示**：试着把你之前写的 `for` 循环改成递归版本（比如数组求和、阶乘）。对比两种写法的代码量，看看递归是否真的更简洁？

[← 上一章：作用域与生命周期](./scope.md) | [下一章：可变参数函数 →](./variadic_functions.md)
