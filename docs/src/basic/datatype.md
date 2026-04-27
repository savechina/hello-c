# 数据类型 (Data Types)

## 开篇故事

有一次，我写了一个简单的程序来计算年龄：

```c
int age = 30;
age = age - 50;
printf("你将在 %d 年后出生！\n", age);  // 输出了负数 😅
```

我很困惑：为什么会出现负数？后来我才明白，C 语言中的每个数据类型就像**一个固定大小的容器**。`int` 容器是有符号的，可以装正数和负数；而 `unsigned int` 容器只能装正数。如果我想表达"无符号"的概念——比如年龄永远不可能是负数——我就选错了容器。

那天我发现：**在 C 语言中，选择正确的数据类型不是小事，而是你理解计算机如何管理内存的第一步**。每种类型都有确定的字节数、范围和精度。理解这些，你就掌握了 C 语言的核心。

## 本章适合谁

- 刚学完"变量与表达式"，想知道 C 语言有哪些数据类型
- 对 `int`、`float`、`char` 只停留在表面认识，想深入理解它们的区别
- 听说过 `int32_t`、`uint64_t` 但不知道为什么需要它们
- 想知道 `sizeof` 运算符和类型修饰符 `const` 的用法
- 希望理解不同平台上类型大小可能不同的问题

## 你会学到什么

- `int8_t` / `int16_t` / `int32_t` / `int64_t` ——精确宽度的整数类型
- `float` 和 `double` ——浮点数的精度差异
- `char`、`signed char`、`unsigned char` ——字符和单字节整数
- `sizeof` 运算符 ——查询类型或变量的字节大小
- `INT_MAX` / `FLT_MAX` 等极限常量 ——了解每种类型的取值范围
- `signed` / `unsigned` 修饰符 ——正负范围的切换
- `const` 类型修饰符 ——定义只读常量

## 前置要求

- 已完成 [变量与表达式](./variables) 章节
- 已配置 C 编译环境（`gcc` 或 `clang`）
- 了解基本的 `printf` 用法

> **💡 小知识**：本教程代码符合 C17 标准（`-std=c17`），使用 `<stdint.h>` 中的精确宽度类型。

## 第一个例子

这是本章最简短的例子——看看 `sizeof` 能告诉你什么：

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    printf("char    = %zu 字节\n", sizeof(char));
    printf("int     = %zu 字节\n", sizeof(int));
    printf("int64_t = %zu 字节\n", sizeof(int64_t));
    printf("float   = %zu 字节\n", sizeof(float));
    printf("double  = %zu 字节\n", sizeof(double));
    return 0;
}
```

完整源码在仓库 [`src/basic/datatype.c`](https://github.com/savechina/hello-c/blob/main/src/basic/datatype.c)。

## 原理解析

### 1. 整数类型：从 `int` 到 `int64_t`

C 语言提供多种整数类型。传统的 `int`、`short`、`long` 在不同平台上的大小可能不同——这在 cross-platform 编程中是个大坑。

从 C99 开始，`<stdint.h>` 提供了一组**精确宽度类型**，保证在所有平台上大小一致：

| 类型 | 宽度 | 取值范围 | 用途 |
|------|------|----------|------|
| `int8_t` | 8 位 | -128 ~ 127 | 状态标志、紧凑数据 |
| `int16_t` | 16 位 | -32,768 ~ 32,767 | 端口号 |
| `int32_t` | 32 位 | ±21 亿 | 最常用的整数 |
| `int64_t` | 64 位 | ±9.2×10¹⁸ | 大文件偏移量、时间戳 |

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int8_t  status   = -1;
    int32_t count    = 42;
    int64_t filesize = 9223372036854775807LL;

    // 使用 PRId8、PRId32、PRId64 格式化宏（来自 <inttypes.h>）
    printf("status:   %" PRId8 "\n", status);
    printf("count:    %" PRId32 "\n", count);
    printf("filesize: %" PRId64 "\n", filesize);
    return 0;
}
```

> **💡 小知识**：`PRId8` 是一个格式化宏，在不同平台上会被展开为 `"d"` 或其他合适的格式。它确保 `printf` 能正确读取 `int8_t` 类型。

### 2. 浮点类型：float vs double

浮点数用于表示小数。C 语言提供两种基本浮点类型：

```c
float  pi_float  = 3.14159f;       // 4 字节，单精度
double pi_double = 3.14159265358979; // 8 字节，双精度
```

- `float`：大约 7 位有效数字
- `double`：大约 15 位有效数字

> **我的经验**：除非在嵌入式系统中内存非常紧张，否则我默认使用 `double`。精度不足的代价可能远超节省的 4 字节。

### 3. 字符类型：不只是字符

`char` 在 C 语言中是一个字节，但它既可以是字符，也可以是小整数：

```c
char        c  = 'A';      // 字符，ASCII 值 65
signed char   sc = -42;     // 有符号，范围 -128 ~ 127
unsigned char uc = 200;     // 无符号，范围 0 ~ 255
```

> **重要**：`char` 到底是有符号还是无符号，取决于编译器。如果你需要明确的符号行为，请始终使用 `signed char` 或 `unsigned char`。

### 4. sizeof 运算符

`sizeof` 是 C 语言的内置运算符（不是函数！），返回类型或变量的字节数。

```c
#include <stdio.h>

int main(void) {
    int x = 42;
    printf("int 类型大小: %zu 字节\n", sizeof(int));   // 类型
    printf("变量 x 大小:  %zu 字节\n", sizeof(x));      // 变量
    printf("表达式大小:   %zu 字节\n", sizeof(x + 1));   // 表达式
    return 0;
}
```

> **🧪 动手试试**：试试 `sizeof(double)` 和 `sizeof(double)` 是否相等。试试 `sizeof(3.14)` 的结果是多少？（提示：3.14 默认是 `double` 类型）

### 5. 类型极限（Limits）

C 标准库在 `<limits.h>` 和 `<float.h>` 中定义了每种类型的最大值和最小值：

```c
#include <stdio.h>
#include <limits.h>
#include <float.h>

int main(void) {
    printf("INT_MAX  = %d\n", INT_MAX);     // 2147483647
    printf("INT_MIN  = %+d\n", INT_MIN);    // -2147483648
    printf("UINT_MAX = %u\n", UINT_MAX);    // 4294967295
    printf("FLT_MAX  = %.2e\n", FLT_MAX);   // 3.40e+38
    printf("DBL_MAX  = %.2e\n", DBL_MAX);   // 1.79e+308
    printf("CHAR_BIT = %d\n", CHAR_BIT);    // 8（一个 char 有多少位）
    return 0;
}
```

我的建议：**在写数值相关的代码时，养成查极限常量的习惯**。它可以帮你避免溢出错误。

### 6. signed 与 unsigned

类型前面的 `signed` 和 `unsigned` 修饰符决定了该类型能否表示负数。

```c
signed int   si = -10;   // 可以表示负数
unsigned int ui = 10;    // 只能表示 0 及正数

// unsigned 的范围更大（正数多一倍）
// signed int:   -2147483648 ~ 2147483647
// unsigned int: 0           ~ 4294967295
```

> **⚠️ 危险**：将 `signed` 和 `unsigned` 混用进行算术运算或比较时，C 会自动将 `signed` 转为 `unsigned`。`-1` 会变成一个巨大的正数！

```c
#include <stdio.h>

int main(void) {
    signed int   a = -1;
    unsigned int b = 2;
    if (a > b) {
        printf("-1 > 2 ？？\n");  // 会打印！因为 -1 转为 unsigned 后是一个巨大的数
    }
    return 0;
}
```

### 7. const 类型修饰符

`const` 告诉编译器：这个变量的值在初始化后不应该被修改。

```c
const int   MAX_RETRIES = 3;
const float GRAVITY     = 9.81f;
const char *GREETING    = "Hello, C!";
```

> 我的理解：`const` 不仅仅是编译时的安全检查——它也向阅读代码的人传达意图。看到 `const`，我就知道"这个值不应该被改变"。

## 常见错误

### ❌ 错误 1：赋值超出类型范围

```c
#include <stdint.h>

int8_t temperature = 200;  // ❌ int8_t 最大值是 127！
```

**编译器警告**（使用 `-Wall -Wextra`）：
```
warning: implicit conversion from 'int' to 'int8_t' changes value from 200 to -56
```

**✅ 修复**：使用更大类型

```c
int16_t temperature = 200;  // ✅ int16_t 范围 -32768 ~ 32767
```

### ❌ 错误 2：修改 const 变量

```c
const int MAX = 100;
MAX = 200;  // ❌ error: assignment of read-only variable 'MAX'
```

**✅ 修复**：如果值需要改变，去掉 `const`

```c
int max = 100;  // ✅ 可以修改
max = 200;
```

### ❌ 错误 3：signed/unsigned 比较陷阱

```c
#include <stdio.h>

int main(void) {
    int array_length = -1;      // -1
    unsigned int size = 5;      // 5
    if (array_length < size) {  // ❌ -1 被当作 unsigned，变成一个很大的数
        printf("-1 < 5\n");     // 这行不会执行！
    }
    return 0;
}
```

**✅ 修复**：确保比较的两个变量类型一致

```c
int array_length = -1;
int size = 5;
if (array_length < size) {      // ✅ 都是 signed int
    printf("-1 < 5\n");
}
```

## 动手练习

### 🟢 练习 1：sizeof 计算器

编写一段代码，依次打印 `char`、`short`、`int`、`long`、`long long`、`float`、`double`、`long double` 的字节大小，并找出哪个是当前平台上最大的基本类型。

<details>
<summary>查看答案</summary>

```c
#include <stdio.h>

int main(void) {
    printf("char        = %zu 字节\n", sizeof(char));
    printf("short       = %zu 字节\n", sizeof(short));
    printf("int         = %zu 字节\n", sizeof(int));
    printf("long        = %zu 字节\n", sizeof(long));
    printf("long long   = %zu 字节\n", sizeof(long long));
    printf("float       = %zu 字节\n", sizeof(float));
    printf("double      = %zu 字节\n", sizeof(double));
    printf("long double = %zu 字节\n", sizeof(long double));
    return 0;
}
```

在 macOS 上，`long double` 通常是 16 字节，是最大的基本类型。
</details>

### 🟡 练习 2：类型安全计数器

编写一个函数 `void safe_increment(uint8_t *counter)`，接受一个 `uint8_t` 指针作为计数器。如果计数器没有达到最大值（255），则加 1；否则打印 "overflow" 并保持值不变。在主函数中演示从 253 加到 256 的过程（注意 256 无法用 `uint8_t` 表示）。

<details>
<summary>查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

void safe_increment(uint8_t *counter) {
    if (*counter == UINT8_MAX) {
        printf("overflow! 计数器已达最大值 %" PRIu8 "\n", *counter);
    } else {
        (*counter)++;
    }
}

int main(void) {
    uint8_t counter = 253;
    for (int i = 0; i < 4; i++) {
        printf("当前值: %" PRIu8 "  → ", counter);
        safe_increment(&counter);
        printf("结果: %" PRIu8 "\n", counter);
    }
    return 0;
}
```

输出：
```
当前值: 253  → 结果: 254
当前值: 254  → 结果: 255
当前值: 255  → overflow! 计数器已达最大值 255
当前值: 255  → overflow! 计数器已达最大值 255
```
</details>

### 🔴 练习 3：自定义类型信息结构体

定义一个结构体 `TypeInfo { const char *name; size_t bytes; long long min_val; unsigned long long max_val; }`，编写函数 `print_type_info(TypeInfo info)` 来格式化打印类型信息。然后为 `int8_t`、`int16_t`、`int32_t`、`int64_t`、`uint8_t` 创建 `TypeInfo` 实例并打印。

<details>
<summary>查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

typedef struct {
    const char       *name;
    size_t           bytes;
    long long        min_val;
    unsigned long long max_val;
} TypeInfo;

void print_type_info(TypeInfo info) {
    printf("  %-10s | %zu 字节 | min: %20lld | max: %20llu\n",
           info.name, info.bytes, info.min_val, info.max_val);
}

int main(void) {
    TypeInfo types[] = {
        {"int8_t",   sizeof(int8_t),   SCHAR_MIN,     UCHAR_MAX},
        {"int16_t",  sizeof(int16_t),  SHRT_MIN,      USHRT_MAX},
        {"int32_t",  sizeof(int32_t),  INT_MIN,       (unsigned)INT_MAX},
        {"int64_t",  sizeof(int64_t),  LLONG_MIN,     ULLONG_MAX},
        {"uint8_t",  sizeof(uint8_t),  0,             UCHAR_MAX},
    };

    int count = sizeof(types) / sizeof(types[0]);
    printf("  类型       | 字节数 | 最小值               | 最大值               \n");
    printf("  -----------|--------|----------------------|----------------------\n");

    for (int i = 0; i < count; i++) {
        print_type_info(types[i]);
    }
    return 0;
}
```

这个练习综合了结构体、`sizeof`、极限常量、格式化输出等多个知识点。
</details>

## 故障排查 (FAQ)

**Q: 为什么 `int` 在我的电脑上是 4 字节，但在别人的电脑上是 2 字节？**

A: C 标准只规定了 `short` ≤ `int` ≤ `long` ≤ `long long` 的相对大小，不规定绝对值。使用 `<stdint.h>` 中的 `int32_t` 等精确宽度类型可以避免这个问题。

**Q: `sizeof` 返回的是什么类型？为什么我用 `%d` 打印会有警告？**

A: `sizeof` 返回 `size_t` 类型（通常等同于 `unsigned long` 或 `unsigned long long`）。应该用 `%zu` 格式打印：`printf("%zu\n", sizeof(int))`。

**Q: `float` 和 `double` 在什么情况下应该选择 `float`？**

A: 三种情况：(1) 大量数据存储时节省内存；(2) GPU 编程中 float 通常比 double 快；(3) 嵌入式系统内存受限。其他情况，用 `double`。

**Q: `char` 到底是有符号还是无符号？**

A: C 标准没有规定，取决于你的平台。x86 上通常是 `signed`，ARM 上通常是 `unsigned`。如果需要明确的行为，始终用 `signed char` 或 `unsigned char`。

## 知识扩展 (选学)

### 类型别名与 typedef

你可以用 `typedef` 为已有类型创建别名，这在项目中非常常见：

```c
typedef uint32_t pixel_t;    // 定义一个表示像素的类型
typedef uint8_t  byte_t;     // byte_t 就是 uint8_t 的别名

pixel_t color = 0xFF0000;    // 比 uint32_t 更具语义
```

### 浮点数精度问题

`float` 和 `double` 使用 IEEE 754 标准表示，**不能精确表示所有小数**：

```c
double x = 0.1 + 0.2;
printf("%.20f\n", x);  // 输出: 0.30000000000000004000，不是 0.3！
```

我的建议：比较浮点数时，不要直接用 `==`，而是**用误差范围**：

```c
#include <math.h>
if (fabs(a - b) < 1e-9) {
    // a 和 b 在误差范围内相等
}
```

### _Bool 类型（C99 起）

C99 引入了 `_Bool` 类型，`<stdbool.h>` 提供了更友好的别名 `bool`：

```c
#include <stdbool.h>

bool is_ready = true;
if (is_ready) {
    printf("Ready!\n");
}
```

## 小结

本章介绍了 C 语言的核心数据类型体系：

| 类别 | 关键类型 | 要点 |
|------|----------|------|
| 整数 | `int8_t` → `int64_t` | 使用精确宽度类型，避免平台差异 |
| 浮点 | `float` / `double` | 注意精度，比较时用误差范围 |
| 字符 | `char` / `signed char` / `unsigned char` | 注意 `char` 符号性取决于平台 |
| 运算符 | `sizeof` | 返回 `size_t`，用 `%zu` 打印 |
| 极限值 | `INT_MAX` / `FLT_MAX` 等 | 写数值代码前查极限常量 |
| 修饰符 | `signed` / `unsigned` / `const` | 明确符号性，用 `const` 表达意图 |

**关键术语**：
- **精确宽度类型**：`intN_t`，保证在所有平台上大小一致
- **溢出**：数值超出类型范围，导致不可预期的行为
- **格式化宏**：`PRIdN` 系列，用于正确打印精确宽度类型
- **IEEE 754**：浮点数的标准表示方式

## 术语表

| 英文 | 中文 | 说明 |
|------|------|------|
| data type | 数据类型 | 值的类型和内存占用 |
| integer type | 整数类型 | 表示整数的类型 |
| floating-point type | 浮点类型 | 表示小数的类型 |
| signed / unsigned | 有符号 / 无符号 | 能否表示负数 |
| precision | 精度 | 浮点数的有效数字位数 |
| overflow | 溢出 | 数值超出类型可表示范围 |
| type modifier | 类型修饰符 | 修饰基础类型的关键词（const, signed 等） |
| format specifier | 格式说明符 | printf 中描述类型的占位符 |
| format macro | 格式化宏 | PRId8 等用于精确宽度类型的宏 |
| byte | 字节 | 最小可寻址内存单元（通常 8 位） |
| size_t | size_t | sizeof 的返回类型，无符号整数 |
| IEEE 754 | IEEE 754 | 浮点数表示的国际标准 |

## 延伸阅读

- **C99 标准 — `<stdint.h>`**：[cppreference — Fixed width integer types](https://en.cppreference.com/w/c/types/integer)
- **浮点数陷阱**：[What Every Programmer Should Know About Floating-Point Arithmetic](https://floating-point-gui.de/)
- **C 语言极限常量**：[cppreference — limits.h](https://en.cppreference.com/w/c/types/climits)
- **C 语言类型系统**：ISO/IEC 9899:2018, §6.2.5 Types

## 继续学习

- 上一章：[变量与表达式](./variables)
- 下一章：[函数](./functions)

---

> 本章代码位于仓库 [`src/basic/datatype.c`](https://github.com/savechina/hello-c/blob/main/src/basic/datatype.c) 和 [`src/basic/datatype_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/datatype_sample.c)。
> 运行 `make build && make run` 查看完整演示输出。
