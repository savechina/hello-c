# 预处理器与宏（Preprocessor & Macros）

> "预处理器像是个隐形助手——你在写代码，它在帮你抄写、替换、修剪。" —— 我发现

## 开篇故事

刚开始学 C 的时候，我特别讨厌改"魔法数字"（Magic Number）。我的代码里到处是 `if (score > 60)`、`char buf[256]`、`for (int i = 0; i < 100; i++)`。有一天老师让我把所有 256 改成 512，我差点崩溃——得一个个找、一个个改，还怕漏掉。

后来我学会了 `#define`，才发现**预处理器就是代码的"批量替换工具"**。定义一次，处处生效。再后来，我学会了用 `#ifdef` 做跨平台代码——同一份代码，在 macOS 和 Linux 上编译出不同的行为。那一刻我感受到了 C 语言的灵活与强大。

但预处理器也有陷阱。我写过 `#define SQUARE(x) x * x`，结果 `SQUARE(3+2)` 算出来是 `11` 而不是 `25`——因为宏只是文本替换，不是函数。这次教训让我深刻理解了**宏展开的优先级问题**。

本章我就带你搞懂 C 预处理器，从基础常量到高级宏技巧，让你的代码更清晰、更安全。

## 本章适合谁

- 代码中满是"魔法数字"，想学会用常量的 C 初学者
- 对 `#include <stdio.h>` 到底做了什么感到好奇的人
- 想写跨平台代码（用 `#ifdef` 区分 macOS/Linux/Windows）的人
- 用过 `#define` 但踩过宏陷阱的人

## 你会学到什么

- `#define` 定义常量与宏函数
- `#include` 的工作原理（文本替换本质）
- 头文件卫士（Include Guard）机制
- `#undef` 取消宏定义
- 条件编译：`#ifdef` / `#ifndef` / `#elif` / `#endif`
- `#error` 在编译期中断并报错
- 字符串化运算符（`#`）：将参数转成字符串
- Token 拼接运算符（`##`）：连接两个标识符
- 多行宏与 `do { ... } while(0)` 惯用法
- 常见宏陷阱与防护措施

## 前置要求

- 能编译运行基本的 C 程序
- 了解函数调用与返回值
- 用过 `#include <stdio.h>`

## 第一个例子：用 `#define` 消除魔法数字

```c
#include <stdio.h>

#define MAX_STUDENTS 30
#define PASS_SCORE   60

int main(void) {
    int scores[] = {85, 45, 72, 58, 91};
    int passed = 0;

    for (int i = 0; i < 5; i++) {
        if (scores[i] >= PASS_SCORE) {
            passed++;
        }
    }
    printf("通过率: %d/%d\n", passed, 5);
    return 0;
}
```

运行结果：

```
通过率: 3/5
```

看起来没什么特别的？但如果我想把及格线从 60 改成 70，只需改一行 `#define PASS_SCORE 70`——比全局搜索替换安全得多。

## 原理解析

### 1. `#define` 常量替换

`#define` 的本质就是**文本替换**——编译器在编译之前，预处理器会把代码中所有出现的宏名替换为定义的值。

```c
#define PI 3.14159
#define APP_NAME "C Tutorial"

double area = PI * 5.0 * 5.0;
/* 预处理后变为: double area = 3.14159 * 5.0 * 5.0; */

printf("Hello from %s\n", APP_NAME);
/* 预处理后变为: printf("Hello from %s\n", "C Tutorial"); */
```

**我的理解**：`#define` 不是变量，不是常量，它就是简单的"查找替换"。预处理器不关心类型、不检查语法——它只负责替换。

### 2. `#include` 的本质

`#include` 做的事情非常朴素：**把被包含文件的全部内容复制粘贴到这个位置。**

```c
/* 你写的代码 */
#include <stdio.h>
int main(void) { printf("hi\n"); return 0; }

/* 预处理后，stdio.h 几千行内容被插入到这里 */
/* ... stdio.h 的全部内容 ... */
int main(void) { printf("hi\n"); return 0; }
```

**`<>` vs `""` 的区别**：
- `#include <stdio.h>`：从**系统头文件路径**查找（如 `/usr/include`）
- `#include "my.h"`：从**当前文件所在目录**开始查找，然后在 `-I` 参数指定的路径中查找

### 3. 头文件卫士（Include Guard）

同一个头文件被多次 `#include` 会导致重复定义错误。头文件卫士解决了这个问题：

```c
/* my.h */
#ifndef MY_H          /* 如果 MY_H 未定义 */
#define MY_H          /* 定义为标记 */

void my_function(void);  /* 实际内容 */

#endif                  /* 结束 */
```

**执行流程**：
1. 第一次包含：`MY_H` 未定义 → 进入 → 定义 `MY_H` → 包含内容
2. 第二次包含：`MY_H` 已定义 → 跳过整个文件 → 不重复包含

**我的理解**：头文件卫士就像一道门——第一次经过时开门并插上门栓，第二次来时发现门已经闩上了，就不会再进来了。

### 4. 宏函数（Macro Functions）

宏可以像函数一样带参数：

```c
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

int x = MAX(10, 20);  /* 展开为: (((10) > (20)) ? (10) : (20)) */
```

**括号为什么这么重要？** 因为宏只是文本替换，如果不加括号，运算符优先级会打乱你的预期：

```c
/* 不好的写法 */
#define BAD_SQUARE(x) x * x

/* BAD_SQUARE(3+2) 展开为: 3 + 2 * 3 + 2 = 3 + 6 + 2 = 11 (预期 25!) */

/* 正确的写法 */
#define GOOD_SQUARE(x) ((x) * (x))

/* GOOD_SQUARE(3+2) 展开为: ((3+2) * (3+2)) = 25 */
```

### 5. `#undef` 取消宏定义

```c
#define TEMP 42
printf("%d\n", TEMP);  /* 42 */

#undef TEMP
/* TEMP 在此之后不再可用 */
```

**用途**：避免宏名冲突，或在局部范围内临时使用某个宏。

### 6. 条件编译

```c
#ifdef __APPLE__
    printf("Running on macOS\n");
#elif defined(__linux__)
    printf("Running on Linux\n");
#elif defined(_WIN32)
    printf("Running on Windows\n");
#else
    printf("Unknown platform\n");
#endif
```

**常用场景**：
- 跨平台代码适配
- 调试信息开关（`#ifdef DEBUG`）
- 功能模块的启用/禁用

### 7. `#error` 编译期错误

```c
#if MAX_BUFFER < 64
    #error "MAX_BUFFER must be >= 64"
#endif
```

`#error` 会在**预编译阶段**立刻停止编译并输出错误信息。比运行时检查更早发现问题。

### 8. 字符串化（`#` 运算符）

`#` 把宏参数转换为字符串字面量：

```c
#define STRINGIFY(x) #x

printf("%s\n", STRINGIFY(hello world));
/* 展开为: printf("%s\n", "hello world"); */

/* 实用的调试宏 */
#define PRINT_VAR(v) printf(#v " = %d\n", v)

int x = 42;
PRINT_VAR(x);
/* 展开为: printf("x" " = %d\n", x); 输出 "x = 42" */
```

### 9. Token 拼接（`##` 运算符）

`##` 把两个标识符拼接成一个：

```c
#define CONCAT(a, b) a##b

int data1 = 100;
int data2 = 200;

printf("%d\n", CONCAT(data, 1));  /* 展开为 data1 → 100 */
```

### 10. 多行宏与 `do { ... } while(0)`

```c
#define SWAP(a, b) do {       \
    int t = (a);              \
    (a) = (b);                \
    (b) = t;                  \
} while (0)
```

**为什么用 `do {...} while(0)`？** 因为它能确保宏在任何上下文中都行为一致——即使在 `if` 语句中不带 `{}`：

```c
/* 如果宏是 { ... }，这个会出问题: */
if (condition)
    SWAP(a, b);  /* 分号导致语法错误 */
else
    return;

/* 但用 do {...} while(0) 就没问题: */
if (condition)
    SWAP(a, b);  /* do {...} while(0); 是完整的语句 */
else
    return;
```

## 常见错误

### ❌ 错误 1：宏函数缺少括号，导致优先级问题

```c
#define SQUARE(x) x * x

int result = SQUARE(3 + 2);
/* 展开为: 3 + 2 * 3 + 2 = 3 + 6 + 2 = 11 */
/* ❌ 期望 25，实际 11 */
```

✅ **修正**：参数和整个表达式都加括号。

```c
#define SQUARE(x) ((x) * (x))  /* ✅ 加上外层和内层括号 */

int result = SQUARE(3 + 2);
/* 展开为: ((3 + 2) * (3 + 2)) = 25 */
```

### ❌ 错误 2：宏头文件卫士拼写错误

```c
/* utils.h */
#ifndef UTILS_H
#define UTILX_H      /* ❌ 拼写错误：UTILX 而不是 UTILS */

void my_func(void);

#endif
```

✅ **修正**：`#define` 后的名字必须与 `#ifndef` 后的完全一致。

```c
/* utils.h */
#ifndef UTILS_H
#define UTILS_H      /* ✅ 一致 */

void my_func(void);

#endif
```

### ❌ 错误 3：宏参数有副作用时被多次求值

```c
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

int v = 5;
int result = MAX(v++, 3);
/* 展开为: (((v++) > (3)) ? (v++) : (3)) */
/* v++ 被求值了两次！v 变成 7 而不是预期 6 */
/* ❌ 副作用被重复执行 */
```

✅ **修正**：有副作用的参数不要传给宏，改用内联函数（`inline`）。

```c
static inline int max_int(int a, int b) {
    return (a > b) ? a : b;
}

int v = 5;
int result = max_int(v++, 3);  /* ✅ 函数只求值一次，v 变成 6 */
```

## 动手练习

### 🟢 练习 1：用 `#define` 定义安全缓冲区大小

```c
/* 定义 BUFFER_SIZE 为 128，用它声明一个字符数组，
   并用 snprintf 安全地写入字符串 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#define BUFFER_SIZE 128

int main(void) {
    char buffer[BUFFER_SIZE];
    const char *msg = "Hello, safe buffer!";
    snprintf(buffer, BUFFER_SIZE, "%s", msg);
    printf("%s\n", buffer);
    return 0;
}
```
</details>

### 🟡 练习 2：用 `#ifdef` 实现调试开关

```c
/* 定义 DEBUG_LEVEL 为 2，根据级别输出不同信息
   DEBUG_LEVEL >= 2: 打印详细调试信息
   DEBUG_LEVEL >= 1: 打印普通信息
   否则: 打印发布信息 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#define DEBUG_LEVEL 2

int main(void) {
#if DEBUG_LEVEL >= 2
    printf("[DEBUG] 详细模式: x = %d, y = %d\n", 42, 100);
#endif
#if DEBUG_LEVEL >= 1
    printf("[INFO] 程序开始执行。\n");
#endif
    return 0;
}
```
</details>

### 🔴 练习 3：实现一个安全的 `MAX` 内联函数 + 宏

```c
/* 实现 max_int 内联函数，并用宏 MAX_SAFE 自动选择
   类型（使用 _Generic，C11 特性）支持 int 和 double */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

static inline int max_int(int a, int b) {
    return (a > b) ? a : b;
}

static inline double max_double(double a, double b) {
    return (a > b) ? a : b;
}

#define MAX_SAFE(a, b) _Generic((a), \
    int:    max_int,                 \
    double: max_double               \
)(a, b)

int main(void) {
    printf("%d\n", MAX_SAFE(10, 20));        /* 20 (int) */
    printf("%.2f\n", MAX_SAFE(3.14, 2.72));  /* 3.14 (double) */
    return 0;
}
```

**`_Generic`** 是 C11 引入的类型泛型选择表达式，可以在编译期根据类型选择不同的函数。
</details>

## 故障排查（FAQ）

### Q: `#define` 和 `const` 有什么区别？该用哪个？

| 对比 | `#define` | `const` |
|------|-----------|---------|
| 类型检查 | 无（纯文本替换） | 有 |
| 调试器可见 | 不可见（已被替换） | 可见 |
| 作用域 | 从定义处到文件末尾（或 `#undef`） | 遵循 C 作用域规则 |
| 内存占用 | 不占内存（编译时替换） | 可能占内存 |

**我的建议**：优先用 `const`（更安全、可调试），只有在需要"代码片段替换"时才用 `#define`。

### Q: `#include "xxx.h"` 找不到文件怎么办？用 `-I` 参数指定搜索路径。

```bash
gcc -I/path/to/headers main.c -o main
```

### Q: 怎么查看预处理后的代码？

```bash
gcc -E main.c       # 输出到终端
gcc -E main.c -o main.i  # 输出到文件
```

### Q: `#define` 可以定义带多个参数的宏吗？

**可以**。参数用逗号分隔：

```c
#define PRINT_TWO(a, b) printf("%s %s\n", a, b)
PRINT_TWO("hello", "world");
```

## 知识扩展（选学）

### C11 `_Static_assert`

编译期断言，比 `#error` 更灵活：

```c
#include <stdio.h>

#define BUFFER_SIZE 32
_Static_assert(BUFFER_SIZE >= 64, "BUFFER_SIZE must be >= 64");

int main(void) { return 0; }
/* 编译错误: static assertion failed: "BUFFER_SIZE must be >= 64" */
```

### GCC 扩展：`#warning`

与 `#error` 类似，但只发出警告而不终止编译：

```c
#if DEBUG_MODE
    #warning "Debug mode is enabled — don't ship like this!"
#endif
```

### X-Macro 模式

一种高级宏技巧，用于生成重复代码（比如把枚举和字符串保持同步）：

```c
#define COLORS \
    X(RED, 0)    \
    X(GREEN, 1)  \
    X(BLUE, 2)

#define X(name, value) name = value,
enum Color { COLORS };
#undef X

#define X(name, value) [name] = #name,
const char *color_names[] = { COLORS };
#undef X
```

## 小结

祝贺！你已经掌握了 C 预处理器的核心机制。让我总结一下——

- **`#define`** 是文本替换，不是变量、不是函数
- **`#include`** 本质是复制粘贴，`<>` 查系统路径、`""` 查当前路径
- **头文件卫士**（`#ifndef`/`#define`/`#endif`）防止重复包含
- **宏函数**必须给参数和整体都加括号，否则优先级会出错
- **`#`** 字符串化，**`##`** token 拼接
- **多行宏**用 `do {...} while(0)` 确保行为正确
- **有副作用的参数**不要传给宏——用内联函数替代
- **`#error`** 在编译期捕获配置错误

> **我的理解**：预处理器的核心原则就是记住——它发生在编译之前，做的是纯文本操作。理解这一点，90% 的宏陷阱都可以避免。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 预处理器（Preprocessor） | 编译前处理代码的工具，以 `#` 开头 |
| 宏（Macro） | 用 `#define` 定义的文本替换规则 |
| 宏函数（Macro Function） | 带参数的宏，看起来像函数 |
| 魔法数字（Magic Number） | 代码中直接写死的硬编码常量 |
| 头文件卫士（Include Guard） | `#ifndef`/`#define`/`#endif` 防止重复包含 |
| 条件编译（Conditional Compilation） | 根据条件决定是否包含某段代码 |
| 字符串化（Stringification） | `#` 将宏参数转为字符串字面量 |
| Token 拼接（Token Pasting） | `##` 将两个标识符合并 |
| 副作用（Side Effect） | 表达式的额外影响（如 `i++` 改变变量值） |
| 内联函数（Inline Function） | 建议编译器内嵌的函数，替代宏函数 |
| `_Static_assert` | C11 编译期断言 |
| Textual Substitution | 预处理器只做文本替换，不理解语义 |

## 延伸阅读

- [cppreference: Preprocessor Directives (C)](https://en.cppreference.com/w/c/preprocessor)
- [GCC Preprocessor Options](https://gcc.gnu.org/onlinedocs/cpp/)
- K&R《C 程序设计语言》第 4 章：宏
- 《C Primer Plus》第 16 章：C 预处理器和 C 库

## 继续学习

预处理器是 C 语言"元编程"的基础。下一章我们将学习**数组**，掌握 C 语言中最基本的集合数据结构——从一维数组到多维数组，从内存布局到指针运算。

> 💡 **提示**：检查你现有代码中的所有"魔法数字"，用 `#define` 或 `const` 替换它们。你会发现代码可读性立刻提升了！

[← 上一章：循环](./loops.md) | [下一章：数组 →](./arrays.md)
