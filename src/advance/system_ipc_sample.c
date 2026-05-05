#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#if defined(__APPLE__) || defined(__linux__)
#include <sys/socket.h>
#endif

#include "advance/system_ipc_sample.h"

/* ====================================================================
 * IPC 类比: 管道像两个房间之间的传声筒 — 这边说话，那边听到
 *           Unix socket 像两个房间之间装了专用电话
 * ==================================================================== */

#if defined(__APPLE__) || defined(__linux__)
#endif

/* ── 1. Basic Pipe (单向) ── */

static void system_ipc_pipe_basic_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 1. 单向管道 (system_ipc_pipe_basic_sample) ===\n");
    printf("  类比: 一根单向管子 — 一头写，一头读\n\n");

    int pipefd[2];
    if (pipe(pipefd) < 0) {
        printf("  [Error] pipe: %s\n", strerror(errno));
        return;
    }

    printf("  pipefd[0] = %d (读取端), pipefd[1] = %d (写入端)\n", pipefd[0], pipefd[1]);

    pid_t pid = fork();

    if (pid < 0) {
        printf("  [Error] fork: %s\n", strerror(errno));
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid == 0) {
        /* 子进程: 读取 */
        close(pipefd[1]); /* 关闭不需要的写入端 */

        char buf[128];
        ssize_t nr = read(pipefd[0], buf, sizeof(buf) - 1);
        if (nr > 0) {
            buf[(size_t)nr] = '\0';
        } else {
            strncpy(buf, "(空)", sizeof(buf) - 1);
            buf[sizeof(buf) - 1] = '\0';
        }
        close(pipefd[0]);

        /* 用 write 避免缓冲区问题 */
        char msg[256];
        int len = snprintf(msg, sizeof(msg), "  [子进程] 收到: [%s]\n", buf);
        if (write(STDOUT_FILENO, msg, (size_t)len) < 0) {
            perror("  write");
        }

        _exit(0);
    } else {
        /* 父进程: 写入 */
        close(pipefd[0]); /* 关闭不需要的读取端 */

        const char *send_msg = "Hello from parent via pipe!";
        ssize_t nw = write(pipefd[1], send_msg, strlen(send_msg));
        close(pipefd[1]);

        printf("  [父进程] 写入管道: \"%s\" (%zd 字节)\n", send_msg, nw);

        int status = 0;
        waitpid(pid, &status, 0);
        printf("  [父进程] 子进程完成\n");

        printf("\n  管道要点:\n");
        printf("    - pipe(fd) 创建单向通道: fd[0] 读, fd[1] 写\n");
        printf("    - fork 后双方关闭不需要的端\n");
        printf("    - 读端全部关闭时，write 返回 SIGPIPE (进程终止)\n");
        printf("    - 写端全部关闭时，read 返回 0 (EOF)\n");
        printf("    - 管道有缓冲区 (通常 64KB)，写满后 write 阻塞\n\n");
    }
#else
    printf("=== 1. 单向管道 (system_ipc_pipe_basic_sample) ===\n");
    printf("  [跳过] 当前平台不支持 pipe\n");
    printf("\n");
#endif
}

/* ── 2. Bidirectional Pipe (双向) ── */

static void system_ipc_pipe_bidirectional_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 2. 双向管道 (system_ipc_pipe_bidirectional_sample) ===\n");
    printf("  类比: 两根管子 — 一个方向传话，一个方向回话\n\n");

    int p1[2]; /* 父 → 子 */
    int p2[2]; /* 子 → 父 */

    if (pipe(p1) < 0 || pipe(p2) < 0) {
        printf("  [Error] pipe: %s\n", strerror(errno));
        return;
    }

    pid_t pid = fork();

    if (pid < 0) {
        printf("  [Error] fork: %s\n", strerror(errno));
        return;
    }

    if (pid == 0) {
        /* 子进程: 读 p1[0], 写 p2[1] */
        close(p1[1]);
        close(p2[0]);

        char buf[128];
        ssize_t nr = read(p1[0], buf, sizeof(buf) - 1);
        if (nr > 0) {
            buf[(size_t)nr] = '\0';

            /* 把收到的消息转大写然后回传 */
            for (ssize_t i = 0; i < nr; i++) {
                if (buf[i] >= 'a' && buf[i] <= 'z') {
                    buf[i] = buf[i] - 'a' + 'A';
                }
            }

            if (write(p2[1], buf, (size_t)nr) < 0) perror("  write");
            char msg[128];
            int len = snprintf(msg, sizeof(msg), "  [子进程] 收到并转大写: [%.*s]\n", (int)nr, buf);
            if (write(STDOUT_FILENO, msg, (size_t)len) < 0) perror("  write");
        }

        close(p1[0]);
        close(p2[1]);
        _exit(0);
    } else {
        /* 父进程: 写 p1[1], 读 p2[0] */
        close(p1[0]);
        close(p2[1]);

        const char *send = "hello from parent";
        ssize_t nw = write(p1[1], send, strlen(send));
        close(p1[1]);

        printf("  [父进程] 发送: \"%s\" (%zd 字节)\n", send, nw);

        char buf[128];
        ssize_t nr = read(p2[0], buf, sizeof(buf) - 1);
        close(p2[0]);

        if (nr > 0) {
            buf[(size_t)nr] = '\0';
            printf("  [父进程] 回复: [%.*s]\n", (int)nr, buf);
        }

        int status = 0;
        waitpid(pid, &status, 0);
        printf("\n  双向管道要点:\n");
        printf("    - 两根管子实现双向通信: p1(父→子), p2(子→父)\n");
        printf("    - 每根管子只允许单向流动\n");
        printf("    - fork 后正确关闭不需要的端防止死锁\n\n");
    }
#else
    printf("=== 2. 双向管道 (system_ipc_pipe_bidirectional_sample) ===\n");
    printf("  [跳过] 当前平台不支持 pipe\n");
    printf("\n");
#endif
}

/* ── 3. Unix Domain Socket (socketpair) ── */

static void system_ipc_socketpair_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 3. Unix Domain Socket (system_ipc_socketpair_sample) ===\n");
    printf("  类比: Unix socket 像两个房间之间的专用电话 — 全双工\n\n");

    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        printf("  [Error] socketpair: %s\n", strerror(errno));
        return;
    }

    printf("  socketpair: sv[0]=%d, sv[1]=%d (全双工!)\n", sv[0], sv[1]);

    pid_t pid = fork();

    if (pid < 0) {
        printf("  [Error] fork: %s\n", strerror(errno));
        return;
    }

    if (pid == 0) {
        /* 子进程: 用 sv[1] 通信 */
        close(sv[0]);

        const char *child_msg = "Hello from child!";
        if (write(sv[1], child_msg, strlen(child_msg)) < 0) {
            perror("  write");
        }

        char buf[128];
        ssize_t nr = read(sv[1], buf, sizeof(buf) - 1);
        if (nr > 0) {
            buf[(size_t)nr] = '\0';
            char msg[256];
            int len = snprintf(msg, sizeof(msg), "  [子进程] 收到回复: [%s]\n", buf);
            if (write(STDOUT_FILENO, msg, (size_t)len) < 0) {
                perror("  write");
            }
        }

        close(sv[1]);
        _exit(0);
    } else {
        /* 父进程: 用 sv[0] 通信 */
        close(sv[1]);

        char buf[128];
        ssize_t nr = read(sv[0], buf, sizeof(buf) - 1);
        if (nr > 0) {
            buf[(size_t)nr] = '\0';
            printf("  [父进程] 收到: [%s]\n", buf);
        }

        const char *reply = "ACK from parent";
        if (write(sv[0], reply, strlen(reply)) < 0) {
            perror("  write");
        }
        printf("  [父进程] 回复: [%s]\n", reply);

        close(sv[0]);

        int status = 0;
        waitpid(pid, &status, 0);

        printf("\n  socketpair vs pipe:\n");
        printf("    pipe:      单向, 只能亲缘进程间使用\n");
        printf("    socketpair: 全双工, 只能亲缘进程间使用\n");
        printf("    AF_UNIX socket: 全双工, 可通过文件系统路径通信\n\n");
    }
#else
    printf("=== 3. Unix Domain Socket (system_ipc_socketpair_sample) ===\n");
    printf("  [跳过] 当前平台不支持 socketpair\n");
    printf("\n");
#endif
}

/* ── Coordinator entry ── */

int main_system_ipc_sample(void)
{
    printf("========================================\n");
    printf("  IPC: 管道与进程间通信\n");
    printf("========================================\n\n");

    printf("  类比: 管道像传声筒, Unix socket 像专用电话\n\n");

    system_ipc_pipe_basic_sample();
    system_ipc_pipe_bidirectional_sample();
    system_ipc_socketpair_sample();

    printf("IPC 演示完毕。\n");
    return 0;
}
