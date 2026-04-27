# 进程管理 (Process Management)

> "fork 像细胞分裂——一个进程分裂成两个独立的进程，各自继续自己的工作。但记住：两个'你'必须协调好谁干什么、谁等谁。"

## 开篇故事

想象你在做一道复杂的数学题。题目需要计算两部分：A 部分（平方根）和 B 部分（对数）。你可以自己算完 A 再算 B——也可以"克隆一个自己"，一个负责 A、一个负责 B，算完后再合并结果。

`fork()` 就是创建这个"克隆"——分裂后的两个进程（父和子）各自独立运行，各自有自己的内存空间。但克隆后的子进程必须知道自己是谁——返回 0 表示子进程，返回非 0 表示父进程（子进程的 PID）。

更有趣的是：**子进程可以"变身"**（exec）——它不再做父进程的事，而是变成一个全新的程序，比如 `ls` 或 `grep`。这就是 `system()` 和 shell 的工作原理。

## 本章适合谁

- 写过 Python `os.fork()` 或 Go `exec.Command()` 但不知道底层 C 怎么做的人
- 好奇"为什么 shell 能同时运行多个程序"的人
- 想写守护进程或并发服务器的人
- 被僵尸进程困扰、不知道如何清理的人

## 你会学到什么

- `fork()` — 创建子进程（克隆自己）
- `exec` 函数族 — 替换子进程映像（变身）
- `wait()` / `waitpid()` — 等待并回收子进程
- `getpid()` / `getppid()` — 获取进程 ID
- 僵尸进程的概念和避免方法
- `_exit()` vs `exit()` 的区别

## 前置要求

- 理解指针和基本数据类型
- 理解父子关系概念
- 知道信号的基本概念
- 会基本 I/O 操作

## 第一个例子

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
    pid_t pid = fork();

    if (pid == 0) {
        /* 子进程 */
        printf("我是子进程! PID=%d\n", getpid());
        _exit(0);  // 子进程退出
    } else if (pid > 0) {
        /* 父进程 */
        printf("子进程 PID=%d\n", pid);
        int status;
        waitpid(pid, &status, 0);  // 等待子进程
        printf("子进程已退出!\n");
    }
    return 0;
}
```

`fork()` 返回两次：子进程得 0，父进程得子进程 PID。

## 原理解析

### 1. fork 的工作原理

```c
pid_t pid = fork();
```

| 进程 | pid 值 | 说明 |
|------|--------|------|
| 子进程 | 0 | 子进程知道自己是被克隆的 |
| 父进程 | > 0 | 子进程的 PID |
| -1 | 失败 | errno 被设置 |

```
fork 前:
  [父进程] PID=1000
          |
        fork()
          |
fork 后:
  [父进程] PID=1000, fork() = 2000
  [子进程] PID=2000, fork() = 0  (ppid=1000)
```

**关键点**：子进程获得父进程的**副本**——内存、打开的文件描述符、环境变量，都是一个独立的拷贝（写时共享）。

### 2. exec 函数族

```c
/* 替换当前进程的映像为 /bin/ls */
char *argv[] = { "ls", "-la", "/tmp", NULL };
execvp("ls", argv);
// exec 成功后，下面的代码不执行!
perror("exec failed");
```

| 函数 | 特点 |
|------|------|
| `execl(path, arg0, arg1, ..., NULL)` | 可变参数列表 |
| `execv(path, argv[])` | 参数数组 |
| `execvp(file, argv[])` | 搜索 PATH |
| `execle(path, ..., NULL, envp[])` | 自定义环境变量 |
| `execve(path, argv[], envp[])` | 系统调用（最底层） |

**注意**：exec 成功时**不返回**。只有失败时才返回 -1。子进程 exec 失败后必须检查并处理。

### 3. wait / waitpid

```c
pid_t waited = waitpid(pid, &status, 0);
```

| 用法 | 含义 |
|------|------|
| `wait(&status)` | 等待任意子进程 |
| `waitpid(pid, &status, 0)` | 等待指定 PID |
| `waitpid(-1, &status, 0)` | 同 wait() |
| `waitpid(pid, &status, WNOHANG)` | 非阻塞等待 |

```c
if (WIFEXITED(status)) {
    int code = WEXITSTATUS(status);  // 子进程退出码
}
if (WIFSIGNALED(status)) {
    int sig = WTERMSIG(status);     // 子进程被信号杀死
}
```

### 4. _exit() vs exit()

```c
/* 子进程退出用 _exit */
_exit(0);   // 直接系统调用退出，不刷新缓冲区

/* 普通程序退出用 exit */
exit(0);    // 调用 atexit handlers + fflush + 退出
```

子进程用 `exit()` 会刷新父进程已经缓冲的数据——导致输出重复。子进程必须用 `_exit()` 或 `_Exit()`。

### 5. 僵尸进程

```
子进程退出但父进程未 waitpid → 僵尸进程 (ZOMBIE)

僵尸进程不消耗 CPU/内存 → 但仍占用进程表条目
系统 PID 有限 → 泄漏过多 → 无法创建新进程
```

```c
/* 正确做法 */
pid_t pid = fork();
if (pid == 0) {
    _exit(0);
} else {
    waitpid(pid, NULL, 0);  // 收尸!
}
```

## 常见错误

### ❌ 错误 1: 子进程用 exit()

```c
// ❌ exit() 刷新父进程缓冲区 → 输出重复
if (pid == 0) {
    printf("子进程");  // printf 输出可能被缓冲
    exit(0);           // 父进程的缓冲也可能被刷新!
}

// ✅ _exit() 直接退出，不碰缓冲区
if (pid == 0) {
    write(STDOUT_FILENO, "子进程\n", 7);
    _exit(0);
}
```

### ❌ 错误 2: 不 wait 产生僵尸

```c
// ❌ 子进程退出变僵尸，父进程不回收
pid_t pid = fork();
if (pid == 0) { _exit(0); }
// 父进程没有 wait → 子进程变僵尸!

// ✅ 父进程必须 wait
pid_t pid = fork();
if (pid == 0) { _exit(0); }
int status;
waitpid(pid, &status, 0);
```

### ❌ 错误 3: exec 后没有检查错误

```c
// ❌ exec 失败后会继续执行下面的代码
execvp("nonexistent", argv);
printf("done!\n");  // 这条会执行! execvp 返回了!

// ✅ 检查 exec 返回
if (execvp("nonexistent", argv) < 0) {
    perror("execvp");
    _exit(1);
}
```

### ❌ 错误 4: fork 后父子共享 fd 不同步

```c
// ❌ fork 后父子都写同一个 fd → 可能交错输出
int fd = open("shared.txt", O_WRONLY);
pid_t pid = fork();
if (pid == 0) write(fd, "child\n", 6);
else          write(fd, "parent\n", 7);
// 两个输出可能交错: "parchild\nent\n"

// ✅ 每个进程用独立的 fd 或注意同步
```

## 动手练习

### 🟢 练习 1: 基本 fork

创建一个子进程，父进程打印自己的 PID，子进程也打印自己的 PID，然后退出。

<details>
<summary>点击查看答案</summary>

```c
pid_t pid = fork();
if (pid == 0) {
    printf("Child: PID=%d, PPID=%d\n", getpid(), getppid());
    _exit(0);
} else {
    printf("Parent: my PID=%d, child PID=%d\n", getpid(), pid);
    waitpid(pid, NULL, 0);
}
```
</details>

### 🟡 练习 2: 用 exec 实现简易 shell

fork + execvp 实现：读取用户输入命令，执行它，等待完成，再读下一条。

<details>
<summary>点击查看答案</summary>

```c
char cmd[256];
while (1) {
    printf("myshell> ");
    if (!fgets(cmd, sizeof(cmd), stdin)) break;
    cmd[strcspn(cmd, "\n")] = '\0';
    
    char *argv[64];
    int argc = 0;
    char *token = strtok(cmd, " ");
    while (token && argc < 63) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
    
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        _exit(1);
    }
    waitpid(pid, NULL, 0);
}
```
</details>

### 🔴 练习 3: 多子进程协作

创建 3 个子进程分别计算素数筛选、斐波那契、阶乘，父进程收集结果。

<details>
<summary>点击查看答案</summary>

```c
int pipefd[2];
pipe(pipefd);

for (int i = 0; i < 3; i++) {
    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[0]);
        // 计算...
        int result = ...;
        write(pipefd[1], &result, sizeof(result));
        close(pipefd[1]);
        _exit(0);
    }
}
close(pipefd[1]);

for (int i = 0; i < 3; i++) {
    int result;
    read(pipefd[0], &result, sizeof(result));
    printf("Child %d result: %d\n", i + 1, result);
    wait(NULL);
}
```
</details>

## 故障排查

### Q: fork() 返回 -1

通常因为进程数超限。`ulimit -u` 检查进程数限制，`ps aux | wc -l` 看当前进程数。

### Q: 僵尸进程清理不掉

父进程已死而子进程变僵尸——此时子进程被 init (PID 1) 收养。如果 init 不回收，用 `kill` 无法杀死僵尸（它已经死了）。需要重启或等系统回收。

### Q: execvp 找不到命令

`execvp` 搜索 PATH 环境变量。如果命令不在 PATH 中，用绝对路径：`execv("/usr/bin/ls", argv)`。

## 知识扩展

### 1. 守护进程 (Daemon)

守护进程是后台运行的服务进程，没有控制终端：

```c
pid_t pid = fork();
if (pid > 0) exit(0);      // 父进程退出
setsid();                   // 创建新会话
chdir("/");                 // 更改工作目录
close(STDIN_FILENO);        // 关闭标准流
close(STDOUT_FILENO);
close(STDERR_FILENO);
```

### 2. 孤儿进程 (Orphan)

父进程先退出、子进程还在运行 → 子进程被 init 收养。孤儿进程不是问题——init 会回收。

### 3. vfork

`vfork()` 是 `fork()` 的轻量版本：子进程和父进程共享地址空间。子进程先运行，`exec` 或 `_exit` 后父进程才继续。用于 fork+exec 场景的性能优化。现代 `fork()` 已有写时共享（COW），`vfork` 逐渐弃用。

## 小结

- **fork** = 克隆自己——返回两次（子=0，父=子PID）
- **exec** = 变身——替换进程映像，成功后不返回
- **wait/waitpid** = 收尸——回收子进程，防止僵尸
- **_exit** = 子进程退出（不用 exit）
- 每个 fork 配一个 wait，否则出现僵尸进程

> **我的教训**：第一次写进程管理时，我忘记 wait，程序跑了几个小时后有几百个僵尸进程。记住：**每个 fork 配 wait**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 进程（Process） | 运行中的程序实例 |
| 父/子进程（Parent/Child） | fork 创建的进程关系 |
| 僵尸进程（Zombie） | 子进程已退出但父未 wait |
| 孤儿进程（Orphan） | 父进程已退出，被 init 收养 |
| 写时复制（Copy-on-Write） | fork 后内存共享，写时拷贝 |
| 进程映像（Process Image） | 进程的代码、数据、堆栈总和 |

## 延伸阅读

- [POSIX fork(2)](https://pubs.opengroup.org/onlinepubs/9699919799/functions/fork.html) — 官方规范
- [Beej's Guide to Unix IPC](https://beej.us/guide/bgipc/) — 进程间通信
- [Advanced Programming in the UNIX Environment](https://en.wikipedia.org/wiki/Advanced_Programming_in_the_UNIX_Environment) — 第 8 章

## 继续学习

你已经学会了如何创建和管理进程。接下来，我们将探索进程之间如何通信——管道和 Unix socket。

> 💡 **提示**：运行 `src/advance/system_process_sample.c` 查看所有演示。`make build && make run`。

[← 上一章：内存映射 I/O](./mmap.md) | [下一章：管道与 IPC →](./ipc.md)
