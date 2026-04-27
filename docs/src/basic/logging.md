# 日志与格式化输出（Logging & Formatted Output）

> "没有日志的程序就像没有仪表的飞机——你能飞，但你不知道飞得怎么样。" —— 我发现

## 开篇故事

我写过最惨烈的一个 bug 是这样的：程序跑了一晚上，第二天早上发现输出文件是空的。我完全不知道中间发生了什么——没有错误信息、没有状态记录、什么都没有。像黑盒子一样。

从那以后我养成了习惯：**关键操作必须有日志**。但 C 语言没有 Python 那种开箱即用的 `logging` 库。C 的日志系统需要你自己搭建。

本章我就带你从零开始，掌握 C 的格式化输出家族，然后一步步构建一个实用的日志宏系统。

## 本章适合谁

- 只用过 `printf`，不知道 `fprintf`/`sprintf`/`snprintf` 区别的人
- 想写自定义日志函数但不知道怎么处理可变参数的人
- 被 `sprintf` 缓冲区溢出坑过的人
- 想了解 Python `logging` vs C 日志差异的人

## 你会学到什么

- `printf` 家族全貌（printf, fprintf, sprintf, snprintf, vprintf 等）
- `va_list` 可变参数函数
- 自定义 `printf`-like 函数
- 日志级别宏（DEBUG / INFO / WARN / ERROR）
- `__FILE__`, `__LINE__`, `__func__` 内置宏
- 带时间戳的日志函数
- `snprintf` 安全使用 vs `sprintf` 溢出风险

## 前置要求

- 熟练使用 `printf` 进行基本输出
- 理解字符串和字符数组

## 第一个例子：从 `printf` 到日志宏

```c
#include <stdio.h>

#define LOG_ERROR(fmt, ...) \
    fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
    fprintf(stdout, "[INFO]  " fmt "\n", ##__VA_ARGS__)

int main(void) {
    LOG_INFO("Starting server on port %d", 8080);
    LOG_ERROR("Failed to bind: %s", "Address already in use");
    return 0;
}
```

运行结果：

```
[INFO]  Starting server on port 8080
[ERROR] Failed to bind: Address already in use
```

`##__VA_ARGS__` 是 GCC 扩展，当可变参数为空时自动移除多余的逗号。标准的做法是用一个中间函数来处理。

## 原理解析

### 1. `printf` 家族全景

C 的格式化输出是一整个家族。它们做的事情相同（格式化字符串），只是输出目标不同：

| 函数 | 输出目标 | 说明 |
|------|---------|------|
| `printf(...)` | stdout | 标准输出 |
| `fprintf(stream, ...)` | 指定 FILE* | stdout, stderr, 文件 |
| `sprintf(buf, ...)` | 字符串数组 | ⚠️ 无边界检查 |
| `snprintf(buf, size, ...)` | 字符串数组 | 安全，有边界检查 |
| `vprintf(fmt, args)` | stdout | 接受 va_list |
| `vfprintf(stream, fmt, args)` | 指定 FILE* | 接受 va_list |
| `vsprintf(buf, fmt, args)` | 字符串数组 | ⚠️ 无边界检查 |
| `vsnprintf(buf, size, fmt, args)` | 字符串数组 | 安全，接受 va_list |

**`v` 前缀**：所有带 `v` 的版本接受 `va_list`，用于在可变参数函数中转发参数。

### 2. `stdout`, `stderr` — 两个标准输出流

```c
#include <stdio.h>

printf("这是正常输出\n");           // → stdout
fprintf(stdout, "显式 stdout\n");   // → stdout
fprintf(stderr, "这是错误输出\n");  // → stderr
```

**为什么需要两个流？**
- `stdout`：程序正常输出（可被管道/重定向）
- `stderr`：错误和日志信息（通常不被重定向，保证错误信息可见）

```bash
# 重定向 stdout 到文件，stderr 仍显示在终端
./my_program 1> output.txt

# 重定向 stderr 到文件
./my_program 2> errors.log

# 重定向两者
./my_program 1> output.txt 2> errors.log
```

### 3. `sprintf` vs `snprintf` — 安全性对比

```c
/* ❌ sprintf: 无边界检查，缓冲区溢出风险 */
char buf[10];
sprintf(buf, "This string is way too long for the buffer!");
/* 溢出了！写入了不属于 buf 的内存 → undefined behavior */

/* ✅ snprintf: 指定最大写入长度，自动截断 */
char buf[10];
int needed = snprintf(buf, sizeof(buf), "This string is way too long!");
/* buf 内容: "This str" (9 chars + null terminator)
   needed = 39 → 如果 buf 够大，需要 39 个字符 */
```

**snprintf 返回值**：告诉你"如果不截断，需要多少个字符"。可以用来检测是否需要更大的缓冲区。

**我的建议**：永远不要用 `sprintf`。用 `snprintf`，它多一个参数，但能救你的命。

### 4. `va_list` 可变参数

C 允许函数接受不确定数量的参数——这就是 `printf` 的秘密。

```c
#include <stdio.h>
#include <stdarg.h>  /* va_list, va_start, va_end */

void my_print(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);   /* 初始化 args，从 fmt 之后的第一个参数开始 */
    vprintf(fmt, args);    /* 使用 vprintf 处理可变参数 */
    va_end(args);          /* 清理 */
}

int main(void) {
    my_print("Name: %s, Age: %d\n", "Alice", 25);
    my_print("PI = %.4f\n", 3.14159265);
    return 0;
}
```

**三步曲**：
```
va_start(args, last_named_param)  → 告诉 va_list 从哪里开始
vprintf / vfprintf / vsnprintf    → "v" 版本函数消费 va_list
va_end(args)                      → 清理（实际上通常是 nullptr）
```

### 5. 自定义 `printf`-like 函数

```c
#include <stdio.h>
#include <stdarg.h>

/** 自定义日志函数：自动添加 [LOG] 前缀 */
void my_log(const char *fmt, ...) {
    fprintf(stdout, "[LOG] ");        /* 前缀 */
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args);     /* 转发可变参数 */
    va_end(args);
}

int main(void) {
    my_log("Hello from C, version %d.%d\n", 17, 2);
    /* 输出: [LOG] Hello from C, version 17.2 */
}
```

### 6. 日志级别宏

最实用的 C 日志系统——通过宏定义不同级别的日志，编译期控制输出：

```c
/* 定义日志级别：1=ERROR, 2=WARN, 3=INFO, 4=DEBUG */
#ifndef LOG_LEVEL
#define LOG_LEVEL 3  /* 默认 INFO 级别 */
#endif

#define LOG_ERROR(...) \
    do { if (LOG_LEVEL >= 1) fprintf(stderr, "[ERROR] " __VA_ARGS__); } while(0)
#define LOG_WARN(...) \
    do { if (LOG_LEVEL >= 2) fprintf(stderr, "[WARN]  " __VA_ARGS__); } while(0)
#define LOG_INFO(...) \
    do { if (LOG_LEVEL >= 3) fprintf(stdout, "[INFO]  " __VA_ARGS__); } while(0)
#define LOG_DEBUG(...) \
    do { if (LOG_LEVEL >= 4) fprintf(stdout, "[DEBUG] " __VA_ARGS__); } while(0)
```

**控制输出等级**：修改 `LOG_LEVEL` 即可：

```c
#define LOG_LEVEL 1   /* 只输出 ERROR */
#define LOG_LEVEL 3   /* 输出 ERROR + WARN + INFO（常用） */
#define LOG_LEVEL 4   /* 全部输出（调试时） */
```

**编译期优化**：现代编译器会发现 `if (LOG_LEVEL >= 4)` 在 `LOG_LEVEL = 1` 时永不执行，会自动优化掉这段代码，零运行时开销。

### 7. `__FILE__`, `__LINE__`, `__func__` 内置宏

编译器自动提供这三个宏，在日志中非常有用：

| 宏 | 类型 | 示例值 |
|----|------|--------|
| `__FILE__` | `const char*` | `"src/main.c"` |
| `__LINE__` | `int` | `42` |
| `__func__` | `const char*` | `"main"` |
| `__DATE__` | `const char*` | `"Apr 27 2026"` |
| `__TIME__` | `const char*` | `"14:30:00"` |

```c
#define LOG_DEBUG(fmt, ...) \
    fprintf(stdout, "[DEBUG] %s:%d %s(): " fmt "\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__)

void process_data(void) {
    int x = 42;
    LOG_DEBUG("x = %d", x);
    /* 输出: [DEBUG] src/main.c:10 process_data(): x = 42 */
}
```

### 8. 带时间戳的日志函数

```c
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

void timestamped_log(const char *level, const char *fmt, ...) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);

    fprintf(stderr, "%s [%s] ", time_buf, level);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

/* 使用 */
timestamped_log("INFO", "Server started on port %d", 8080);
/* 输出: 14:30:00 [INFO] Server started on port 8080 */
```

## 常见错误

### ❌ 错误 1：`sprintf` 缓冲区溢出

```c
char buf[8];
sprintf(buf, "Hello, %s!", username);  /* ❌ username 太长 → 溢出 */
```

✅ **修复**：用 `snprintf`。

```c
char buf[8];
snprintf(buf, sizeof(buf), "Hello, %s!", username);
/* 自动截断，不会溢出 */
```

### ❌ 错误 2：忘记 `va_end`

```c
void broken_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    /* 忘记 va_end(args); → ❌ 未定义行为 */
}
```

✅ **修复**：每次 `va_start` 必须配对 `va_end`。

### ❌ 错误 3：`va_list` 被多次使用

```c
void multiuse(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    /* args 状态不确定了，不能再次使用 */
    vprintf(fmt, args);  /* ❌ 可能崩溃 */
    va_end(args);
}
```

✅ **修复**：用 `va_copy` 复制一份：

```c
va_list args1, args2;
va_start(args1, fmt);
va_copy(args2, args1);
vprintf(fmt, args1);    /* 第一次使用 */
vprintf(fmt, args2);    /* 第二次使用 */
va_end(args2);
va_end(args1);
```

### ❌ 错误 4：日志级别宏没有 `do { } while(0)`

```c
#define LOG_INFO(...) fprintf(stdout, __VA_ARGS__);  /* ❌ 注意分号 */

if (condition)
    LOG_INFO("info");  /* 展开成: if (c) fprintf(...); ← 后面的 else 挂掉 */
else
    printf("no info");
```

✅ **修复**：用 `do { } while(0)` 包裹宏体，保证在 `if/else` 中行为正确。

## 动手练习

### 🟢 练习 1：用 `snprintf` 安全拼接路径

```c
/* 使用 snprintf 拼接目录和文件名，缓冲区 64 字节
   如果超出 64 字节，打印警告信息 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

int main(void) {
    char path[64];
    int needed = snprintf(path, sizeof(path), "/tmp/%s/data", "project_name_12345");
    if (needed >= (int)sizeof(path)) {
        fprintf(stderr, "Warning: path truncated (%d chars needed, %zu buffer)\n",
                needed, sizeof(path));
    }
    printf("path: %s\n", path);
    return 0;
}
```
</details>

### 🟡 练习 2：实现带级别的日志宏

```c
/* 实现 LOG(level, fmt, ...) 宏，
   根据 level 参数输出不同颜色（终端 ANSI 转义码）：
   DEBUG=绿色, INFO=蓝色, WARN=黄色, ERROR=红色 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

#define LOG_DEBUG(...) fprintf(stdout, "\033[32m[DEBUG]\033[0m " __VA_ARGS__)
#define LOG_INFO(...)  fprintf(stdout, "\033[34m[INFO]\033[0m  " __VA_ARGS__)
#define LOG_WARN(...)  fprintf(stderr, "\033[33m[WARN]\033[0m  " __VA_ARGS__)
#define LOG_ERROR(...) fprintf(stderr, "\033[31m[ERROR]\033[0m " __VA_ARGS__)

int main(void) {
    LOG_INFO("Program starting\n");
    LOG_WARN("Memory usage high\n");
    LOG_ERROR("Disk full\n");
    return 0;
}
```
</details>

### 🔴 练习 3：实现 `printf` 级别的格式化写入文件

```c
/* 实现 flog(FILE *f, const char *fmt, ...) 函数，
   类似 fprintf 但自动添加 [LOG] 前缀和换行 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdarg.h>

void flog(FILE *f, const char *fmt, ...) {
    fprintf(f, "[LOG] ");
    va_list args;
    va_start(args, fmt);
    vfprintf(f, fmt, args);
    va_end(args);
    fprintf(f, "\n");
}

int main(void) {
    FILE *fp = fopen("app.log", "w");
    if (fp) {
        flog(fp, "Server PID = %d", 12345);
        flog(fp, "Listening on port %d", 8080);
        fclose(fp);
    }
    return 0;
}
```
</details>

## 与 Python 对比

| 特性 | C (`printf` 家族 + 宏) | Python (`logging` 模块) |
|------|----------------------|------------------------|
| 开箱即用 | `printf` 一行代码 | 需要 `import logging` + 配置 |
| 级别控制 | 自己写宏实现 | `logging.basicConfig(level=logging.DEBUG)` |
| 文件/行号 | 需要 `__FILE__`, `__LINE__` 宏 | 自动记录 |
| 自定义格式 | 自己写 | `Formatter` 对象 |
| Handler | 自己用 `fprintf` 选择输出 | `FileHandler`, `StreamHandler` 等 |
| 线程安全 | 需要自己保证 | 内置线程安全 |
| 性能 | 极快（编译后直接调用 C 函数） | 较慢（解释器开销） |

**我的理解**：Python 的 `logging` 是一个功能完善的库，开箱即用；C 需要你自己组装。但 C 的优势是——你完全控制每个细节，而且运行速度极快。

## 故障排查（FAQ）

### Q: 为什么 `fprintf(stderr, ...)` 立即输出，但 `printf` 有时延迟？

`stdout` 是**行缓冲**的（line-buffered）——遇到 `\n` 或缓冲区满时才输出。`stderr` 是**无缓冲**的——立即输出。这就是为什么错误信息用 `stderr`：即使程序崩溃，错误信息也已经写出来了。

### Q: `printf` 和 `sprintf` 的性能有区别吗？

`sprintf` 写入内存，`printf` 写入终端/文件——实际上 `sprintf` 更快（不需要 I/O 系统调用）。但**不要用 `sprintf`**——`snprintf` 的安全保证值得一丁点性能损失。

### Q: 可变参数的类型安全吗？

**不**。格式字符串和实际参数类型不匹配时，`printf` 不会报错，但会产生垃圾输出或崩溃：

```c
printf("%d\n", 3.14);  /* 把 double 按 int 解读 → 垃圾值 */
printf("%s\n", 42);    /* 把 42 当指针 → 段错误！ */
```

编译器的 `-Wall -Wformat` 会检查常见不匹配，但不会检查所有情况。

### Q: `vsnprintf` 返回值为负数意味着什么？

编码错误。正常的返回值是非负数（包括截断的情况）。负数表示格式化过程中出了问题。

## 知识扩展（选学）

### `%m` 格式符（glibc 扩展）

自动输出 `strerror(errno)` 的内容：

```c
#include <stdio.h>
#include <errno.h>

FILE *fp = fopen("nonexistent.txt", "r");
if (!fp) {
    perror("open");           /* open: No such file or directory */
    fprintf(stderr, "%m\n");   /* 等效: No such file or directory */
}
```

### `asprintf`（GNU 扩展）

自动分配足够大小的缓冲区：

```c
#include <stdio.h>

char *buf = NULL;
asprintf(&buf, "Hello, %s! Score: %d", "Alice", 95);
printf("%s\n", buf);
free(buf);  /* 必须 free */
```

### 颜色输出（ANSI 转义码）

```c
#define RED   "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

printf(RED "Error!" RESET "\n");  /* 终端显示红色 */
```

## 小结

祝贺！你已经掌握了 C 语言的日志与格式化输出。让我总结一下——

- **`printf` 家族**：`printf`(stdout), `fprintf`(任意流), `sprintf`(字符串，危险), `snprintf`(字符串，安全), `v` 前缀版本处理 `va_list`
- **`stdout` vs `stderr`**：正常输出 vs 错误输出
- **`snprintf`** 永远替代 `sprintf`——多一个参数，救一条命
- **`va_list`**：`va_start` → `vprintf` 系列 → `va_end`（三步曲）
- **日志级别宏**：用 `#define LOG_LEVEL` 控制输出等级，编译期优化
- **`__FILE__`, `__LINE__`, `__func__`**：编译器内置宏，日志中的定位利器
- **`do { } while(0)`**：多语句宏的安全用法

> **我的理解**：C 的日志哲学是"你给我一个格式字符串和参数，我帮你拼出来"——它不关心你的日志级别、不关心你的输出目标、不关心时间戳。这些都需要你自己实现。但正因为如此，你可以完全控制日志系统的每个细节。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 格式化输出（Formatted Output） | 按格式字符串输出数据 |
| 可变参数（Variadic Arguments） | 函数接受不确定数量的参数 |
| `va_list` | 可变参数列表类型 |
| 标准输出（stdout） | 正常的程序输出流 |
| 标准错误（stderr） | 程序错误/日志输出流 |
| 缓冲区溢出（Buffer Overflow） | 写入超出缓冲区边界的内存 |
| 日志级别（Log Level） | DEBUG / INFO / WARN / ERROR |
| 编译期优化（Compile-time Optimization） | 编译器在编译时移除不可达代码 |
| 行缓冲（Line-buffered） | 遇到 `\n` 才输出 |
| 无缓冲（Unbuffered） | 立即输出 |

## 延伸阅读

- [cppreference: Formatted Output (C)](https://en.cppreference.com/w/c/io/fprintf)
- [cppreference: Variadic Functions (C)](https://en.cppreference.com/w/c/variadic)
- [Beej's Guide to C: Variadic Functions](https://beej.us/guide/bgc/)
- K&R《C 程序设计语言》第 7.3 章：变参函数

## 继续学习

你现在已经掌握了 C 的格式化输出系统。下一章我们将学习**调试与错误处理**，掌握 `errno`、`assert`、`gdb` 调试技巧和信号处理，让你的程序更健壮、更好调试。

> 💡 **提示**：替换代码中所有 `sprintf` 为 `snprintf`，把 `printf` 错误信息改为 `fprintf(stderr, ...)`。你会立刻拥有更安全的程序。

[← 上一章：头文件与模块系统](./headers.md) | [下一章：调试与错误处理 →](./debugging.md)
