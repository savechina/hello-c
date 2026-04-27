# 系统调用 (System Calls)

> "操作系统是一个房东——它把钥匙（文件描述符）给你，把门铃（信号）装好，你可以直接开窗看水管（mmap），但如果你不敲门就闯进去，房东会毫不留情地请你在外面。"

## 开篇故事

想象你住在一个大型公寓楼里。大楼管理员（操作系统）管理着一切：水管、电线、门锁。

你不能直接改水管——你得先申请钥匙（打开文件描述符 `open`）。如果有紧急事件（比如火灾报警器响了），管理员会按你的门铃（POSIX 信号 `signal`），你必须放下手里的事去处理。如果你想查看水管布局，不需要跑到地下室——管理员允许你在墙上开窗（`mmap` 内存映射），直接看到水管的样子。甚至你还可以克隆一个自己去帮忙干活（`fork` 子进程），通过一根管子（`pipe`）和分身沟通。

## 本章适合谁

- 学过文件 I/O，但想理解"底层到底发生了什么"的人
- 听说过"信号"、"进程"、"内存映射"但不清楚它们在 C 中怎么用的人
- 想编写系统级别工具（监控、守护进程、网络服务）的人
- 好奇"操作系统到底是个什么东西"的人

## 你会学到什么

- 文件描述符（File Descriptors）— `open`、`read`、`write`、`close`
- POSIX 信号（Signals）— `signal`、`sigaction`、信号处理函数
- 内存映射 I/O（`mmap`/`munmap`）— 文件和内存的"直接窗口"
- 进程管理（`fork`/`wait`）— 克隆自己、收集子进程
- 管道 IPC（`pipe`）— 父子进程之间的通信

> ⚠️ **注意**：本章有完整源代码 `src/advance/system_sample.c`（约 396 行），可直接运行。以下文档是概念讲解。

## 前置要求

- 掌握指针、文件 I/O (`fopen`/`fclose`)
- 理解 `errno` 错误码模式
- 了解进程和线程的基本概念

## 第一个例子

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    /* 打开文件 (拿到钥匙) */
    int fd = open("/tmp/hello.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed");
        return 1;
    }

    /* 写出数据 */
    const char *msg = "Hello, System Call!\n";
    write(fd, msg, strlen(msg));

    /* 归还钥匙 */
    close(fd);
    return 0;
}
```

四步走：`open → write → close`——和 `fopen → fprintf → fclose` 思路一样，但更底层。

## 原理解析

### 1. 文件描述符 (File Descriptors)

Linux/macOS 中，**一切皆文件**。键盘、鼠标、网络、普通文件——全部用 fd（整数）表示。

```
标准 fd:
  STDIN_FILENO  = 0  (标准输入 — 键盘)
  STDOUT_FILENO = 1  (标准输出 — 屏幕)
  STDERR_FILENO = 2  (标准错误 — 屏幕)

自定义:
  open() 返回 ≥ 3 的整数
```

```c
int fd = open("/tmp/test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
// fd = 3 (假设 0,1,2 已被标准流占用)

write(fd, "hello", 5);  // 向 fd=3 写 5 字节
read(fd, buf, 10);      // 从 fd=3 读 10 字节（需 O_RDWR）
close(fd);              // 归还 fd=3
```

**我的理解**：fd 就是"钥匙编号"。0、1、2 是标配的三把钥匙（输入、输出、错误），`open()` 给你第四把、第五把……用完必须 `close()` 归还，否则钥匙不够用。

### 2. POSIX 信号 (Signals)

信号是操作系统发给你的"门铃"——通知你发生了事件：

| 信号 | 含义 |
|------|------|
| `SIGINT` | Ctrl+C 终止 |
| `SIGTERM` | 温柔的终止请求 |
| `SIGKILL` | 强制终止（无法捕） |
| `SIGSEGV` | 段错误（访问非法内存） |
| `SIGPIPE` | 写入已关闭的管道 |
| `SIGALRM` | 闹钟超时 |

```c
#include <signal.h>

static volatile sig_atomic_t g_interrupted = 0;

void handler(int sig) {
    g_interrupted = 1;  /* 只能做简单操作！不能 printf/printf/malloc */
}

int main(void) {
    signal(SIGINT, handler);  /* 注册 Ctrl+C 处理 */

    while (!g_interrupted) {
        /* 主循环 */
    }
    printf("收到 SIGINT，安全退出。\n");
    return 0;
}
```

**关键规则**：信号处理函数里**只能**用 async-signal-safe 函数（如 `write`），不能用 `printf`、`malloc`、`free` 等——它们不是重入安全的。

### 3. 内存映射 I/O (mmap / munmap)

`mmap` 把文件直接映射到进程的地址空间——读文件就像读内存一样快。

```c
#include <sys/mman.h>

int fd = open("data.bin", O_RDWR);
ftruncate(fd, 1024);  /* 设置文件大小 */

void *mapped = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
// mapped 现在指向一段 1024 字节的内存

// 直接读写这片内存——相当于读写字
memcpy(mapped, "hello", 5);
printf("%s\n", (char *)mapped);

munmap(mapped, 1024);  /* 释放映射 */
close(fd);
```

**类比**：`mmap` 是在你家墙上开一扇窗，直接看到水管（文件内容），不需要每次跑到地下室（调 read/write）。

### 4. 进程管理: fork / wait

```c
pid_t pid = fork();

if (pid < 0) {
    // fork 失败
} else if (pid == 0) {
    // 子进程: 返回 0
    printf("我是子进程!\n");
    _exit(0);
} else {
    // 父进程: 返回子进程 PID
    printf("子进程 PID=%d\n", pid);
    int status;
    waitpid(pid, &status, 0);  /* 等子进程结束 */
}
```

**关键规则**：
- fork 返回**两次**：子进程得 0，父进程子 PID
- 子进程必须用 `_exit()`（不是 `exit()`），避免刷新父进程的缓冲区
- 父进程必须 `wait()`，否则子进程变成僵尸（zombie）

### 5. 管道 IPC (pipe)

```c
int pipefd[2];
pipe(pipefd);  /* pipefd[0] = 读, pipefd[1] = 写 */

pid_t pid = fork();
if (pid == 0) {
    close(pipefd[1]);      /* 子: 关写入端 */
    read(pipefd[0], buf, sizeof(buf));  /* 读 */
    close(pipefd[0]);
} else {
    close(pipefd[0]);      /* 父: 关读取端 */
    write(pipefd[1], msg, strlen(msg)); /* 写 */
    close(pipefd[1]);
    waitpid(pid, NULL, 0);
}
```

**类比**：pipe 是父子之间一根单向管子——一头写、一头读。

## 常见错误

### ❌ 错误 1: 不检查 open 返回值

```c
// ❌ 如果 open 失败，fd = -1，后续 write 全部失败
int fd = open("/nonexistent", O_RDONLY);
write(fd, buf, 10);  /* fd=-1，写入失败 */

// ✅ 检查返回值
int fd = open("/nonexistent", O_RDONLY);
if (fd < 0) {
    perror("open");
    return 1;
}
```

### ❌ 错误 2: 信号处理函数里用 printf

```c
// ❌ printf 不是 async-signal-safe!
void handler(int sig) {
    printf("收到信号 %d\n", sig);  /* 可能死锁或崩溃 */
}

// ✅ 用 write
void handler(int sig) {
    const char msg[] = "SIGINT received\n";
    write(STDERR_FILENO, msg, sizeof(msg) - 1);
}
```

### ❌ 错误 3: 忘记关闭 fd 导致泄漏

```c
// ❌ 循环中打开但忘记关闭
for (int i = 0; i < 10000; i++) {
    int fd = open("test.txt", O_RDONLY);  /* fd 泄漏 */
}

// ✅ 配对 close
for (int i = 0; i < 10000; i++) {
    int fd = open("test.txt", O_RDONLY);
    // ... do something ...
    close(fd);  /* 归还 */
}
```

### ❌ 错误 4: 忽略 SIGPIPE

```c
// ❌ 写入已关闭的 socket/pine → 默认 SIGPIPE 终止进程
write(pipefd[1], "hello", 5);  /* 如果读端已关闭 → 进程被 SIGPIPE 杀死 */

// ✅ 忽略 SIGPIPE 并检查 write 返回值
signal(SIGPIPE, SIG_IGN);
ssize_t n = write(pipefd[1], "hello", 5);
if (n < 0) {
    perror("write to broken pipe");
}
```

## 动手练习

### 🟢 练习 1: 文件描述符读写

用 `open`/`write`/`read`/`close` 复制一个文件（从 `src/data.txt` 复制到 `src/copy.txt`）。

<details>
<summary>点击查看答案</summary>

```c
int src = open("data.txt", O_RDONLY);
int dst = open("copy.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
char buf[4096];
ssize_t n;
while ((n = read(src, buf, sizeof(buf))) > 0) {
    write(dst, buf, (size_t)n);
}
close(src);
close(dst);
```
</details>

### 🟡 练习 2: SIGALRM 超时

设置 2 秒的 `SIGALRM`，在等待用户输入时超时打印 "Timeout!"。

<details>
<summary>点击查看答案</summary>

```c
volatile sig_atomic_t alarm_flag = 0;
void alarm_handler(int sig) { (void)sig; alarm_flag = 1; }

int main(void) {
    signal(SIGALRM, alarm_handler);
    alarm(2);
    printf("输入你的名字 (2秒内): ");
    char buf[64];
    if (fgets(buf, sizeof(buf), stdin)) {
        alarm(0);  /* 取消闹钟 */
        printf("你好, %s", buf);
    } else if (alarm_flag) {
        printf("Timeout!\n");
    }
    return 0;
}
```
</details>

### 🔴 练习 3: 管道 + 子进程

用 fork + pipe 实现：父进程通过管道发送一个整数给子进程，子进程计算平方后通过另一个管道返回给父进程。

<details>
<summary>点击查看答案</summary>

```c
int p1[2], p2[2];  /* p1: 父→子, p2: 子→父 */
pipe(p1);
pipe(p2);

pid_t pid = fork();
if (pid == 0) {
    /* 子进程: 读 p1[0]，算平方，写 p2[1] */
    close(p1[1]); close(p2[0]);
    int n;
    read(p1[0], &n, sizeof(n));
    int result = n * n;
    write(p2[1], &result, sizeof(result));
    close(p1[0]); close(p2[1]);
    _exit(0);
} else {
    /* 父进程: 写 p1[1]，读 p2[0] */
    close(p1[0]); close(p2[1]);
    int n = 7;
    write(p1[1], &n, sizeof(n));
    close(p1[1]);
    int result;
    read(p2[0], &result, sizeof(result));
    printf("%d^2 = %d\n", n, result);
    close(p2[0]);
    waitpid(pid, NULL, 0);
}
```
</details>

## 故障排查

### Q: open() 返回 -1，errno = 13

**错误码 13 = EACCES (Permission denied)**。检查文件权限：`ls -la filename`。用 `O_RDONLY` 试试，不要 `O_RDWR`。

### Q: fork() 返回 -1

**fork 失败**通常因为进程数超限。用 `ulimit -u` 检查，或 `ps aux | wc -l` 看当前进程数。

### Q: mmap 出现 SIGSEGV（段错误）

可能原因：
1. 写入只读映射（`PROT_READ` 却写）
2. 越界访问（写入 offset ≥ 文件大小）
3. 文件大小为 0 却映射了区域

### Q: 子进程变成了僵尸（Zombie）

父进程没有 `wait()` / `waitpid()` 收尸。修复：在父进程中调用 `waitpid(pid, &status, 0)`。

## 知识扩展

### 1. sigaction vs signal

`signal()` 是简单的，但不可移植。**生产代码应该用 `sigaction()`**：

```c
struct sigaction sa;
memset(&sa, 0, sizeof(sa));
sa.sa_handler = my_handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;  /* 自动重启被中断的系统调用 */
sigaction(SIGINT, &sa, NULL);
```

### 2. O_DIRECT / O_SYNC

`open()` 时可以加标志：
- `O_SYNC`: 每次 write 都等待磁盘确认（慢但安全）
- `O_DIRECT`: 绕过内核缓冲区，直接磁盘 I/O（数据库常用）

### 3. epoll / kqueue

处理大量网络连接时，`select`/`poll` 不够用。Linux 用 `epoll`，macOS/FreeBSD 用 `kqueue`——这是 Nginx 等高性能服务器的核心。

## 本节是系统调用文档，已覆盖: fd、信号、mmap、fork、pipe。

- **文件描述符** = 钥匙（open 拿钥匙，close 还钥匙）
- **信号** = 门铃（只能做简单操作，不能阻塞）
- **mmap** = 直接开窗看水管（文件当内存用）
- **fork** = 克隆自己（子进程必须 _exit，父进程必须 wait）
- **pipe** = 单向管子（一端写，一端读）

> **我的教训是**：系统调用是 C 的"核武器"——能力强，但一旦出错就是段错误、僵尸进程、信号死锁。记住：**每个 open 配 close，每个 fork 配 wait，每个信号处理函数简单粗暴**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 文件描述符（File Descriptor） | 整数，标识已打开的文件/管道/socket |
| 信号（Signal） | 操作系统发送的异步事件通知 |
| 信号处理函数（Signal Handler） | 收到信号时执行的回调函数 |
| 内存映射（Memory-Mapped I/O） | 将文件映射到进程地址空间 |
| 进程（Process） | 运行中的程序实例 |
| 父/子进程（Parent/Child） | fork 创建的进程关系 |
| 僵尸进程（Zombie） | 子进程已退出但父未 wait |
| 管道（Pipe） | 父子进程间的单向通信通道 |
| 异步信号安全（Async-Signal-Safe） | 可在信号处理函数中安全调用的函数 |
| 系统调用（System Call） | 用户态程序请求内核服务的接口 |

## 延伸阅读

- [Beej's Guide to Unix IPC](https://beej.us/guide/bgipc/) — 管道、信号、共享内存
- [Linux 系统调用手册](https://man7.org/linux/man-pages/man2/syscall.2.html) — man 2 查阅
- [Advanced Programming in the UNIX Environment (APUE)](https://en.wikipedia.org/wiki/Advanced_Programming_in_the_UNIX_Environment) — 经典教材
- [UNIX 环境高级编程](https://zh.wikipedia.org/wiki/UNIX环境高级编程) — 中文版

## 继续学习

系统调用是你和操作系统之间的"契约"。理解 fd、信号、fork 之后，你可以构建更复杂的系统。

在下一章节中，我们将用 POSIX sockets 构建一个**HTTP 服务器**——把系统调用的所有知识整合到一起。

> 💡 **提示**：尝试运行 `src/advance/system_sample.c`——它演示了所有本节提到的概念。在 macOS/Linux 上编译：`make build && make run`。

[← 上一章：数据库](./database.md) | [下一章 → HTTP 服务器](./web.md)
