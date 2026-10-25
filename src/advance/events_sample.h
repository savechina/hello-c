#ifndef EVENTS_SAMPLE_H
#define EVENTS_SAMPLE_H

/**
 * @brief 多平台事件模型 (kqueue / epoll / Solaris ports / poll)
 *
 * 同一套 self-pipe 故事, 按编译平台切换事件后端:
 *   1. kqueue/kevent — macOS / FreeBSD   (#if __APPLE__ || __FreeBSD__)
 *   2. epoll         — Linux             (#elif __linux__)
 *   3. event ports   — illumos/Solaris   (#elif __sun__)
 *   4. poll()        — 其它 POSIX 兜底    (#else)
 *
 * 每个后端跑两轮: 第一轮写 1 字节 → 200ms 内就绪 → 读 1 字节;
 * 第二轮不写数据 → 故意超时, 打印 "timeout re/arm"。全程无阻塞死循环。
 *
 * Called from main_advance() → advance.c → hello.c → main.c
 */
int main_events_sample(void);

#endif /* EVENTS_SAMPLE_H */
