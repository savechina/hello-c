# 管道与进程间通信 (Pipes and IPC)

> "管道像两个房间之间的传声筒——这边说话，那边听到。Unix socket 像两个房间之间装了专用电话——双方都能同时说话和听。"

## 开篇故事

你有两个分身（父子进程）分工协作。你需要告诉分身"帮我算这个"——怎么传达？

最简单的办法：**插一根管子**（pipe）。管子一头在父进程（写），一头在子进程（读）。父进程往管子塞纸条（write），子进程从另一头取出（read）。

但单根管只能一个方向传。如果需要双向通信——父说完了子也要回话——就需要两根管子（双向管道）。

更高级的方式：**用专用电话**（Unix domain socket）。一根电话线，双方都能说话和听——全双工。

## 本章适合谁

- 想实现父子进程间数据交换的人
- 好奇管道命令（`|`）底层原理的人
- 想用进程间通信但不知道选 pipe 还是 socket 的人
- 想了解 Unix "一切皆文件"哲学的人

## 你会学到什么

- `pipe()` — 创建单向管道
- 双向管道 — 两根 pipe 实现双向通信
- `socketpair(AF_UNIX)` — 全双工 Unix 域套接字
- 管道缓冲区 — 写入满时阻塞，读完时阻塞
- SIGPIPE — 写已关闭的管道时的默认终止信号
- 关闭不需要的管道端 — 防死锁的关键

## 前置要求

- 理解 fork 的基本原理
- 理解文件描述符操作
- 熟悉 write/read 用法
- 知道进程间通信的基本概念

## 第一个例子

```c
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    int pipefd[2];
    pipe(pipefd);  // pipefd[0] = 读, pipefd[1] = 写

    pid_t pid = fork();
    if (pid == 0) {
        /* 子进程: 读 */
        close(pipefd[1]);  // 关闭写端
        char buf[64];
        read(pipefd[0], buf, sizeof(buf));
        printf("子进程收到: %s\n", buf);
        close(pipefd[0]);
        _exit(0);
    } else {
        /* 父进程: 写 */
        close(pipefd[0]);  // 关闭读端
        write(pipefd[1], "Hello!", 6);
        close(pipefd[1]);
        wait(NULL);
    }
    return 0;
}
```

pipe → fork → 关不需要的端 → 读/写 → 收尸。

## 原理解析

### 1. pipe — 单向管道

```c
int pipefd[2];
pipe(pipefd);
// pipefd[0] = 读端
// pipefd[1] = 写端
```

```
  父进程                    子进程
    │                         │
    │    pipe: [0]←───[1]     │
    │     读端      写端      │
    │                         │
  close[0]                 close[1]
      │                         │
      │    ←── write("hi")    │
      │    ←── read() —──────→│
      │                         │
```

管道是**单向**的：一端写（pipefd[1]），一端读（pipefd[0]）。

### 2. 阻塞行为

```
管道有缓冲区（通常 64KB）：

情况 1: 管道空 — read() 阻塞等待
  [写端未关闭] → read() 阻塞直到有数据
  
情况 2: 管道满 — write() 阻塞等待
  [管道 64KB 满了] → write() 阻塞直到有空间

情况 3: 所有读端关闭 — write() 触发 SIGPIPE
  进程被 SIGPIPE 杀死（默认行为！）
  
情况 4: 所有写端关闭 — read() 返回 0 (EOF)
  read() 返回 0 → 数据结束
```

### 3. 双向管道

```c
int p1[2];  // 父→子
int p2[2];  // 子→父
pipe(p1);
pipe(p2);
```

```
  父进程                    子进程
    │                         │
  p1[1]→───────读 p1[0]       │
    │                         │
  读 p2[0]←───────p2[1]       │
    │                         │
```

双向通信需要两根管子（或全双工 socket），因为每根 pipe 只能单向流动。

### 4. socketpair — 全双工通信

```c
int sv[2];
socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
// sv[0] 和 sv[1] 都是全双工！
```

```
  父进程          子进程
    │               │
  sv[0]←——→sv[1]
    │               │
  读写均可         读写均可
```

`socketpair` 创建一对全双工 socket。双方都可以同时读写，比两根 pipe 简单得多。

### 5. SIGPIPE 处理

```c
/* 默认行为：写已关闭的管道 → SIGPIPE 终止进程 */
signal(SIGPIPE, SIG_IGN);  // 忽略 SIGPIPE
ssize_t n = write(fd, "hi", 2);
if (n < 0 && errno == EPIPE) {
    // 管道破裂，优雅处理
}
```

## 常见错误

### ❌ 错误 1: fork 后没关闭不需要的端

```c
// ❌ 子进程不关写的端，父进程读不到 EOF
int pipefd[2];
pipe(pipefd);
pid_t pid = fork();
if (pid == 0) {
    // 子: 只读，但忘了关 pipefd[1] (写端)
    char buf[64];
    read(pipefd[0], buf, sizeof(buf));  // 阻塞！因为写端还开着
}
// 父: 关读端，写, 关写端
close(pipefd[0]);
write(pipefd[1], "hi", 2);
close(pipefd[1]);
// 子进程死锁！

// ✅ fork 后关闭不需要的端!
```

### ❌ 错误 2: write 不检查返回值

```c
// ❌ 管道可能缓冲区满，write 只写了一部分
write(pipefd[1], big_data, big_len);  // 可能只写了部分!

// ✅ 检查并处理
ssize_t nw = write(pipefd[1], big_data, big_len);
if (nw < 0) perror("write to pipe");
```

### ❌ 错误 3: 忽略 SIGPIPE 导致进程异常退出

```c
// ❌ 写已关闭的读端 → SIGPIPE 杀死进程
close(pipefd[0]);  // 子关读端
write(pipefd[1], "hi", 2);  // SIGPIPE! 进程终止!

// ✅ 忽略 SIGPIPE
signal(SIGPIPE, SIG_IGN);
ssize_t n = write(pipefd[1], "hi", 2);
if (n < 0) perror("broken pipe");
```

### ❌ 错误 4: pipe 大小限制

```c
// ❌ 假设写入 64KB 以上不会阻塞（会！）
char big[128 * 1024];  // 128KB > 管道缓冲区 (64KB)
write(pw, big, sizeof(big));  // 写满 64KB 后阻塞!

// ✅ 大文件不用 pipe，用 mmap 或文件
```

## 动手练习

### 🟢 练习 1: 简单管道通信

用 fork + pipe 实现：父进程发消息给子进程，子进程打印收到。

<details>
<summary>点击查看答案</summary>

```c
int pf[2]; pipe(pf);
pid_t pid = fork();
if (pid == 0) {
    close(pf[1]);
    char buf[64];
    read(pf[0], buf, sizeof(buf));
    printf("Child: %s\n", buf);
    close(pf[0]);
    _exit(0);
} else {
    close(pf[0]);
    write(pf[1], "Hello!", 6);
    close(pf[1]);
    wait(NULL);
}
```
</details>

### 🟡 练习 2: 双向管道

父进程发 "ping"，子进程回 "pong"，父进程打印 "pong"。

<details>
<summary>点击查看答案</summary>

```c
int p1[2], p2[2];
pipe(p1); pipe(p2);

pid_t pid = fork();
if (pid == 0) {
    close(p1[1]); close(p2[0]);
    char buf[16];
    read(p1[0], buf, sizeof(buf));
    write(p2[1], "pong", 4);
    close(p1[0]); close(p2[1]);
    _exit(0);
} else {
    close(p1[0]); close(p2[1]);
    write(p1[1], "ping", 4);
    close(p1[1]);
    char buf[16];
    read(p2[0], buf, sizeof(buf));
    printf("Got: %s\n", buf);
    close(p2[0]);
    wait(NULL);
}
```
</details>

### 🔴 练习 3: 管道实现 grep 功能

用管道连接 `cat file.txt` 和 `grep pattern`（实现 `cat file.txt | grep pattern`）。

<details>
<summary>点击查看答案</summary>

```c
int pf[2];
pipe(pf);

pid_t cat_pid = fork();
if (cat_pid == 0) {
    close(pf[0]);
    dup2(pf[1], STDOUT_FILENO);  // 重定向 stdout → pipe
    close(pf[1]);
    execlp("cat", "cat", "file.txt", NULL);
    _exit(1);
}

pid_t grep_pid = fork();
if (grep_pid == 0) {
    close(pf[1]);
    dup2(pf[0], STDIN_FILENO);  // 重定向 stdin ← pipe
    close(pf[0]);
    execlp("grep", "grep", "pattern", NULL);
    _exit(1);
}

close(pf[0]); close(pf[1]);  // 父进程关闭两端
waitpid(cat_pid, NULL, 0);
waitpid(grep_pid, NULL, 0);
```
</details>

## 故障排查

### Q: read 无限阻塞

原因：1) 写端没关闭但也没写入 2) fork 后忘了关闭子进程的写端。修复：确保所有不需要的管道端都关闭。

### Q: write 返回 SIGPIPE 终止

原因：读端全部关闭，但还在写。修复：`signal(SIGPIPE, SIG_IGN)`，然后检查 write 返回 EPIPE。

### Q: pipe 和 socketpair 选哪个？

- 简单单向通信 → pipe
- 双向简单通信 → 两根 pipe 或 socketpair
- 非亲缘进程通信 → socketpair (AF_UNIX路径) 或 TCP socket

## 知识扩展

### 1. dup2 — 文件描述符重定向

```c
// 把 stdout (fd=1) 重定向到管道的写端
dup2(pipefd[1], STDOUT_FILENO);
// 之后 printf/write(1,...) 实际写入管道
```

### 2. 有名管道 (Named Pipe / FIFO)

```c
mkfifo("/tmp/myfifo", 0644);
// 其他进程可以 open("/tmp/myfifo", O_RDONLY) 连接
// 不需要 fork 亲缘关系!
```

### 3. 其他 IPC 方式

| 方式 | 特点 | 适用 |
|------|------|------|
| Pipe | 单向，亲缘进程 | 简单父子通信 |
| Socketpair | 全双工，亲缘进程 | 双向父子通信 |
| Named Pipe (FIFO) | 单向，任意进程 | 跨进程通信 |
| Shared Memory | 最快，需同步 | 大数据量 |
| Message Queue | 消息队列，内核持久 | 多对多 |

## 小结

- **pipe** 创建单向通道：一端写，一端读
- **fork 后必须关闭不需要的端**，否则死锁!
- **双向通信**用两根 pipe 或 socketpair
- **SIGPIPE** — 写已关闭的管道时终止进程
- **socketpair** 提供全双工，比两根 pipe 简单

> **我的教训**：第一次写管道时，我忘了关闭子进程的写端，父进程写完后子进程的 read 永远不返回 EOF——死锁。记住：**fork 后关闭不需要的端**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 管道（Pipe） | 单向进程间通信通道 |
| 全双工（Full-Duplex） | 双方可同时收发 |
| 有名管道（Named Pipe/FIFO） | 通过文件名访问的管道 |
| EOF | End Of File，读端全部关闭 |
| SIGPIPE | 写已关闭管道的信号 |

## 延伸阅读

- [POSIX pipe(2)](https://pubs.opengroup.org/onlinepubs/9699919799/functions/pipe.html) — 官方规范
- [Beej's Guide to Unix IPC](https://beej.us/guide/bgipc/) — IPC 完整指南
- [Advanced Programming in the UNIX Environment](https://en.wikipedia.org/wiki/Advanced_Programming_in_the_UNIX_Environment) — 第 15 章 IPC

## 继续学习

你已经掌握了进程间通信的管道路径。最后，我们将探索如何编写**用户友好的命令行工具**——参数解析、退出码、使用指南。

> 💡 **提示**：运行 `src/advance/system_ipc_sample.c` 查看所有演示。`make build && make run`。

[← 上一章：进程管理](./process.md) | [下一章：CLI 开发 →](./cli.md)
