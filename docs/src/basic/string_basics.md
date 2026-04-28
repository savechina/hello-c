# 字符串基础 (String Basics)

C 字符串像一列火车，最后一节车厢永远是 `'\0'`。没有这节车厢，整列火车就不知道在哪一站该到站——它会一直开下去，驶进未知的荒野。

## 开篇故事

第一次学 C 语言的时候，我以为字符串就像一个 Python 字符串——一个自带长度的对象。结果我被 `'\0'` 教训了一顿。

```
Python 的字符串:
s = "Hello"
→ str 对象内部: [数据指针] [长度=5] [哈希值] ...

C 的字符串:
char s[] = "Hello";
→ 内存: ['H']['e']['l']['l']['o']['\0']
→ 没有长度！C 语言从第一个字符开始读，读到 '\0' 就停。
```

我写了下面的代码，编译器没报错，运行的时候却输出了一大堆乱码：

```c
char no_null[5] = {'H', 'e', 'l', 'l', 'o'};  /* 没有 '\0'！ */
printf("%s\n", no_null);   /* 💣 输出乱码！ */
```

`printf` 沿着内存一直读下去，直到偶然撞见一个 `0` 字节——那可能是一段完全无关的数据。这次踩坑让我记住了：**C 字符串没有长度属性，全靠 `'\0'` 收尾。**

## 本章适合谁

- 刚学完 [数据类型](./datatype.md) 和 [数组](./arrays.md)，想知道字符串到底是什么
- 用过 Python/JavaScript 的 `str`，对 C 的 `char*` 感到困惑
- 被 `printf` 输出乱码伤害过
- 想知道 `char[]` 和 `char*` 到底有什么区别

## 你会学到什么

1. C 字符串的本质：`char` 数组 + `'\0'` 终止符
2. `char[]` 和 `char*` 的区别——栈内存 vs 只读字面量
3. ASCII 内存图：字符串在栈上怎么存储
4. Python `str` vs C `char[]`：自动管理 vs 手动管理
5. 字符串字面量的特性（编译器是否合并、转义字符）
6. 为什么忘记 `'\0'` 会导致未定义行为

## 前置要求

- 已完成 [数据类型](./datatype.md) 章节（理解 `char` 是一个字节）
- 已完成 [数组](./arrays.md) 章节（理解数组的声明和索引）
- 已配置 C 编译环境（`gcc` 或 `clang`）

> **💡 编译命令**：本章代码使用 `-Wall -Wextra -Werror -std=c17` 编译。

## 第一个例子

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    /* char[]: 在栈上分配的字符串 */
    char greeting[] = "Hello, C!";

    printf("内容: %s\n", greeting);
    printf("长度: %zu (不含 '\\0')\n", strlen(greeting));
    printf("大小: %zu 字节 (含 '\\0' 的空间)\n", sizeof(greeting));

    /* 检查最后一个字符 */
    printf("最后一个字符的 ASCII 值: %d (就是 '\\0'!)\n", greeting[strlen(greeting)]);

    return 0;
}
```

编译并运行：

```bash
gcc -Wall -Wextra -Werror -std=c17 -o demo demo.c
./demo
```

输出：

```
内容: Hello, C!
长度: 9 (不含 '\0')
大小: 10 字节 (含 '\0' 的空间)
最后一个字符的 ASCII 值: 0 (就是 '\0'!)
```

## 原理解析

### 1. C 字符串的本质

C 语言没有内置的"字符串类型"。字符串本质上就是一个 `char` 数组，最后一个元素必须是 `'\0'`（ASCII 值 0，也叫 **null terminator**）。

```c
char greeting[] = "Hello, C!";
```

编译器在背后做了两件事：

1. 分配足够容纳 `"Hello, C!"` 加 `'\0'` 的字节（共 10 字节）
2. 逐字节填充内容

**ASCII 内存图**：

```
char greeting[10] = "Hello, C!" 在栈上：

地址偏移  +0    +1    +2    +3    +4    +5    +6    +7    +8    +9
         ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐
greeting │ 'H' │ 'e' │ 'l' │ 'l' │ 'o' │ ',' │ ' ' │ 'C' │ '!' │ '\0'│
         │ 72  │ 101 │ 108 │ 108 │ 111 │ 44  │ 32  │ 67  │ 33  │  0  │
         └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘
```

**三条铁律**：

1. `'\0'` 必须存在——没有它，`printf("%s")` 和 `strlen()` 无法判断边界
2. `strlen()` 返回 `'\0'` 前面的字符数（**不含** `'\0'`）
3. `sizeof` 返回数组的总字节数（**含** `'\0'` 的空间）

### 2. `char[]` vs `char*`

这是最常见的困惑来源：

```c
/* char[]: 在栈上分配完整数组，内容可修改 */
char greeting[] = "Hello";
greeting[0] = 'h';   /* ✅ OK */

/* char*: 指向只读字符串字面量（.rodata 段），不可修改 */
const char *literal = "Hello";
literal[0] = 'h';    /* ❌ Segmentation Fault! 只读内存 */
```

**`sizeof` 的差异**：

```c
printf("sizeof(greeting) = %zu\n", sizeof(greeting));  /* 6 (整个数组) */
printf("sizeof(literal)  = %zu\n", sizeof(literal));   /* 8 (指针本身!) */
```

| 特性 | `char greeting[] = "Hello"` | `const char *p = "Hello"` |
|------|-----------------------------|---------------------------|
| 存储位置 | 栈 (可修改) | .rodata (只读) + 栈上的指针 |
| 内容可修改 | ✅ | ❌ |
| `sizeof` | 数组总大小（6） | 指针大小（8） |
| 重新赋值 | ❌（数组名不能改） | ✅（指针可以改指向） |

**经验法则**：需要修改内容 → `char[]`；只需要引用 → `const char*`。

### 3. Python `str` vs C `char[]` 对比

```
┌── Python str ────────────────────────┐
│ s = "Hello"                           │
│                                      │
│ 内部结构:                             │
│ ┌────────────────────┐               │
│ │ PyObject_HEAD      │               │
│ │ Py_ssize_t len=5   │ ← len() O(1)  │
│ │ char data[]="Hello"│               │
│ │ hash, flags, ...   │               │
│ └────────────────────┘               │
│ 自动管理内存 | 不可变 | 有边界检查    │
└──────────────────────────────────────┘

┌── C string ──────────────────────────┐
│ char s[] = "Hello";                   │
│                                      │
│ 内存布局:                             │
│ ['H']['e']['l']['l']['o']['\0']       │
│                                      │
│ strlen(s) → O(n) 遍历找 '\0'          │
│ char[] 可修改 | 无边界检查            │
└──────────────────────────────────────┘
```

Python 把长度藏在对象里——你看不见但安全。C 把长度甩给你算——你看得见但危险。

### 4. 字符串字面量

```c
const char *s1 = "Hello";
const char *s2 = "Hello";

printf("s1 = %p\n", (void *)s1);
printf("s2 = %p\n", (void *)s2);
/* 现代编译器可能会合并 s1 和 s2 指向同一个地址 */
```

字符串字面量存储在只读段。即使 `s1 == s2` 可能为真（编译器优化），你也**不应该修改**它们中的任何一个。

**转义字符**：

```c
char escapes[] = "Tab\tNewline\nBackslash\\";
// → Tab + Tab字符 + Newline + 回车换行 + Backslash + 反斜杠
```

**多行字符串字面量拼接**：

```c
printf("%s\n", "Hello, "
               "world!");
/* → "Hello, world!" — 编译器自动拼接相邻字面量 */
```

### 5. 为什么 `'\0'` 如此重要？

```c
char no_null[5] = {'H', 'e', 'l', 'l', 'o'};
printf("%s\n", no_null);
/* → "Hello]" + 随机垃圾，直到内存中偶然遇到 '\0' */
```

没有 `'\0'`，所有以 `char*` 为参数的标准库函数（`printf`, `strlen`, `strcpy`, `strcmp`...）都会越界读取。这是**未定义行为 (Undefined Behavior)**——程序可能看起来正常，可能崩溃，可能 silently 损坏数据。

## 常见错误

### ❌ 错误 1：忘记 `'\0'`

```c
/* ❌ 危险 */
char buf[5] = {'H', 'e', 'l', 'l', 'o'};  /* 没有 \0! */
printf("%s\n", buf);   /* 💣 输出随机垃圾 */

/* ✅ 修复 */
char buf[6] = {'H', 'e', 'l', 'l', 'o', '\0'};  /* 显式 \0 */
/* 或者更简单： */
char buf[] = "Hello";  /* 编译器自动加 \0 */
```

### ❌ 错误 2：用 `char*` 却想修改内容

```c
/* ❌ 危险 */
char *s = "Hello";
s[0] = 'h';   /* 💣 段错误！字面量在只读段 */

/* ✅ 修复 */
char s[] = "Hello";  /* char[] 在栈上，可修改 */
s[0] = 'h';
```

### ❌ 错误 3：混淆 `sizeof` 和 `strlen`

```c
char greeting[] = "Hello";

printf("sizeof = %zu\n", sizeof(greeting));  /* 6 (含 \0 的空间) */
printf("strlen = %zu\n", strlen(greeting));  /* 5 (不含 \0)  */

/* ❌ 错误: 以为 sizeof 就是字符串长度 */
if (sizeof(greeting) >= 5) { ... }  /* 得到的是 6，不是 5! */

/* ✅ 正确: 字符串长度用 strlen */
if (strlen(greeting) >= 5) { ... }  /* 得到的是 5 */
```

### ❌ 错误 4：`char*` 的 `sizeof` 得到指针大小

```c
const char *p = "Hello";
printf("sizeof(p) = %zu\n", sizeof(p));  /* 8 (在 64 位机器上) */
/* 这不是字符串的长度！这只是指针本身的大小 */
```

## 动手练习

### 🟢 入门：验证字符串长度

声明 `char msg[] = "Hello, World!"`，分别用 `strlen` 和 `sizeof` 查看区别。再逐一打印每个字符的 ASCII 值，直到遇到 `'\0'`。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char msg[] = "Hello, World!";
    printf("strlen = %zu, sizeof = %zu\n", strlen(msg), sizeof(msg));

    for (size_t i = 0; i < sizeof(msg); i++) {
        if (msg[i] == '\0') {
            printf("  msg[%zu] = '\\0' (0)\n", i);
        } else {
            printf("  msg[%zu] = '%c' (%d)\n", i, msg[i], msg[i]);
        }
    }
    return 0;
}
```

</details>

### 🟡 中级：手动拼接两个字符

用 `char` 数组手动构建字符串 `"Hi"`——先声明足够大的数组，再逐个填入字符，最后加上 `'\0'`。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char s[3];
    s[0] = 'H';
    s[1] = 'i';
    s[2] = '\0';  /* 不能忘！ */

    printf("s = \"%s\", length = %zu\n", s, strlen(s));
    return 0;
}
```

</details>

### 🔴 挑战：实现自己的 `my_strlen`

不使用 `<string.h>`，手动遍历 `char` 数组直到找到 `'\0'`，返回字符数。再用不同字符串（含空串、含空格、含 emoji）测试。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>

size_t my_strlen(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

int main(void) {
    printf("len(\"Hi\") = %zu\n", my_strlen("Hi"));
    printf("len(\"\") = %zu\n", my_strlen(""));
    printf("len(\"Hello World\") = %zu\n", my_strlen("Hello World"));
    return 0;
}
```

</details>

## 故障排查

**Q：为什么我的字符串输出后面跟着一堆乱码？**

A：很可能忘记 `'\0'` 了。`printf("%s")` 会一直读到内存中偶然遇到的第一个 `0` 字节。修复：

```c
/* ❌ 错误 */
char buf[3] = {'A', 'B', 'C'};  /* 没有 \0 */
printf("%s\n", buf);            /* 乱码 */

/* ✅ 修复 */
char buf[4] = {'A', 'B', 'C', '\0'};
/* 或者 */
char buf[] = "ABC";
```

**Q：`strlen` 和 `sizeof` 我应该用哪个？**

A：
- 想知道字符串里有多少个字符（不含 `'\0'`）→ `strlen`
- 想知道数组占了多少字节 → `sizeof`
- 想知道还能往数组里塞多少字符 → `sizeof(buf) - 1`（留一个给 `'\0'`）

**Q：为什么我打印 `char *p = "abc"` 的 `sizeof(p)` 得到 8 而不是 4？**

A：`sizeof(p)` 得到的是**指针本身**的大小（在 64 位机器上是 8 字节），不是字符串的大小。用 `sizeof("abc")` 才能得到 4（含 `'\0'`）。

**Q：UTF-8 编码的中文/emoji 怎么处理？**

A：`strlen("你好")` 返回 6（UTF-8 每个汉字 3 字节），**不是 2**。C 语言对 UTF-8 一视同仁——每个字节都算一个单位。如果需要 Unicode 字符计数，需要专门的库。

## 知识扩展

### 字符串字面量的存储位置

现代编译器将字符串字面量存放在 `.rodata`（只读数据）段：

```
内存分段:
┌──────────────────┐
│ .text (代码段)   │
├──────────────────┤
│ .rodata (只读)   │ ← "Hello" 字面量在这里
│ .data (已初始)   │
│ .bss (未初始)    │
├──────────────────┤
│ Stack            │ ← char s[] 在这里
│ Heap             │
└──────────────────┘
```

这就是为什么 `char *p = "Hello"; p[0] = 'h';` 会崩溃——你正在尝试写入只读段。

### C11 的 `_Static_assert`

你可以用静态断言确保字符串相关的假设在编译时成立：

```c
#include <assert.h>
#include <string.h>

char greeting[] = "Hello";
_Static_assert(sizeof(greeting) == 6, "greeting should be 6 bytes");
```

## 小结

这一章我发现：

- **C 字符串** = `char` 数组 + 末尾 `'\0'`，没有对象包装
- `strlen()` 返回字符数（不含 `'\0'`），`sizeof` 返回总字节数（含 `'\0'` 空间）
- **`char[]`** 在栈上可修改；**`const char*`** 指向只读字面量
- Python 自动管理长度，C 全靠 `'\0'` 收尾——多控制，多责任
- 忘记 `'\0'` = 未定义行为（乱码、崩溃、安全漏洞）
- 字符串字面量编译期可能合并，存放在只读段

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 字符串 | String | 以 '\0' 结尾的 char 数组 |
| 空终止符 | Null Terminator | '\0'，ASCII 值 0 |
| 字符串字面量 | String Literal | "..." 双引号括起来的文本 |
| 字符串常量 | String Constant | 存储在只读段的字面量 |
| 退化 | Decay | 数组名在某些表达式中转换为指针 |
| 未定义行为 | Undefined Behavior (UB) | C 标准不规定结果的行为 |
| 只读数据段 | .rodata | 编译后字符串字面量的存储位置 |
| ASCII 码 | ASCII Code | 字符的数字编码，如 'A' = 65 |

## 延伸阅读

- **cppreference — string.h**: [C 字符串库](https://en.cppreference.com/w/c/string/byte) — 完整函数参考
- **C17 §7.1.1**: 字符串的定义——"连续的 multibyte 字符序列，以 '\0' 终止"
- **K&R 第 2 版 §5.5**: C 字符串的经典讲解

## 继续学习

- [上一章](./pointer_arith.md)：指针算术
- [下一章](./string_operations.md)：字符串操作（strlen, strcpy, strcat, strcmp）
