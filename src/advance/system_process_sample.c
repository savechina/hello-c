#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "advance/system_process_sample.h"

/* ====================================================================
 * 进程管理类比: fork 像细胞分裂 — 一个进程分裂成两个独立进程
 * ==================================================================== */

#if defined(__APPLE__) || defined(__linux__)
#endif

/* ── 1. Basic fork ── */

static void system_process_fork_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 1. fork 创建子进程 (system_process_fork_sample) ===\n");
    printf("  类比: 克隆一个自己去干活\n\n");

    printf("  [父进程] 准备 fork, PID=%d, PPID=%d\n", (int)getpid(), (int)getppid());

    pid_t pid = fork();

    if (pid < 0) {
        printf("  [Error] fork failed: %s\n", strerror(errno));
        return;
    }

    if (pid == 0) {
        /* 子进程 */
        const char *child_msg = "  [子进程] PID=%d, PPID=%d — 我出生了!\n";
        /* 用 write 避免 printf 缓冲问题 (子进程继承父的缓冲区) */
        char buf[128];
        int len = snprintf(buf, sizeof(buf), child_msg, (int)getpid(), (int)getppid());
        write(STDOUT_FILENO, buf, (size_t)len);

        /* 子进程必须 _exit */
        _exit(0);
    } else {
        /* 父进程 */
        printf("  [父进程] fork 返回子进程 PID = %d\n", (int)pid);
        printf("  fork 返回两次: 子进程得 0, 父进程得子 PID\n\n");

        /* 等待子进程 */
        int status = 0;
        pid_t waited = waitpid(pid, &status, 0);
        if (waited > 0) {
            if (WIFEXITED(status)) {
                printf("  [父进程] 子进程 %d 正常退出, code=%d\n", (int)waited, WEXITSTATUS(status));
            } else {
                printf("  [父进程] 子进程 %d 异常退出\n", (int)waited);
            }
        }
    }
    printf("\n");

    printf("  fork 要点回顾:\n");
    printf("    - fork 返回两次 (子=0, 父=子PID)\n");
    printf("    - 子进程获得父进程内存和 fd 的副本\n");
    printf("    - 子进程用 _exit() 不要 exit()\n");
    printf("    - 父子进程执行顺序不确定 (依赖调度器)\n\n");
#else
    printf("=== 1. fork 创建子进程 (system_process_fork_sample) ===\n");
    printf("  [跳过] 当前平台不支持 fork\n");
    printf("\n");
#endif
}

/* ── 2. exec (replace process image) ── */

static void system_process_exec_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 2. exec 替换进程 (system_process_exec_sample) ===\n");
    printf("  类比: 子进程吃下「变身药」，变成另一个程序\n\n");

    printf("  [父进程] 准备 fork + exec...\n");

    pid_t pid = fork();

    if (pid < 0) {
        printf("  [Error] fork: %s\n", strerror(errno));
        return;
    }

    if (pid == 0) {
        /* 子进程: 用 execvp 替换自己为 /bin/ls */
        printf("  [子进程] 准备 exec 替换...\n");

        char *argv[] = { "ls", "-la", "/tmp", NULL };
        if (execvp(argv[0], argv) < 0) {
            /* exec 失败时继续执行 (通常意味着命令不存在) */
            const char *err = "  [子进程] exec 失败: ";
            write(STDERR_FILENO, err, strlen(err));
            const char *estr = strerror(errno);
            write(STDERR_FILENO, estr, strlen(estr));
            write(STDERR_FILENO, "\n", 1);
            _exit(1);
        }
        /* 如果 exec 成功，这行永远达不到 */
    } else {
        /* 父进程: 等待 */
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("  [父进程] exec 子进程退出, code=%d\n", WEXITSTATUS(status));
        }

        printf("\n  exec 函数族:\n");
        printf("    execl(path, arg0, arg1, ..., NULL)     — 参数列表\n");
        printf("    execv(path, argv[])                    — 参数数组\n");
        printf("    execvp(file, argv[])                   — 搜索 PATH\n");
        printf("    execle(path, arg0, ..., NULL, envp)    — 自定义环境变量\n");
        printf("    execve(path, argv[], envp)             — 系统调用\n\n");
    }
#else
    printf("=== 2. exec 替换进程 (system_process_exec_sample) ===\n");
    printf("  [跳过] 当前平台不支持 fork/exec\n");
    printf("\n");
#endif
}

/* ── 3. wait / waitpid ── */

static void system_process_wait_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 3. 等待子进程 (system_process_wait_sample) ===\n");
    printf("  类比: 父进程等分身干完活再收工\n\n");

    /* 创建 3 个子进程 */
    int num_children = 3;
    pid_t children[3];

    for (int i = 0; i < num_children; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            /* 子进程: 模拟不同工作时长 */
            char msg[64];
            int len = snprintf(msg, sizeof(msg),
                               "  [子进程 #%d] PID=%d, 工作时间 %ds...\n",
                               i + 1, (int)getpid(), i + 1);
            write(STDOUT_FILENO, msg, (size_t)len);
            /* 每个子进程 sleep 不同时间 */
            unsigned int sleep_time = (unsigned int)(i + 1);
            /* 用 short sleep to demo */
            usleep(sleep_time * 50000); /* 50ms, 100ms, 150ms */
            _exit(i + 1);
        } else if (pid > 0) {
            children[i] = pid;
            printf("  [父进程] 创建子进程 #%d, PID=%d\n", i + 1, (int)pid);
        }
    }

    printf("  [父进程] 等待所有子进程...\n");

    for (int i = 0; i < num_children; i++) {
        int status = 0;
        pid_t waited = waitpid(children[i], &status, 0);
        if (waited > 0 && WIFEXITED(status)) {
            printf("  [父进程] 收集子进程 PID=%d, exit_code=%d\n",
                   (int)waited, WEXITSTATUS(status));
        }
    }

    printf("\n  wait() vs waitpid():\n");
    printf("    wait(&status)          — 等待任意子进程\n");
    printf("    waitpid(pid, &status, 0) — 等待指定子进程\n");
    printf("    waitpid(-1, &status, 0) — 同 wait()\n");
    printf("    waitpid(pid, &status, WNOHANG) — 非阻塞等待\n\n");
#else
    printf("=== 3. 等待子进程 (system_process_wait_sample) ===\n");
    printf("  [跳过] 当前平台不支持 fork/wait\n");
    printf("\n");
#endif
}

/* ── 4. Zombie process concept ── */

static void system_process_zombie_sample(void)
{
#if defined(__APPLE__) || defined(__linux__)
    printf("=== 4. 僵尸进程 (system_process_zombie_sample) ===\n");
    printf("  类比: 分身干完活了但父进程不确认 → 变成幽灵状态\n\n");

    pid_t pid = fork();

    if (pid < 0) {
        printf("  [Error] fork: %s\n", strerror(errno));
        return;
    }

    if (pid == 0) {
        /* 子进程: 立即退出 */
        _exit(42);
    }

    /* 父进程: 先不 wait，制造僵尸状态 */
    printf("  [父进程] 子进程 PID=%d 已创建并退出\n", (int)pid);
    printf("  此时子进程处于 ZOMBIE 状态:\n");
    printf("    - 子进程已退出，但父进程未调用 wait()\n");
    printf("    - 子进程在进程表中仍占一席之地\n");
    printf("    - 可以用 `ps -A | grep Z` 查看僵尸进程\n\n");

    printf("  [父进程] 现在调用 waitpid 收尸...\n");
    int status = 0;
    pid_t waited = waitpid(pid, &status, 0);
    if (waited > 0 && WIFEXITED(status)) {
        printf("  [父进程] 已回收子进程 PID=%d, exit_code=%d\n",
               (int)waited, WEXITSTATUS(status));
        printf("  子进程不再占用进程表资源\n\n");
    }

    printf("  僵尸进程要点:\n");
    printf("    - 子进程退出后 → 父进程未 wait() → 僵尸进程\n");
    printf("    - 僵尸进程不消耗 CPU/内存，但占用进程表条目\n");
    printf("    - 系统进程数有限 (PID_MAX)，泄漏过多导致无法创建进程\n");
    printf("    - 父进程退出后 → 僵尸被 init (PID 1) 收养并回收\n\n");
#else
    printf("=== 4. 僵尸进程 (system_process_zombie_sample) ===\n");
    printf("  [跳过] 当前平台不支持 fork\n");
    printf("\n");
#endif
}

/* ── Coordinator entry ── */

int main_system_process_sample(void)
{
    printf("========================================\n");
    printf("  进程管理 (Process Management)\n");
    printf("========================================\n\n");

    printf("  类比: fork 像细胞分裂 — 一个进程分裂成两个独立进程\n\n");

    system_process_fork_sample();
    system_process_exec_sample();
    system_process_wait_sample();
    system_process_zombie_sample();

    printf("进程管理演示完毕。\n");
    return 0;
}
