#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/* Platform-specific includes */
#if defined(__APPLE__) || defined(__linux__)
#define POSIX_AVAILABLE 1
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#endif

#include "advance/system_sample.h"

/* ====================================================================
 * 系统调用演示 (System Calls — POSIX Signals, mmap, Process)
 *
 * 本节目演 POSIX 系统调用的核心概念：
 *  - SIGBUS/SIGSEGV 信号处理 (signal, sigaction)
 *  - mmap/munmap 内存映射
 *  - 文件描述符: open, read, write, close
 *  - 子进程 (fork/wait) 及管道 (pipe) — 条件编译
 *
 * 类比: OS 是房东，fd 是钥匙， signals 是门铃，mmap 是直接看水管
 * ==================================================================== */

/* ── 1. File Descriptors (文件描述符) ── */

static void demo_file_descriptors(void)
{
    printf("=== 1. 文件描述符 (File Descriptors) ===\n");
    printf("  类比: fd 是房东给的「钥匙」—— 每一把钥匙打开一个房间\n\n");

    printf("  标准 fd:\n");
    printf("    STDIN_FILENO  = %d  (标准输入)\n", STDIN_FILENO);
    printf("    STDOUT_FILENO = %d  (标准输出)\n", STDOUT_FILENO);
    printf("    STDERR_FILENO = %d  (标准错误)\n", STDERR_FILENO);

    /* 用 write() 直接写 fd — 绕过 printf 的缓冲 */
    const char *msg = "  直接 write(STDOUT, \"hello fd\") 输出:\n";
    (void)write(STDOUT_FILENO, msg, (size_t)strlen(msg));

#ifdef POSIX_AVAILABLE
    printf("\n  文件读写示例:\n");

    const char *test_file = "/tmp/hello_c_fd_test.txt";
    const char *write_data = "Hello from file descriptor!\n";

    /* 写入文件 */
    int fd_w = (int)open(test_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_w < 0) {
        printf("    [Error] open write: %s\n", strerror(errno));
    } else {
        ssize_t nw = write(fd_w, write_data, strlen(write_data));
        printf("    write(fd=%d, \"%s\") → %zd bytes\n",
               fd_w, write_data, nw);
        close(fd_w);
    }

    /* 读取文件 */
    int fd_r = (int)open(test_file, O_RDONLY);
    if (fd_r < 0) {
        printf("    [Error] open read: %s\n", strerror(errno));
    } else {
        char buf[64];
        ssize_t nr = read(fd_r, buf, sizeof(buf) - 1);
        if (nr > 0) {
            buf[(size_t)nr] = '\0';
            printf("    read(fd=%d) → [%.*s]", fd_r, (int)nr, buf);
        } else {
            printf("    read(fd=%d) → %zd bytes (empty?)\n", fd_r, nr);
        }
        close(fd_r);
    }

    /* 清理临时文件 */
    unlink(test_file);
#endif

    printf("\n");
}

/* ── 2. POSIX Signals (信号处理) ── */

#ifdef POSIX_AVAILABLE

static volatile sig_atomic_t g_signal_received = 0;

static void signal_handler(int signum)
{
    const char *msg;
    switch (signum) {
        case SIGINT:  msg = "SIGINT (Ctrl+C)\n";  break;
        case SIGTERM: msg = "SIGTERM (终止请求)\n"; break;
        case SIGHUP:  msg = "SIGHUP (挂断)\n";     break;
        case SIGUSR1: msg = "SIGUSR1 (用户定义 1)\n"; break;
        case SIGUSR2: msg = "SIGUSR2 (用户定义 2)\n"; break;
        default:      msg = "未知信号\n";           break;
    }
    (void)write(STDERR_FILENO, msg, strlen(msg));
    g_signal_received = 1;
}

static volatile sig_atomic_t g_alarm_fired = 0;

static void alarm_handler(int signum)
{
    (void)signum;
    g_alarm_fired = 1;
}

#endif /* POSIX_AVAILABLE */

static void demo_signals(void)
{
    printf("=== 2. POSIX 信号 (Signals) ===\n");
    printf("  类比: signals 是「门铃」—— 打断你正在做的事\n\n");

#ifdef POSIX_AVAILABLE

    /* 注册信号处理器 */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    printf("  [1] sigaction 注册: SIGUSR1, SIGUSR2 → signal_handler\n");
    printf("      向自己发送 SIGUSR1...\n");
    raise(SIGUSR1);
    printf("      g_signal_received = %d (应为 1)\n\n", (int)g_signal_received);

    /* alarm 信号演示 */
    struct sigaction sa_alarm;
    memset(&sa_alarm, 0, sizeof(sa_alarm));
    sa_alarm.sa_handler = alarm_handler;
    sigemptyset(&sa_alarm.sa_mask);
    sa_alarm.sa_flags = 0;
    sigaction(SIGALRM, &sa_alarm, NULL);

    printf("  [2] SIGALRM 演示:\n");
    printf("      设置 1 秒闹钟...\n");
    unsigned int remaining = alarm(1);
    (void)remaining;
    printf("      alarm(1) 已设置，等待信号...\n");

    /* 忙等待直到信号到达 */
    while (!g_alarm_fired) {
        /* 空循环: 仅用于演示 */
    }
    printf("      SIGALRM 已接收! g_alarm_fired = %d\n\n", (int)g_alarm_fired);

    /* SIGPIPE 忽略示例 */
    printf("  [3] SIGPIPE 处理:\n");
    printf("      写已关闭的 pipe 会发 SIGPIPE → 默认行为是终止进程\n");
    printf("      修复: signal(SIGPIPE, SIG_IGN) 忽略它\n\n");

#else

    printf("  [跳过] 当前平台不支持 POSIX 信号\n");
    printf("  需要在 macOS / Linux 上编译才能运行此演示\n\n");

#endif /* POSIX_AVAILABLE */
}

/* ── 3. Memory-Mapped I/O (mmap / munmap) ── */

static void demo_mmap(void)
{
    printf("=== 3. 内存映射 I/O (mmap / munmap) ===\n");
    printf("  类比: mmap 是在你家墙上开一扇窗，直接看到水管\n\n");

#ifdef POSIX_AVAILABLE

    const char *test_file = "/tmp/hello_c_mmap_test.txt";
    const char *content = "Memory-Mapped File Content!\n";
    size_t content_len = strlen(content);

    /* 创建并写入文件 */
    int fd = (int)open(test_file, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        printf("    [Error] open: %s\n", strerror(errno));
        printf("    (跳过 mmap 演示)\n\n");
        return;
    }

    /* 设置文件大小 */
    if (ftruncate(fd, (off_t)content_len) < 0) {
        printf("    [Error] ftruncate: %s\n", strerror(errno));
        close(fd);
        printf("    (跳过 mmap 演示)\n\n");
        return;
    }

    /* mmap: 将文件映射到内存 */
    void *mapped = mmap(NULL, content_len, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        printf("    [Error] mmap: %s\n", strerror(errno));
        close(fd);
        printf("    (跳过 mmap 演示)\n\n");
        return;
    }

    /* 拷贝数据到映射区域 */
    memcpy(mapped, content, content_len);

    printf("    mmap 映射了 %zu 字节\n", content_len);
    printf("    通过指针直接写入映射区域\n");
    printf("    读取映射内容: [%.*s]", (int)content_len, (char *)mapped);

    /* 强制写回磁盘 */
    msync(mapped, content_len, MS_SYNC);
    printf("    msync() 已强制写回磁盘\n");

    /* munmap: 取消映射 */
    if (munmap(mapped, content_len) < 0) {
        printf("    [Error] munmap: %s\n", strerror(errno));
    } else {
        printf("    munmap() 成功\n");
    }

    close(fd);
    unlink(test_file);

    /* mmap 常见错误演示 */
    printf("\n  错误-first 教训:\n");
    printf("    ❌ 越界写入: mmap(100 bytes) 后写 offset=200 → SIGSEGV\n");
    printf("    ✅ 修复: 严格跟踪 mapped_size，写入前检查 offset < size\n");
    printf("    ❌ 忘记 msync: 修改 mmap 区域后直接 munmap → 数据丢失\n");
    printf("    ✅ 修复: munmap 前调用 msync(addr, len, MS_SYNC)\n");

#else

    printf("  [跳过] 当前平台不支持 mmap\n");
    printf("  需要在 macOS / Linux 上编译才能运行此演示\n");

#endif /* POSIX_AVAILABLE */

    printf("\n");
}

/* ── 4. Process Management (fork / wait / exec) ── */

static void demo_process(void)
{
    printf("=== 4. 进程管理 (Process: fork / wait / exec) ===\n");
    printf("  类比: fork 是克隆你自己，等分身干完活再收工\n\n");

#ifdef POSIX_AVAILABLE

    printf("  [1] fork() 创建子进程:\n");

    pid_t pid = fork();

    if (pid < 0) {
        printf("    [Error] fork failed: %s\n", strerror(errno));
    } else if (pid == 0) {
        /* 子进程: stdout 可能已被缓冲，用 write 确保输出 */
        const char *child_msg = "    [子进程] 我是 clone!\n";
        write(STDOUT_FILENO, child_msg, strlen(child_msg));

        /* 子进程必须 exit */
        _exit(0);
    } else {
        /* 父进程: 等待子进程 */
        printf("    [父进程] fork() 返回子进程 PID = %d\n", (int)pid);
        printf("    [父进程] wait() 等待子进程...\n");

        int status = 0;
        pid_t waited = waitpid(pid, &status, 0);
        if (waited > 0) {
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                printf("    [父进程] 子进程退出, code=%d\n", exit_code);
            } else {
                printf("    [父进程] 子进程异常退出\n");
            }
        } else {
            printf("    [Error] waitpid failed: %s\n", strerror(errno));
        }

        printf("\n  [2] fork 要点:\n");
        printf("    - fork() 返回两次: 子进程→0, 父进程→子 PID\n");
        printf("    - 子进程获得父进程内存和 fd 的副本\n");
        printf("    - 子进程必须 _exit()，不能 exit()（避免刷新父的缓冲区）\n");
        printf("    - 父进程必须 wait()，否则产生僵尸进程 (zombie)\n");
    }

#else

    printf("  [跳过] 当前平台不支持 fork/wait\n");
    printf("  需要在 macOS / Linux 上编译\n");

#endif /* POSIX_AVAILABLE */

    printf("\n");
}

/* ── 5. Pipe IPC (管道进程间通信) ── */

static void demo_pipe(void)
{
    printf("=== 5. 管道 IPC (Pipe) ===\n");
    printf("  类比: pipe 是父子之间的一根管子—— 一个写一个读\n\n");

#ifdef POSIX_AVAILABLE

    int pipefd[2];

    if (pipe(pipefd) < 0) {
        printf("    [Error] pipe: %s\n", strerror(errno));
        return;
    }

    printf("    pipe() 创建: pipefd[0]=读取端, pipefd[1]=写入端\n");

    pid_t pid = fork();

    if (pid < 0) {
        printf("    [Error] fork: %s\n", strerror(errno));
    } else if (pid == 0) {
        /* 子进程: 读取 */
        close(pipefd[1]); /* 关闭写入端 */

        char buf[64];
        ssize_t nr = read(pipefd[0], buf, sizeof(buf) - 1);
        if (nr > 0) {
            buf[(size_t)nr] = '\0';
        } else {
            strcpy(buf, "(read failed)");
        }
        close(pipefd[0]);

        const char *child_msg = "    [子进程] 收到: ";
        write(STDOUT_FILENO, child_msg, strlen(child_msg));
        write(STDOUT_FILENO, "[", 1);
        write(STDOUT_FILENO, buf, strlen(buf));
        const char *end_msg = "]\n";
        write(STDOUT_FILENO, end_msg, strlen(end_msg));

        _exit(0);
    } else {
        /* 父进程: 写入 */
        close(pipefd[0]); /* 关闭读取端 */

        const char *msg = "Hello from parent via pipe!";
        ssize_t nw = write(pipefd[1], msg, strlen(msg));
        close(pipefd[1]);

        printf("    [父进程] write pipe: \"%s\" (%zd bytes)\n", msg, nw);

        /* 等待子进程读完 */
        int status = 0;
        waitpid(pid, &status, 0);
        printf("    [父进程] 子进程完成\n");

        printf("\n  pipe 要点:\n");
        printf("    - pipe(fd) 创建单向通道: fd[0] 读, fd[1] 写\n");
    }

#else

    printf("  [跳过] 当前平台不支持 pipe/fork\n");

#endif /* POSIX_AVAILABLE */

    printf("\n");
}

/* ── Coordinator entry ── */

int main_system_sample(void)
{
    printf("========================================\n");
    printf("  系统调用 (System Calls / POSIX)\n");
    printf("========================================\n\n");

    demo_file_descriptors();
    demo_signals();
    demo_mmap();
    demo_process();
    demo_pipe();

    printf("系统调用演示完毕。\n");
    return 0;
}
