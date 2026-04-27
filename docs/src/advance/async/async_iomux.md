# I/O 多路复用 (I/O Multiplexing — select/poll/epoll)

> "I/O 多路复用像保安盯着一排监控屏幕——哪个摄像头有动静，就派保安去哪个。不需要每个摄像头配一个保安。"——我发现

---

## 开篇故事

你开了一家客服中心，有 10 条电话线。如果每条线配一个接线员——10 个人坐在那里，大部分时间只是**等电话响**。更好的方式是：

1. **1 个接线员**负责监听所有 10 条线路
2. 系统告诉他："第 3 号线有声音了"
3. 接线员去接第 3 号线，处理完再听下一条

这就是 **I/O 多路复用 (I/O Multiplexing)**。一个线程同时监控多个文件描述符（socket、pipe、file），**有 I/O 可读/可写时才处理**，避免了「每连接一线程」的资源浪费。

```
每连接一线程:                     I/O 多路复用:
主线程──连接 A                   主线程
  ├──连接 B                       ├── 1 个 select/epoll
  ├──连接 C                       ├── 监听: [fd A, fd B, fd C...]
  └──连接 D                       └── 哪个有数据就去哪个
  (4 个线程, 4 份栈)               (1 个线程, 1 份栈)
```

## 本章适合谁

- 已经会写 socket 服务器，想知道「高并发是怎么跑的」
- 好奇 Nginx/Redis 为什么单线程也能处理上万连接
- 需要理解 `select` 和 `epoll` 的区别
- 准备面试后端开发岗位

## 你会学到什么

1. **select()**——监控多个文件描述符的可读/可写状态
2. **Pipe 多路复用**——用 pipe 模拟多路 I/O
3. **poll()**——select 的增强版（无 fd 数量限制）
4. **epoll (Linux)**——高并发利器，O(1) 检测就绪 fd
5. **跨平台**——`#ifdef __linux__` 条件编译

## 前置要求

- 已掌握：文件描述符 (fd) 的基本概念
- 已掌握：`pipe()` 创建命名/匿名管道
- 了解 socket 的基本概念

## 第一个例子

```c
#include <sys/select.h>
#include <unistd.h>

/* 监控 stdin 是否可读 */
fd_set set;
FD_ZERO(&set);
FD_SET(STDIN_FILENO, &set);

struct timeval timeout = {5, 0};  /* 5 秒 */
int ret = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);

if (ret > 0 && FD_ISSET(STDIN_FILENO, &set)) {
    char buf[64];
    read(STDIN_FILENO, buf, sizeof(buf));
    printf("你输入了: %s", buf);
} else {
    printf("超时或错误\n");
}
```

编译：`gcc -Wall -Wextra -std=c17 -o iomux demo.c`

## 原理解析

### select() 三组 fd_set

```c
int select(int nfds,
           fd_set *readfds,   /* 监控哪些 fd 可读 */
           fd_set *writefds,  /* 监控哪些 fd 可写 */
           fd_set *exceptfds, /* 监控异常 */
           struct timeval *timeout);
```

- **nfds**：监控的最大 fd + 1
- **readfds**：被监控的 fd 集合（输出参数，select 返回后只保留就绪的）
- **返回值**：就绪的 fd 数量，-1 表示错误，0 表示超时

### select 内存布局

```
select 返回前:     [fd0] [fd1] [fd2] [fd3] ...
                   设置   设置  未设置 设置

select 返回后:      [fd0] [fd1] [fd2] [fd3] ...
                    就绪   未就绪 未设置 就绪
```

调用后 `readfds` 被修改——只保留就绪的 fd。每次调用前需要**重新设置**。

### epoll 工作流程 (Linux)

```
epoll_create1()   → 创建 epoll 实例
epoll_ctl(ADD)    → 注册 fd 到 epoll
epoll_wait()      → 等待就绪事件 (高效!)
```

与 select 的区别：

| 特性 | select | epoll |
|------|--------|-------|
| fd 数量 | 有限制 (FD_SETSIZE=1024) | 无限制 |
| 性能 | O(n) 每次扫描 | O(1) 内核维护就绪列表 |
| 触发模式 | 水平触发 (LT) | 边缘触发 (ET) + LT |
| 平台 | 跨平台 | Linux 独有 |

### Linux vs macOS 平台差异

```c
#ifdef __linux__
    // 使用 epoll
    int epfd = epoll_create1(0);
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    epoll_wait(epfd, events, n, timeout);
#else
    // macOS: 使用 kqueue (BSD 系列)
    // 或退回到 select/poll
#endif
```

## 常见错误

### ❌ 错误 1: select 的 nfds 传错

```c
/* ❌ nfac=5 但实际 fd=10 */
select(5, &readfds, NULL, NULL, &timeout);
// ✅ nfds = max(fd) + 1
int maxfd = fd_a > fd_b ? fd_a : fd_b;
select(maxfd + 1, &readfds, NULL, NULL, &timeout);
```

### ❌ 错误 2: 每次 select 前忘记重置 fd_set

```c
/* ❌ select 会修改 fd_set，需要每次都重设 */
FD_SET(fd, &set);
select(..., &set, ...);
FD_SET(fd, &set);  /* ✅ 第二次 select 前需要重新设置 */
select(..., &set, ...);
```

### ❌ 错误 3: epoll 不关闭 fd

```c
int epfd = epoll_create1(0);
// ...
// ❌ 忘记 close(epfd);
// ✅ 用完后 close(epfd)
```

## 动手练习

### 🟢 入门：select 监控 stdin

用 select 监控标准输入，设置 5 秒超时。如果 5 秒内没有输入，打印 "超时"。否则读入并打印。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

int main(void) {
    fd_set set;
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    struct timeval tv = {5, 0};
    if (select(STDIN_FILENO + 1, &set, NULL, NULL, &tv) > 0) {
        char buf[64];
        read(STDIN_FILENO, buf, sizeof(buf));
        printf("输入: %s", buf);
    } else {
        printf("超时!\n");
    }
    return 0;
}
```

</details>

### 🟡 中级：pipe 多路检测

创建 3 个 pipe，只向 pipe[1] 和 pipe[2] 写数据。用 select 检测哪些 pipe 可读并打印数据。

<details><summary>点击查看答案</summary>

见本章节代码示例 `demo_pipe_multiplex()`，核心步骤：
1. 创建 N 个 pipe
2. FD_SET 所有读端
3. select 后遍历 FD_ISSET 检测就绪的 fd
4. 从就绪的 fd 读取数据

</details>

### 🔴 挑战：简单 epoll 服务器

用 epoll 创建服务端 socket，监听 127.0.0.1:8899。客户端连接后发送 "hello"，服务端回复 "world"。

<details><summary>查看答案提示</summary>

需要：socket() → bind() → listen() → epoll_ctl(ADD, listen_fd) → epoll_wait() → accept() → epoll_ctl(ADD, client_fd) → 读写数据。这是高并发服务器的标准架构。

</details>

## 故障排查

**Q：select 返回 0？**

A：超时了，没有任何 fd 就绪。检查是否真的写入了数据到管道/socket。

**Q：epoll_wait 一直返回 -1？**

A：检查 errno。常见原因是 epoll fd 已关闭、events 数组为 NULL。

**Q：select 在 macOS 上有限制 1024 个 fd？**

是的。macOS/BSD 上用 `kqueue` 替代，或者用 `poll()`（理论上无限制，但性能随 fd 数量下降）。

## 知识扩展

### poll() 对比 select()

```c
struct pollfd fds[3];
fds[0].fd = fd1; fds[0].events = POLLIN;
fds[1].fd = fd2; fds[1].events = POLLIN;
// ...
poll(fds, 3, 5000);  /* 5 秒 */
if (fds[0].revents & POLLIN) { /* fd1 可读 */ }
```

poll 不需要 `nfds` 参数，fd 集用结构体数组表示，没有 FD_SETSIZE 限制。

### Edge Triggered (ET) vs Level Triggered (LT)

| 模式 | select | epoll |
|------|--------|-------|
| 行为 | 只要 fd 有数据，每次都通知 | ET: 只在状态变化时通知一次 |
| 使用 | `while(data) read()` | ET: 必须 `while(EAGAIN)` |
| 难度 | 简单 | ET 需要仔细处理 |

## 小结

- **select**——跨平台、监控多 fd、但有 1024 限制和 O(n) 性能
- **poll**——无数量限制、但仍有 O(n) 性能
- **epoll**——Linux 独有、O(1) 性能、高并发首选
- **kqueue**——macOS/BSD 的 epoll 替代方案
- 核心思路：**一个线程监控所有 fd，有 I/O 才处理**

## 术语表

| 英文 | 中文 |
|------|------|
| I/O Multiplexing | I/O 多路复用 |
| File Descriptor (fd) | 文件描述符 |
| select() | 监控 fd 集合 (跨平台) |
| poll() | select 增强版 (无 fd 上限) |
| epoll | Linux I/O 多路复用 (高性能) |
| kqueue | macOS/BSD I/O 多路复用 |
| Edge Triggered (ET) | 边缘触发 |
| Level Triggered (LT) | 级别触发 |
| nfds | select 的最大 fd+1 |
| fd_set | select 的 fd 集合 |

## 延伸阅读

- [select(2) man page](https://man7.org/linux/man-pages/man2/select.2.html)
- [epoll(7) man page](https://man7.org/linux/man-pages/man7/epoll.7.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

## 继续学习

你已经理解了 I/O 多路复用——用少量线程处理大量连接。现在你已经具备了高并发编程的核心知识：线程管理、同步原语、线程池、I/O 多路复用。把它们组合起来，你就能写出高性能的网络服务。

- [上一章](./async_pool.md)：线程池模式
- [下一章](../iterators.md)：数据结构遍历
