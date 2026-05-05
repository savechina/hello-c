# 函数作用域（Function Scope & Visibility）

> "函数作用域就像办公室的门——有的门谁都看得见，有的门只有内部员工能进。" —— 我发现

## 开篇故事

想象一栋办公大楼。一楼大门的标识所有人都能看到（全局函数），但某些办公室只有佩戴特定门禁卡的员工才能进入（`static` 函数）。有些功能虽然大楼里存在，但前台没有登记（缺少前向声明），你即使知道它的名字也找不到路。

C 语言中的函数也有这样的"可见性"规则：有的函数整个程序都能调用，有的函数只能在自己的 `.c` 文件内部使用，还有的函数需要先"预告"才能调用。理解这些规则，你就能写出模块化、可维护的代码——并且避免各种"找不到函数"的链接错误。

函数作用域不是关于"函数能访问哪些变量"，而是关于**"哪些代码能访问这个函数"**。

> "函数在哪里声明，就在哪里可见；加了 `static`，就只能在自己的文件里称王。"

## 本章适合谁

- 已经写了几个函数，但对 `static` 函数的作用一知半解的人
- 被过"undefined reference" 链接错误，想知道为什么的人
- 想理解头文件中声明和 `.c` 文件中定义之间关系的人
- 好奇"前向声明"到底解决了什么问题的人

## 你会学到什么

- 局部变量 vs 全局变量在函数中的行为差异
- 变量遮蔽（Shadowing）及其风险
- `static` 函数：限制函数只在当前文件可见（内部链接）
- `extern` 函数：跨文件调用函数（外部链接）
- 前向声明（Forward Declaration）：为什么需要它、怎么用
- 函数作用域与变量作用域的本质区别

## 前置要求

- 已完成「函数基础](./functions.md) 章节，理解函数声明、定义、参数传递
- 已完成「变量与表达式](./variables.md) 章节，理解变量声明与作用域

> 如果还没学函数基础，建议先看「函数」章节。

## 第一个例子：局部变量 vs 全局变量

```c
#include <stdio.h>

int global_value = 100;  /* 全局变量：所有函数可见 */

void print_values(void) {
    int local_value = 50;  /* 局部变量：仅本函数可见 */
    printf("global = %d, local = %d\n", global_value, local_value);
}

/* 下列函数无法访问 local_value！ */
void try_access(void) {
    /* printf("%d", local_value);  ❌ 编译错误：局部变量不可见 */
    printf("global = %d\n", global_value);  /* ✅ 全局变量可见 */
}

int main(void) {
    print_values();
    try_access();
    return 0;
}
```

运行结果：

```
global = 100, local = 50
global = 100
```

关键点：
- `global_value` 在 `main()` 外面声明，**整个文件**中所有函数都能看到它
- `local_value` 在 `print_values()` 里面声明，只有 `print_values()` 能看到它
- `try_access()` 试图访问 `local_value` 会触发编译错误

## 原理解析

### 1. 变量作用域 vs 函数作用域

在学习函数之前，我们已经知道了变量有作用域（块作用域、文件作用域）。函数的"作用域"规则类似，但多了一个维度：**链接类型（Linkage）**。

| 作用域层次 | 变量的可见性 | 函数的可见性 |
|-----------|------------|------------|
| 块作用域（Block） | `{ }` 内可见 | `goto` 标签仅在函数内 |
| 函数作用域（Function） | 参数和局部变量 | 函数内的标签 |
| 文件作用域（File） | 全局变量、`static` 变量 | 普通函数、`static` 函数 |
| 程序作用域（Program） | `extern` 变量 | `extern` 函数 |

**ASCII 作用域金字塔**：

```
┌──────────────────────────────────────────────────────┐
│              变量作用域金字塔 (Scope Pyramid)          │
│                                                      │
│             ┌────────────────────┐                    │
│             │   程序作用域        │ ← extern 跨文件   │
│             │  (Program Scope)   │                    │
│             └────────┬───────────┘                    │
│                      │                               │
│             ┌────────┴───────────┐                    │
│             │   文件作用域        │ ← 全局/static    │
│             │  (File Scope)      │                    │
│             └────────┬───────────┘                    │
│                      │                               │
│             ┌────────┴───────────┐                    │
│             │   函数作用域        │ ← 参数/局部变量   │
│             │ (Function Scope)   │                    │
│             └────────┬───────────┘                    │
│                      │                               │
│             ┌────────┴───────────┐                    │
│             │   块作用域          │ ← { } 内变量     │
│             │  (Block Scope)     │                    │
│             └────────────────────┘                    │
│                                                      │
│  作用域越内层，可见范围越小，生命周期越短                │
└──────────────────────────────────────────────────────┘
```

### 2. 函数的链接类型（Linkage）

C 语言中，每个函数都有一个**链接类型**，决定了其他文件能否看到它：

```c
/* file_a.c */

void public_func(void) {  /* 外部链接：其他文件可以调用 */
    /* ... */
}

static void private_func(void) {  /* 内部链接：只有 file_a.c 能调用 */
    /* ... */
}
```

```c
/* file_b.c */

extern void public_func(void);  /* ✅ 可以调用 file_a.c 中的 public_func */

int main(void) {
    public_func();    /* ✅ OK */
    /* private_func();  ❌ 链接错误：找不到! */
    return 0;
}
```

**核心概念**：
- 没有 `static` 的函数 = **外部链接（External Linkage）** = 其他文件可见
- 有 `static` 的函数 = **内部链接（Internal Linkage）** = 只有当前文件可见

### 3. `static` 函数详解

`static` 用在函数前面，意思是"这个函数只属于当前 `.c` 文件，别让它出去"：

```c
/* calculator.c */

static int clamp(int value, int min, int max) {
    /* 辅助函数：确保值在 [min, max] 范围内 */
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

/* 公开函数：其他文件可以调用 */
int calculate_score(int raw_score) {
    return clamp(raw_score, 0, 100);  /* ✅ clamp 只在内部用 */
}
```

为什么用 `static`？
1. **隐藏实现细节**：把辅助函数标记为 `static`，外部代码无法直接调用它
2. **避免命名冲突**：两个不同的 `.c` 文件可以各有一个叫 `helper()` 的 `static` 函数，不会冲突
3. **编译器优化**：编译器知道 `static` 函数只在本文件调用，可以做出更好的优化决策

### 4. `extern` 关键字与跨文件调用

`extern` 告诉编译器"这个函数在别处定义，但我想在这里用它"：

```c
/* module_a.c */

void greeting(const char *name) {
    printf("Hello, %s!\n", name);
}
```

```c
/* module_b.c */

extern void greeting(const char *name);  /* 声明：greeting 在别处定义 */

void say_hi(void) {
    greeting("World");  /* ✅ 跨文件调用 */
}
```

### 5. 前向声明（Forward Declaration）

C 编译器从上往下读取代码。如果函数 `A` 要调用函数 `B`，但 `B` 的定义在 `A` 的**后面**，编译器就不认识 `B`：

```c
/* ❌ 错误示例 —— 没有前向声明 */

void print_result(void) {
    int value = compute_value();  /* ❌ 编译器不认识 compute_value */
    printf("Result: %d\n", value);
}

int compute_value(void) {
    return 42;
}
```

编译器报错：
```
error: implicit declaration of function 'compute_value'
```

✅ **修复：加前向声明**

```c
/* ✅ 正确 —— 前向声明 */

int compute_value(void);  /* 前向声明：告诉编译器 compute_value 存在 */

void print_result(void) {
    int value = compute_value();  /* ✅ 编译器已认识 */
    printf("Result: %d\n", value);
}

int compute_value(void) {
    return 42;  /* 定义在后面 */
}
```

### 6. 前向声明 vs 头文件

在实际项目中，前向声明通常放在**头文件**中统一管理：

```c
/* compute.h */
#ifndef COMPUTE_H
#define COMPUTE_H

int compute_value(void);  /* 前向声明放在头文件 */

#endif
```

```c
/* compute.c */
#include "compute.h"

int compute_value(void) {  /* 定义 */
    return 42;
}
```

```c
/* main.c */
#include <stdio.h>
#include "compute.h"  /* 通过头文件获得前向声明 */

int main(void) {
    printf("value = %d\n", compute_value());
    return 0;
}
```

**最佳实践**：永远用头文件管理函数声明，不要在各处重复手写 `extern` 声明。

### 7. 局部变量遮蔽全局变量

当局部变量和全局变量同名时，局部变量会"遮蔽"全局变量：

```c
#include <stdio.h>

int mode = 1;  /* 全局变量 */

void do_work(void) {
    int mode = 2;  /* 局部变量遮蔽了全局变量 */
    printf("inside: mode = %d\n", mode);  /* 输出 2，不是 1 */
}

int main(void) {
    printf("before: mode = %d\n", mode);  /* 输出 1 */
    do_work();
    printf("after:  mode = %d\n", mode);  /* 还是 1 —— 全局变量没被改 */
    return 0;
}
```

运行结果：
```
before: mode = 1
inside: mode = 2
after:  mode = 1
```

## 常见错误

### ❌ 错误 1：变量遮蔽导致混淆

```c
#include <stdio.h>

int counter = 10;

void increment(void) {
    int counter = 0;  /* ❌ 遮蔽了全局变量！ */
    counter++;
    printf("%d\n", counter);  /* 每次输出 1，不是递增！ */
}

int main(void) {
    increment();  /* 输出 1 */
    increment();  /* 输出 1（而不是 2） */
    return 0;
}
```

✅ **修复**：使用不同的名字，或者不加 `int` 声明直接修改全局变量：

```c
void increment(void) {
    /* int counter = 0;  ← 删除这行 */
    counter++;  /* ✅ 直接修改全局变量 */
    printf("%d\n", counter);
}
```

### ❌ 错误 2：调用 `static` 函数导致链接错误

```c
/* utils.c */
static int helper(int x) {
    return x * 2;
}

/* main.c */
int main(void) {
    int result = helper(5);  /* ❌ 链接错误：找不到 helper */
    return 0;
}
```

编译错误：
```
undefined reference to `helper'
collect2: error: ld returned 1 exit status
```

✅ **修复**：去掉 `static`，或通过公开函数间接调用：

```c
/* utils.c */
static int helper(int x) {
    return x * 2;
}

int public_api(int x) {  /* ✅ 公开函数 */
    return helper(x);
}

/* main.c */
extern int public_api(int x);
int main(void) {
    printf("%d\n", public_api(5));  /* ✅ 通过公开接口调用 */
    return 0;
}
```

### ❌ 错误 3：缺少前向声明导致隐式声明警告

```c
void print_hello(void) {
    greet();  /* ❌ greet 还没声明，编译器假设它返回 int */
}

void greet(void) {
    printf("Hello!\n");
}
```

现代编译器会用 `-Werror=implicit-function-declaration` 将其视为错误。

✅ **修复**：添加前向声明：

```c
void greet(void);  /* 前向声明 */

void print_hello(void) {
    greet();  /* ✅ OK */
}
```

## 动手练习

### 🟢 练习 1：创建一个 `static` 辅助函数

```c
/* 写一个 .c 文件：
   - 定义一个 static 函数 is_positive(int n)，判断 n 是否大于 0
   - 定义一个公开函数 print_sign(int n)，调用 is_positive() 打印 "+", "-" 或 "0"
   在另一个 .c 文件中尝试直接调用 is_positive()，观察链接错误 */
```

<details>
<summary>点击查看答案</summary>

```c
/* sign.c */
#include <stdio.h>

static int is_positive(int n) {
    return n > 0;
}

static int is_negative(int n) {
    return n < 0;
}

void print_sign(int n) {
    if (is_positive(n)) {
        printf("+");
    } else if (is_negative(n)) {
        printf("-");
    } else {
        printf("0");
    }
    printf("\n");
}
```

```c
/* main.c */
#include <stdio.h>

extern void print_sign(int n);
/* extern int is_positive(int n);  ← 链接错误！is_positive 是 static */

int main(void) {
    print_sign(5);   /* + */
    print_sign(-3);  /* - */
    print_sign(0);   /* 0 */
    return 0;
}
```

</details>

### 🟡 练习 2：演示变量遮蔽

```c
/* 写一个程序：
   - 声明全局变量 x = 100
   - 在一个函数内声明同名局部变量 x = 200
   - 在函数内外分别打印 x，观察遮蔽效应
   - 然后用另一个名字重新实现，避免遮蔽 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

int x = 100;  /* 全局变量 */

void demo_shadow(void) {
    int x = 200;  /* 遮蔽全局变量 */
    printf("inside (shadowed): x = %d\n", x);  /* 200 */
}

void demo_no_shadow(void) {
    int y = 200;  /* 使用不同的名字 */
    printf("inside (no shadow): global x = %d, y = %d\n", x, y);
}

int main(void) {
    printf("outside: x = %d\n", x);  /* 100 */
    demo_shadow();
    demo_no_shadow();
    printf("outside again: x = %d\n", x);  /* 100（没被改） */
    return 0;
}
```

</details>

### 🔴 练习 3：跨文件的前向声明

```c
/* 创建两个 .c 文件和一个头文件：
   - mathlib.h：声明 add(int, int) 和 subtract(int, int)
   - mathlib.c：实现这两个函数（add 公开，subtract 用 static）
   - main.c：包含头文件，调用 add()；尝试调用 subtract() 观察效果 */
```

<details>
<summary>点击查看答案</summary>

```c
/* mathlib.h */
#ifndef MATHLIB_H
#define MATHLIB_H

int add(int a, int b);
int subtract(int a, int b);  /* 声明在这里，但实现是 static */

#endif
```

```c
/* mathlib.c */
#include "mathlib.h"

int add(int a, int b) {
    return a + b;
}

static int subtract(int a, int b) {  /* static: 只有 mathlib.c 内部可见 */
    return a - b;
}
```

```c
/* main.c */
#include <stdio.h>
#include "mathlib.h"

int main(void) {
    printf("add(3, 4) = %d\n", add(3, 4));
    /* printf("%d\n", subtract(7, 2));  ← 链接错误! */
    return 0;
}
```

</details>

## 故障排查（FAQ）

### Q: "undefined reference" 是什么错误？

这是**链接错误**，不是编译错误。意思是：编译器找到了函数声明（所以编译通过了），但链接器在整个项目中找不到这个函数的**定义**（函数体）。

常见原因：
1. 函数名拼写错误（大小写不同也算）
2. 函数定义在另一个 `.c` 文件中，但你忘记把它加入编译
3. 函数被标记为 `static`，所以其他文件看不到它
4. 头文件声明了函数，但 `.c` 文件中没有实现

✅ **修复方法**：
- 检查拼写（`gcc` 的错误信息会告诉你函数名）
- 确保所有 `.c` 文件都被编译（`make` 或 `gcc *.c`）
- 去掉 `static` 或改用公开函数间接调用

### Q: 什么时候该用 `static` 函数？

**原则：默认把辅助函数标为 `static`，只在需要跨文件调用时才去掉 `static`。**

典型场景：
- 工具函数（字符串解析、数据校验、内部计算）—— 用 `static`
- 公开的 API 接口函数 —— 不用 `static`
- 两个 `.c` 文件需要同名辅助函数 —— 都用 `static`，不冲突

### Q: `static` 函数和 `static` 变量是一回事吗？

不完全一样，但逻辑类似：
- `static` **变量**（在函数内）：延长生命周期到整个程序
- `static` **变量**（在文件级）：限制为内部链接，其他文件不可见
- `static` **函数**：限制为内部链接，其他文件不可见

文件级的 `static` 变量和 `static` 函数含义相同——**限制可见性**。

### Q: 前向声明和头文件有什么关系？

前向声明是一个概念：在函数定义之前先告诉编译器它的存在。头文件是承载前向声明的**载体**。

```
  前向声明（概念）
       │
       ▼
  头文件（载体）: .h 文件中放函数声明
       │
       ▼
  .c 文件（使用者）: #include ".h" 获得声明
```

### Q: 可以把 `static` 函数声明在头文件中吗？

可以，但**不推荐**。`static` 意味着"每个包含这个头文件的 `.c` 文件都有自己的一份副本"，这通常不是你想要的效果。建议：`static` 函数直接在 `.c` 文件内部定义，不放头文件。

## 知识扩展（选学）

### 函数指针指向 `static` 函数

虽然 `static` 函数不能从其他文件直接调用，但你可以通过**函数指针**在文件内部间接调用它：

```c
#include <stdio.h>

static int add(int a, int b) {
    return a + b;
}

static int multiply(int a, int b) {
    return a * b;
}

/* 函数指针数组——两个 static 函数都能放入 */
int main(void) {
    int (*operations[2])(int, int) = {add, multiply};

    printf("add(3, 4)    = %d\n", operations[0](3, 4));
    printf("multiply(3, 4) = %d\n", operations[1](3, 4));
    return 0;
}
```

函数指针打破了"只能按名字调用"的限制——只要拿到函数指针，就能调用函数。但这个技巧只在**同一个文件内**有效（因为 `static` 函数的地址也无法被其他文件获取）。

### `extern "C"` 与 C++ 混合编程

如果你在 C++ 项目中调用 C 函数，需要用 `extern "C"` 告诉 C++ 编译器使用 C 语言的链接方式（C++ 有名称修饰，C 没有）：

```cpp
// C++ code
extern "C" {
    #include "my_c_lib.h"
}
```

这超出了纯 C 的范围，但作为扩展知识了解是有益的。

## 小结

函数作用域的核心规则可以浓缩为三条：

- **默认情况下，函数是公开的**（外部链接）：其他文件可以通过前向声明调用它
- **加了 `static`，函数变成私有的**（内部链接）：只有自己的 `.c` 文件能调用
- **前向声明是函数的"预告"**：告诉编译器函数存在，定义可以在别处

你还学到了：
- 局部变量可以遮蔽全局变量（但这是**反模式**，应该避免）
- `extern` 关键字用于声明跨文件的函数/变量
- 头文件是管理前向声明的最佳载体，不要在手写 `extern`
- "undefined reference" 链接错误的常见原因和修复方法

> **我的理解**：把函数作用域想象成权限管理——`static` 是最小权限原则，`extern` 是共享协作。好的代码应该是"该私有的私有，该公开的公开"，而不是所有函数都敞开门等着被调用。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 函数作用域（Function Scope） | 函数的可见范围 |
| 链接（Linkage） | 符号在文件间的可见性规则 |
| 外部链接（External Linkage） | 可被其他文件中 `extern` 引用 |
| 内部链接（Internal Linkage） | 仅当前文件可见（`static`） |
| 前向声明（Forward Declaration） | 在函数定义之前先声明它的原型 |
| 函数原型（Function Prototype） | 包含返回类型、函数名、参数类型的声明 |
| 变量遮蔽（Shadowing） | 局部变量隐藏同名的全局变量 |
| 公开函数（Public Function） | 无 `static`，可被其他文件调用 |
| 私有函数（Private Function） | 有 `static`，只能在本文件调用 |
| 链接错误（Linker Error） | 编译通过但链接时找不到函数定义 |
| 名称修饰（Name Mangling） | C++ 编译器对函数名进行编码的技术 |

## 延伸阅读

- [cppreference: Scope and Linkage (C)](https://en.cppreference.com/w/c/language/scope)
- [cppreference: Functions in C](https://en.cppreference.com/w/c/language/functions)
- [Beej's Guide to C: Static and Global Variables](https://beej.us/guide/bgc/)
- K&R《C 程序设计语言》第 4.4、4.5 章

## 继续学习

你现在已经理解 C 语言中函数的"可见性规则"。在后续章节中，我们将继续深入作用域体系——**头文件与模块系统**，学习如何正确地组织多文件项目，通过头文件管理接口与实现的分离。

> 💡 **提示**：检查你的代码里所有函数：辅助函数是否标记了 `static`？头文件中的声明与 `.c` 文件中的定义是否一致？有没有遗漏的前向声明？

[← 上一章：函数基础](./functions.md) | [下一章：作用域与生命周期 →](./scope.md)
