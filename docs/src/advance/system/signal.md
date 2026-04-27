# POSIX 信号处理 (Signal Handling)

> "信号像门铃——你正在房间里做事（主程序），门铃响了（信号），你去开门（信号处理函数），处理完继续做事。但记住：开门的时间不能太长，否则你的事就没人做了。"

## 开篇故事

你在房间里专心看书（主程序循环），突然门铃响了（SIGINT）——有人按 Ctrl+C 通知你该离开了。你放下书去开门（执行信号处理函数），告诉对方"我收到了"，然后回来继续看书。但如果门铃在你看书时一直响，你就会不断被打断。

更极端的情况：如果门铃代表火灾报警（SIGSEGV 段错误），你看到非法内存访问时就必须终止——没有"继续做事"的选项。

信号处理系统的核心规则：**信号处理函数里只能做简单的事**——设置一个标志，或者写一条消息到 stderr。不能用 `printf`（可能死锁），不能用 `malloc`（可能损坏堆）。

## 本章适合谁

- 程序被 Ctrl+C 杀死但想做些清理工作的人
- 听说过"信号处理函数"但不知道里面能写什么的人
- 想编写守护进程（daemon）或长时间运行的服务的人
- 好奇"为什么信号处理函数有这么多限制"的人

## 你会学到什么

- `sigaction()` — 注册信号处理器（替代旧的 `signal()`）
- `SIGINT`、`SIGTERM`、`SIGSEGV` — 常见信号
- `sigprocmask()` — 阻塞/解除阻塞信号
- `sigset_t` 信号集合操作
- 可重入函数（reentrant function）概念
- `SA_RESTART` 标志的影响

## 前置要求

- 理解函数指针
- 知道 `volatile` 和 `sig_atomic_t` 的作用
- 理解进程和系统的基本概念
- 了解 `errno` 基本概念

## 第一个例子

```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

static volatile sig_atomic_t interrupted = 0;

void handler(int sig) {
    (void)sig;
    const char msg[] = "Received SIGINT!\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
    interrupted = 1;
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    printf("Press Ctrl+C to interrupt...\n");
    while (!interrupted) {
        sleep(1);  // 等待信号
    }
    printf("Clean exit!\n");
    return 0;
}
```

注册信号处理器 → 主循环检查标志 → 信号到达设置标志 → 循环退出。

## 原理解析

### 1. 信号是什么

信号是操作系统发给进程的**异步通知**——一个整数编号的事件：

| 信号 | 编号 | 含义 |
|------|------|------|
| `SIGINT` | 2 | Ctrl+C 终止 |
| `SIGTERM` | 15 | 温柔的终止请求（默认 `kill`） |
| `SIGKILL` | 9 | 强制终止（无法捕获、无法忽略） |
| `SIGSEGV` | 11 | 段错误（访问非法内存） |
| `SIGPIPE` | 13 | 写入已关闭的管道 |
| `SIGALRM` | 14 | 闹钟超时 |
| `SIGUSR1/SIGUSR2` | 10/12 | 用户自定义信号 |

### 2. sigaction：注册信号处理器

```c
struct sigaction sa;
memset(&sa, 0, sizeof(sa));
sa.sa_handler = my_handler;    // 处理函数
sigemptyset(&sa.sa_mask);      // 处理期间不额外阻塞其他信号
sa.sa_flags = 0;               // 不使用特殊标志
sigaction(SIGINT, &sa, NULL);  // 注册
```

**为什么用 `sigaction` 而不是 `signal`？** `signal()` 的行为在不同系统上不一样（BSD vs System V）。`sigaction()` 行为一致、功能完整，是生产代码的选择。

### 3. 信号处理函数的限制

信号处理函数中**只能调用 async-signal-safe 函数**：

```c
/* ✅ 安全 */
void handler(int sig) {
    g_flag = 1;                              // 写 volatile sig_atomic_t
    write(STDERR_FILENO, msg, len);          // 不缓冲
}

/* ❌ 不安全 */
void handler(int sig) {
    printf("got signal\n");   // 可能死锁（printf 内部有锁）
    malloc(100);              // 可能损坏堆（不是重入安全的）
    free(ptr);                // 同上
    exit(0);                  // 可能导致 atexit 处理重复
}
```

**关键规则**：信号处理函数里做的事情越少越好——设置一个标志，主循环检查该标志。

### 4. sigprocmask：阻塞信号

```c
sigset_t set, old_set;
sigemptyset(&set);
sigaddset(&set, SIGUSR1);

sigprocmask(SIG_BLOCK, &set, &old_set);  // 阻塞 SIGUSR1
// ... 关键代码段 ...
sigprocmask(SIG_SETMASK, &old_set, NULL);  // 恢复
```

```
信号阻塞流程:
  1. SIG_BLOCK   — 添加到阻塞集合（新信号到达时被挂起）
  2. SIG_UNBLOCK — 从阻塞集合移除
  3. SIG_SETMASK — 替换整个集合
```

阻塞的信号不会丢失——解除阻塞后**挂起的信号会递送**。

### 5. SA_RESTART 标志

```c
sa.sa_flags = SA_RESTART;  // 被信号中断的系统调用自动重启
```

| 无 SA_RESTART | 有 SA_RESTART |
|---------------|---------------|
| read() 被中断 → 返回 -1, errno=EINTR | read() 被中断 → 自动重启 |
| 需要手动检查 EINTR | 无感知 |

## 常见错误

### ❌ 错误 1: 信号处理函数里用 printf

```c
// ❌ printf 内部有锁，如果主程序也持有锁 → 死锁
void handler(int sig) {
    printf("Got signal %d\n", sig);  // DEADLOCK!
}

// ✅ 用 write (async-signal-safe)
void handler(int sig) {
    const char msg[] = "Got signal\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
}
```

### ❌ 错误 2: 忘记用 sigemptyset

```c
// ❌ sa_mask 未初始化 → 可能意外阻塞随机信号
struct sigaction sa;
sa.sa_handler = handler;
// sa.sa_mask 垃圾值!
sigaction(SIGINT, &sa, NULL);

// ✅ 正确初始化
struct sigaction sa;
memset(&sa, 0, sizeof(sa));
sigemptyset(&sa.sa_mask);  // 清空信号集
sa.sa_handler = handler;
sigaction(SIGINT, &sa, NULL);
```

### ❌ 错误 3: 忽略 EINTR

```c
// ❌ read 被信号中断返回 -1, errno=EINTR — 这不是真错误!
ssize_t n = read(fd, buf, len);
if (n < 0) {
    perror("read");  // 误导！
}

// ✅ 正确处理
ssize_t n;
while ((n = read(fd, buf, len)) < 0 && errno == EINTR)
    ; // 被信号中断，重试
```

### ❌ 错误 4: volatile 没加

```c
// ❌ 不加 volatile — 编译器可能优化掉循环检查
int g_flag = 0;
while (!g_flag) { /* spin */ }

// ✅ volatile 告诉编译器：这个变量可能被信号处理函数修改
volatile sig_atomic_t g_flag = 0;
while (!g_flag) { /* spin */ }
```

## 动手练习

### 🟢 练习 1: 捕获 Ctrl+C

写一个程序，注册 SIGINT 处理函数，每次按 Ctrl+C 打印计数，按 3 次后退出。

<details>
<summary>点击查看答案</summary>

```c
volatile sig_atomic_t count = 0;
void handler(int sig) {
    (void)sig;
    count++;
    if (count >= 3) {
        const char msg[] = "Bye!\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        _exit(0);
    }
    const char msg[] = "Press 3 more times: ";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
    char buf[2] = { '0' + (char)(3 - count), '\0' };
    write(STDERR_FILENO, buf, 1);
    write(STDERR_FILENO, "\n", 1);
}

int main(void) {
    struct sigaction sa = { .sa_handler = handler };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    while (1) pause();
}
```
</details>

### 🟡 练习 2: SIGALRM 超时

设置 2 秒闹钟，如果在等待用户输入时超时，打印 "Timeout!" 并退出。

<details>
<summary>点击查看答案</summary>

```c
volatile sig_atomic_t timed_out = 0;
void alarm_handler(int sig) { (void)sig; timed_out = 1; }

int main(void) {
    struct sigaction sa = { .sa_handler = alarm_handler };
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    
    alarm(2);
    printf("Input your name (2 seconds): ");
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin)) {
        alarm(0);  // 取消闹钟
        printf("Hello, %s", buf);
    } else if (timed_out) {
        printf("Timeout!\n");
    }
    return 0;
}
```
</details>

### 🔴 练习 3: 信号掩码保护临界区

在关键代码段（如修改全局数据结构）到来时，临时阻塞相关信号。

<details>
<summary>点击查看答案</summary>

```c
sigset_t block, old;
sigemptyset(&block);
sigaddset(&block, SIGUSR1);

sigprocmask(SIG_BLOCK, &block, &old);
// 临界区：g_data 此时不会被 SIGUSR1 处理器修改
modify_shared_data();
sigprocmask(SIG_SETMASK, &old, NULL);
// SIGUSR1 挂起期间不会递送，解除后递送
```
</details>

## 故障排查

### Q: 信号处理函数没被调用

检查：1) 是否用 `sigaction` 正确注册 2) 信号号码是否正确 3) 信号是否被 `sigprocmask` 阻塞。

### Q: 程序突然终止，没有调用信号处理函数

某些信号**无法捕获/忽略**：`SIGKILL` (9) 和 `SIGSTOP` (17)。用 `SIGTERM` 代替 `SIGKILL`。

### Q: printf 在信号处理函数中导致死锁

主程序正在执行 `printf`（持有锁），信号到达打断 `printf`，信号处理函数也调 `printf`——死锁。改用 `write`。

## 知识扩展

### 1. sigaction 的 sa_mask

`sa_mask` 指定**信号处理期间额外阻塞的信号**。如果你的 handler 中也处理 SIGTERM，设 `sigaddset(&sa.sa_mask, SIGTERM)` 避免 SIGTERM 打断 SIGINT 处理。

### 2. SA_RESTART 何时用

- **读取交互输入**（tty）：通常不需要 SA_RESTART，因为用户输入本身就是阻塞等待
- **网络 socket**：建议 SA_RESTART，避免信号中断导致 read 返回 EINTR
- **需要超时的场景**：不用 SA_RESTART，手动处理 EINTR 实现超时

### 3. sigsetjmp / siglongjump

比 `setjmp/longjmp` 多一个功能：保存/恢复信号掩码。信号处理函数中可以用 `siglongjmp` 跳转回安全点。

## 小结

- **信号** = 操作系统的异步通知（门铃）
- **sigaction** 注册信号处理器，**不要**用旧的 `signal()`
- **信号处理函数**只写 `volatile sig_atomic_t` 标志，用 `write` 输出
- **sigprocmask** 阻塞信号，保护临界区
- **SA_RESTART** 让被中断的系统调用自动重启
- **不要**在信号处理函数中用 `printf`/`malloc`/`free`

> **我的教训是**：第一次写信号处理函数时，我用了 `printf` 和 `exit()`，程序有时正常运行，有时死锁。后来才知道 `printf` 不是可重入的。记住：**信号处理函数简单粗暴**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 信号（Signal） | 操作系统发送的异步事件通知 |
| 信号处理器（Signal Handler） | 收到信号时执行的回调函数 |
| 可重入（Reentrant） | 可被中断后重新进入仍安全的函数 |
| 异步信号安全（Async-Signal-Safe） | 可在信号处理函数中安全调用的函数 |
| 信号掩码（Signal Mask） | 当前阻塞的信号集合 |
| SA_RESTART | 自动重启被中断的系统调用 |

## 延伸阅读

- [POSIX sigaction(2)](https://pubs.opengroup.org/onlinepubs/9699919799/functions/sigaction.html) — 官方规范
- [Beej's Guide to Unix IPC](https://beej.us/guide/bgipc/) — 信号深入
- [Advanced Programming in the UNIX Environment](https://en.wikipedia.org/wiki/Advanced_Programming_in_the_UNIX_Environment) — 经典教材第 10 章

## 继续学习

你已经学会了如何用信号处理器"接听门铃"。接下来，我们将探索一种更高效的 I/O 方式——**内存映射 (mmap)**，让文件内容直接变成内存。

> 💡 **提示**：运行 `src/advance/system_signal_sample.c` 查看所有演示。`make build && make run`。

[← 上一章：文件与目录操作](./file.md) | [下一章：内存映射 I/O →](./mmap.md)
