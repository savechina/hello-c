# 文件与目录操作 (File and Directory Operations)

> "文件描述符像房间的钥匙——open() 拿到钥匙，read/write 进出房间，close() 还钥匙。如果钥匙不还，房间就会越来越不够用。"

## 开篇故事

我住在一栋大型公寓楼里。大楼管理员（操作系统）负责管理所有房间（文件）。想进房间？你得先找管理员拿钥匙（`open()`）。拿到钥匙后，你可以在房间里读东西（`read()`）或放东西（`write()`）。用完之后，必须把钥匙还给管理员（`close()`），否则其他人就没钥匙可用了。

如果你想查看某个房间的状态——有多大、谁建的、什么时候装修的——你可以查房产登记（`stat()`）。如果你想看整栋楼有哪些房间，你拿着一份楼层表挨个查看（`opendir()` + `readdir()`），就像保安巡逻。

## 本章适合谁

- 学过 `fopen/fprintf/fclose`，但想知道"底层到底发生了什么"的人
- 听说过"文件描述符"但不知道它和文件指针区别的人
- 想写系统工具（文件管理器、日志轮转、目录扫描器）的人
- 好奇"为什么一切皆文件"的人

## 你会学到什么

- 文件描述符（File Descriptor）—— `open()`、`read()`、`write()`、`close()`
- `fcntl()` 文件控制——设置文件状态标志
- `stat()` 文件元数据——大小、权限、时间戳
- `opendir()` / `readdir()` 目录扫描——像 `ls` 一样遍历目录
- 标准文件描述符：STDIN(0)、STDOUT(1)、STDERR(2)

## 前置要求

- 理解指针和基本数据类型
- 知道 stdio (`fopen`/`fclose`) 的基本用法
- 理解 `errno` 错误码模式
- 理解路径概念（绝对路径 vs 相对路径）

## 第一个例子

```c
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    /* 拿到钥匙 */
    int fd = open("/tmp/hello.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open failed");
        return 1;
    }

    /* 走进房间放东西 */
    const char *msg = "Hello, System Call!\n";
    write(fd, msg, strlen(msg));

    /* 还钥匙 */
    close(fd);
    return 0;
}
```

四步走：`open → write → close`——和 `fopen → fprintf → fclose` 思路一样，但更底层、更直接。

## 原理解析

### 1. 文件描述符 (File Descriptors)

Linux/macOS 中，**一切皆文件**。键盘、鼠标、网络套接字、普通文件——全部用整数 fd 表示。

```
标准 fd:
  STDIN_FILENO  = 0  (标准输入 — 键盘)
  STDOUT_FILENO = 1  (标准输出 — 屏幕)
  STDERR_FILENO = 2  (标准错误 — 屏幕)

自定义:
  open() 返回 ≥ 3 的整数（最小的可用 fd）
```

```c
int fd = open("/tmp/test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
// fd = 3 (假设 0,1,2 已被标准流占用)

write(fd, "hello", 5);  // 向 fd=3 写 5 字节
read(fd, buf, 10);      // 从 fd=3 读 10 字节（需 O_RDWR）
close(fd);              // 归还 fd=3
```

**我的理解**：fd 就是"钥匙编号"。0、1、2 是标配的三把钥匙，`open()` 给你第四把、第五把……用完必须 `close()` 归还，否则钥匙不够用（`EMFILE` 错误）。

### 2. open() 的打开模式

```c
// 只读 — 只能 read
open("file.txt", O_RDONLY);

// 只写 — 只能 write, 创建或覆盖
open("file.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

// 追加 — 只能 write, 每次从末尾写入
open("file.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);

// 读写 — 可以 read 和 write
open("file.txt", O_RDWR | O_CREAT, 0644);
```

| 标志 | 含义 |
|------|------|
| `O_RDONLY` | 只读 |
| `O_WRONLY` | 只写 |
| `O_RDWR` | 读写 |
| `O_CREAT` | 文件不存在则创建（需要 mode 参数） |
| `O_TRUNC` | 存在则截断为 0 |
| `O_APPEND` | 每次写入追加到文件末尾 |

### 3. stat() — 文件元数据

```c
#include <sys/stat.h>

struct stat st;
stat("/tmp/test.txt", &st);

printf("大小: %lld\n", (long long)st.st_size);     // 文件大小（字节）
printf("类型: %s\n", S_ISREG(st.st_mode) ? "普通" : "目录");
printf("权限: %04o\n", st.st_mode & 07777);        // 如 0644
printf("修改: %lld\n", (long long)st.st_mtime);
```

`stat` 不打开文件——它查的是文件系统 inode 里的信息。即使你无权读取文件内容，也能 `stat`。

### 4. opendir / readdir — 目录扫描

```c
#include <dirent.h>

DIR *dir = opendir("/tmp");
struct dirent *ent;
while ((ent = readdir(dir)) != NULL) {
    printf("%s\n", ent->d_name);  // 文件名
    // ent->d_type: DT_DIR, DT_REG, DT_LNK, ...
}
closedir(dir);
```

`opendir` 打开目录流，`readdir` 逐个返回目录条目，`closedir` 关闭。这就像 `ls` 命令的核心逻辑。

### 5. fcntl — 文件控制

```c
#include <fcntl.h>

int flags = fcntl(fd, F_GETFL);        // 获取文件状态标志
fcntl(fd, F_SETFL, flags | O_APPEND);  // 追加设置 O_APPEND 标志
```

`fcntl` 是文件描述符的"万能遥控器"——获取或修改文件状态、设置文件锁、复制 fd 等。

## 常见错误

### ❌ 错误 1: 不检查 open 返回值

```c
// ❌ open 失败返回 -1，后续 write 全部失败
int fd = open("/nonexistent", O_RDONLY);
write(fd, buf, 10);  // fd=-1，无意义操作

// ✅ 检查返回值
int fd = open("/nonexistent", O_RDONLY);
if (fd < 0) {
    perror("open");
    return 1;
}
```

### ❌ 错误 2: 忘记 close 导致 fd 泄漏

```c
// ❌ 循环中打开但忘记关闭
for (int i = 0; i < 10000; i++) {
    int fd = open("test.txt", O_RDONLY);  // fd 泄漏
    // ... do something ...
}
// 最终 open 返回 -1 (EMFILE: Too many open files)

// ✅ 配对 close
for (int i = 0; i < 10000; i++) {
    int fd = open("test.txt", O_RDONLY);
    // ... do something ...
    close(fd);  // 归还钥匙
}
```

### ❌ 错误 3: read/write 不检查返回值

```c
// ❌ write 可能只写入部分数据
write(fd, big_buf, big_len);  // 可能只写了部分!

// ✅ 检查并处理
ssize_t nw = write(fd, big_buf, big_len);
if (nw < 0) {
    perror("write");
} else if ((size_t)nw < big_len) {
    // 部分写入 — 需要补写剩余部分
}
```

### ❌ 错误 4: stat 前不检查路径是否存在

```c
// ❌ 如果路径不存在，st 结构的内容是未定义的
struct stat st;
stat("/nonexistent", &st);
printf("大小: %lld\n", (long long)st.st_size);  // 垃圾值!

// ✅ 检查返回值
if (stat("/nonexistent", &st) == 0) {
    printf("大小: %lld\n", (long long)st.st_size);
} else {
    perror("stat");
}
```

## 动手练习

### 🟢 练习 1: 文件描述符复制

用 `open`/`write`/`read`/`close` 复制一个文件（从 `src/data.txt` 复制到 `src/copy.txt`）。

<details>
<summary>点击查看答案</summary>

```c
int src = open("data.txt", O_RDONLY);
int dst = open("copy.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
char buf[4096];
ssize_t n;
while ((n = read(src, buf, sizeof(buf))) > 0) {
    write(dst, buf, (size_t)n);
}
close(src);
close(dst);
```
</details>

### 🟡 练习 2: stat 文件信息

写一个 `my_stat` 函数，接收文件路径，打印文件类型、大小、权限、修改时间。

<details>
<summary>点击查看答案</summary>

```c
void my_stat(const char *path) {
    struct stat st;
    if (stat(path, &st) < 0) {
        perror("stat");
        return;
    }
    printf("类型: %s\n", S_ISDIR(st.st_mode) ? "目录" : "文件");
    printf("大小: %lld\n", (long long)st.st_size);
    printf("权限: %04o\n", st.st_mode & 07777);
    printf("mtime: %lld\n", (long long)st.st_mtime);
}
```
</details>

### 🔴 练习 3: 递归目录扫描

写一个递归函数遍历目录树，打印所有文件的绝对路径和大小。

<details>
<summary>点击查看答案</summary>

```c
void scan_dir(const char *path, int depth) {
    DIR *dir = opendir(path);
    if (!dir) return;
    
    struct dirent *ent;
    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
            continue;
        
        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", path, ent->d_name);
        
        struct stat st;
        stat(full, &st);
        for (int i = 0; i < depth; i++) printf("  ");
        printf("%s (%lld bytes)\n", full, (long long)st.st_size);
        
        if (S_ISDIR(st.st_mode)) scan_dir(full, depth + 1);
    }
    closedir(dir);
}
```
</details>

## 故障排查

### Q: open() 返回 -1，errno = 13 (EACCES)

**权限不足**。用 `ls -la filename` 检查文件权限，确认你有读/写权限。

### Q: open() 返回 -1，errno = 2 (ENOENT)

**文件不存在**。如果用 `O_CREAT`，确保提供了 mode 参数（如 `0644`）。

### Q: "Too many open files" (EMFILE)

进程打开了太多文件描述符。检查是否有 `open` 没有配对 `close`。`ulimit -n` 查看当前限制。

### Q: stat() 和 lstat() 区别？

`stat()` 会跟符号链接到目标文件；`lstat()` 查的是符号链接本身。判断一个路径是否是软链接用 `lstat()`。

## 知识扩展

### 1. O_DIRECT / O_SYNC

- `O_SYNC`: 每次 write 都等待磁盘确认（慢但安全，适合数据库日志）
- `O_DIRECT`: 绕过内核缓冲区，直接磁盘 I/O（数据库常用，减少内存占用）

### 2. /dev/null, /dev/zero, /dev/urandom

```c
// Linux/macOS 的特殊文件
int dev_null = open("/dev/null", O_WRONLY);   // 丢弃所有写入
int dev_zero = open("/dev/zero", O_RDONLY);   // 永远返回 \x00
int dev_rand = open("/dev/urandom", O_RDONLY); // 随机数
```

### 3. 软链接 vs 硬链接

```bash
ln target link     # 硬链接 — 同一个 inode，两个名字
ln -s target link  # 软链接 — 独立文件，内容是目标路径
```

硬链接不能跨文件系统、不能链接目录。软链接可以。

## 小结

- **文件描述符** = 钥匙编号（0,1,2 是标准流）
- **open → read/write → close** 是底层 I/O 的基本模式
- **stat** 查文件元数据（大小、权限、时间），不打开文件
- **opendir/readdir** 扫描目录内容
- 每个 `open()` 必须配对 `close()`，否则 fd 泄漏

> **我的教训**：第一次写文件操作时，我忘了 `close()`，程序运行几小时后 `open` 全部返回 -1。记住：**每个 open 配 close**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 文件描述符（File Descriptor） | 整数，标识已打开的文件 |
| 标准输入/输出/错误（STDIN/STDOUT/STDERR） | fd 0/1/2 |
| 文件元数据（File Metadata） | 大小、权限、时间戳等 inode 信息 |
| 目录条目（Directory Entry） | `dirent` 结构，含文件名和类型 |
| 硬链接（Hard Link） | 同一 inode 的多个名称 |
| 软链接（Symbolic Link） | 指向目标路径的特殊文件 |

## 延伸阅读

- [Beej's Guide to Unix IPC](https://beej.us/guide/bgipc/) — 文件描述符深入
- [POSIX open(2) 手册](https://pubs.opengroup.org/onlinepubs/9699919799/functions/open.html) — 官方规范
- [Advanced Programming in the UNIX Environment (APUE)](https://en.wikipedia.org/wiki/Advanced_Programming_in_the_UNIX_Environment) — 经典教材

## 继续学习

你已经掌握了低层文件 I/O 的基本模式。接下来，我们将探索如何**响应外部事件**——POSIX 信号处理，让程序能优雅地对 Ctrl+C 等事件作出反应。

> 💡 **提示**：运行 `src/advance/system_file_sample.c` 查看所有演示。`make build && make run`。

[← 上一章：系统调用总览](../system.md) | [下一章：信号处理 →](./signal.md)
