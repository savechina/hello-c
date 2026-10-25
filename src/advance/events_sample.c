/**
 * @file events_sample.c
 * @brief 多平台事件模型 (kqueue / epoll / Solaris ports / poll)
 *
 * 同一套 self-pipe 故事, 按编译平台切换事件后端:
 *   1. kqueue/kevent — macOS / FreeBSD
 *   2. epoll         — Linux
 *   3. event ports   — illumos / Oracle Solaris
 *   4. poll()        — 其它 POSIX 平台兜底
 *
 * 每个后端两轮: 第一轮写 1 字节 → 200ms 内就绪 → 读 1 字节;
 * 第二轮不写数据 → 故意超时, 打印 "timeout re/arm"。全程无阻塞死循环。
 * (kqueue 额外追加一轮 EVFILT_TIMER, 演示定时器也是事件。)
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#if defined(__APPLE__) || defined(__FreeBSD__)
  #include <sys/event.h>
  #include <sys/time.h>
#elif defined(__linux__)
  #include <sys/epoll.h>
#elif defined(__sun__)
  #include <poll.h>
  #include <port.h>
#else
  #include <poll.h>
#endif

#include "advance/events_sample.h"

/* 两轮等待统一用 200ms — 超时路径必须能跑到, 且绝不无限阻塞 */
#define EVENTS_WAIT_MS 200

/* ============================================================
 * Demo 1: kqueue/kevent — macOS / FreeBSD
 * ============================================================ */

#if defined(__APPLE__) || defined(__FreeBSD__)

static void events_kqueue_demo(void) {
  printf("--- 1. kqueue/kevent (macOS / FreeBSD) ---\n");
  printf("  后端名: kqueue — BSD 系列的通用事件接口\n");
  printf("  原理: kevent() 把「注册」和「等待」合并成一次系统调用,\n");
  printf("        内核用平衡树维护 (ident, filter) 事件集合,\n");
  printf("        插入/查找 O(log n), 且不必像 select 每次重传整个 fd 集\n\n");

  int pipefd[2];
  if (pipe(pipefd) != 0) {
    perror("  pipe 创建失败");
    return;
  }

  /* kqueue 句柄刻意命名为 kq — 不能叫 kqueue, 否则遮蔽 libc 的 kqueue() */
  int kq = kqueue();
  if (kq < 0) {
    perror("  kqueue() 创建失败");
    close(pipefd[0]);
    close(pipefd[1]);
    return;
  }

  /* 注册 pipe 读端的可读事件: EV_ADD 加入集合, EV_ENABLE 启用它 */
  struct kevent change;
  EV_SET(&change, (uintptr_t)pipefd[0], EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
         NULL);
  if (kevent(kq, &change, 1, NULL, 0, NULL) < 0) {
    perror("  kevent 注册 EVFILT_READ 失败");
    close(kq);
    close(pipefd[0]);
    close(pipefd[1]);
    return;
  }

  /* ── 第 1 轮: self-pipe 写 1 字节 → 事件就绪 ── */
  char byte = 'K';
  if (write(pipefd[1], &byte, 1) != 1) {
    perror("  write self-pipe 失败");
    close(kq);
    close(pipefd[0]);
    close(pipefd[1]);
    return;
  }

  struct kevent event;
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = EVENTS_WAIT_MS * 1000 * 1000;

  int n = kevent(kq, NULL, 0, &event, 1, &ts);
  if (n > 0 && event.filter == EVFILT_READ) {
    char buf[8] = {0};
    ssize_t r = read(pipefd[0], buf, sizeof(buf) - 1);
    if (r > 0) {
      printf("  [round 1] EVFILT_READ 就绪 → 读到 %zd 字节: '%c'\n", r, buf[0]);
    } else {
      printf("  [round 1] EVFILT_READ 就绪但读取失败 (r=%zd)\n", r);
    }
  } else if (n == 0) {
    printf("  [round 1] %dms 超时 (意外: 数据本应已就绪)\n", EVENTS_WAIT_MS);
  } else {
    perror("  kevent 等待失败");
  }

  /* ── 第 2 轮: 不写数据 → 故意超时, 展示 timeout 语义 ── */
  ts.tv_nsec = EVENTS_WAIT_MS * 1000 * 1000;
  n = kevent(kq, NULL, 0, &event, 1, &ts);
  if (n == 0) {
    printf("  [round 2] timeout re/arm — %dms 内无事件, kevent 返回 0\n",
           EVENTS_WAIT_MS);
  } else if (n > 0) {
    printf("  [round 2] 意外收到事件 filter=%d\n", (int)event.filter);
  } else {
    perror("  kevent 等待失败");
  }

  /* ── 第 3 轮: EVFILT_TIMER — kqueue 连定时器也是事件 ── */
  EV_SET(&change, 1, EVFILT_TIMER, EV_ADD | EV_ENABLE | EV_ONESHOT, 0, 50,
         NULL);
  if (kevent(kq, &change, 1, NULL, 0, NULL) < 0) {
    perror("  kevent 注册 EVFILT_TIMER 失败");
  } else {
    ts.tv_nsec = 500 * 1000 * 1000; /* 最多等 500ms, 定时器 50ms 触发 */
    n = kevent(kq, NULL, 0, &event, 1, &ts);
    if (n > 0 && event.filter == EVFILT_TIMER) {
      printf("  [round 3] EVFILT_TIMER 触发 (50ms 单发定时器)\n");
    } else {
      printf("  [round 3] 定时器未触发 (n=%d)\n", n);
    }
  }

  close(kq);
  close(pipefd[0]);
  close(pipefd[1]);
  printf("  ✅ kqueue 演示完毕 (fd 全部回收, 无泄漏)。\n\n");
}

/* ============================================================
 * Demo 1: epoll — Linux
 * ============================================================ */

#elif defined(__linux__)

static void events_epoll_demo(void) {
  printf("--- 1. epoll (Linux) ---\n");
  printf("  后端名: epoll — Linux 专属的就绪通知接口\n");
  printf("  原理: epoll_ctl 把 fd 挂进内核红黑树 (插入/删除 O(log n)),\n");
  printf("        真正就绪的 fd 被挂入就绪链表,\n");
  printf("        epoll_wait 只取链表头部 → 取事件 O(1), 不随 fd 总数增长\n\n");

  int epfd = epoll_create1(0);
  if (epfd < 0) {
    perror("  epoll_create1 失败");
    return;
  }

  int pipefd[2];
  if (pipe(pipefd) != 0) {
    perror("  pipe 创建失败");
    close(epfd);
    return;
  }

  struct epoll_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.events = EPOLLIN;
  ev.data.fd = pipefd[0];
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev) != 0) {
    perror("  epoll_ctl ADD 失败");
    close(pipefd[0]);
    close(pipefd[1]);
    close(epfd);
    return;
  }

  /* ── 第 1 轮: self-pipe 写 1 字节 → 事件就绪 ── */
  char byte = 'E';
  if (write(pipefd[1], &byte, 1) != 1) {
    perror("  write self-pipe 失败");
    close(pipefd[0]);
    close(pipefd[1]);
    close(epfd);
    return;
  }

  struct epoll_event ready;
  int n = epoll_wait(epfd, &ready, 1, EVENTS_WAIT_MS);
  if (n > 0 && (ready.events & EPOLLIN) != 0) {
    char buf[8] = {0};
    ssize_t r = read(pipefd[0], buf, sizeof(buf) - 1);
    if (r > 0) {
      printf("  [round 1] EPOLLIN 就绪 → 读到 %zd 字节: '%c'\n", r, buf[0]);
    } else {
      printf("  [round 1] EPOLLIN 就绪但读取失败 (r=%zd)\n", r);
    }
  } else if (n == 0) {
    printf("  [round 1] %dms 超时 (意外: 数据本应已就绪)\n", EVENTS_WAIT_MS);
  } else {
    perror("  epoll_wait 失败");
  }

  /* ── 第 2 轮: 不写数据 → 故意超时, 展示 timeout 语义 ── */
  n = epoll_wait(epfd, &ready, 1, EVENTS_WAIT_MS);
  if (n == 0) {
    printf("  [round 2] timeout re/arm — epoll_wait 返回 0\n");
  } else if (n > 0) {
    printf("  [round 2] 意外收到事件 events=0x%x\n", ready.events);
  } else {
    perror("  epoll_wait 失败");
  }

  close(epfd);
  close(pipefd[0]);
  close(pipefd[1]);
  printf("  ✅ epoll 演示完毕 (fd 全部回收, 无泄漏)。\n\n");
}

/* ============================================================
 * Demo 1: event ports — illumos / Oracle Solaris (彩蛋 API)
 * ============================================================ */

#elif defined(__sun__)

static void events_ports_demo(void) {
  printf("--- 1. Solaris event ports (illumos / Oracle Solaris) ---\n");
  printf("  后端名: event ports — Solaris/illumos 专属接口\n");
  printf("  原理: port_create() 建一个「事件端口」,\n");
  printf("        port_associate(PORT_SOURCE_FD) 把 fd 挂到端口上,\n");
  printf("        port_get() 取回一个就绪事件 (超时返回 -1, errno=ETIME),\n");
  printf("        语义是「取走即消费」— 水平触发下要重新 associate\n\n");

  int port = port_create();
  if (port < 0) {
    perror("  port_create 失败");
    return;
  }

  int pipefd[2];
  if (pipe(pipefd) != 0) {
    perror("  pipe 创建失败");
    close(port);
    return;
  }

  if (port_associate(port, PORT_SOURCE_FD, pipefd[0], POLLIN, NULL) != 0) {
    perror("  port_associate 失败");
    close(pipefd[0]);
    close(pipefd[1]);
    close(port);
    return;
  }

  /* ── 第 1 轮: self-pipe 写 1 字节 → 事件就绪 ── */
  char byte = 'P';
  if (write(pipefd[1], &byte, 1) != 1) {
    perror("  write self-pipe 失败");
    close(pipefd[0]);
    close(pipefd[1]);
    close(port);
    return;
  }

  port_event_t pev;
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = EVENTS_WAIT_MS * 1000 * 1000;

  int rc = port_get(port, &pev, &ts);
  if (rc == 0 && pev.portev_source == PORT_SOURCE_FD) {
    char buf[8] = {0};
    ssize_t r = read(pipefd[0], buf, sizeof(buf) - 1);
    if (r > 0) {
      printf("  [round 1] PORT_SOURCE_FD 就绪 (events=0x%x) → 读到 %zd 字节: "
             "'%c'\n",
             pev.portev_events, r, buf[0]);
    } else {
      printf("  [round 1] PORT_SOURCE_FD 就绪但读取失败 (r=%zd)\n", r);
    }
    /* 取走即消费: 水平触发要重新 associate 才能收到下一次通知 */
    if (port_associate(port, PORT_SOURCE_FD, pipefd[0], POLLIN, NULL) != 0) {
      perror("  重新 port_associate 失败");
    }
  } else if (rc < 0 && errno == ETIME) {
    printf("  [round 1] %dms 超时 (意外: 数据本应已就绪)\n", EVENTS_WAIT_MS);
  } else {
    perror("  port_get 失败");
  }

  /* ── 第 2 轮: 不写数据 → 故意超时, 展示 timeout 语义 ── */
  ts.tv_nsec = EVENTS_WAIT_MS * 1000 * 1000;
  rc = port_get(port, &pev, &ts);
  if (rc < 0 && errno == ETIME) {
    printf("  [round 2] timeout re/arm — port_get 返回 -1, errno=ETIME\n");
  } else if (rc == 0) {
    printf("  [round 2] 意外收到事件 source=%d\n", pev.portev_source);
  } else {
    perror("  port_get 失败");
  }

  close(pipefd[0]);
  close(pipefd[1]);
  close(port);
  printf("  ✅ event ports 演示完毕 (fd 全部回收, 无泄漏)。\n\n");
}

/* ============================================================
 * Demo 1: poll() — 其它 POSIX 平台兜底
 * ============================================================ */

#else

static void events_poll_demo(void) {
  printf("--- 1. poll() 兜底 (其它 POSIX 平台) ---\n");
  printf("  后端名: poll — select 的增强版, 无 FD_SETSIZE 上限\n");
  printf("  原理: 内核每次调用都要遍历整个 pollfd 数组 → O(n),\n");
  printf("        但接口统一、可移植, 适合做跨平台的兜底路径\n\n");

  int pipefd[2];
  if (pipe(pipefd) != 0) {
    perror("  pipe 创建失败");
    return;
  }

  struct pollfd pfd;
  pfd.fd = pipefd[0];
  pfd.events = POLLIN;
  pfd.revents = 0;

  /* ── 第 1 轮: self-pipe 写 1 字节 → 事件就绪 ── */
  char byte = 'L';
  if (write(pipefd[1], &byte, 1) != 1) {
    perror("  write self-pipe 失败");
    close(pipefd[0]);
    close(pipefd[1]);
    return;
  }

  int n = poll(&pfd, 1, EVENTS_WAIT_MS);
  if (n > 0 && (pfd.revents & POLLIN) != 0) {
    char buf[8] = {0};
    ssize_t r = read(pipefd[0], buf, sizeof(buf) - 1);
    if (r > 0) {
      printf("  [round 1] POLLIN 就绪 → 读到 %zd 字节: '%c'\n", r, buf[0]);
    } else {
      printf("  [round 1] POLLIN 就绪但读取失败 (r=%zd)\n", r);
    }
  } else if (n == 0) {
    printf("  [round 1] %dms 超时 (意外: 数据本应已就绪)\n", EVENTS_WAIT_MS);
  } else {
    perror("  poll 失败");
  }

  /* ── 第 2 轮: 不写数据 → 故意超时, 展示 timeout 语义 ── */
  pfd.revents = 0;
  n = poll(&pfd, 1, EVENTS_WAIT_MS);
  if (n == 0) {
    printf("  [round 2] timeout re/arm — poll 返回 0\n");
  } else if (n > 0) {
    printf("  [round 2] 意外收到事件 revents=0x%x\n", pfd.revents);
  } else {
    perror("  poll 失败");
  }

  close(pipefd[0]);
  close(pipefd[1]);
  printf("  ✅ poll 演示完毕 (fd 全部回收, 无泄漏)。\n\n");
}

#endif

/* ============================================================
 * Coordinator
 * ============================================================ */

int main_events_sample(void) {
  printf("========================================\n");
  printf("  多平台事件模型 (kqueue · epoll · ports · poll)\n");
  printf("========================================\n\n");

  printf("  同一套 self-pipe 故事, 按编译平台切换事件后端:\n");
  printf("    #if   __APPLE__ / __FreeBSD__ → kqueue/kevent\n");
  printf("    #elif __linux__               → epoll\n");
  printf("    #elif __sun__                 → Solaris event ports\n");
  printf("    #else                         → poll() 兜底\n\n");

#if defined(__APPLE__) || defined(__FreeBSD__)
  events_kqueue_demo();
#elif defined(__linux__)
  events_epoll_demo();
#elif defined(__sun__)
  events_ports_demo();
#else
  events_poll_demo();
#endif

  printf("多平台事件模型演示完毕。\n");
  return 0;
}
