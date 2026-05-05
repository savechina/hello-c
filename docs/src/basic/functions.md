# 函数（Functions）

> "代码应该像一首诗，函数就是它的韵律。" —— 我发现

## 开篇故事

走进一家专业厨房，你会看到切菜台、灶台、烘焙区各自独立，每个岗位有专门的厨师。如果让一个厨师同时切菜、炒菜、装盘，不仅手忙脚乱，出一盘菜的时间会被拉长好几倍。分工，是效率的来源。

C 语言的函数就是厨房里的分工。把一段代码打包成一个函数，等于在程序里开了一个专门的工作台。`main()` 不必包办一切——它只需要调度：调用 `calculate()` 算结果，调用 `print_result()` 打输出，调用 `save_data()` 存数据。每个函数只做一件事，但把这件事做好。代码从「一锅粥」变成了「流水线」，改一处不再崩三处。

**函数，就是把复杂问题切分成小块的艺术**。

## 本章适合谁

- 学过变量和数据类型，但没用过函数的 C 初学者
- 写过几百行 `main()`，想学会拆分代码的人
- 想知道函数声明（declaration）和定义（definition）区别的人

## 你会学到什么

- 函数的声明与定义（Declaration vs. Definition）
- 参数传递：值传递（Pass by Value）
- 返回值（Return Value）与 `void` 函数
- 函数原型（Prototype）与前向声明（Forward Declaration）
- 递归（Recursion）入门
- `static` 函数的作用域

## 前置要求

- 了解 C 基本数据类型（`int`、`float`、`char` 等）
- 会写基本的 `printf` 和 `scanf`
- 能编译运行单个 `.c` 文件

> 如果你还没学变量，建议先看「变量」章节。

## 第一个例子：计算两个数的和

```c
#include <stdio.h>

// 函数定义（Definition）
int add(int a, int b) {
    return a + b;
}

int main(void) {
    int x = 3, y = 5;
    int result = add(x, y);  // 函数调用（Call）
    printf("%d + %d = %d\n", x, y, result);
    return 0;
}
```

运行结果：

```
3 + 5 = 8
```

看起来很简单？但这里面有好几个关键概念，我们逐一拆解。

## 原理解析

### 1. 函数的组成部分

一个完整的函数包含四个部分：

| 部分 | 说明 | 示例 |
|------|------|------|
| 返回类型（Return Type） | 函数返回值的类型 | `int` |
| 函数名（Function Name） | 调用时使用的标识符 | `add` |
| 参数列表（Parameter List） | 传入函数数据的变量 | `(int a, int b)` |
| 函数体（Body） | 花括号 {} 中的代码 | `{ return a + b; }` |

### 2. 声明 vs. 定义

**声明（Declaration）**：告诉编译器"这个函数存在"，也叫函数原型（Prototype）。

```c
int add(int a, int b);  // 声明，以分号结尾
```

**定义（Definition）**：包含完整的函数体，是实际实现。

```c
int add(int a, int b) {  // 定义，有函数体
    return a + b;
}
```

> **为什么需要声明？** 因为 C 编译器从上往下读代码。如果 `main()` 在 `add()` 前面调用它，编译器还不认识 `add`。

### 3. 前向声明（Forward Declaration）

```c
#include <stdio.h>

// 前向声明：在 main 之前告诉编译器 max 存在
int max(int a, int b);

int main(void) {
    int result = max(10, 20);  // 编译器已认识 max
    printf("max = %d\n", result);
    return 0;
}

// 函数定义放在 main 之后
int max(int a, int b) {
    return (a > b) ? a : b;
}
```

### 4. void 函数

不是所有函数都需要返回值。`void` 表示"无返回"。

```c
void print_separator(int length) {
    for (int i = 0; i < length; i++) {
        putchar('-');
    }
    putchar('\n');
}
```

### 5. 值传递（Pass by Value）

C 语言的参数默认是值传递——函数拿到的是原值的副本，修改不会影响原变量。

```c
void try_to_modify(int x) {
    x = 100;  // 只改了副本，不影响原值
}

int main(void) {
    int a = 5;
    try_to_modify(a);
    printf("a = %d\n", a);  // 输出: a = 5
    return 0;
}
```

```
函数调用与返回流程 (以 add(3,5) 为例):

─── 调用栈的变化 ─────────────────────────────────────────

  main() 栈帧                add() 栈帧
  ┌────────────────────┐    ┌────────────────────┐
  │  x = 3             │    │  a = 3 (x 的副本)   │ ← ① 压入参数
  │  y = 5             │    │  b = 5 (y 的副本)   │
  │  result = ?        │    │  返回地址 = &main+N │ ← ② 保存返回位置
  │  ...               │    └─────────┬──────────┘
  └────────┬───────────┘              │
           │ ③ PC 跳转到 add          │ ④ 计算 a+b=8, return
           │ ⑤ 返回值 8 存入 result    │ ⑥ 栈帧弹出, main 恢复执行
           └──────────────────────────┘

  流程: ① 参数入栈 → ② 返回地址入栈 → ③ 跳转函数
        → ④ 执行函数体 → ⑤ 返回值传回 → ⑥ 恢复调用者
```

## 常见错误

### ❌ 错误 1：函数返回类型不匹配

```c
// 错误：声明为 int，但返回了字符串
int get_name(void) {
    return "hello";  // ❌ 编译错误：incompatible types
}
```

✅ **修正**：返回类型要与实际返回值一致。

```c
const char *get_name(void) {
    return "hello";  // ✅ 正确
}
```

### ❌ 错误 2：忘记写 return

```c
int add(int a, int b) {
    a + b;  // ❌ 计算了但没返回！控制到达非 void 函数末尾
}
```

✅ **修正**：使用 `return` 关键字。

```c
int add(int a, int b) {
    return a + b;  // ✅ 正确
}
```

### ❌ 错误 3：参数名在声明中与定义中不一致

虽然参数名可以不同，但类型必须一致：

```c
// 声明
double divide(double a, double b);

// 定义 — 名字不同没问题，但类型必须一致
double divide(double x, double y) {  // ✅ 正确，类型一致
    if (y != 0.0) {
        return x / y;
    }
    return 0.0;
}
```

## 动手练习

### 🟢 练习 1：实现 `swap_display` 函数

```c
// 写一个函数，接收两个 int，按从小到大的顺序打印它们
```

<details>
<summary>点击查看答案</summary>

```c
void swap_display(int a, int b) {
    if (a > b) {
        printf("从小到大: %d, %d\n", b, a);
    } else {
        printf("从小到大: %d, %d\n", a, b);
    }
}
```
</details>

### 🟡 练习 2：实现 `factorial` 函数（递归版）

```c
// 写一个递归函数，计算 n 的阶乘（n!）
// factorial(5) = 5 * 4 * 3 * 2 * 1 = 120
```

<details>
<summary>点击查看答案</summary>

```c
long long factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return (long long)n * factorial(n - 1);
}
```
</details>

### 🔴 练习 3：实现 `is_prime` 函数

```c
// 写一个函数，判断一个正整数是否为素数
// is_prime(7)  = 1 (true)
// is_prime(12) = 0 (false)
```

<details>
<summary>点击查看答案</summary>

```c
int is_prime(int n) {
    if (n <= 1) {
        return 0;
    }
    if (n <= 3) {
        return 1;
    }
    if (n % 2 == 0 || n % 3 == 0) {
        return 0;
    }
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return 0;
        }
    }
    return 1;
}
```

> **技巧**：只需检查到 `sqrt(n)` 即可。循环步长为 6，因为大于 3 的素数一定是 6k ± 1 的形式。
</details>

## 故障排查（FAQ）

### Q: 函数可以返回数组吗？

**不能直接返回**。C 语言中数组不是"一等公民"。你可以返回指针（指向静态数组或动态分配的内存），但不能直接返回数组类型。

```c
// ✅ 正确：返回指向静态数组的指针
const int *get_numbers(void) {
    static int arr[] = {1, 2, 3};
    return arr;
}
```

### Q: `return` 能返回多个值吗？

**不能**。但你可以用结构体（struct）包装多个值返回，或者通过指针参数修改原变量（后续章节会讲）。

### Q: 函数的参数名必须在声明和定义中一致吗？

**不需要**。编译器只关心参数类型和数量。但**推荐保持一致**以提高可读性。

### Q: 函数中可以再定义函数吗？

**C 语言不支持嵌套函数定义**。但你可以嵌套*调用*函数。

## 知识扩展（选学）

### `_Noreturn` 属性（C11）

C11 引入了 `_Noreturn` 关键字，告诉编译器这个函数永远不会返回：

```c
#include <stdlib.h>

_Noreturn void fatal_error(const char *msg) {
    fprintf(stderr, "Fatal: %s\n", msg);
    exit(EXIT_FAILURE);
}
```

使用 `_Noreturn` 可以让编译器生成更精确的警告。

### `static` 函数（内部链接）

给函数加 `static` 关键字，可以限制函数只在当前文件可见：

```c
// file_utils.c

static int validate_path(const char *path) {
    // 仅本文件可调用，不会污染全局命名空间
    return (path != NULL && path[0] == '/');
}

int open_file(const char *path) {
    if (!validate_path(path)) {
        return -1;
    }
    // ... 打开文件
    return 0;
}
```

## 小结

恭喜你学完了 C 语言函数的核心概念！让我们回顾一下——

- 函数 = 返回类型 + 函数名 + 参数列表 + 函数体
- **声明**（Prototype）告诉编译器函数存在，**定义**包含完整实现
- 前向声明解决了"先调用后定义"的问题
- `void` 函数不返回值，但仍然可以提前退出（`return;`）
- C 语言参数传递是**值传递**，函数内修改不影响原变量
- `_Noreturn` 和 `static` 是进阶特性，能让代码更安全、更模块化

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 函数（Function） | 可重复调用的代码块 |
| 声明（Declaration） | 告知编译器函数原型，不含函数体 |
| 定义（Definition） | 包含完整函数体 |
| 原型（Prototype） | 函数的声明形式，包含返回类型、参数类型 |
| 前向声明（Forward Declaration） | 在函数被使用前做的声明 |
| 参数（Parameter） | 函数定义中的变量（形参） |
| 实参（Argument） | 调用函数时传入的具体值 |
| 返回值（Return Value） | 函数执行完毕后返回的数据 |
| 值传递（Pass by Value） | 传入的是副本，不影响原值 |
| 递归（Recursion） | 函数调用自身 |
| `void` | 无类型，用于无返回值的函数 |
| `_Noreturn` | C11 关键字，标记不会返回的函数 |
| `static`（用在函数上） | 限制函数只在当前文件内可见 |
| 调用栈（Call Stack） | 跟踪函数调用的数据结构 |

## 延伸阅读

- [cppreference: Functions in C](https://en.cppreference.com/w/c/language/functions)
- [Beej's Guide to C: Functions](https://beej.us/guide/bgc/)
- K&R《C 程序设计语言》第 1.7、4 章
- 《C Primer Plus》第 9 章：函数

## 继续学习

函数是 C 语言模块化的基础。下一章我们将学习**运算符与表达式**，了解 C 语言中丰富的运算符以及它们的优先级规则——这将让你写出更简洁、更精准的代码。

> 💡 **提示**：试试把之前写的代码改写成函数形式！把你的 `main()` 拆成 3-5 个函数，你会发现代码立刻变清爽了。
