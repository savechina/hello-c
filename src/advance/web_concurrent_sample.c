/**
 * @file web_concurrent_sample.c
 * @brief 并发服务器模型 — fork / pthread / I/O 复用
 *
 * 演示三种服务器并发模型:
 *   1. fork per connection — 每个连接 fork 一个子进程
 *   2. thread per connection — 每个连接创建一个 pthread
 *   3. select I/O multiplexing — 单线程多路复用
 *
 * 类比: 餐厅服务模式 — fork 是每位客人配专属服务员,
 *       thread 是每位客人配服务员但共享厨房,
 *       I/O 复用是一位服务员但手脚麻利落服务所有客人。
 *
 * 注意: 实际 bind/listen/accept 被 #ifdef DEMO_ACTUAL_SERVER 保护,
 *       教程运行时不会占用真实端口。
 *
 * Platform: POSIX (fork, pthread, select)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(__APPLE__) || defined(__linux__)
#define POSIX_AVAILABLE 1
#endif

#include "advance/web_concurrent_sample.h"

#ifdef POSIX_AVAILABLE
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#endif

/* ====================================================================
 *  Demo 1: Fork Server — fork 每个连接
 * ==================================================================== */

#ifdef POSIX_AVAILABLE

/* 子进程处理函数 — 模拟 HTTP 响应 */
__attribute__((unused))
static void handle_client(int client_fd)
{
    /* 实际服务器: recv → parse → send 在这里 */
    char resp[] =
        "HTTP/1.0 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 13\r\n"
        "\r\n"
        "Hello World!!!";
    ssize_t n = send(client_fd, resp, strlen(resp), 0);
    (void)n;  /* demo: 不检查发送结果 */
    close(client_fd);
}

static void web_concurrent_fork_server_sample(void)
{
    printf("  [1] Fork 并发服务器 (Fork Per Connection):\n");
    printf("    类比: 每位客人配专属服务员\n\n");

    printf("    核心结构:\n\n");

#ifdef DEMO_ACTUAL_SERVER
    /* 只在显式开启时真实运行 */

    /* fork 前忽略 SIGCHLD — 自动回收子进程 */
    signal(SIGCHLD, SIG_IGN);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("    [Error] socket(): %s\n", strerror(errno));
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("    [Error] bind: %s\n", strerror(errno));
        close(server_fd);
        return;
    }

    listen(server_fd, 128);

    printf("    [Server] 监听端口 8080, backlog=128\n\n");

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t cl = sizeof(client_addr);
        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr, &cl);

        if (client_fd < 0) {
            printf("    [Error] accept: %s\n", strerror(errno));
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            printf("    [Error] fork: %s\n", strerror(errno));
            close(client_fd);
        } else if (pid == 0) {
            /* 子进程 */
            close(server_fd);  /* 子进程不需要监听 fd */
            handle_client(client_fd);
            _exit(0);          /* 子进程退出 */
        } else {
            /* 父进程 */
            close(client_fd);  /* 父进程不需要客户端 fd */
        }
    }
#else
    printf("    int server_fd = socket(AF_INET, SOCK_STREAM, 0);\n");
    printf("    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, ...);\n");
    printf("    bind(server_fd, ...);\n");
    printf("    listen(server_fd, 128);\n");
    printf("    signal(SIGCHLD, SIG_IGN);  // 自动回收僵尸\n\n");
    printf("    for (;;) {\n");
    printf("        int client_fd = accept(server_fd, ...);\n\n");
    printf("        pid_t pid = fork();\n");
    printf("        if (pid < 0) {\n");
    printf("            // fork 失败\n");
    printf("            close(client_fd);\n");
    printf("        } else if (pid == 0) {\n");
    printf("            // 子进程: 关闭监听 fd, 处理客户端\n");
    printf("            close(server_fd);\n");
    printf("            handle_client(client_fd);\n");
    printf("            _exit(0);  // 子进程退出\n");
    printf("        } else {\n");
    printf("            // 父进程: 关闭客户端 fd, 继续 accept\n");
    printf("            close(client_fd);\n");
    printf("        }\n");
    printf("    }\n\n");

    printf("    特点:\n");
    printf("    ✅ 每个连接隔离 — 一个崩溃不影响其他\n");
    printf("    ✅ 编程简单 — 每个子进程独立逻辑\n");
    printf("    ❌ 开销大 — 每个连接一个进程 (内存 ~几MB)\n");
    printf("    ❌ 连接数受系统进程数限制\n");
    printf("    ❌ fork 成本高 (复制页表、地址空间)\n\n");

    printf("    ❌→✅ 教训:\n");
    printf("      ❌ 子进程用 exit() → stdout 缓冲区刷新, 可能双重输出\n");
    printf("      ✅ 子进程用 _exit() — 立即退出，不清空缓冲区\n");
    printf("      ❌ 父进程不 close(client_fd) → fd 泄漏\n");
    printf("      ✅ 父进程 accept 后立即 close(client_fd)\n");
    printf("      ❌ 不处理 SIGCHLD → 僵尸进程堆积\n");
    printf("      ✅ signal(SIGCHLD, SIG_IGN) 或 waitpid 回调\n");
#endif

    printf("\n");
}

#else

static void web_concurrent_fork_server_sample(void)
{
    printf("  [1] 跳过 — 平台不支持 fork\n\n");
}

#endif

/* ====================================================================
 *  Demo 2: Thread per Connection — pthread 每个连接
 * ==================================================================== */

#ifdef POSIX_AVAILABLE

/* 伪处理函数 */
__attribute__((unused))
static void *thread_handle_client(void *arg)
{
    /* 实际: int fd = *(int *)arg; recv → parse → send; free(arg) */
    int client_fd = *(int *)arg;
    free(arg);  /* 释放分配给线程的 fd */
    close(client_fd);
    return NULL;
}

static void web_concurrent_thread_per_connection_sample(void)
{
    printf("  [2] 线程并发服务器 (Thread Per Connection):\n");
    printf("    类比: 每位客人配服务员，共享厨房 (线程共享内存)\n\n");

#ifdef DEMO_ACTUAL_SERVER

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("    [Error] socket(): %s\n", strerror(errno));
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        printf("    [Error] bind: %s\n", strerror(errno));
        close(server_fd);
        return;
    }

    listen(server_fd, 128);

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t cl = sizeof(client_addr);
        int client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr, &cl);

        if (client_fd < 0) continue;

        /* 给线程传递 fd */
        int *arg = malloc(sizeof(int));
        *arg = client_fd;

        pthread_t tid;
        if (pthread_create(&tid, NULL, thread_handle_client, arg) != 0) {
            printf("    [Error] pthread_create\n");
            close(client_fd);
            free(arg);
        }
        /* 主线程 detached — 不用 pthread_join */
    }

#else

    printf("    pthread_t tid;\n");
    printf("    int *arg = malloc(sizeof(int));\n");
    printf("    *arg = client_fd;\n\n");
    printf("    pthread_create(&tid, NULL, thread_handle_client, arg);\n");
    printf("    // 主线程 detached — 不 join\n\n");

    printf("    Thread 函数:\n");
    printf("    void *thread_handle_client(void *arg) {\n");
    printf("        int client_fd = *(int *)arg;\n");
    printf("        free(arg);\n");
    printf("        handle_client(client_fd);\n");
    printf("        close(client_fd);\n");
    printf("        return NULL;\n");
    printf("    }\n\n");

    printf("    特点:\n");
    printf("    ✅ 比 fork 轻量 — 共享内存，不需复制\n");
    printf("    ✅ 线程间数据共享容易 (全局变量、缓存)\n");
    printf("    ❌ 需要处理并发安全 (mutex, atomic)\n");
    printf("    ❌ 栈溢出影响整个进程 (一个线程崩溃 = 所有线程完蛋)\n");
    printf("    ❌ 线程数太多时上下文切换开销也大\n\n");

    printf("    ❌→✅ 教训:\n");
    printf("      ❌ 直接传 &client_fd → 线程读到新 accept 的值\n");
    printf("      ✅ malloc(int) 给每个线程独立的 fd 拷贝\n");
    printf("      ❌ 共享数据不加锁 → 数据竞争 (data race)\n");
    printf("      ✅ 用 pthread_mutex 或 __atomic 保护共享状态\n");
    printf("      ❌ pthread_create 失败时泄漏 fd → 客户端 hang\n");
    printf("      ✅ create 失败时 close(client_fd) + free(arg)\n");
#endif

    printf("\n");
}

#else

static void web_concurrent_thread_per_connection_sample(void)
{
    printf("  [2] 跳过 — 平台不支持 pthread\n\n");
}

#endif

/* ====================================================================
 *  Demo 3: I/O Multiplexing (select) — 一位服务员服务所有客人
 * ==================================================================== */

#ifdef POSIX_AVAILABLE

static void web_concurrent_iomux_server_sample(void)
{
    printf("  [3] I/O 多路复用服务器 (select Multiplexing):\n");
    printf("    类比: 一位服务员但手脚麻利，同时服务所有客人\n\n");

#ifdef DEMO_ACTUAL_SERVER

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return;

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);
    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 128);

    /* 将 server socket 设为非阻塞 */
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);

    int maxfd = server_fd;
    fd_set read_set;

    for (;;) {
        FD_ZERO(&read_set);
        FD_SET(server_fd, &read_set);

        for (int i = 0; i <= maxfd; i++) {
            /* 如果有活跃连接就添加到 set */
            /* (本 demo 用简化版 — 实际应维护 fd 数组) */
        }

        int active = select(maxfd + 1, &read_set, NULL, NULL, NULL);
        if (active < 0) continue;

        if (FD_ISSET(server_fd, &read_set)) {
            /* 新连接 */
            int client_fd = accept(server_fd, NULL, NULL);
            if (client_fd > maxfd) {
                maxfd = client_fd;
            }
            /* 把 client_fd 加入 fd 数组 */
        }

        /* 检查所有 client fd 是否可读 */
        /* (实际实现遍历 fd 数组) */
    }

#else

    printf("    fd_set read_set;\n");
    printf("    int maxfd = server_fd;\n\n");
    printf("    for (;;) {\n");
    printf("        FD_ZERO(&read_set);\n");
    printf("        FD_SET(server_fd, &read_set);\n");
    printf("        // FD_SET(client, &read_set);  // 每个活跃客户端\n\n");
    printf("        int active = select(maxfd + 1, &read_set,\n");
    printf("                              NULL, NULL, NULL);\n\n");
    printf("        if (FD_ISSET(server_fd, &read_set)) {\n");
    printf("            // 新连接 — accept\n");
    printf("            int client_fd = accept(server_fd, ...);\n");
    printf("            // FD_SET(client_fd, &read_set);\n");
    printf("        }\n\n");
    printf("        // for (each client) {\n");
    printf("        //     if (FD_ISSET(client, &read_set))\n");
    printf("        //         handle(client);\n");
    printf("        // }\n");
    printf("    }\n\n");

    printf("    特点:\n");
    printf("    ✅ 单线程管理几百/几千连接 — 资源消耗极低\n");
    printf("    ✅ 没有进程/线程上下文切换开销\n");
    printf("    ❌ 编程复杂度显著上升 (状态机、非阻塞 I/O)\n");
    printf("    ❌ select 有 FD_SETSIZE 限制 (通常 1024)\n");
    printf("    ❌ 每次 select 前需重建 fd_set — O(n) 开销\n\n");

    printf("    现代替代:\n");
    printf("    Linux  → epoll (O(1) 事件通知)\n");
    printf("    macOS  → kqueue (同样 O(1))\n");
    printf("    Windows→ IOCP (I/O Completion Ports)\n\n");

    printf("    这就是 Nginx 的核心模型 — 一个 worker 处理 10000+\n\n");

    printf("    ❌→✅ 教训:\n");
    printf("      ❌ socket 阻塞模式 + select → accept 可能永远阻塞\n");
    printf("      ✅ accept 后用 fcntl(F_SETFL, O_NONBLOCK)\n");
    printf("      ❌ 不更新 maxfd → 新 fd 被 select 忽略\n");
    printf("      ✅ 每次 connect 后 maxfd = MAX(maxfd, client_fd)\n");
    printf("      ❌ FD_SET 不加边界检查 → FD_SETSIZE 溢出 → 内存破坏\n");
    printf("      ✅ client_fd < FD_SETSIZE 才 FD_SET\n");
#endif

    printf("\n");
}

#else

static void web_concurrent_iomux_server_sample(void)
{
    printf("  [3] 跳过 — 平台不支持 select\n\n");
}

#endif

/* ====================================================================
 *  Coordinator
 * ==================================================================== */

int main_web_concurrent_sample(void)
{
    printf("========================================\n");
    printf("  并发服务器模型 (Concurrent Server)\n");
    printf("========================================\n\n");

    printf("  类比: 餐厅服务模式\n");
    printf("    Fork     = 每位客人专属服务员 (隔离好但贵)\n");
    printf("    Thread   = 每位客人服务员，共享厨房 (轻量但需要同步)\n");
    printf("    I/O 复用 = 一位服务员服务所有客人 (高效但需手脚麻利)\n\n");

    web_concurrent_fork_server_sample();
    web_concurrent_thread_per_connection_sample();
    web_concurrent_iomux_server_sample();

    printf("并发服务器模型演示完毕。\n");
    return 0;
}
