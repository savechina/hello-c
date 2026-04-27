# 调试与错误处理（Debugging & Error Handling）

> "调试不是找 bug 的过程——是证明你的代码没有 bug 的过程，然后你会发现还有。" —— 我发现

## 开篇故事

想象一位侦探调查犯罪现场。他需要放大镜（`assert`）找到线索，指纹粉（`errno`）识别嫌疑人，还有嫌疑板（`gdb backtrace`）还原事件经过。

调试也是如此。程序崩溃时，`errno` 告诉你「出了什么错」，`assert` 帮你守住「不应该发生」的边界，`gdb` 让你逐步回放代码的执行过程。没有工具就靠猜，就像侦探不带工具进现场——你可能找到答案，但效率极低。

C 没有异常的魔法，你亲手拿起每一样工具。本章就带你认识 C 语言中所有的调试和错误处理工具。

## 本章适合谁

- 程序崩溃但不知道哪里错的人
- 被 `Segmentation fault` 折磨过的人
- 不知道 `errno`、`perror`、`assert` 的人
- 没用过 gdb（或只用过 printf 调试）的人

## 你会学到什么

- `errno` 和 `errno.h` 错误码系统
- `perror` 和 `strerror`——让错误信息可读
- `assert()` 断言与 `NDEBUG` 模式
- `gdb` 基本调试（断点、单步、查看、回溯）
- 信号处理（`SIGINT`、`SIGSEGV`）
- 错误返回约定（0 = 成功，-1 = 失败）

## 前置要求

- 能编译运行基本 C 程序
- 了解函数返回值的基本概念

## 第一个例子：检查 `fopen` 错误

```c
#include <stdio.h>
#include <errno.h>

int main(void) {
    FILE *fp = fopen("nonexistent.txt", "r");
    
    if (fp == NULL) {
        /* 方式 1: perror 自动打印 errno 对应的文本 */
        perror("fopen failed");
        /* 输出: fopen failed: No such file or directory */
        
        /* 方式 2: strerror 返回错误字符串 */
        printf("errno = %d, message: %s\n", errno, strerror(errno));
        
        return 1;  /* 错误返回码 */
    }
    
    fclose(fp);
    return 0;  /* 成功返回码 */
}
```

## 原理解析

### 1. `errno` — C 的全局错误码

`errno` 是一个线程局部变量（thread-local），由 C 标准库函数在出错时自动设置。

```c
#include <stdio.h>
#include <errno.h>
#include <math.h>

int main(void) {
    errno = 0;  /* ← 重要：使用前清零 */
    
    double result = sqrt(-1.0);
    
    if (errno != 0) {
        printf("sqrt(-1) 出错了! errno = %d\n", errno);
    }
    
    return 0;
}
```

**关键规则**：

| 规则 | 说明 |
|------|------|
| 使用前清零 | 成功时不会清零 errno，所以调用前设置 `errno = 0` |
| 只在出错时设置 | 库函数成功时不修改 errno |
| 不覆盖 | 连续出错时，后面的错误会覆盖前面的 errno |
| 线程局部 | 多线程中每个线程有独立的 errno |

**常见 errno 值**：

```
 1  EPERM      Operation not permitted
 2  ENOENT     No such file or directory
13  EACCES     Permission denied
22  EINVAL     Invalid argument
```

### 2. `perror` — 快速打印错误

```c
#include <stdio.h>

FILE *fp = fopen("missing.txt", "r");
if (fp == NULL) {
    perror("Error opening file");
    /* 输出: Error opening file: No such file or directory */
}
```

`perr` 自动拼接你的前缀和 `errno` 对应的文本——它是调试时最快获取可读错误信息的方法。

### 3. `strerror` — 获取错误字符串

```c
#include <string.h>
#include <errno.h>

printf("%s\n", strerror(errno));   /* 当前错误 */
printf("%s\n", strerror(2));       /* "No such file or directory" */
printf("%s\n", strerror(13));      /* "Permission denied" */
```

`strerror` 返回一个 `char*`，你可以自由使用它（比如写入日志文件）。

### 4. `assert()` — 编译期断言

`assert()` 是用来检查"理论上不可能发生"的情况。如果断言失败，程序立即中止。

```c
#include <stdio.h>
#include <assert.h>

int main(void) {
    int *ptr = malloc(100);
    assert(ptr != NULL);  /* 如果 malloc 失败，程序中止 */
    
    int x = 10;
    assert(x > 0);        /* 通过 */
    /* assert(x < 0);      ← 如果取消注释，程序中止并打印:
     * assertion "x < 0" failed: file "main.c", line 12 */
    
    free(ptr);
    return 0;
}
```

**assert 的输出格式**：

```
assertion "ptr != NULL" failed: file "src/main.c", line 8, function: main
程序中止（收到 SIGABRT 信号）
```

**`NDEBUG` — 关闭 assert**：

在发布版中，用 `-DNDEBUG` 编译可以禁用所有 assert，零运行时开销：

```bash
# 调试版: assert 生效
gcc -g main.c -o main_debug

# 发布版: assert 全部变成空操作
gcc -DNDEBUG -O2 main.c -o main_release
```

**我的建议**：`assert` 只用于检查**编程错误**（不应该发生的情况），不用于检查**运行时错误**（用户输入错误、文件不存在等）。

### 5. `gdb` 调试基础

```bash
# 1. 编译时加 -g 参数（包含调试信息）
gcc -g -O0 main.c -o main

# 2. 启动 gdb
gdb ./main

# 3. gdb 常用命令
(gdb) break main          # 在 main 处设置断点
(gdb) run                 # 执行程序
(gdb) next                # 单步执行（不进入函数）
(gdb) step                # 单步执行（进入函数）
(gdb) print variable      # 打印变量值
(gdb) print *ptr@10       # 打印指向前 10 个元素
(gdb) backtrace           # 查看调用栈
(gdb) list                # 显示当前代码
(gdb) continue            # 继续执行到下一个断点
(gdb) quit                # 退出 gdb
```

**一行启动**：

```bash
# 直接运行程序
gdb -batch -ex run ./main

# 自动崩溃后查看回溯
gdb -batch -ex run -ex bt ./main
```

### 6. 信号处理（Signal Handling）

C 程序可以捕获系统信号（如 Ctrl+C、段错误等）。

```c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void handle_sigint(int sig) {
    printf("\n收到 SIGINT (Ctrl+C)，正在清理...\n");
    /* 执行清理工作 */
    exit(0);
}

int main(void) {
    signal(SIGINT, handle_sigint);  /* 注册信号处理函数 */
    
    printf("按 Ctrl+C 退出...\n");
    while (1) {
        /* 等待信号 */
    }
}
```

**常见信号**：

| 信号 | 编号 | 触发方式 | 默认动作 |
|------|------|---------|---------|
| SIGINT | 2 | Ctrl+C | 终止 |
| SIGSEGV | 11 | 非法内存访问 | 终止 + core dump |
| SIGTERM | 15 | `kill` 命令 | 终止 |
| SIGABRT | 6 | `abort()` 调用 | 终止 + core dump |

### 7. 错误返回约定

函数返回值的惯例：

```c
/* 约定: 返回 0 = 成功, -1 = 失败 */
int open_config(const char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("fopen config");
        return -1;  /* 失败 */
    }
    /* ... 处理 ... */
    fclose(fp);
    return 0;  /* 成功 */
}

/* 调用方 */
if (open_config("/etc/app.conf") != 0) {
    fprintf(stderr, "Failed to open config\n");
    return 1;
}
```

**为什么是 -1 而不是其他值？** 这是 POSIX 的约定——`open()`、`read()`、`write()` 等系统调用都返回 -1 表示失败。保持一致让你的代码风格统一。

## 常见错误

### ❌ 错误 1：不检查函数返回值

```c
FILE *fp = fopen("important.txt", "r");  /* ❌ 假设立刻成功 */
char buf[100];
fgets(buf, 100, fp);  /* ❌ 如果 fopen 失败 → fp = NULL → fgets 崩溃! */
```

✅ **修复**：永远检查可能失败的函数。

```c
FILE *fp = fopen("important.txt", "r");
if (fp == NULL) {
    perror("fopen failed");
    return -1;
}
```

### ❌ 错误 2：错误使用 errno（没清零）

```c
/* ❌ 没清零 errno */
double r = sqrt(4.0);  /* 成功 */
if (errno != 0) {      /* 如果之前有错误残留 → 误判！ */
    printf("Error!\n");
}
```

✅ **修复**：调用前清零。

```c
errno = 0;
double r = sqrt(4.0);
if (errno != 0) {
    perror("sqrt failed");
}
```

### ❌ 错误 3：用 assert 处理运行时错误

```c
int read_input(int *value) {
    assert(value != NULL);  /* ✅ OK: 这是编程错误（调用方传了 NULL） */
    
    if (*value < 0) {
        assert(*value >= 0);  /* ❌ 不 OK: 这是运行时错误（用户输入了负数） */
        /* assert 在发布版会被关闭，检查就消失了！*/
    }
}
```

✅ **修复**：assert 只检查编程错误，运行时错误用 `if + return` 处理。

```c
if (*value < 0) {
    errno = EINVAL;
    return -1;  /* 发布版也有效 */
}
```

### ❌ 错误 4：信号处理函数中使用非异步安全的函数

```c
void handler(int sig) {
    printf("Signal received\n");  /* ❌ printf 不是异步安全的! */
    /* 在信号处理函数中调用非异步安全函数 → undefined behavior */
}
```

✅ **修复**：信号处理函数中只使用异步安全函数（如 `write`），或只设置一个 `volatile sig_atomic_t` 标志位。

```c
volatile sig_atomic_t got_signal = 0;

void handler(int sig) {
    got_signal = 1;  /* ✅ 安全 */
}
```

## 动手练习

### 🟢 练习 1：检查 `malloc` 失败

```c
/* 分配 1GB 内存（大概率失败），用 perror 打印错误
   然后用 assert 确保指针非 NULL */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void) {
    size_t huge = (size_t)1024 * 1024 * 1024 * 1024;
    void *ptr = malloc(huge);
    
    if (ptr == NULL) {
        perror("malloc huge memory");
        assert(ptr != NULL);  /* 会中止程序 */
    }
    
    free(ptr);
    return 0;
}
```
</details>

### 🟡 练习 2：实现带错误码的除法函数

```c
/* 实现 int safe_div(int a, int b, int *result)
   - b == 0 → errno = EINVAL, 返回 -1
   - result == NULL → errno = EINVAL, 返回 -1
   - 成功 → 返回 0
   测试所有分支 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <errno.h>

int safe_div(int a, int b, int *result) {
    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (b == 0) {
        errno = EINVAL;
        return -1;
    }
    *result = a / b;
    return 0;
}

int main(void) {
    int r;
    if (safe_div(10, 3, &r) == 0) {
        printf("10/3 = %d\n", r);
    }
    if (safe_div(10, 0, &r) != 0) {
        fprintf(stderr, "除以零: %s\n", strerror(errno));
    }
    return 0;
}
```
</details>

### 🔴 练习 3：用 gdb 调试段错误

```bash
# 写一个会触发段错误的程序：
int main(void) {
    int *p = NULL;
    *p = 42;  # 写入 NULL 指针 → SIGSEGV
    return 0;
}

# 在 gdb 中运行，用 backtrace 查看崩溃位置
```

<details>
<summary>点击查看答案</summary>

```bash
gcc -g -o crash crash.c
gdb ./crash
(gdb) run
(gdb) bt          # 查看调用栈
(gdb) info locals # 查看局部变量
(gdb) quit
```
</details>

## 故障排查（FAQ）

### Q: "Segmentation fault (core dumped)" 是什么？

访问了不属于自己的内存。常见原因：
- 解引用 NULL 指针
- 使用已 `free()` 的内存
- 数组越界
- 栈溢出（无限递归）

### Q: 怎么看 core dump？

```bash
# 确认 core dump 已启用
ulimit -c unlimited

# 运行程序触发崩溃
./my_program

# 用 gdb 查看 core dump
gdb ./my_program core
(gdb) bt  # 查看崩溃时的调用栈
```

### Q: `errno` 和返回值同时检查会冲突吗？

不会。典型模式：

```c
errno = 0;
long result = strtol("abc", NULL, 10);
if (result == 0 && errno != 0) {
    /* 出错了，errno 告诉你为什么 */
}
```

### Q: 发布版要不要关闭 assert？

推荐。用 `-DNDEBUG` 编译发布版，`assert` 变为空操作，零性能开销。但保留错误处理代码（`if + return -1` 模式）。

## 知识扩展（选学）

### `setjmp` / `longjmp` — C 的"异常"机制

C 没有 try/catch，但可以用 `setjmp`/`longjmp` 实现类似效果：

```c
#include <setjmp.h>

jmp_buf env;

void might_fail(void) {
    if (some_error) {
        longjmp(env, 1);  /* 跳回 setjmp 处 */
    }
}

int main(void) {
    if (setjmp(env) == 0) {
        might_fail();  /* 正常执行 */
    } else {
        /* 错误恢复路径 */
        printf("Caught error!\n");
    }
    return 0;
}
```

### AddressSanitizer（ASan）

GCC/Clang 内置的内存错误检测工具：

```bash
gcc -fsanitize=address -g main.c -o main
./main
# 自动检测: 越界、use-after-free、栈溢出等
```

### Valgrind

运行时内存错误检测工具（更强大）：

```bash
gcc -g main.c -o main
valgrind ./main
# 报告: 内存泄漏、未初始化变量、越界等
```

## 小结

祝贺！你已经掌握了 C 语言的调试与错误处理。让我总结一下——

- **`errno`**：库函数的全局错误码，使用前需清零
- **`perror`**：快速打印错误信息（`前缀: 错误文本`）
- **`strerror`**：获取错误码对应的字符串
- **`assert()`**：编译期检查编程错误，发布版用 `NDEBUG` 关闭
- **`gdb`**：断点、单步、查看变量、回溯调用栈
- **信号处理**：用 `signal()` 捕获 SIGINT/SIGSEGV 等
- **错误返回**：0 = 成功，-1 = 失败，errno 存详细信息

> **我的理解**：C 的错误处理哲学是"检查每一个返回值"——没有异常机制，没有 try/catch。每次函数调用都可能失败，你的代码必须检查。这很繁琐，但它让你完全掌控每个错误场景。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| errno | C 库函数的全局错误码 |
| perror | 打印 errno 对应的错误信息 |
| strerror | 返回错误码对应的字符串 |
| assert | 编译期断言，失败则中止程序 |
| NDEBUG | 关闭 assert 的编译宏 |
| Segmentation fault | 非法内存访问导致的崩溃 |
| Signal | 操作系统发送到进程的信号 |
| SIGINT | Ctrl+C 产生的中断信号 |
| SIGSEGV | 段错误信号（非法内存） |
| Backtrace | 调用栈回溯 |
| Core dump | 程序崩溃时的内存快照 |
| Asynchronous-safe | 可以在信号处理函数中安全调用的函数 |

## 延伸阅读

- [cppreference: Error Handling (C)](https://en.cppreference.com/w/c/error)
- [cppreference: assert (C)](https://en.cppreference.com/w/c/error/assert)
- [GDB Quick Reference](https://darkdust.net/assets/GDB%20Cheat%20Sheet.pdf)
- K&R《C 程序设计语言》第 7.4 章：错误处理

## 继续学习

你已经掌握了 C 语言的核心调试工具。现在你可以更自信地写出健壮的代码——每个函数都有错误检查，关键路径都有日志，调试时知道用 `assert` 和 `gdb`。下一章我们将学习**字符串高级操作**，包括字符串解析、格式化和 Unicode 处理。

> 💡 **提示**：在你现有代码中搜索所有没有检查返回值的 `malloc`/`fopen`/`strtol` 调用，加上 `NULL` 检查。你会立刻消灭一批潜在的崩溃点。

[← 上一章：日志与格式化输出](./logging.md) | [下一章：字符串高级操作 →](./strings.md)
