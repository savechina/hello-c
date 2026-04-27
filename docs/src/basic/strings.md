# 字符串深度 (Strings Deep Dive)

## 开篇故事

想象你在读一本书，但书页上没有页码，也没有目录。你怎么知道这本书什么时候结束？答案很简单——作者在最后一页写了一个特殊符号，比如一个句号，或者一张空白书签。看到它，你就知道：故事到此为止。

C 语言的字符串就是这样工作的。它没有内置的"长度"字段，没有对象包装，就是一块连续的 `char` 内存——用 `\0`（null 终止符）标记结尾。`strlen` 的工作就是从第一个字符开始读，一路读到 `\0` 停手。少了这个标记，字符串就会一直读下去，直到偶然撞见一个 0 字节，读出一堆毫不相关的随机数据。

```c
char greeting[] = "Hello, C!";
// 编译器自动在末尾加上 '\0'：
// ['H','e','l','l','o',',',' ','C','!','\0']
//                          ↑ 故事到此为止
```

这就是为什么在 C 语言里处理字符串从来不是一件理所当然的事。每一步都要自己管：空间够不够？`\0` 有没有写？边界有没有守住。Python 替你做好的事，C 选择交给你——多一分控制，也多一分责任。

> "字符串不是对象，而是一块需要你亲自收尾的内存。" —— C 语言的第一堂安全课

## 本章适合谁

- 刚学完"数据类型"和"数组"，想知道字符串在 C 中到底是什么
- 用过 Python/JavaScript 的 `str`，对 C 的 `char*` 感到困惑
- 听说过"缓冲区溢出"但不清楚具体原因
- 希望写出安全的字符串处理代码，而不是只会 `strcpy` 然后祈祷

## 你会学到什么

1. C 字符串的本质：null-terminated `char` 数组，`\0` 终止符的作用
2. ASCII 内存图：字符串在栈内存中如何存储，`\0` 的位置
3. Python `str` vs C `char*`：Python 自动管理内存，C 需要手动 null 终止
4. `<string.h>` 核心函数：`strlen`、`strcpy`、`strncpy`、`strcmp`、`strchr`、`strstr`、`strtok`
5. **安全核心**：`strcpy` vs `strncpy` 的安全差异（缓冲区溢出演示 + 修复）
6. **安全核心**：`sprintf` vs `snprintf`（格式化安全）
7. `strlen` 手动实现 vs 标准库实现
8. `strtok` 字符串分隔（修改原字符串的注意事项）
9. 实战：安全解析 CSV 格式字符串

## 前置要求

- 已完成 [数据类型](./datatype) 章节
- 理解 `char` 类型（一个字节，可存储 ASCII 字符）
- 理解数组基础（`int arr[10]` 的声明和访问）
- 已配置 C 编译环境（`gcc` 或 `clang`）

> **💡 编译命令**：本章代码使用 `-Wall -Wextra -Werror -std=c17` 编译，所有警告视为错误。

## 第一个例子

最简短的 C 字符串示例——看看 `char[]` 和 `strlen` 的工作方式：

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char greeting[] = "Hello, C!";  /* 自动推导长度为 10（含 '\0'） */

    printf("内容: %s\n", greeting);
    printf("长度: %zu\n", strlen(greeting));    /* 7 (不含 \0) */
    printf("大小: %zu 字节\n", sizeof(greeting)); /* 10 (含 \0) */

    return 0;
}
```

编译并运行：

```bash
gcc -Wall -Wextra -Werror -std=c17 -o demo demo.c
./demo
```

完整源码在仓库 [`src/basic/strings_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/strings_sample.c)。

## 原理解析

### 1. C 字符串是什么？

C 语言没有内置的"字符串类型"。C 字符串本质上是一个 `char` 类型的数组，以 **null 终止符 `\0`** 结尾。

```c
char greeting[] = "Hello, C!";
```

编译器会在背后做两件事：
1. 为 `"Hello, C!"` 分配 10 个字节的内存（9 个字符 + 1 个 `\0`）
2. 逐字节填充内容

**ASCII 内存图**：

```
  ┌── char greeting[10] = "Hello, C!" in Stack ──┐
  │                                               │
  │  Address  +0  +1  +2  +3  +4  +5  +6  +7  +8 +9│
  │          ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  │  →      │'H'│'e'│'l'│'l'│'o'│','│' '│'C'│'!'│'\0'│
  │          │ 72│101│108│108│111│ 44│ 32│ 67│ 33│  0│
  │          └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
  │                                               │
  │  strlen(greeting) = 9 (从开始数，到 '\0' 前)   │
  │  sizeof(greeting) = 10 (整个数组大小)           │
  └───────────────────────────────────────────────┘
```

**关键规则**：
- `\0`（ASCII 值 0）必须存在，否则 `"Hello, C!"` 的结尾在哪里？没有 `\0`，`printf("%s")` 会一直读下去，直到偶然遇到一个 0 字节——**缓冲区溢出读取**。
- `strlen()` 返回 `\0` 前面的字符数（**不包含** `\0`）
- `sizeof` 返回数组的总字节数（**包含** `\0`）

### 2. Python str vs C char*

| 特性 | Python `str` | C `char*` / `char[]` |
|------|--------------|----------------------|
| 类型 | 内置对象 | 裸指针 / 数组 |
| 长度存储 | 有（`.len` 字段）| 无，需要 `strlen()` 遍历找 `\0` |
| 内存管理 | 自动（引用计数 + GC）| 手动（`char[]` 栈分配, `malloc` 堆分配）|
| 不可变性 | 字符串不可变 | `char[]` 可修改, `char*` 指向字面量不可改 |
| 越界检查 | 有（抛出 `IndexError`）| **无**，越界 = Undefined Behavior |

```python
# Python — 自动管理一切
s = "Hello"
print(len(s))   # O(1)，直接读长度字段
s[0] = 'h'      # ❌ TypeError: 字符串不可变
```

```c
// C — 你自己管理一切
char s[] = "Hello";
printf("%zu\n", strlen(s));  // O(n)，逐个字节查找 \0
s[0] = 'h';                  // ✅ 可以修改（char[] 不是字面量）
```

**我的理解**：C 的字符串设计哲学是"不提供任何保护，但给你完全的控制权"。这既是 C 的强大之处（极致性能），也是它的危险之处（缓冲区溢出).

### 3. char[] vs char* 初始化

```c
/* char[]: 在栈上分配完整数组，内容可修改 */
char greeting[] = "Hello";
greeting[0] = 'h';  // ✅ OK

/* char*: 指向只读字符串字面量（通常放在 .rodata 段），不可修改 */
const char *literal = "Hello";
literal[0] = 'h';   // ❌ Segmentation Fault! 字面量只读
```

**经验法则**：需要修改内容 → `char[]`；只读引用 → `const char*`。

### 4. strcpy vs strncpy — 安全 vs 不安全

> **⚠️ 本章核心安全规则**：在任何实际项目中，`strcpy` 都不应该出现在工作代码中。它没有边界检查，永远存在缓冲区溢出风险。

```c
/* ❌ 危险：strcpy — 无边界检查 */
char small[5];
strcpy(small, "Hello World!");
/* "Hello World!" 有 13 字节（含 '\0'），small 只有 5 字节。
   结果：写入超出 small 边界，覆盖栈上的相邻变量和返回地址 →
   → Undefined Behavior → 崩溃、数据损坏、安全漏洞。 */

/* ✅ 安全：strncpy — 指定最大写入字节数 */
char small[5];
strncpy(small, "Hello World!", sizeof(small) - 1);
small[sizeof(small) - 1] = '\0';
/* 最多写入 4 个字符 + 手动设 '\0' → small = "Hell" */
```

`strncpy` 的三个参数：目标缓冲区、源字符串、**最大写入字节数**。

**重要陷阱**：如果源字符串长度 >= n，`strncpy` 不会自动添加 `\0`。所以**必须手动保证 null 终止**：

```c
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';  /* 始终保证 null termination */
```

### 5. snprintf vs sprintf

```c
/* ❌ 危险：sprintf — 无边界检查 */
char buf[5];
sprintf(buf, "Hello, %s!", "World");
/* "Hello, World!" 共 13 字节，buf 只有 5 → 缓冲区溢出 */

/* ✅ 安全：snprintf — 指定缓冲区大小 */
char buf[5];
int ret = snprintf(buf, sizeof(buf), "Hello, %s!", "World");
/* buf = "Hell" (截断), ret = 13 (完整输出需要 13 个字符) */
```

`snprintf` 的返回值非常有用：它告诉你"如果缓冲区足够大，完整输出需要多少个字符"。如果返回值 >= 缓冲区大小，说明发生了截断。

### 6. strlen — 长度测量

```c
#include <string.h>

size_t len = strlen("Hello");  // 返回 5
size_t emp = strlen("");       // 返回 0 (只有 '\0')
```

手动实现很简单——逐个字节查找 `\0`：

```c
size_t my_strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}
```

**注意**：`strlen()` 返回的是**字节数**，不是字符数。UTF-8 多字节字符（如 `🌍`）会被计为多个字节（4 个字节）。如果需要Unicode字符计数，需要专门的 Unicode 库。

### 7. strcmp — 字符串比较

```c
int result = strcmp("abc", "abcd");
// result = 负数 ("abc" < "abcd")

result = strcmp("hello", "hello");
// result = 0 (完全相同)

result = strcmp("world", "hello");
// result = 正数 ("world" > "hello")
```

**关键**：永远不要用 `==` 比较 C 字符串！

```c
char *a = "hello", *b = "hello";
if (a == b)  // ❌ 比较指针地址，不是字符串内容！
```

### 8. strtok — 分隔字符串

`strtok` 用于将一个字符串按分隔符拆分成多个"令牌"（token）。

```c
char text[] = "apple,banana,cherry";  /* 注意：必须是 char[]，不是 char* */
char *token = strtok(text, ",");
while (token != NULL) {
    printf("  [%s]\n", token);
    token = strtok(NULL, ",");  /* 后续调用传入 NULL */
}
// 输出: [apple]  [banana]  [cherry]
```

**⚠️ 重要注意事项**：
1. `strtok` **修改原字符串**——它在分隔符位置写入 `\0`。如果原字符串是字面量（`char*`）或需要保留，必须先复制一份。
2. `strtok` **不是线程安全的**——它内部使用静态变量保存状态。多线程环境使用 `strtok_r`（POSIX）。
3. 第一次调用传入字符串，后续调用传入 `NULL`。

## 常见错误

### ❌ 错误 1：缓冲区太小

```c
/* ❌ 错误代码 */
char buf[5];
strcpy(buf, "Hello");   /* "Hello" 需要 6 字节（含 '\0'）*/

/* ✅ 修复 */
char buf[6];            /* 至少 6 字节 */
strncpy(buf, "Hello", sizeof(buf) - 1);
buf[sizeof(buf) - 1] = '\0';
```

### ❌ 错误 2：忘记 null terminator

```c
/* ❌ 错误代码 */
char buf[5];
strncpy(buf, "ABCD", 5);   /* strncpy 写满 5 字节，没有空间放 '\0' */
printf("%s\n", buf);       /* 越界读取，输出随机数据 */

/* ✅ 修复 */
char buf[5];
strncpy(buf, "ABCD", sizeof(buf) - 1);
buf[sizeof(buf) - 1] = '\0';  /* 手动终止 */
```

### ❌ 错误 3：用 == 比较字符串

```c
/* ❌ 错误代码 */
char a[] = "hello", b[] = "hello";
if (a == b) { /* 永远 false，比较的是指针地址 */ }

/* ✅ 修复 */
if (strcmp(a, b) == 0) { /* 比较内容 */ }
```

### ❌ 错误 4：修改字符串字面量

```c
/* ❌ 错误代码 */
char *s = "hello";
s[0] = 'H';   /* Segmentation Fault! 只读内存 */

/* ✅ 修复 */
char s[] = "hello";  /* char[] 在栈上分配，可修改 */
s[0] = 'H';
```

## 动手练习

### 🟢 入门：strlen 实践

编写代码，计算字符串 `"Hello, C Programming!"` 的长度。不要使用 `strlen()`——手动实现一个 `count_chars` 函数，遍历字符串直到找到 `\0`。

<details><summary>查看答案</summary>

```c
#include <stdio.h>

size_t count_chars(const char *str) {
    size_t count = 0;
    while (str[count] != '\0') {
        count++;
    }
    return count;
}

int main(void) {
    const char *text = "Hello, C Programming!";
    printf("长度: %zu\n", count_chars(text));
    return 0;
}
```

</details>

### 🟡 中级：strncpy 安全复制函数

写一个函数 `void safe_copy(char *dest, size_t dest_size, const char *src)`，使用 `strncpy` 安全地将 `src` 复制到 `dest`。确保：
1. 永远不会写入超过 `dest_size` 字节
2. 始终保证 `dest` 以 `\0` 终止
3. 如果发生截断，打印警告信息

<details><summary>查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

void safe_copy(char *dest, size_t dest_size, const char *src) {
    size_t src_len = strlen(src);
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\0';
    if (src_len >= dest_size) {
        printf("⚠️ 警告：截断 \"%s\" (%zu 字符 → %zu 字符)\n",
               src, src_len, dest_size - 1);
    }
}

int main(void) {
    char short_buf[10];
    safe_copy(short_buf, sizeof(short_buf), "This is a very long string");
    printf("结果: \"%s\"\n", short_buf);
    return 0;
}
```

</details>

### 🔴 挑战：用 strtok 安全解析 CSV

编写一个函数 `int parse_csv(const char *csv, char fields[][64], int max_fields)`，安全地解析 CSV 字符串。要求：
1. 不修改原始 `csv` 字符串（先复制一份再 `strtok`）
2. 每个字段最多 63 个字符 + `\0`
3. 用 `strncpy` 安全复制每个字段
4. 返回实际解析到的字段数
5. 测试数据：`"John,25,Engineer,New York"`

<details><summary>查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

#define FIELD_SIZE 64

int parse_csv(const char *csv, char fields[][FIELD_SIZE], int max_fields) {
    char work_buf[256];
    strncpy(work_buf, csv, sizeof(work_buf) - 1);
    work_buf[sizeof(work_buf) - 1] = '\0';

    int count = 0;
    char *token = strtok(work_buf, ",");
    while (token != NULL && count < max_fields) {
        strncpy(fields[count], token, FIELD_SIZE - 1);
        fields[count][FIELD_SIZE - 1] = '\0';
        count++;
        token = strtok(NULL, ",");
    }
    return count;
}

int main(void) {
    char fields[4][FIELD_SIZE];
    int n = parse_csv("John,25,Engineer,New York", fields, 4);
    for (int i = 0; i < n; i++) {
        printf("  fields[%d] = \"%s\"\n", i, fields[i]);
    }
    return 0;
}
```

输出：
```
  fields[0] = "John"
  fields[1] = "25"
  fields[2] = "Engineer"
  fields[3] = "New York"
```

</details>

## 故障排查 (FAQ)

**Q：为什么 `strlen("Hello")` 返回 5，但 `sizeof("Hello")` 返回 6？**

A：`strlen` 只计算 `\0` 之前的字符数（5），`sizeof` 计算整个字符串字面量占用的内存（5 字符 + 1 个 `\0` = 6 字节）。

**Q：能不能用 `char*` 代替 `char[]`？**

A：可以只读访问，但不能修改。`char *s = "Hello"` 指向只读段（.rodata），`s[0] = 'h'` 会导致 Segmentation Fault。（技术上未定义行为 UB）

**Q：`strtok` 可以嵌套使用吗？**

A：标准版 `strtok` **不能嵌套**——它内部用静态变量保存上次位置。如果需要嵌套分隔（比如先按行分隔，再按逗号分隔），使用 `strtok_r`（POSIX, 可重入）。

**Q：`strncpy` 和 `snprintf` 哪个更安全？**

A：两者都有边界检查。`snprintf` 更好：(1) 始终保证 `\0` 终止，(2) 返回值告诉你是否需要更大缓冲区，(3) 可以同时格式化多个值。

**Q：UTF-8 字符串怎么正确计算字符数？**

A：`strlen()` 返回的是字节数，不是 Unicode 字符数。例如 `strlen("🌍") = 4`（UTF-8 编码需 4 字节）。如果需要 Unicode 字符计数，使用 `<uchar.h>`（C11）或第三方库如 `libunistring`。

## 知识扩展 (选学)

### 缓冲区溢出攻击原理

缓冲区溢出不仅是 bug，还是经典的安全攻击方式。攻击者可以向栈溢出输入数据，覆盖函数的返回地址，让程序跳转到恶意代码：

```
栈内存布局：
  [ char buf[5] ] [ saved EBP ] [ return address ]

  strcpy(buf, "AAAAAAAAAAAAAAAAAAAAAAAA");
  → 'A' 覆盖 return address → 程序跳转到 0x41414141 → 崩溃或被劫持
```

这就是为什么 **`strcpy` 被称为"危险的函数"**。在安全关键代码中，它被直接列为禁止使用的函数。

### C11 的 bounds-checking 扩展

C11 标准引入了可选的"边界检查"扩展（`__STDC_LIB_EXT1__`），提供了更安全的替代：

```c
// C11 Annex K（可选实现）
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>

strcpy_s(dest, dest_size, src);   // 类似 strncpy，但自动 \0 终止
strcat_s(dest, dest_size, src);
sprintf_s(buf, buf_size, "%s", str);
```

并非所有编译器都实现这些函数（glibc 默认不启用）。`strncpy` + 手动 `\0` 仍然是最广泛使用的安全模式。

### Unicode 与多字节字符串

C 语言处理 Unicode 有多层方案：

| 方案 | 头文件 | 说明 |
|------|--------|------|
| `char` + UTF-8 | 无 | 最简单，`strlen` 返回字节数 |
| `wchar_t` | `<wchar.h>` | 宽字符（通常 4 字节），`L"你好"` |
| `char16_t/char32_t` | `<uchar.h>` | C11 标准 Unicode 字符类型 |

对于大多数场景，UTF-8 `char[]` 是最实用的方案。`strlen` 得到字节数，如果需要字符数，需要解码 UTF-8 序列。

## 小结

本章深入学习了 C 语言字符串的安全处理方式：

- **字符串本质**：null-terminated `char` 数组，`\0` 必不可少
- **ASCII 内存图**：字符串在栈上逐字节存储，末尾有 `\0`（ASCII 0）
- **Python vs C**：Python 自动管理内存和长度；C 需要手动管理一切
- **安全模式**：`strcpy → strncpy`，`sprintf → snprintf`，始终手动设 `\0`
- **核心函数**：`strlen`（长度）、`strcmp`（比较）、`strchr/strstr`（搜索）、`strtok`（分隔）
- **实战**：安全 CSV 解析 = 复制原串 → 分隔 → `strncpy` 到字段缓冲区

**核心术语**：
- Null-terminated string / 以 null 结尾的字符串
- Null terminator (`\0`) / 字符串终止符
- Buffer overflow / 缓冲区溢出
- Undefined Behavior (UB) / 未定义行为
- Bounded string operation / 有界字符串操作

## 术语表

| 英文 | 中文 | 说明 |
|------|------|------|
| Null-terminated string | 以 null 结尾的字符串 | C 语言中的字符串表示方式 |
| Null terminator (`\0`) | 字符串终止符 | ASCII 值 0，标记字符串结束 |
| Buffer overflow | 缓冲区溢出 | 写入超出缓冲区边界 |
| Undefined Behavior (UB) | 未定义行为 | 编译器不保证结果的行为 |
| String literal | 字符串字面量 | `"..."` 双引号括起来的文本 |
| `strncpy` | 安全复制 | 指定最大写入字节数的复制函数 |
| `snprintf` | 安全格式化 | 指定缓冲区大小的格式化函数 |
| `strcmp` | 字符串比较 | 按字典序比较两个字符串 |
| `strtok` | 字符串分隔 | 按分隔符拆分子串的函数 |
| Token | 令牌 | `strtok` 拆解出的子串 |
| Truncation | 截断 | 超出缓冲区长度时被强制缩短 |

## 延伸阅读

- **C 字符串安全**：[OWASP Buffer Overflow](https://owasp.org/www-community/vulnerabilities/Buffer_Overflow) — 缓冲区溢出漏洞详解
- **strlcpy/strlcat**：OpenBSD 的替代方案（比 strncpy 更安全，但非标准）
- **C11 Annex K**：[Bounds-checking interfaces](https://en.cppreference.com/w/c/types/limits) — 标准库的安全扩展
- **cppreference — string.h**：[C 字符串库参考](https://en.cppreference.com/w/c/string/byte)

## 继续学习

- 上一章：[数据类型](./datatype)
- 下一章：[数组](./arrays)

---

> 本章代码位于仓库 [`src/basic/strings_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/strings_sample.c)。
> 运行 `make build && make run` 查看完整演示输出。
