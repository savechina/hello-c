# 并发服务器模型 (Concurrent Server Models)

> "并发服务器像餐厅服务模式——fork 是每位客人配专属服务员，thread 是每位客人配服务员但共享厨房，I/O 复用是一位服务员服务所有客人但手脚麻利。我花了一周理解这三种模式后发现，它们的核心矛盾只有一个：如何同时服务多个客人，还能让服务员活得轻松。"

## 1. 开篇故事

想象你开了一家餐厅。客人来了，你怎么服务？

**方案 A — Fork（每位客人配专属服务员）：** 每进来一位客人，就雇佣一个新服务员，专属服务这位客人。服务完服务员离职。好处：服务员之间完全隔离，一个服务员晕倒了不影响其他客人。坏处：太烧钱了——每位客人都要多一个服务员（进程内存 + 创建成本）。

**方案 B — Thread（每位客人配服务员，共享厨房）：** 同样是每位客人配服务员，但服务员们共享一个厨房（内存）。好处：比 fork 省资源（不需要复制厨房）。坏处：如果两个服务员去同一个冰箱拿东西，需要协调（mutex），不然会打架（data race）。而且一个服务员把厨房烧了，整个店都没了。

**方案 C — I/O 复用（一位服务员服务所有客人）：** 只有一位服务员，但手脚极其麻利——他同时记下所有客人的需求，谁需要上菜就去上，谁要点单就记录。好处：省人（单线程管理数百连接）。坏处：服务员不能在任何一件事上"卡住"（必须非阻塞），而且大脑要维护所有状态（编程复杂度高）。

本章我们将逐一实现并对比这三种模型。

## 2. 本章适合谁

- 学过了第一章 Socket，想知道"服务器怎么同时处理多个请求"的人
- 在 Python/Go 里用过 asyncio/goroutine，想理解底层 C 实现的人
- 好奇 Nginx 为什么用一个进程能扛 10000+ 并发的人
- 想了解 fork 和 pthread 在 Web 服务器中怎么应用的人

## 3. 你会学到什么

- Fork 并发模型：`fork()` + `SIGCHLD` 信号 + 父子进程 fd 分工
- Thread 并发模型：`pthread_create()` + 线程参数传递 + 共享内存
- I/O 多路复用：`select()` + `fd_set` + 非阻塞 I/O
- 三种模型的优缺点对比
- 资源隔离 vs 资源共享的权衡
- 错误处理：fork 失败、pthread 失败、select 限制

## 4. 前置要求

- 理解第一章的内容（Socket 创建、HTTP 请求/响应）
- 了解 fork/wait 基本概念
- 了解 pthread 基础（线程创建、参数传递）
- 理解文件描述符在 fork/线程间的共享行为

## 5. 第一个例子

```c
/* Fork 模型 — 每个连接一个子进程 */
pid_t pid = fork();

if (pid < 0) {
    // fork 失败
    close(client_fd);
} else if (pid == 0) {
    // 子进程: 关闭监听 fd, 处理客户端
    close(server_fd);
    handle_client(client_fd);
    _exit(0);
} else {
    // 父进程: 关闭客户端 fd, 继续 accept
    close(client_fd);
}
```

三步走：**fork → 子进程处理 → 父进程继续**。

## 6. 原理解析

### 6.1 Fork 模型 — 每位客人专属服务员

```
┌─ 父进程 (accept 循环)
│
├─── fork() → 子进程 A ──→ handle_client(客户1) → _exit(0)
│
├─── fork() → 子进程 B ──→ handle_client(客户2) → _exit(0)
│
└─── fork() → 子进程 C ──→ handle_client(客户3) → _exit(0)
```

完整结构：

```c
signal(SIGCHLD, SIG_IGN);  /* 自动回收子进程，避免僵尸 */
listen(server_fd, 128);
for (;;) {
    int client_fd = accept(server_fd, ...);
    pid_t pid = fork();
    if (pid < 0) {
        close(client_fd);  /* fork 失败 */
    } else if (pid == 0) {
        close(server_fd);      /* 子进程不要监听 fd */
        handle_client(client_fd);
        _exit(0);              /* ⚠️ 不是 exit()! */
    } else {
        close(client_fd);      /* 父进程不要客户端 fd */
    }
}
```

**为什么子进程用 `_exit()` 而不是 `exit()`？** `exit()` 会刷新 stdio 缓冲区（包括父进程缓冲区中尚未写入的内容），导致父进程的输出被子进程重复打印。`_exit()` 是系统调用，立即退出，不动任何缓冲区。

**父子 fd 分工规则**：
- 子进程：`close(server_fd)` — 不需要接受新连接
- 父进程：`close(client_fd)` — 不需要处理这个客户端

### 6.2 Thread 模型 — 共享厨房的服务员

```c
// 主线程
for (;;) {
    int client_fd = accept(server_fd, ...);
    int *arg = malloc(sizeof(int));  /* ← 关键！ */
    *arg = client_fd;
    pthread_create(&tid, NULL, thread_handle_client, arg);
    /* 主线程 detached — 不 join */
}

// 线程函数
void *thread_handle_client(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);
    handle_client(client_fd);
    close(client_fd);
    return NULL;
}
```

**为什么 malloc 传 fd？** 如果直接传 `&client_fd`，所有线程都读同一个变量——当 `accept` 产生新的 `client_fd` 值时，之前等待中的线程会读到错误值。每个线程必须有自己的 fd 拷贝。

**线程安全的代价**：

| 场景 | 问题 | 解决方案 |
|------|------|----------|
| 多个线程写同一个全局计数器 | data race | `pthread_mutex_lock` 或 `__atomic_add_fetch` |
| 多个线程读共享缓存 | 可能读到一半更新 | `pthread_rwlock_rdlock` |
| 一个线程崩溃（段错误） | 杀死整个进程 | 无法完全避免 |

### 6.3 I/O 多路复用 — 一位麻利服务员

```
select() 原理:
  ┌──────────────────────────────────┐
  │  fd_set read_set                 │
  │  ├─ server_fd                    │
  │  ├─ client_fd_1                  │
  │  ├─ client_fd_2                  │
  │  └─ client_fd_3                  │
  │                                  │
  │  select() 阻塞等待:              │
  │  "谁有数据可读？告诉我。"         │
  │                                  │
  │  返回: client_fd_2 有数据!       │
  │  → recv(client_fd_2)             │
  │  → 处理 → send() → 回到 select() │
  └──────────────────────────────────┘
```

```c
fd_set read_set;
int maxfd = server_fd;

for (;;) {
    FD_ZERO(&read_set);
    FD_SET(server_fd, &read_set);
    /* FD_SET(client, &read_set);  — 每个活跃客户端 */

    int active = select(maxfd + 1, &read_set, NULL, NULL, NULL);

    if (FD_ISSET(server_fd, &read_set)) {
        int client = accept(server_fd, ...);
        /* 把 client 加入 fd 数组 */
        if (client > maxfd) maxfd = client;
    }

    /* 遍历所有客户端 fd，检查 FD_ISSET(client, &read_set) */
}
```

**为什么需要非阻塞 I/O？** 如果 `accept()` 或 `recv()` 是阻塞模式，在 `fd_set` 中标记了该 fd 可读才调用 select 是对的。但如果逻辑有瑕疵（比如 `accept` 在没有人连接时被调用），就会阻塞整个 event loop。所以 server socket 和所有 client socket 都建议设 `O_NONBLOCK`。

**select 的局限**：
- `FD_SETSIZE` 通常 = 1024 — 最多 1024 个并发连接
- 每次 `select` 前必须 `FD_ZERO` + 逐个 `FD_SET` — O(n) 重建
- `select` 返回后不知道是哪个 fd 触发了，必须 `FD_ISSET` 遍历 O(n)

**现代替代**：
- Linux → `epoll`：O(1) 事件通知，无需重建 fd_set
- macOS → `kqueue`：同样 O(1)，功能更强
- Windows → `IOCP`：重叠 I/O 完成端口

### 6.4 三种模型对比

| 维度 | Fork | Thread | I/O 复用 (select) |
|------|------|--------|-------------------|
| 并发模型 | 多进程 | 多线程 | 单线程 |
| 每个连接开销 | ~几 MB（进程内存） | ~几 MB（线程栈） | ~几 KB（fd 记录） |
| 创建成本 | 高（复制页表+地址空间） | 中（分配线程栈） | 无 |
| 隔离性 | ✅ 进程隔离 | ❌ 共享内存 | ❌ 单线程 |
| 一个崩溃 | ✅ 不影响其他 | ❌ 全完 | ❌ 全完 |
| 数据共享 | ❌ 需要 IPC（pipe/shm） | ✅ 直接共享 | ✅ 直接共享 |
| 编程复杂度 | ⭐⭐ 简单 | ⭐⭐⭐ 中等（需 mutex） | ⭐⭐⭐⭐ 复杂（状态机） |
| 最大并发数 | 受限于进程数 | 受限于内存 | 受限于 FD_SETSIZE |
| 代表项目 | Apache (prefork) | Apache (worker/nginx threads) | Nginx/Haproxy |

## 7. 常见错误

### ❌ 错误 1: 子进程忘记 close 监听 fd

```c
// ❌ 子进程持有 server_fd — 端口无法释放
pid_t pid = fork();
if (pid == 0) {
    handle_client(client_fd);
    _exit(0);  // server_fd 未关闭！
}

// ✅
if (pid == 0) {
    close(server_fd);  // 子进程不需要监听
    handle_client(client_fd);
    _exit(0);
}
```

### ❌ 错误 2: 线程传 &client_fd

```c
// ❌ 所有线程读同一个变量
int client_fd = accept(...);
pthread_create(&tid, NULL, handler, &client_fd);

// handler 中: *(int *)arg 可能被下一个 accept 覆盖！

// ✅ malloc 独立拷贝
int *fdp = malloc(sizeof(int));
*fdp = client_fd;
pthread_create(&tid, NULL, handler, fdp);
```

### ❌ 错误 3: select 忘记更新 maxfd

```c
// ❌ maxfd 一直是 server_fd → 新 client fd 不被 select 检测
// 新客户端的数据到达，但 select 不知道

// ✅
if (client_fd > maxfd) {
    maxfd = client_fd;
}
```

### ❌ 错误 4: 父进程忘记 close 客户端 fd

```c
// ❌ 父进程 hold 住 client_fd → fd 泄漏
pid_t pid = fork();
if (pid > 0) {
    // 忘记 close(client_fd)!
}

// ✅
if (pid > 0) {
    close(client_fd);  // 父进程不需要客户端 fd
}
```

### ❌ 错误 5: 子进程用 exit() 而不是 _exit()

```c
// ❌ exit() 刷新父进程的 stdio 缓冲区
if (pid == 0) {
    handle_client();
    exit(0);  // → 父进程的 printf 可能被打印两次
}

// ✅
    _exit(0);  // 立即退出，不动缓冲区
```

### ❌ 错误 6: 不处理 SIGCHLD → 僵尸进程

```c
// ❌ 子进程退出后变成僵尸 (Z+), 占用进程表项

// ✅
signal(SIGCHLD, SIG_IGN);  // 自动回收

// 或者在信号处理函数中 waitpid
```

## 8. 动手练习

### 🟢 练习 1: Fork 最小实现

写一个 fork 演示：父进程 fork 子进程，子进程打印 "hello from child"，父进程打印 "hello from parent + child PID"。

<details>
<summary>点击查看答案</summary>

```c
pid_t pid = fork();
if (pid < 0) {
    perror("fork");
} else if (pid == 0) {
    printf("hello from child (PID=%d)\n", getpid());
    _exit(0);
} else {
    printf("hello from parent (child PID=%d)\n", (int)pid);
    int status;
    waitpid(pid, &status, 0);
}
```
</details>

### 🟡 练习 2: 线程传递参数

创建 3 个线程，每个线程接收一个独立整数参数（1, 2, 3），打印出来。用 `malloc` 传参。

<details>
<summary>点击查看答案</summary>

```c
void *printer(void *arg) {
    int val = *(int *)arg;
    free(arg);
    printf("Thread received: %d\n", val);
    return NULL;
}

pthread_t tids[3];
for (int i = 1; i <= 3; i++) {
    int *p = malloc(sizeof(int));
    *p = i;
    pthread_create(&tids[i - 1], NULL, printer, p);
}
for (int i = 0; i < 3; i++) {
    pthread_join(tids[i], NULL);
}
```
</details>

### 🔴 练习 3: select 双 fd 监视

用 pipe 创建两个 fd，用 select 同时监视它们。父进程写 pipe1，子进程写 pipe2，select 谁先有数据就读谁。

<details>
<summary>点击查看答案</summary>

```c
int p1[2], p2[2];
pipe(p1);
pipe(p2);

pid_t child = fork();
if (child == 0) {
    close(p1[0]); close(p2[1]);
    sleep(1);
    write(p2[1], "child", 5);
    _exit(0);
}
close(p1[1]); close(p2[0]);

fd_set fds;
FD_ZERO(&fds);
FD_SET(p1[0], &fds);
FD_SET(p2[0], &fds);
int maxfd = p2[0];  /* assuming p2[0] > p1[0] */

select(maxfd + 1, &fds, NULL, NULL, NULL);

if (FD_ISSET(p1[0], &fds)) {
    // pipe1 有数据（父→子方向，本例无）
}
if (FD_ISSET(p2[0], &fds)) {
    char buf[16];
    read(p2[0], buf, sizeof(buf));
    printf("Received: %s\n", buf);
}
```
</details>

## 9. 故障排查

### Q: fork() 返回 -1, errno = EAGAIN

**原因**：系统进程数超限。`ulimit -u` 查看限制，或 `ps aux | wc -l` 看当前进程数。

### Q: pthread_create 返回 EAGAIN

**原因**：线程数超限或内存不足（每个线程默认栈 ~8MB）。

**解决**：创建线程属性，减小栈大小：
```c
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setstacksize(&attr, 256 * 1024);  // 256KB 栈
pthread_create(&tid, &attr, handler, arg);
pthread_attr_destroy(&attr);
```

### Q: select() 返回 -1, errno = EBADF

**原因**：fd_set 中包含已关闭的 fd。检查 `FD_SET` 的 fd 是否还有效。

### Q: 僵尸进程堆积

子进程退出但父进程不 `wait()`。
- 方案 1: `signal(SIGCHLD, SIG_IGN)`
- 方案 2: `waitpid(-1, NULL, WNOHANG)` 在父进程中定期回收

## 10. 知识扩展

### 1. 进程 vs 线程 vs 协程

| 维度 | 进程 | 线程 | 协程 |
|------|------|------|------|
| 内存隔离 | ✅ 完全隔离 | ❌ 共享 | ❌ 共享 |
| 切换开销 | 高（内核态） | 中（内核态） | 低（用户态） |
| 崩溃影响 | 仅自身 | 整个进程 | 整个进程 |
| 并发方式 | 多核并行 | 多核并行 | 单核时分 |

### 2. epoll / kqueue vs select

`select` 是"每次都重新告诉所有人名单"，`epoll` 是"提前注册好名单，有人的时候叫名字"。

```c
// epoll: 先注册
epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &event);
// 然后等事件
int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
// 直接遍历触发的 events — 不需要 O(n) 遍历！
```

### 3. 线程池 vs 每连接一线程

每连接一线程在高并发时创建/销毁频繁。线程池是"预先创建好 N 个服务员，客人来了分配给空闲的服务员"。

### 4. 为什么 Nginx 用 I/O 复用

Nginx 单个 worker 用 `epoll`（Linux）或 `kqueue`（macOS），一个进程处理 10000+ 连接。因为 Web 服务器主要是 I/O 密集（读请求、写响应），不是 CPU 密集。

## 11. 小结

- **Fork 模型**：每个连接一个进程，隔离好但贵，Apache prefork 经典
- **Thread 模型**：每个连接一个线程，轻量但需要同步，Apache worker
- **I/O 复用**：单线程管理所有连接，高效但代码复杂，Nginx 核心
- **子进程用 `_exit()`**，**父进程 close(client_fd)**，**子进程 close(server_fd)**
- **线程传参用 `malloc`** 独立拷贝，直接传 `&variable` 会数据竞争
- **select 的 maxfd 必须更新**，`FD_SETSIZE` 是硬限制
- **`signal(SIGCHLD, SIG_IGN)`** 防止僵尸进程

> **我的教训是**：第一次写 fork 服务器时，父进程忘记 `close(client_fd)` 了——导致每个 accept 都泄漏一个 fd。跑了几天后 `accept` 返回 -1，`errno = EMFILE`（进程 fd 已满）。后来加了 `close(client_fd)` 就好了。记住：fork 后，父子进程各自关闭自己不需要的 fd——这是一条铁律。

## 12. 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 分叉（Fork） | 创建子进程，父进程地址空间的副本 |
| 僵尸进程（Zombie） | 子进程已退出但父进程未 wait |
| 线程（Thread） | 进程内的轻量执行单元，共享内存 |
| pthread_create | 创建新线程 |
| 数据竞争（Data Race） | 多线程同时访问共享数据无同步 |
| 互斥锁（Mutex） | 保护共享数据的同步原语 |
| I/O 多路复用（I/O Multiplexing） | 单线程监视多个 fd |
| select / poll / epoll | I/O 复用的三种 API |
| FD_SETSIZE | select 最大监视 fd 数（通常 1024） |
| SIGCHLD | 子进程退出时父进程收到的信号 |
| 非阻塞 I/O（Non-blocking I/O） | 不会阻塞调用的 I/O 操作 |

## 13. 延伸阅读

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — select/epoll 详解
- [Linux epoll 手册](https://man7.org/linux/man-pages/man7/epoll.7.html) — 高性能 I/O 复用
- [《UNIX环境高级编程》](https://zh.wikipedia.org/wiki/UNIX环境高级编程)第 5 章— fork 细节
- [Nginx 架构揭秘](https://www.nginx.com/blog/inside-nginx-how-we-designed-for-performance-scale/) — 官方性能设计文章

## 14. 源码参考

完整源代码: `src/advance/web_concurrent_sample.c`

- `demo_fork_server()` — fork 并发模型，带 `#ifdef DEMO_ACTUAL_SERVER` 保护
- `demo_thread_per_connection()` — pthread 每连接一线程模型
- `demo_iomux_server()` — select I/O 多路复用模型

源码默认只展示结构（打印伪代码），不会占用真实端口。要编译为真实服务器：`gcc -DDEMO_ACTUAL_SERVER ...`。

## 15. 继续学习

你理解了三种并发模型的精髓—— fork 的隔离、thread 的共享、I/O 复用的麻利。这是网络编程从"能跑"到"能扛"的分水岭。

现在你已经掌握了：
- Socket 创建 and HTTP 协议（第一章）
- 并发服务器模型（本章）

你可以把它们组合起来——用 I/O 复用构建一个能同时服务 1000 个请求的 HTTP 服务器。这就是 Nginx 做的事，你现在理解它了。

[← 上一章: Socket 与 HTTP 协议](./socket.md)
