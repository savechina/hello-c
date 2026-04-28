/**
 * @file async_iomux_sample.c
 * @brief I/O 多路复用 (I/O Multiplexing — select/poll/epoll)
 *
 * Demonstrates:
 *   1. select() — monitor multiple file descriptors
 *   2. Pipe multiplexing — create pipes, detect which is ready
 *   3. epoll (Linux only) — #ifdef __linux__, fallback on macOS
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef __linux__
#include <sys/epoll.h>
#endif

#include "advance/async_iomux_sample.h"

/* ============================================================
 * Demo 1: select() 监控多个文件描述符
 * ============================================================ */

static void async_iomux_select_sample(void)
{
    printf("--- 1. select() 监控多文件描述符 ---\n");

    /* 创建两个管道 */
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) != 0 || pipe(pipe2) != 0) {
        perror("  pipe 创建失败");
        return;
    }

    /* 向 pipe2 写数据 — 让 pipe2 的读端变为可读 */
    const char *msg2 = "hello from pipe2";
    ssize_t w = write(pipe2[1], msg2, strlen(msg2));
    if (w < 0) {
        perror("  write pipe2 失败");
    }
    (void)w;

    /* 用 select 监控两个读端 */
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(pipe1[0], &readfds);
    FD_SET(pipe2[0], &readfds);

    /* 找出最大 fd + 1 */
    int maxfd = (pipe1[0] > pipe2[0] ? pipe1[0] : pipe2[0]) + 1;

    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int ret = select(maxfd, &readfds, NULL, NULL, &timeout);
    if (ret < 0) {
        perror("  select 失败");
    } else {
        printf("  select 返回: %d 个 fd 就绪\n", (int)ret);
        if (FD_ISSET(pipe1[0], &readfds)) {
            printf("  pipe1[0] 可读\n");
        } else {
            printf("  pipe1[0] 不可读（没写入数据）\n");
        }
        if (FD_ISSET(pipe2[0], &readfds)) {
            char buf[64];
            ssize_t r = read(pipe2[0], buf, sizeof(buf) - 1);
            if (r > 0) {
                buf[r] = '\0';
                printf("  pipe2[0] 可读 → 读取到: \"%s\"\n", buf);
            }
        }
    }

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);
    printf("  ✅ select() 演示完毕。\n\n");
}

/* ============================================================
 * Demo 2: Pipe 多路复用 (Pipe Multiplexing)
 * ============================================================ */

static void async_iomux_pipe_multiplex_sample(void)
{
    printf("--- 2. Pipe 多路复用 — 检测哪个管道有数据 ---\n");

    /* 创建 3 个管道 */
    const int n = 3;
    int pipes[3][2];
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) != 0) {
            perror("  pipe 创建失败");
            return;
        }
    }

    /* 向 pipe[1] 和 pipe[2] 写入，pipe[0] 留空 */
    const char *data[] = {"", "data for pipe1", "data for pipe2"};
    for (int i = 0; i < n; i++) {
        if (strlen(data[i]) > 0) {
            ssize_t w = write(pipes[i][1], data[i], strlen(data[i]));
            (void)w;
        }
    }

    /* select 监控所有读端 */
    fd_set readfds;
    FD_ZERO(&readfds);
    int maxfd = 0;
    for (int i = 0; i < n; i++) {
        FD_SET(pipes[i][0], &readfds);
        if (pipes[i][0] > maxfd) {
            maxfd = pipes[i][0];
        }
    }
    maxfd++;

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 500000;  /* 500ms */

    int ret = select(maxfd, &readfds, NULL, NULL, &timeout);
    if (ret < 0) {
        perror("  select");
    } else {
        printf("  select 返回 %d 个 fd 就绪: ", (int)ret);
        for (int i = 0; i < n; i++) {
            if (FD_ISSET(pipes[i][0], &readfds)) {
                char buf[64];
                ssize_t r = read(pipes[i][0], buf, sizeof(buf) - 1);
                if (r > 0) {
                    buf[r] = '\0';
                    printf("pipe[%d]=%s ", i, buf);
                }
            }
        }
        printf("\n");
    }

    for (int i = 0; i < n; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    printf("  ✅ Pipe 多路复用演示完毕。\n\n");
}

/* ============================================================
 * Demo 3: epoll (Linux 专用)
 * ============================================================ */

static void async_iomux_epoll_sample(void)
{
    printf("--- 3. epoll (Linux 专用 I/O 多路复用) ---\n");

#ifdef __linux__
    int epfd = epoll_create1(0);
    if (epfd < 0) {
        perror("  epoll_create1 失败");
        return;
    }

    /* 创建管道并注册到 epoll */
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        perror("  pipe 失败");
        close(epfd);
        return;
    }

    struct epoll_event ev;
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = pipefd[0];
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, pipefd[0], &ev) != 0) {
        perror("  epoll_ctl 失败");
        close(pipefd[0]);
        close(pipefd[1]);
        close(epfd);
        return;
    }

    /* 写数据让 fd 可读 */
    const char *msg = "hello epoll";
    ssize_t w = write(pipefd[1], msg, strlen(msg));
    (void)w;

    /* epoll_wait */
    struct epoll_event events[4];
    int n = epoll_wait(epfd, events, 4, 1000);
    if (n > 0) {
        printf("  epoll_wait 返回 %d 个事件\n", n);
        for (int i = 0; i < n; i++) {
            if (events[i].events & EPOLLIN) {
                char buf[64];
                ssize_t r = read(events[i].data.fd, buf, sizeof(buf) - 1);
                if (r > 0) {
                    buf[r] = '\0';
                    printf("  fd %d 可读 → \"%s\"\n", events[i].data.fd, buf);
                }
            }
        }
    }

    close(pipefd[0]);
    close(pipefd[1]);
    close(epfd);
    printf("  ✅ epoll 演示完毕。\n\n");

#else
    printf("  ⚠️  epoll 是 Linux 独有的 API。\n");
    printf("  当前平台不是 Linux，跳过 epoll 演示。\n");
    printf("  💡 macOS 可以使用 kqueue 替代，Solaris 使用 /dev/poll\n\n");
#endif
}

/* ============================================================
 * Coordinator
 * ============================================================ */

int main_async_iomux_sample(void)
{
    printf("========================================\n");
    printf("  I/O 多路复用 (select · poll · epoll)\n");
    printf("========================================\n\n");

    async_iomux_select_sample();
    async_iomux_pipe_multiplex_sample();
    async_iomux_epoll_sample();

    printf("I/O 多路复用演示完毕。\n");
    return 0;
}
