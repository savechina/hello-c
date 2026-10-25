# Unix 环境编程 (Unix Environment Programming)

> "进程不是活在真空里——它出生时带着一张入职信息条（environ），挂着一张工牌（uid），受一栋写字楼（会话）的规矩约束。学会读这些信息，才算真正住进了 Unix。"

## 开篇故事

你写了一个工具，跑在自己机器上好好的，发给同事就崩了。原因？你假设了 `/tmp` 一定可写、假设了 PATH 里一定有某个目录、假设了自己是普通用户。

Unix 给了你一整套"问系统"的接口：`getenv()` 问环境，`sysconf()` 问内核限制，`getuid()` 问身份，`getpwuid()` 问账号档案。**不要猜，要问。**

这一章把散落在 `unistd.h`、`stdlib.h`、`pwd.h` 里的环境编程接口串成一条线：进程从环境变量里继承什么、用什么身份在运行、受哪些系统限制约束、出错后怎么报告。

## 本章适合谁

- 写命令行工具，想正确处理选项和环境变量的人
- 好奇 `uid` 和 `euid` 为什么是两个值的人
- 想让程序在 macOS/Linux/FreeBSD/Solaris 上都能跑的人
- 被 `errno`、`strerror`、`perror` 绕晕过的人

## 你会学到什么

- `environ` / `getenv()` — 环境变量的读取与安全边界
- `getopt()` — POSIX 短选项解析
- `getuid/geteuid/getgid` + `struct passwd` — 进程身份
- 进程组 / 会话 — 信号分发的层级结构
- `sysconf()` / `pathconf()` — 运行时查询系统限制
- `errno` / `strerror` — 错误报告的正确姿势

## 前置要求

- 理解进程与 `main(argc, argv)` 的关系
- 知道指针与字符串基础
- 了解条件编译 `#ifdef` 的作用

## 第一个例子

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char **environ;  /* NULL 结尾的 "KEY=VALUE" 数组 */

int main(void) {
    printf("PATH = %s\n", getenv("PATH"));
    printf("HOME = %s\n", getenv("HOME"));

    for (char **e = environ; *e != NULL; e++) {
        printf("%s\n", *e);  /* 遍历全部环境变量 */
    }
    printf("open files max = %ld\n", sysconf(_SC_OPEN_MAX));
    return 0;
}
```

`getenv()` 读单个，`environ` 遍历全部，`sysconf()` 问内核——三个"问系统"的入口。

## 原理解析

### 1. environ / getenv — 环境变量

```c
extern char **environ;

int count = 0;
while (environ[count] != NULL) {
  count++;
}
/* 只打印名字, 避免泄漏密钥 */
const char *eq = strchr(environ[i], '=');
printf("[%2d] %.*s\n", i, (int)(eq - environ[i]), environ[i]);

const char *val = getenv("HOME");  /* 未设置返回 NULL! */
```

```
  environ (char *[])
    │
    ├─[0] "PATH=/usr/bin:/bin"
    ├─[1] "HOME=/Users/you"
    ├─[2] "LANG=zh_CN.UTF-8"
    └─ ...
         NULL 结尾
```

| 变量 | 含义 |
|------|------|
| `PATH` | 可执行文件搜索路径（冒号分隔） |
| `HOME` | 用户主目录 |
| `SHELL` | 登录 shell 路径 |
| `LANG` / `LC_ALL` | locale：排序、编码、大小写 |
| `TMPDIR` | 临时目录 |
| `PWD` / `OLDPWD` | 当前/上一个工作目录（shell 维护） |

**安全警示（security caveats）**：

- **环境变量注入** — 攻击者可控的 env 会操纵 setuid 程序行为
- **`LD_PRELOAD` / `LD_LIBRARY_PATH`** — 动态链接器会加载指定库；setuid 二进制必须忽略它们，否则等于让攻击者注入代码
- **Trojan source（木马源）** — 不可信输入伪装成可信配置混入 env
- **IFS 劫持** — shell 按 IFS 分词，篡改后命令被拆错
- 安全决策不要直接信 `getenv()`（如 `HTTP_PROXY`）——先白名单校验

### 2. getopt — 选项解析

```c
char *fake_argv[] = {"mytool", "-v", "-n", "3", "-o", "out.txt",
                     "input.txt", NULL};
int fake_argc = (int)(sizeof(fake_argv) / sizeof(fake_argv[0])) - 1;
int verbose = 0;
long count = 1;
const char *output = NULL;

optind = 1;  /* 重置解析器状态 */

int ch;
while ((ch = getopt(fake_argc, fake_argv, "vn:o:h")) != -1) {
  switch (ch) {
    case 'v': verbose = 1; break;
    case 'n': {
      char *end = NULL;
      errno = 0;
      long v = strtol(optarg, &end, 10);
      if (errno != 0 || end == optarg || *end != '\0' || v <= 0) {
        fprintf(stderr, "  [错误] -n 需要正整数\n");
      } else {
        count = v;
      }
      break;
    }
    case 'o': output = optarg; break;
    case 'h': printf("用法: mytool [-v] [-n count] [-o file]\n"); break;
    case '?': default: /* 未知选项 — 必须处理 */ break;
  }
}
/* 剩余位置参数: argv[optind .. argc-1] */
```

optstring `"vn:o:h"` 规则：

| 片段 | 含义 |
|------|------|
| `v` | 开关选项，不带参数 |
| `n:` | 必需参数（`-n 3`） |
| `o:` | 必需参数（`-o file`） |
| `?` | 未知选项 / 缺少必需参数 |

### 3. 进程身份 — uid/euid/passwd

```c
uid_t uid = getuid();    /* 真实身份: 谁启动了我 */
uid_t euid = geteuid();  /* 当前权限: access check 用它 */
gid_t gid = getgid();
gid_t egid = getegid();

struct passwd *pw = getpwuid(uid);       /* uid → 档案 */
if (pw != NULL) {
  printf("pw_name=%s pw_dir=%s pw_shell=%s\n",
         pw->pw_name, pw->pw_dir, pw->pw_shell);
}

struct passwd *root = getpwnam("root");  /* 名字 → 档案 */
```

```
  setuid 程序的两张卡:
    real uid (工牌)     — 永远是启动者, 决定"你是谁"
    effective uid (门禁) — 决定"你现在能干什么"

    root 启动 → seteuid(普通用户) 干活 → seteuid(0) 再提权
```

| 函数 | 作用 |
|------|------|
| `setuid(uid)` | 同时设置 real/effective/saved uid |
| `seteuid(euid)` | 只改 effective uid，可来回切换 |
| `setreuid(r,e)` | 分别设置 real 与 effective |

⚠ `getpwuid`/`getpwnam` 返回**静态缓冲区**——多线程改用 `getpwuid_r`/`getpwnam_r`。

### 4. 进程组与会话

```
  会话 (session) ── 最多一个 controlling terminal
    └── 进程组 (process group) ── 信号分发单位
          └── 进程 (process)
```

```c
printf("pid=%d pgrp=%d sid=%d\n",
       (int)getpid(), (int)getpgrp(), (int)getsid(getpid()));
```

- `setsid()` 创建新会话 + 新进程组，调用者成为组长（**daemon 第一步**）
- 终端关闭 → `SIGHUP` 发给前台进程组
- `Ctrl+C`（`SIGINT`）发给整个前台进程组——组内成员全被中断
- 本章演示**只读不改**：不调用 `setsid()`，避免脱离控制终端

### 5. sysconf / pathconf — 系统限制

```c
long open_max = sysconf(_SC_OPEN_MAX);   /* 每进程最大 fd 数 */
long page_size = sysconf(_SC_PAGESIZE);  /* 内存页大小 */

long name_max = pathconf("/", _PC_NAME_MAX);  /* 文件名最大长度 */
long path_max = pathconf("/", _PC_PATH_MAX);  /* 路径最大长度 */

if (value < 0) {
  /* -1 = 无限制 或 不确定 — 不一定是错误 */
}
```

| 查询 | 含义 |
|------|------|
| `_SC_OPEN_MAX` | 每个进程可打开的最大 fd 数 |
| `_SC_PAGESIZE` | 内存页大小（字节） |
| `_SC_CLK_TCK` | 每秒时钟滴答数 |
| `_SC_ARG_MAX` | exec 参数 + 环境最大字节数 |
| `_PC_NAME_MAX` | 该文件系统上文件名最大长度（`NAME_MAX`） |
| `_PC_PATH_MAX` | 该文件系统上路径最大长度（`PATH_MAX`） |

传统宏 `NAME_MAX`/`PATH_MAX` 是**编译期猜测**，可能未定义或过时；不同文件系统限制不同，用 `pathconf` 按路径查询最准。

### 6. errno / strerror — 错误报告

```c
errno = 0;                                   /* 调用前清零 */
FILE *fp = fopen("/nonexistent/x.txt", "r"); /* 失败返回 NULL */
if (fp == NULL) {
  printf("errno=%d (%s)\n", errno, strerror(errno));
}
```

常见 errno 速查（数值随平台略有差异）：

| 宏 | 含义 |
|----|------|
| `EPERM` | 操作不允许 |
| `ENOENT` | 文件/路径不存在 |
| `EINTR` | 被信号中断，可重试 |
| `EAGAIN` | 暂不可用，重试（`EWOULDBLOCK`） |
| `EACCES` | 权限拒绝 |
| `EINVAL` | 参数无效 |
| `EPIPE` | 管道破裂（`SIGPIPE`） |
| `ERANGE` | 数值超出范围 |
| `ENOSYS` | 功能未实现 |

使用规则：

1. **只在失败后读 errno**——成功时它可能残留旧值
2. 调用前先 `errno = 0`，才能区分"失败且值为 0"与"没失败"
3. `strerror` **不是线程安全的**——多线程用 `strerror_r`
4. `perror(prefix)` 快捷打印 `"prefix: <strerror(errno)>\n"`

## 常见错误

### ❌ 错误 1: getenv 返回 NULL 直接用

```c
// ❌ 变量未设置时 segfault
printf("%s\n", getenv("MY_VAR"));

// ✅ 判 NULL
const char *v = getenv("MY_VAR");
printf("%s\n", v != NULL ? v : "(未设置)");
```

### ❌ 错误 2: 不处理 getopt 的 '?'

```c
// ❌ 非法选项被静默吞掉, 用户不知道输错了
while ((ch = getopt(argc, argv, "vo:n:")) != -1) { ... }

// ✅ 必须处理 '?', 且 optarg 用前判 NULL
case '?':
  fprintf(stderr, "未知选项, 用 -h 查看帮助\n");
  return EXIT_FAILURE;
```

### ❌ 错误 3: 把 uid 当权限用

```c
// ❌ 真实 uid 是 root 不代表现在有 root 权限
if (getuid() == 0) { /* 错! */ }

// ✅ 权限检查用 effective uid
if (geteuid() == 0) { /* 现在确实有 root 权限 */ }
```

### ❌ 错误 4: 成功后也读 errno

```c
// ❌ fopen 成功但 errno 残留着上一次的旧值
FILE *f = fopen("ok.txt", "r");
printf("%d\n", errno);  // 可能打印一个无关的错误码!

// ✅ 先清零, 只在失败分支读
errno = 0;
FILE *f = fopen("ok.txt", "r");
if (f == NULL) { /* 这里读 errno 才有意义 */ }
```

## 平台兼容性

本章代码只使用四个目标平台的**公共交集**（`unistd.h`、`stdlib.h`、`string.h`、`pwd.h`），全部 API 均为 POSIX 标准：

| 知识点 / API | macOS (clang) | Linux (gcc) | FreeBSD | Solaris/illumos |
|--------------|:-------------:|:-----------:|:-------:|:---------------:|
| `environ` / `getenv()` | ✅ | ✅ | ✅ | ✅ |
| `getopt()`（短选项） | ✅ | ✅ | ✅ | ✅ |
| `getuid/geteuid/getgid` | ✅ | ✅ | ✅ | ✅ |
| `getpwuid` / `getpwnam`（`<pwd.h>`） | ✅ | ✅ | ✅ | ✅ |
| `getpgrp` / `getsid` | ✅ | ✅ | ✅ | ✅ |
| `sysconf()`（`_SC_*`） | ✅ | ✅ | ✅ | ✅ |
| `pathconf()`（`_PC_*`） | ✅ | ✅ | ✅ | ✅ |
| `errno` / `strerror` | ✅ | ✅ | ✅ | ✅ |
| `getopt_long`（长选项） | ✅ | ✅ | ✅ | ✅（需 `<getopt.h>`） |

注意事项：

- **errno 数值不同**：`EAGAIN` 在 macOS/FreeBSD 是 35，在 Linux 是 11——永远用宏名，不要硬编码数字
- **限制值不同**：`_SC_OPEN_MAX`、`_PC_PATH_MAX` 随平台和文件系统变化——运行时查询，不要写死
- **Windows 不支持**：本章所有 POSIX 接口在 `_WIN32` 下编译期跳过（打印"不支持"）
- Linux 下构建需 `-D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE`（本项目 Makefile 已配置）

## 动手练习

### 🟢 练习 1: 环境变量检查器

写一个程序：读取 `MY_APP_CONFIG` 环境变量，未设置时打印帮助并以退出码 2 退出。

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  const char *cfg = getenv("MY_APP_CONFIG");
  if (cfg == NULL) {
    fprintf(stderr, "错误: 未设置 MY_APP_CONFIG\n");
    fprintf(stderr, "用法: MY_APP_CONFIG=/path/to/cfg ./app\n");
    return 2;
  }
  printf("config = %s\n", cfg);
  return 0;
}
```
</details>

### 🟡 练习 2: 完整 getopt 工具

实现 `mytool [-v] [-n count] [-o file] [input...]`：`-n` 必须是正整数（非法时报错退出码 2），`-o` 的文件名判 NULL，最后打印解析结果与剩余参数。

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int verbose = 0;
  long count = 1;
  const char *output = NULL;
  int ch;

  while ((ch = getopt(argc, argv, "vn:o:h")) != -1) {
    switch (ch) {
      case 'v': verbose = 1; break;
      case 'n':
        count = strtol(optarg, NULL, 10);
        if (count <= 0) {
          fprintf(stderr, "-n 需要正整数\n");
          return 2;
        }
        break;
      case 'o': output = optarg; break;
      case 'h':
        printf("用法: %s [-v] [-n count] [-o file] [input...]\n", argv[0]);
        return 0;
      default: return 2;
    }
  }

  printf("verbose=%d count=%ld output=%s\n", verbose, count,
         output ? output : "(未指定)");
  for (int i = optind; i < argc; i++) {
    printf("operand: %s\n", argv[i]);
  }
  return 0;
}
```
</details>

### 🔴 练习 3: 限制查询报告

写一个程序：用 `sysconf` 打印 5 项系统限制，用 `pathconf` 打印 `/tmp` 所在文件系统的 `NAME_MAX` 和 `PATH_MAX`，对返回 -1 的项打印"无限制/不确定"。

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <unistd.h>

static void show(const char *label, long v) {
  if (v < 0) {
    printf("%-18s = (无限制/不确定)\n", label);
  } else {
    printf("%-18s = %ld\n", label, v);
  }
}

int main(void) {
  show("_SC_OPEN_MAX", sysconf(_SC_OPEN_MAX));
  show("_SC_PAGESIZE", sysconf(_SC_PAGESIZE));
  show("_SC_CLK_TCK", sysconf(_SC_CLK_TCK));
  show("_SC_ARG_MAX", sysconf(_SC_ARG_MAX));
  show("_SC_NGROUPS_MAX", sysconf(_SC_NGROUPS_MAX));
  show("_PC_NAME_MAX(/tmp)", pathconf("/tmp", _PC_NAME_MAX));
  show("_PC_PATH_MAX(/tmp)", pathconf("/tmp", _PC_PATH_MAX));
  return 0;
}
```
</details>

## 故障排查

### Q: `getenv()` 总是返回 NULL

原因：变量确实没设置。用 `env` 命令确认；子进程默认**继承**父进程环境，但 `execve` 可以传自定义 `envp`。

### Q: `getopt` 报 "option requires an argument"

原因：optstring 里写了 `o:`（冒号 = 必需参数），但调用时 `-o` 后面没跟值。检查是 `-o file` 还是 `-o` 单独出现。

### Q: `getpwuid()` 多线程下数据错乱

原因：返回静态缓冲区，下一次调用会覆盖。多线程用 `getpwuid_r()`，由调用方提供缓冲区。

### Q: `sysconf(_SC_OPEN_MAX)` 返回 -1

原因：该限制无上限（unlimited）或不确定——**不是错误**。POSIX 允许 -1 且不保证设置 errno。

## 小结

- **`environ`/`getenv()`** — 环境变量是继承来的攻击面：`LD_PRELOAD`、`IFS` 都能被注入
- **`getopt()`** — 短选项解析三要素：optstring、`optarg` 判 NULL、处理 `'?'`
- **uid vs euid** — 工牌 vs 门禁卡；权限检查永远看 `geteuid()`
- **会话 > 进程组 > 进程** — 信号按组分发，`setsid()` 是 daemon 第一步
- **`sysconf`/`pathconf`** — 不要硬编码限制值，运行时问；-1 ≠ 错误
- **`errno`** — 只在失败后读，先清零；多线程用 `strerror_r`

> **我的教训**：第一次写 CLI 工具时，我用 `getenv("HOME")` 拼临时文件路径却没判 NULL——在一台没设 `HOME` 的 CI 容器里直接段错误。记住：**`getenv()` 的返回值永远可能为 NULL**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 环境变量（Environment Variable） | 进程启动时继承的 KEY=VALUE 键值对 |
| 有效用户 ID（Effective UID） | 决定当前权限的真实 uid |
| 口令文件（Password File） | `/etc/passwd`，`struct passwd` 的数据源 |
| 进程组（Process Group） | 信号分发的单位，同组进程共享前台 |
| 会话（Session） | 进程组的上级，最多一个控制终端 |
| 系统限制（System Limits） | 内核/文件系统的可配置上限 |
| 错误号（errno） | 线程局部的错误码全局变量 |

## 延伸阅读

- [POSIX getenv(3)](https://pubs.opengroup.org/onlinepubs/9699919799/functions/getenv.html) — 官方规范
- [POSIX getopt(3)](https://pubs.opengroup.org/onlinepubs/9699919799/functions/getopt.html) — 选项解析规范
- [Beej's Guide to Unix Programming](https://beej.us/guide/bgipc/) — Unix 编程指南
- *Advanced Programming in the UNIX Environment* (Stevens & Rago) — 第 1、3、4、18 章

## 继续学习

你已经掌握了"问系统"的全套接口——环境、身份、限制、错误。接下来探索内核暴露给用户态的更底层接口：系统调用的边界与封装。

> 💡 **提示**：运行 `src/advance/unix_env_sample.c` 查看所有演示。`make build && make run`。
