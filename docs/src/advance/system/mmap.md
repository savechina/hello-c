# 内存映射 I/O (Memory-Mapped I/O)

> "mmap 像给文件开了扇窗——不用走门（syscall），直接就能看到里面的东西。传统 read/write 是敲门等管理员开门；mmap 是直接把墙打通。"

## 开篇故事

想象你要从房间搬运一箱书。传统方式是：敲门（syscall）→ 管理员开门 → 你把一箱箱书搬出来（read）→ 再把书放到另一个房间（write）→ 敲门（syscall）→ 管理员开门 → 完成。

mmap 的方式是：直接把这面墙打通（mmap），书箱变成房间里的一部分。你伸手就能拿到书（直接访问内存指针），放回去也是直接移动（直接写内存指针）。没有敲门，没有等待——文件内容就像内存一样。

但要注意：打通的墙大小是固定的（文件大小）。如果你试图超出范围拿书——砰！段错误（SIGSEGV）。

## 本章适合谁

- 写过 `fopen/fread/fwrite`，但觉得"每次都要调用函数太麻烦"的人
- 听说过 mmap 很快但不知道原理的人
- 想理解"零拷贝"（zero-copy）概念的人
- 好奇"为什么数据库用 mmap 加速"的人

## 你会学到什么

- `mmap()` — 将文件映射到内存地址空间
- `munmap()` — 取消映射
- `PROT_READ` / `PROT_WRITE` — 保护标志
- `MAP_SHARED` vs `MAP_PRIVATE` — 共享还是私有不写回
- `msync()` — 强制同步到磁盘
- 性能对比 mmap vs read/write

## 前置要求

- 理解指针和内存地址
- 理解文件描述符和基本 I/O
- 了解虚拟内存基本概念
- 会 `open`/`close` 基本用法

## 第一个例子

```c
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    /* 创建并准备文件 */
    int fd = open("/tmp/mmap_test.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
    ftruncate(fd, 256);  /* 设置文件大小为 256 字节 */

    /* 映射文件到内存 */
    void *mapped = mmap(NULL, 256, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    /* 直接通过指针写文件内容 */
    memcpy(mapped, "Hello mmap!", 11);

    printf("映射内容: %s\n", (char *)mapped);

    /* 写回磁盘并取消映射 */
    msync(mapped, 256, MS_SYNC);
    munmap(mapped, 256);
    close(fd);
    return 0;
}
```

关键：`memcpy` 而不是 `write`——直接操作内存指针！

## 原理解析

### 1. mmap 参数详解

```c
void *mmap(void *addr,    /* 推荐 NULL（让系统选地址）*/
           size_t len,    /* 映射长度（字节）*/
           int prot,      /* 保护模式 */
           int flags,     /* 映射类型 */
           int fd,        /* 文件描述符 */
           off_t offset); /* 偏移（必须是页大小整数倍）*/
```

| prot 标志 | 含义 |
|-----------|------|
| `PROT_NONE` | 不可访问 |
| `PROT_READ` | 可读 |
| `PROT_WRITE` | 可写 |
| `PROT_EXEC` | 可执行 |

| flags 标志 | 含义 |
|-----------|------|
| `MAP_SHARED` | 修改写回文件，其他映射进程可见 |
| `MAP_PRIVATE` | 写时复制，不写回文件 |
| `MAP_ANONYMOUS` | 不关联文件（匿名映射，等价于 malloc） |
| `MAP_FIXED` | 尝试在 addr 指定地址映射（危险） |

### 2. 读取文件

```c
int fd = open("data.bin", O_RDONLY);
void *m = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);

/* read-like */
char first_byte = ((char *)m)[0];
printf("First byte: %c\n", first_byte);

/* memcmp 直接比较 */
if (memcmp(m, "HEADER", 6) == 0) {
    printf("Valid header!\n");
}

munmap(m, size);
close(fd);
```

### 3. 写入文件

```c
int fd = open("data.bin", O_RDWR);
ftruncate(fd, 1024);  /* 确保文件足够大 */

void *m = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

/* write-like */
memcpy(m, "Hello, World!", 13);
((char *)m)[13] = '\n';

/* msync 强制同步 */
msync(m, 1024, MS_SYNC);  // MS_SYNC = 同步写回

munmap(m, 1024);
close(fd);
```

### 4. MAP_SHARED vs MAP_PRIVATE

```c
/* MAP_SHARED — 多个进程共享修改 */
void *m = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
// 修改 → 其他进程可见 → 写回磁盘

/* MAP_PRIVATE — 写时复制 */
void *m = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
// 修改 → 私有副本 → 不写回磁盘 → munmap 后修改丢失
```

**类比**：
- `MAP_SHARED` = 公共白板（大家都写同一面墙）
- `MAP_PRIVATE` = 复印一份再修改（修改不影响原件）

### 5. 性能比较：mmap vs read/write

| 场景 | mmap 优势 | 原因 |
|------|-----------|------|
| 大文件随机访问 | ✅ 快 | 零拷贝，直接指针访问 |
| 大文件顺序读写 | ≈ 差不多 | 内核页缓存加速了 read |
| 小文件 (<4KB) | ❌ 慢 | 映射开销 > 直接读写 |
| 多进程共享数据 | ✅ 快 | 共享内存，无拷贝 |
| 频繁部分更新 | ✅ 快 | 只修改需要的字节 |

**核心优势**：mmap 消除了"文件→内核缓冲区→用户缓冲区"的拷贝。文件内容直接在进程的页表中，访问就像访问内存。

## 常见错误

### ❌ 错误 1: 越界写入

```c
void *m = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
memcpy(m + 200, "overflow", 8);  // mmap 的是 100 字节！
// → SIGSEGV (Segmentation Fault)

// ✅ 严格跟踪 mapped_size
if (offset + len > mapped_size) {
    fprintf(stderr, "Out of bounds!\n");
    return -1;
}
```

### ❌ 错误 2: 文件大小为 0 却映射

```c
int fd = open("empty.txt", O_RDWR);
// 文件大小 = 0
void *m = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
// → mmap 成功但写入时 SIGBUS!

// ✅ 先设置文件大小
ftruncate(fd, 100);  // 设置文件大小
void *m = mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
// 现在安全
```

### ❌ 错误 3: 忘记 msync

```c
void *m = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
memcpy(m, data, size);
munmap(m, size);  // 修改可能被延迟写，直接 munmap → 数据丢失!

// ✅ munmap 前 msync
memcpy(m, data, size);
msync(m, size, MS_SYNC);  // 强制写回
munmap(m, size);
```

### ❌ 错误 4: offset 不是页大小整数倍

```c
// Linux 页大小 = 4096
void *m = mmap(NULL, 100, PROT_READ, MAP_SHARED, fd, 50);  // offset=50!
// → EINVAL

// ✅ offset 必须是 sysconf(_SC_PAGE_SIZE) 的整数倍
void *m = mmap(NULL, 100, PROT_READ, MAP_SHARED, fd, 0);  // offset=0 ✓
```

## 动手练习

### 🟢 练习 1: mmap 读文件

用 mmap 读取一个文本文件，打印前 80 字符。

<details>
<summary>点击查看答案</summary>

```c
int fd = open("test.txt", O_RDONLY);
struct stat st;
fstat(fd, &st);

void *m = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
printf("%.80s\n", (char *)m);

munmap(m, st.st_size);
close(fd);
```
</details>

### 🟡 练习 2: mmap 复制文件

用 mmap 实现文件复制（源文件 mmap 读，目标文件 mmap 写）。

<details>
<summary>点击查看答案</summary>

```c
int src = open("src.bin", O_RDONLY);
struct stat st;
fstat(src, &st);

int dst = open("dst.bin", O_RDWR | O_CREAT | O_TRUNC, 0644);
ftruncate(dst, st.st_size);

void *src_m = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, src, 0);
void *dst_m = mmap(NULL, st.st_size, PROT_WRITE, MAP_SHARED, dst, 0);

memcpy(dst_m, src_m, st.st_size);
msync(dst_m, st.st_size, MS_SYNC);

munmap(src_m, st.st_size); munmap(dst_m, st.st_size);
close(src); close(dst);
```
</details>

### 🔴 练习 3: 共享内存 (匿名 mmap)

用 `MAP_ANONYMOUS | MAP_SHARED` 实现父子进程共享内存通信。

<details>
<summary>点击查看答案</summary>

```c
int size = 4096;
void *shm = mmap(NULL, size, PROT_READ | PROT_WRITE,
                  MAP_SHARED | MAP_ANONYMOUS, -1, 0);

pid_t pid = fork();
if (pid == 0) {
    strcpy(shm, "Hello from child!");
    _exit(0);
} else {
    wait(NULL);  // 等子进程写完
    printf("Shared memory: %s\n", (char *)shm);
    munmap(shm, size);
}
```
</details>

## 故障排查

### Q: mmap 返回 MAP_FAILED 并显示 EINVAL

检查：1) offset 是否页对齐 2) fd 是否有效 3) 文件大小是否为 0

### Q: 写入 mmap 区域后 SIGBUS / SIGSEGV

原因：1) 越界写入 2) 文件大小太小 3) 写入只读映射（PROT_READ 时写）

### Q: munmap 后修改丢失

`MAP_PRIVATE` 映射的修改不写回文件。改用 `MAP_SHARED` 或在 munmap 前 `msync()`。

## 知识扩展

### 1. mmap 与页缓存

内核会将文件内容缓存到页缓存（page cache）。mmap 直接映射页缓存，绕过了从内核到用户的缓冲区拷贝。这就是 mmap 快的原因。

### 2. MADVISE 提示

```c
madvise(mapped, size, MADV_SEQUENTIAL);  // 告诉内核：我将顺序访问
madvise(mapped, size, MADV_RANDOM);      // 随机访问
madvise(mapped, size, MADV_WILLNEED);    // 预读
```

### 3. 大页映射 (Huge Pages)

对于超大文件 (>1GB)，标准 4KB 页会导致大量页表项。Linux 支持 2MB 大页和 1GB 巨页，减少页表开销。

## 小结

- **mmap** 把文件映射到内存——直接指针访问，零拷贝
- **PROT_READ/WRITE** 控制访问权限
- **MAP_SHARED** 写回文件，**MAP_PRIVATE** 写入时复制不写回
- **msync** 强制写回磁盘，**munmap** 释放映射
- 大文件随机访问 mmap 明显快于 read/write

> **我的教训**：第一次用 mmap 时，我把一个 0 字节文件映射成 100 字节，写入时 SIGBUS 崩溃。后来才明白：**必须先 ftruncate 设置文件大小**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 内存映射（Memory-Mapped I/O） | 将文件映射到进程地址空间 |
| 零拷贝（Zero-Copy） | 无需内核→用户缓冲区拷贝 |
| 写时复制（Copy-on-Write） | MAP_PRIVATE 的特性 |
| 页对齐（Page-Aligned） | 地址/偏移是页大小的整数倍 |
| 页缓存（Page Cache） | 内核维护的文件内容缓存 |

## 延伸阅读

- [POSIX mmap(2) 手册](https://pubs.opengroup.org/onlinepubs/9699919799/functions/mmap.html) — 官方规范
- [Understanding mmap](https://www.morethantechnical.com/2017/08/16/understanding-mmap/) — 深入理解
- [Linux 页缓存详解](https://www.kernel.org/doc/html/latest/admin-guide/mm/index.html) — 内核文档

## 继续学习

你已经掌握了 mmap 高效文件 I/O。接下来，我们将探索进程管理——用 fork 创建子进程，用 exec 替换进程映像。

> 💡 **提示**：运行 `src/advance/system_mmap_sample.c` 查看所有演示和性能对比。`make build && make run`。

[← 上一章：信号处理](./signal.md) | [下一章：进程管理 →](./process.md)
