# C 标准库精要（Standard Library Essentials）

> "C 语言虽小，标准库却藏着一个世界。" —— 我发现原来 `atoi`、`rand`、`time`、`isdigit` 都来自一个标准库。

## 开篇故事

想象你的口袋里有一把瑞士军刀。你刚拿到手时可能只用主刀（`printf`），但慢慢你会发现，刀柄里还藏着剪刀、螺丝刀、开瓶器……它们安静地收在那里，不占额外空间，需要时随时展开。

C 标准库就是这把瑞士军刀。`<stdio.h>`、`<stdlib.h>`、`<math.h>`、`<time.h>`、`<ctype.h>`、`<limits.h>` 每一把工具都有明确分工。你不需要记住每一把刀的名字，但你需要知道「口袋里有一把刀」——当你需要随机数时知道找 `rand()`，需要时间时知道找 `time()`。

这一章帮你建立标准库的地图，而不是死记每一个函数。

## 本章适合谁

- 完成了基础章节，想建立**C 标准库知识地图**的开发者
- 经常搜索"how to convert string to int in C"（然后才发现有 `atoi`）的开发者

## 你会学到什么

1. C 标准库的 6 大核心头文件及其关键函数
2. 数字转换：`atoi`、`atof`、`strtol`（安全转换）
3. 随机数：`rand`、`srand` 的正确用法
4. 数学函数：`sqrt`、`pow`、`floor`、`ceil`
5. 时间函数：`time`、`localtime`、`strftime`
6. 字符分类：`isdigit`、`isalpha`、`tolower`
7. 类型极限：`INT_MAX`、`LONG_MIN` 等

## 前置要求

完成"变量与表达式"、"预处理器与宏"章节

## 第一个例子

```c
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

int main() {
    /* 数字转换 */
    int n = atoi("42");       /* "42" → 42 */

    /* 随机数 */
    srand(time(NULL));         /* 用时间播种 */
    int r = rand();            /* 随机整数 */

    /* 数学 */
    double s = sqrt(144);        /* 12.0 */

    /* 时间 */
    time_t now = time(NULL);     /* 当前 Unix 时间戳 */

    printf("n=%d, r=%d, s=%.0f, time=%ld\n", n, r, s, (long)now);
    return 0;
}
```

> [完整源码](https://github.com/[your-repo]/hello-c/blob/main/src/basic/stdlib_sample.c)

## 原理解析

C 标准库是 ISO C 标准定义的一部分。每个符合标准的编译器都**必须**提供这些头文件。

**C 标准库核心模块一览**：

```
┌─────────────────────────────────────────────────────────┐
│               C 标准库核心模块一览                        │
│                                                         │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐       │
│  │  <stdio.h>  │ │ <stdlib.h>  │ │ <string.h>  │       │
│  │  输入/输出  │ │  内存/工具  │ │  字符串操作  │       │
│  │  printf     │ │  malloc     │ │  strlen     │       │
│  │  fopen      │ │  atoi       │ │  strcpy     │       │
│  │  fgets      │ │  rand       │ │  strcmp     │       │
│  │  scanf      │ │  exit       │ │  strstr     │       │
│  └─────────────┘ └─────────────┘ └─────────────┘       │
│                                                         │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐       │
│  │  <math.h>   │ │  <time.h>   │ │  <ctype.h>  │       │
│  │  数学运算   │ │  时间处理   │ │  字符分类   │       │
│  │  sqrt       │ │  time       │ │  isdigit    │       │
│  │  pow        │ │  localtime  │ │  isalpha    │       │
│  │  floor      │ │  strftime   │ │  tolower    │       │
│  └─────────────┘ └─────────────┘ └─────────────┘       │
└─────────────────────────────────────────────────────────┘
```

### <stdlib.h>: 通用工具

| 函数 | 用途 | 示例 |
|------|------|------|
| `atoi()` | char* → int | `atoi("42")` → `42` |
| `atof()` | char* → double | `atof("3.14")` → `3.14` |
| `strtol()` | char* → long (安全) | `strtol("123", &end, 10)` |
| `rand()` | 随机整数 [0, RAND_MAX] | `rand() % 100` |
| `srand()` | 随机数种子 | `srand(time(NULL))` |
| `abs()` | 绝对值 | `abs(-5)` → `5` |
| `malloc()` | 动态分配 | `malloc(sizeof(int))` |
| `free()` | 释放内存 | `free(ptr)` |
| `exit()` | 终止程序 | `exit(0)` / `exit(1)` |

### <math.h>: 数学函数

需要编译时加 `-lm`（链接数学库）。

| 函数 | 用途 | 示例 |
|------|------|------|
| `sqrt()` | 平方根 | `sqrt(144)` → `12.0` |
| `pow()` | 幂运算 | `pow(2, 10)` → `1024.0` |
| `floor()` | ⌊x⌋ 向下取整 | `floor(3.7)` → `3.0` |
| `ceil()` | ⌈x⌉ 向上取整 | `ceil(3.2)` → `4.0` |
| `round()` | 四舍五入 (C99) | `round(3.5)` → `4.0` |
| `sin()`/`cos()`/`tan()` | 三角函数 | `sin(M_PI/2)` → `1.0` |

### <time.h>: 时间函数

```c
time_t now = time(NULL);                    /* 当前 Unix 时间戳 */
struct tm *tm_info = localtime(&now);       /* 转换为本地时间 */
char buf[64];
strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info); /* 格式化 */
```

### <ctype.h>: 字符分类与转换

| 函数 | 用途 | 示例 |
|------|------|------|
| `isalnum()` | 字母或数字 | `isalnum('A')` → `true` |
| `isalpha()` | 字母 | `isalpha('1')` → `false` |
| `isdigit()` | 数字 (0-9) | `isdigit('5')` → `true` |
| `isspace()` | 空白字符 | `isspace(' ')` → `true` |
| `isupper()` | 大写 | `isupper('A')` → `true` |
| `islower()` | 小写 | `islower('a')` → `true` |
| `tolower()` | 转小写 | `tolower('A')` → `'a'` |
| `toupper()` | 转大写 | `toupper('a')` → `'A'` |

### <limits.h>: 类型极限

```c
printf("INT_MIN = %d, INT_MAX = %d\n", INT_MIN, INT_MAX);
printf("LONG_MAX = %ld\n", LONG_MAX);
printf("SIZE_MAX = %zu\n", SIZE_MAX);
```

### <string.h>: 字符串操作

（已在"字符串"章节详细介绍）

| 函数 | 用途 |
|------|------|
| `strlen()` | 长度 |
| `strcpy()`/`strncpy()` | 复制 |
| `strcmp()` | 比较 |
| `strcat()`/`strncat()` | 拼接 |
| `strstr()` | 子字符串搜索 |

## 常见错误

### ❌ 错误 1: `rand()` 不播种 → 每次相同序列

```c
int r = rand();  /* 每次运行产生相同的值 */
```

**✅ 修复:** 在程序开头播种一次:
```c
srand((unsigned int)time(NULL));  /* 只在 main() 开头调用一次 */
int r = rand();
```

### ❌ 错误 2: `atoi()` 失败返回 0，无法区分

```c
int val = atoi("xyz");  /* 返回 0，但 "0" 也返回 0 */
```

**✅ 修复:** 用 `strtol()` 进行安全的转换:
```c
char *end;
long val = strtol("xyz", &end, 10);
if (end == str) {
    printf("无效输入\n");
}
```

### ❌ 错误 3: 忘记 `-lm` 链接数学库

```bash
gcc program.c        # 错误! undefined reference to `sqrt`
gcc program.c -lm    # ✅ 正确, 链接数学库
```

> 注：macOS 的 clang 通常不需要 `-lm`，但 Linux 的 gcc 需要。

## 动手练习

### 🟢 入门: 随机数猜数游戏

用 `rand()` 和 `srand()` 实现一个猜数游戏：生成 1-100 的随机数，用户输入猜测，提示"太大"或"太小"。

<details><summary>点击查看答案</summary>

```c
srand(time(NULL));
int target = rand() % 100 + 1;
int guess;
printf("猜 1-100 的数字: ");
scanf("%d", &guess);
if (guess == target) printf("正确!\n");
else if (guess < target) printf("太小了!\n");
else printf("太大了!\n");
```

</details>

### 🟡 中级: 字符串→数字安全转换

实现一个函数 `safe_atoi(const char *str, int *out)`，用 `strtol()` 安全转换。成功返回 0，失败返回 -1（设置 errno）。

<details><summary>点击查看答案</summary>

```c
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

int safe_atoi(const char *str, int *out) {
    char *end;
    errno = 0;
    long val = strtol(str, &end, 10);
    if (errno != 0 || end == str || *end != '\0') return -1;
    if (val < INT_MIN || val > INT_MAX) return -1;
    *out = (int)val;
    return 0;
}
```

</details>

### 🔴 挑战: 简易计算器

实现一个函数 `double calc(const char *expr)`, 解析简单表达式如 `"3 + 4 * 2"`, 使用 `strtol()` 提取数字，支持 +、-、*、/。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* 简化版: 仅支持 "A op B" 格式 */
double calc(const char *expr) {
    char *p = (char *)expr;
    while (isspace(*p)) p++;  /* 跳过前导空白 */

    char *end;
    double a = strtod(p, &end);
    while (isspace(*end)) end++;

    char op = *end;
    end++;
    while (isspace(*end)) end++;

    double b = strtod(end, NULL);

    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b != 0 ? a / b : 0.0;
        default: return 0.0;
    }
}
```

</details>

## 故障排查 (FAQ)

**Q: 为什么编译时需要 `-lm` 链接数学库？**

**A:** 历史原因——早期 UNIX 系统把数学函数放在独立的 libm 中。现代编译器（macOS clang）通常已内联，但 Linux gcc 仍需要 `-lm`。

**Q: `atoi()` 和 `strtol()` 的区别是什么？**

**A:** `atoi()` 是旧的 C 库函数，失败时返回 0（无法区分输入 `"0"` 和无效输入 `"xyz"`）。`strtol()` 返回 `long` 并通过第二个参数返回解析停止的位置，是**安全的替代方案**。

**Q: `rand()` 是好的随机数生成器吗？**

**A:** 对于简单的用途（游戏、非安全随机），`rand()` 足够了。但 `rand()` 不是加密安全的（predictable），且实现质量因平台而异。对于密码学用途，应该使用 `/dev/urandom` 或 `getrandom()`。

## 知识扩展 (选学)

### C99/C11/C17 标准库新增

| 标准 | 新增头文件 | 关键新增 |
|------|----------|---------|
| C89/C90 | 原始标准 | stdio, stdlib, string, math 等 |
| C99 | `<stdbool.h>`, `<stdint.h>`, `<inttypes.h>` | `bool`, `int32_t`, `PRIx32` |
| C11 | `<stdalign.h>`, `<stdatomic.h>`, `<threads.h>` | atomic, threads |

### Python vs C 标准库对比

| 功能 | Python | C (头文件) |
|------|--------|-----------|
| 随机数 | `import random; random.random()` | `rand()` (`<stdlib.h>`) |
| 时间 | `import time; time.time()` | `time(NULL)` (`<time.h>`) |
| 数学 | `import math; math.sqrt(x)` | `sqrt(x)` (`<math.h>`) |
| 字符分类 | `c.isdigit()` | `isdigit(c)` (`<ctype.h>`) |
| 类型极限 | `import sys; sys.maxsize` | `INT_MAX` (`<limits.h>`) |

Python 把这些封装成模块 (`import xxx`)，C 用 `#include <xxx.h>` 暴露函数。

## 小结

**核心要点:**
1. C 标准库提供 6 大核心头文件：stdlib, math, time, ctype, limits, string
2. 随机数: 必须 `srand(time(NULL))` 播种一次
3. 字符串→数字: 用 `strtol()` 而非 `atoi()`（安全）
4. 数学函数编译需加 `-lm`（Linux gcc）
5. `isdigit()`, `isalpha()`, `tolower()` 是每个 C 程序员该记住的字符工具

**关键术语:**
C 标准库 → ISO C 标准定义的内置函数集合 → 每个编译器必须提供

## 术语表

| English | 中文 |
|---------|------|
| Standard Library | 标准库 |
| Header File | 头文件 |
| Random Number Seed | 随机数种子 |
| Type Limits | 类型极限 |
| Character Classification | 字符分类 |
| Time Stamp | 时间戳 |
| Feature Test Macro | 功能测试宏 |

## 延伸阅读

- [C Standard Library Reference](https://en.cppreference.com/w/c/header) — 完整的 C 标准库参考
- [C99 Standard Library](https://en.cppreference.com/w/c/99) — C99 新增内容

## 继续学习

← [位运算与内存操作](bit_ops.md) | [命令行参数](cli_args.md) →
