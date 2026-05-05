#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#include "advance/system_signal_sample.h"

/* ====================================================================
 * 信号类比: 信号像门铃 — 你正在房间里做事（主程序），门铃响了（信号），
 *           你去开门（信号处理函数），处理完继续做事
 * ==================================================================== */

#if defined(__APPLE__) || defined(__linux__)

static volatile sig_atomic_t g_interrupted = 0;

static void sigint_handler(int signum)
{
    (void)signum;
    const char msg[] = "  [信号处理] SIGINT 收到! g_interrupted = 1\n";
    ssize_t _r = write(STDERR_FILENO, msg, sizeof(msg) - 1);
    (void)_r;
    g_interrupted = 1;
}

static volatile sig_atomic_t g_user_signal = 0;

static void user_handler(int signum)
{
    (void)signum;
    g_user_signal = 1;
}

#endif /* POSIX */

/* ── 1. Signal Handler with sigaction ── */

static void system_signal_handler_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 1. 信号处理 (system_signal_handler_sample) ===\n");
    printf("  类比: 用 sigaction 安装一个门铃 — 按下 Ctrl+C 或发信号触发\n\n");

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        printf("  [Error] sigaction(SIGUSR1): %s\n", strerror(errno));
        return;
    }

    printf("  已注册 SIGUSR1 → user_handler\n");
    printf("  向自己发送 SIGUSR1...\n");
    raise(SIGUSR1);
    printf("  g_user_signal = %d (应为 1)\n\n", (int)g_user_signal);

    /* Demonstrate sigaction with SA_RESTART */
    printf("  SA_RESTART 说明:\n");
    printf("    没有 SA_RESTART: 被信号中断的系统调用(e.g. read)返回 EINTR\n");
    printf("    有 SA_RESTART:   系统调用自动重启，不返回 EINTR\n");
    printf("    ✅ 生产代码建议始终使用 sigaction 而非 signal()\n\n");
#else
    printf("=== 1. 信号处理 (system_signal_handler_sample) ===\n");
    printf("  [跳过] 当前平台不支持 POSIX 信号\n");
    printf("\n");
#endif
}

/* ── 2. Signal Block/Unblock ── */

static void system_signal_block_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 2. 信号阻塞 (system_signal_block_sample) ===\n");
    printf("  类比: 挂一个「请勿打扰」牌子 — 门铃按了也不响\n\n");

    sigset_t block_set, old_set;
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGUSR1);

    /* Block SIGUSR1 */
    if (sigprocmask(SIG_BLOCK, &block_set, &old_set) < 0) {
        printf("  [Error] sigprocmask BLOCK: %s\n", strerror(errno));
        return;
    }
    printf("  [1] 已阻塞 SIGUSR1\n");

    /* 尝试发送 — 会被挂起直到解除阻塞 */
    g_user_signal = 0;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = user_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGUSR1, &sa, NULL);

    printf("  在阻塞期间发送 SIGUSR1...\n");
    raise(SIGUSR1);
    printf("  阻塞期间: g_user_signal = %d (应为 0，信号被挂起)\n", (int)g_user_signal);

    /* Unblock */
    if (sigprocmask(SIG_SETMASK, &old_set, NULL) < 0) {
        printf("  [Error] sigprocmask SETMASK: %s\n", strerror(errno));
        return;
    }
    printf("  [2] 已解除阻塞\n");

    /* 发送一个短暂延迟，让挂起的信号递送 */
    printf("  短暂延迟让挂起信号递送...\n");
    usleep(10000); /* 10ms */
    printf("  解除后: g_user_signal = %d (应为 1，挂起信号递送)\n", (int)g_user_signal);

    printf("\n  sigprocmask 要点:\n");
    printf("    SIG_BLOCK   — 添加到阻塞集合\n");
    printf("    SIG_UNBLOCK — 从阻塞集合移除\n");
    printf("    SIG_SETMASK — 替换整个阻塞集合\n");
    printf("\n");
#else
    printf("=== 2. 信号阻塞 (system_signal_block_sample) ===\n");
    printf("  [跳过] 当前平台不支持 POSIX 信号\n");
    printf("\n");
#endif
}

/* ── 3. SA_RESTART Flag ── */

static void system_signal_sigaction_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 3. SA_RESTART 标志 (system_signal_sigaction_sample) ===\n");
    printf("  类比: SA_RESTART 像「自动重拨」——被挂断的电话自动再拨一次\n\n");

    struct sigaction sa_restart, sa_no_restart;
    memset(&sa_restart, 0, sizeof(sa_restart));
    sa_restart.sa_handler = user_handler;
    sigemptyset(&sa_restart.sa_mask);
    sa_restart.sa_flags = SA_RESTART;

    memset(&sa_no_restart, 0, sizeof(sa_no_restart));
    sa_no_restart.sa_handler = user_handler;
    sigemptyset(&sa_no_restart.sa_mask);
    sa_no_restart.sa_flags = 0;

    printf("  sigaction 结构体:\n");
    printf("    sa_handler:  信号处理函数指针\n");
    printf("    sa_mask:     处理期间额外阻塞的信号集\n");
    printf("    sa_flags:    标志位 (SA_RESTART, SA_NOCLDSTOP, ...)\n\n");

    printf("  SA_RESTART 效果:\n");
    printf("    设置 SA_RESTART:\n");
    printf("      read()/select()/accept() 被信号中断后 → 自动重启\n");
    printf("      返回正常结果\n\n");
    printf("    不设置 SA_RESTART:\n");
    printf("      read()/select()/accept() 被信号中断后 → 返回 -1, errno=EINTR\n");
    printf("      需要手动检查 EINTR 并重启\n\n");

    printf("  错误-first 教训:\n");
    printf("    ❌ 错误: 假设 read() 一定成功\n");
    printf("      ssize_t n = read(fd, buf, len);\n");
    printf("      // n == -1, errno == EINTR → 没读到数据但不是真错误!\n\n");
    printf("    ✅ 正确: 检查 EINTR 并重启\n");
    printf("      while ((n = read(fd, buf, len)) < 0 && errno == EINTR)\n");
    printf("          ; // 重启\n\n");

    printf("    ❌ 错误: 信号处理函数里用 printf/malloc\n");
    printf("      void handler(int sig) {\n");
    printf("          printf(\"got signal\\n\"); // DEADLOCK!\n");
    printf("      }\n\n");
    printf("    ✅ 正确: 只写 volatile sig_atomic_t 变量\n");
    printf("      void handler(int sig) { g_flag = 1; }\n\n");

    printf("    ❌ 错误: 忘记用 sigemptyset\n");
    printf("      struct sigaction sa;\n");
    printf("      // sa.sa_mask 未初始化 → 可能意外阻塞随机信号!\n\n");
    printf("    ✅ 正确:\n");
    printf("      struct sigaction sa;\n");
    printf("      memset(&sa, 0, sizeof(sa));\n");
    printf("      sigemptyset(&sa.sa_mask); // 清空\n\n");

    if (sigaction(SIGUSR2, &sa_restart, NULL) < 0) {
        printf("  [Error] sigaction SIGUSR2: %s\n", strerror(errno));
    } else {
        printf("  SA_RESTART 已注册到 SIGUSR2\n");
        raise(SIGUSR2);
        printf("  SIGUSR2 已递送, g_user_signal = %d\n\n", (int)g_user_signal);
    }
#else
    printf("=== 3. SA_RESTART 标志 (system_signal_sigaction_sample) ===\n");
    printf("  [跳过] 当前平台不支持 POSIX 信号\n");
    printf("\n");
#endif
}

/* ── Coordinator entry ── */

int main_system_signal_sample(void)
{
    printf("========================================\n");
    printf("  POSIX 信号处理 (Signal Handling)\n");
    printf("========================================\n\n");

    printf("  类比: 信号像门铃 — 你正在做事，门铃响了去处理\n\n");

    system_signal_handler_sample();
    system_signal_block_sample();
    system_signal_sigaction_sample();

    printf("POSIX 信号处理演示完毕。\n");
    return 0;
}
