# 多平台事件模型 (kqueue / epoll / Solaris ports / poll)

> "select/poll 教你『怎么等』，本章教你『各家怎么等得更快』——同一套故事，四种后端。"——我发现

---

## 开篇故事

上一章 [I/O 多路复用](../async/async_iomux.md) 我们用 `select`/`poll` 监控文件描述符，已经理解了「一个线程等多个 fd」的核心思想。但那里留了一个尾巴：

> select 每次调用都要把整个 fd 集合交给内核，内核还要 O(n) 扫描一遍。连接一多，就慢了。

于是各家操作系统各自造了更快的轮子：

- **macOS / FreeBSD** 说：我们有 `kqueue`
- **Linux** 说：我们有 `epoll`
- **Solaris / illumos** 说：我们有 `event ports`
- **其它 POSIX 平台**：兜底用 `poll`

问题来了——**同一份业务逻辑，难道要写四遍？**

答案是：**故事只写一遍，后端用条件编译切换**。这正是本章 demo 的结构：

```
同一套 self-pipe 故事:
  pipe() + write 1 字节 → 等待 (200ms 超时) → 读 1 字节 → 打印
  第二轮不写数据 → 故意超时 → 打印 "timeout re/arm"

按平台切换后端:
  #if   __APPLE__ / __FreeBSD__  → kqueue/kevent
  #elif __linux__                → epoll
  #elif __sun__                  → Solaris event ports
  #else                          → poll()
```

## 本章适合谁

- 已经读完 [select/poll 章节](../async/async_iomux.md)，想知道「生产级事件循环长什么样」
- 要写跨平台的网络库 / 事件循环，需要一套代码跑多个 OS
- 好奇 Nginx/libuv/Redis 在不同系统上到底调用了什么
- 面试被问过「epoll 和 kqueue 有什么区别」

## 你会学到什么

1. **kqueue/kevent**——BSD 系的通用事件接口，连定时器、信号都是事件
2. **epoll**——红黑树 + 就绪链表，O(1) 取事件
3. **Solaris event ports**——`port_create/port_associate/port_get` 三段式语义（彩蛋 API）
4. **poll 兜底**——没有后端时的可移植退路
5. **条件编译组织多平台代码**——`#if/#elif/#else` 的正确姿势

## 前置要求

- 已掌握：文件描述符、`pipe()` 的读写端
- 已读：[I/O 多路复用 (select/poll/epoll)](../async/async_iomux.md)
- 了解：`#ifdef` 条件编译（见 [条件编译章节](../../basic/conditional_comp.md)）

## 平台对照表

| 平台 | 后端 | 创建 | 注册 | 等待 | 就绪检测复杂度 |
|------|------|------|------|------|----------------|
| macOS / FreeBSD | **kqueue** | `kqueue()` | `kevent(kq, &chg, 1, ...)` | `kevent(kq, NULL, 0, &ev, 1, &ts)` | O(log n)（平衡树） |
| Linux | **epoll** | `epoll_create1(0)` | `epoll_ctl(ADD)` | `epoll_wait(...)` | O(1)（就绪链表） |
| Solaris / illumos | **event ports** | `port_create()` | `port_associate(PORT_SOURCE_FD)` | `port_get(...)` | 取走即消费，需重新 associate |
| 其它 POSIX | **poll** | 不需要 | 不需要 | `poll(&pfd, 1, ms)` | O(n)（每次全扫描） |

共同点：**都是「注册一次，反复等待」的就绪通知模型**，都不必像 `select` 那样每次重传整个 fd 集。

## 第一个例子

```c
#include <unistd.h>

#if defined(__APPLE__) || defined(__FreeBSD__)
  #include <sys/event.h>
#elif defined(__linux__)
  #include <sys/epoll.h>
#elif defined(__sun__)
  #include <poll.h>
  #include <port.h>
#else
  #include <poll.h>
#endif
```

后端头文件**按平台隔离包含**——Linux 上根本没有 `<sys/event.h>`，无条件包含会直接编译失败。

## 代码讲解

完整代码见 `src/advance/events_sample.c`。四个分支讲的是**同一个故事**，我们逐段拆解。

### 故事骨架：self-pipe 两轮（+ kqueue 定时器加轮）

四个后端都跑这两轮；kqueue 额外演示一轮 `EVFILT_TIMER`（50ms 单发定时器）——
BSD 事件模型里**定时器也是事件**，这是其它后端没有的能力，故只加在 kqueue 路径：

```c
#define EVENTS_WAIT_MS 200   /* 两轮统一 200ms — 超时路径必须能跑到 */

/* 第 1 轮: 写 1 字节 → 等待 → 就绪 → 读 1 字节 */
write(pipefd[1], &byte, 1);
wait_for_event(200ms);
read(pipefd[0], buf, 1);

/* 第 2 轮: 不写数据 → 等待 → 超时 */
wait_for_event(200ms);   /* 返回 0 / -1+ETIME → 打印 "timeout re/arm" */
```

**为什么用 self-pipe？** 它是最小的可等待 fd：不依赖网络、不阻塞、事件来源完全可控。这也是 `libevent`/`libuv` 内部唤醒事件循环的经典手法。

**为什么第二轮故意超时？** 因为「超时」是事件循环的一等公民——定时器、心跳、空闲检测全靠它。一个只会等到事件、不会等到超时的 demo 是不完整的。两轮加起来约 400ms，**绝不产生阻塞死循环**。

### kqueue 分支 (macOS / FreeBSD)

```c
int kq = kqueue();   /* 句柄叫 kq — 千万别叫 kqueue, 会遮蔽 libc 函数 */

struct kevent change;
EV_SET(&change, (uintptr_t)pipefd[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
kevent(kq, &change, 1, NULL, 0, NULL);        /* 注册: 一次调用搞定 */

struct kevent event;
struct timespec ts = {0, 200 * 1000 * 1000};  /* 200ms */
int n = kevent(kq, NULL, 0, &event, 1, &ts);  /* 等待: n=0 即超时 */
```

三个关键点：

1. **注册和等待是同一个 `kevent()` 函数**，靠参数区分：有 changelist 就注册，有 eventlist 就等待，可以一次调用同时做两件事。
2. **`EV_SET` 是构造 changelist 的宏**，字段依次是 `ident, filter, flags, fflags, data, udata`。
3. **kqueue 的野心不止 I/O**——demo 第 3 轮注册了 `EVFILT_TIMER`（50ms 单发定时器），定时器到期也是事件。信号 (`EVFILT_SIGNAL`)、进程退出 (`EVFILT_PROC`) 同理，所以一个 kqueue 就能撑起整个事件循环。

内核用**平衡树**维护 `(ident, filter)` 事件集合，插入/查找 O(log n)——这就是「evqueue 时间复杂度 O(log n)」的出处。

### epoll 分支 (Linux)

```c
int epfd = epoll_create1(0);

struct epoll_event ev;
ev.events = EPOLLIN;
ev.data.fd = pipefd[0];
epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev);   /* 挂进红黑树 */

struct epoll_event ready;
int n = epoll_wait(epfd, &ready, 1, 200);          /* n=0 即超时 */
```

内核内部是**两层结构**：

```
epoll_ctl(ADD)                 epoll_wait()
      │                             │
      ▼                             ▼
  红黑树 (全部被监控 fd)  ──就绪──▶  就绪链表 (有数据的 fd)
  插入/删除/查找 O(log n)          取链表头 O(1)
```

- fd 进出红黑树只发生在 `epoll_ctl`（注册时），**等待路径不随 fd 总数变慢**——这是 epoll 对 select 的本质超越。
- `epoll_wait` 直接把就绪链表拷给用户态，O(1) 取事件。

### Solaris ports 分支 (illumos / Oracle Solaris 彩蛋)

```c
int port = port_create();                              /* 1. 建端口 */
port_associate(port, PORT_SOURCE_FD, pipefd[0], POLLIN, NULL);  /* 2. 挂 fd */

port_event_t pev;
struct timespec ts = {0, 200 * 1000 * 1000};
int rc = port_get(port, &pev, &ts);                    /* 3. 取事件 */
if (rc < 0 && errno == ETIME) { /* 超时 */ }
```

`port_*` 三段式的语义和 kqueue/epoll 都不同，两个要点：

1. **「取走即消费」**：`port_get()` 返回后，这个 fd 的注册关系**默认被移除**。demo 第 1 轮读完数据后必须重新 `port_associate()`，否则收不到下一次通知。这与 epoll/kqueue 的「持续注册」形成鲜明对比。
2. **超时靠 errno**：`port_get()` 超时返回 `-1` 且 `errno == ETIME`（不是返回 0）——每个后端的超时约定都不一样，这正是跨平台封装的价值所在。

> 这是 illumos/Oracle Solaris 专属 API，头文件是 `<port.h>`。多数人写了一辈子 C 也没见过它——面试冷知识 +1。

### poll 分支 (兜底)

```c
struct pollfd pfd = {.fd = pipefd[0], .events = POLLIN, .revents = 0};
int n = poll(&pfd, 1, 200);        /* n=0 超时, n>0 就绪 */
```

没有注册步骤（每次调用都重新提交整个数组），内核 O(n) 扫描。胜在**到处都有**——拿不到任何现代后端时的可移植退路。

### 四个分支的超时语义对照

| 后端 | 就绪返回 | 超时返回 |
|------|----------|----------|
| kqueue | `kevent()` → 事件数 `> 0` | `kevent()` → `0` |
| epoll | `epoll_wait()` → `> 0` | `epoll_wait()` → `0` |
| ports | `port_get()` → `0` | `port_get()` → `-1`, `errno=ETIME` |
| poll | `poll()` → `> 0` | `poll()` → `0` |

## 常见错误

### ❌ 错误 1: 局部变量/函数起名叫 `kqueue`

```c
int kqueue = kqueue();   /* ❌ 'kqueue' 先被声明为 int, 后面的 kqueue() 找不到 */
```

**✅ 正确**：句柄命名避开符号名（demo 用 `kq`）：

```c
int kq = kqueue();
```

### ❌ 错误 2: 无条件包含平台头文件

```c
#include <sys/event.h>   /* ❌ Linux 上没有这个头, 直接编译失败 */
#include <sys/epoll.h>   /* ❌ macOS 上没有这个头 */
```

**✅ 正确**：头文件和实现一起按平台隔离：

```c
#if defined(__APPLE__) || defined(__FreeBSD__)
  #include <sys/event.h>
#elif defined(__linux__)
  #include <sys/epoll.h>
#endif
```

### ❌ 错误 3: 忘记超时参数 → 事件循环卡死

```c
kevent(kq, NULL, 0, &event, 1, NULL);   /* ❌ timeout=NULL = 永久阻塞 */
```

**✅ 正确**：永远给超时，让循环有机会做别的事（处理定时器、响应信号）：

```c
struct timespec ts = {0, 200 * 1000 * 1000};
kevent(kq, NULL, 0, &event, 1, &ts);    /* ✅ 200ms 内没事件就返回 0 */
```

### ❌ 错误 4: ports 取走事件后不重新 associate

```c
port_get(port, &pev, &ts);              /* 事件被取走, 注册关系也没了 */
/* ... 处理 ... */
/* ❌ 忘了重新 associate → 这个 fd 永远不会再通知你 */
```

**✅ 正确**：处理完重新挂回去：

```c
port_associate(port, PORT_SOURCE_FD, fd, POLLIN, NULL);
```

### ❌ 错误 5: 忘记关闭事件句柄和管道 fd

```c
close(kq);          /* 事件队列 fd */
close(pipefd[0]);   /* 管道读端 */
close(pipefd[1]);   /* 管道写端 */
```

demo 的每条错误路径都做了完整回收——**错误路径才是泄漏的高发区**。用 `make asan` 可以验证。

## 动手练习

### 🟢 入门：说出后端名

不看代码，写出本章四个后端分别对应的 `#if` 条件，以及各自的「创建 / 注册 / 等待」三个调用。

<details><summary>点击查看答案</summary>

| 平台 | 条件 | 创建 | 注册 | 等待 |
|------|------|------|------|------|
| macOS/FreeBSD | `__APPLE__ \|\| __FreeBSD__` | `kqueue()` | `kevent()` (changelist) | `kevent()` (eventlist) |
| Linux | `__linux__` | `epoll_create1()` | `epoll_ctl(ADD)` | `epoll_wait()` |
| Solaris | `__sun__` | `port_create()` | `port_associate()` | `port_get()` |
| 其它 | `#else` | 不需要 | 不需要 | `poll()` |

</details>

### 🟡 中级：把超时改成可配置

把 `EVENTS_WAIT_MS` 从宏改成函数参数 `events_wait_ms`，让调用方决定超时；同时把第 2 轮的「故意超时」改成「可选」（传 0 就跳过超时轮）。

<details><summary>点击查看思路</summary>

1. 删掉 `#define EVENTS_WAIT_MS`，给每个 `events_*_demo()` 加 `int wait_ms` 参数
2. 超时轮用 `if (show_timeout)` 包起来
3. 注意：kqueue 的 `timespec` 要从毫秒换算 `tv_nsec = wait_ms * 1000 * 1000`，别忘了 `tv_sec` 进位（`wait_ms >= 1000` 时）

</details>

### 🔴 挑战：写一个跨平台 `wait_readable(fd, ms)`

封装一个函数，四个平台各走一条编译路径，语义统一为：**fd 在 ms 毫秒内可读则返回 1，超时返回 0，出错返回 -1**。

<details><summary>点击查看答案提示</summary>

```c
int wait_readable(int fd, int ms) {
#if defined(__APPLE__) || defined(__FreeBSD__)
  /* kqueue: 临时建队列, 注册 EVFILT_READ, kevent 等待, 关闭 */
#elif defined(__linux__)
  /* epoll: epoll_create1 → ADD → wait → close */
#elif defined(__sun__)
  /* ports: port_create → associate → port_get (ETIME 即超时) → close */
#else
  /* poll: struct pollfd + poll() */
#endif
}
```

生产级做法（libuv 风格）是**进程级单例后端**，注册一次复用，而不是每次等待都新建——想想为什么。

</details>

## 故障排查

**Q：`kevent()` 第一轮就返回 0（超时）？**

A：检查 `write()` 是否真的写进了管道写端，以及注册的 `ident` 是不是**读端** fd。注册错端是最常见的笔误。

**Q：`port_get()` 一直返回 -1？**

A：看 `errno`。`ETIME` 是正常超时；其它错误检查 port 是否已关闭、`obj` 是否传对（`PORT_SOURCE_FD` 要传 int fd）。

**Q：为什么我的 Linux 程序在 macOS 上编译不过？**

A：大概率是无条件 `#include <sys/epoll.h>` 或直接调 `epoll_*`。所有平台专属 API 都必须关进 `#if defined(__linux__)` 里，见「常见错误 2」。

**Q：四个后端该选哪个？**

A：不用选——**用封装库**（libuv / libevent / libboost::asio），它们内部就是本章这张对照表。读本章是为了看懂它们的底层。

## 小结

- **kqueue**——BSD/macOS 通用事件接口，注册+等待合一，平衡树 O(log n)，定时器/信号也是事件
- **epoll**——Linux 专属，红黑树管注册、就绪链表管等待，取事件 O(1)
- **event ports**——Solaris/illumos 彩蛋，`port_create/associate/get` 三段式，**取走即消费**，超时返回 `ETIME`
- **poll**——O(n) 但到处都有，兜底路径
- 组织多平台代码的铁律：**头文件、实现、调用点三处 `#if` 必须同步**
- 超时是事件循环的一等公民——**永远给等待加超时，绝不无限阻塞**

## 术语表

| 英文 | 中文 |
|------|------|
| Event Backend | 事件后端 |
| kqueue / kevent | BSD 系通用事件接口 |
| EVFILT_READ / EVFILT_TIMER | kqueue 读过滤器 / 定时器过滤器 |
| epoll | Linux 事件就绪通知接口 |
| Red-Black Tree | 红黑树 (epoll 注册表) |
| Ready List | 就绪链表 (epoll 等待队列) |
| Event Ports | Solaris/illumos 事件端口 |
| port_associate / port_get | 挂载事件 / 取走事件 |
| ETIME | ports 超时错误码 |
| Self-pipe | 自管道 (可控事件源) |
| Level Triggered | 水平触发 |
| Re-arm | 重新武装 (重新注册/等待) |

## 延伸阅读

- [kqueue(2) man page](https://www.freebsd.org/cgi/man.cgi?query=kqueue&sektion=2)
- [epoll(7) man page](https://man7.org/linux/man-pages/man7/epoll.7.html)
- [port_create(3C) — Oracle Solaris](https://docs.oracle.com/cd/E88353_01/html/E37843/port-create-3c.html)
- [libuv 设计文档](https://docs.libuv.org/en/v1.x/design.html) — 生产级跨平台事件循环
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

## 继续学习

你已经看清了事件循环的底层：同一个「等待就绪」的故事，四家系统给出了四种实现。理解了这层，再读 Nginx/libuv 源码就不会被 `#ifdef` 迷宫吓住——它们和本章 demo 是同一套骨架。

- [上一章](../async/async_iomux.md)：I/O 多路复用 (select/poll/epoll)
- [下一章](../system.md)：系统编程 (file/signal/mmap/fork/IPC)
