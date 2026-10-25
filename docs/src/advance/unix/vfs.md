# VFS 与挂载点 (VFS & Mount Points)

> "VFS 是图书馆的总目录，挂载点是每个书架的门牌号——读者只认目录，管理员（内核）负责路由到书架。"

## 开篇故事

我第一次在 macOS 上敲 `mount`，终端冷冷地回了一句 `mount: /System/Volumes/Data on /System/Volumes/Data: requires superuser`。
为什么**看一眼**挂载表也要 root？

因为挂载表不只是"一张清单"——它暴露了整台机器的存储拓扑：哪块磁盘、哪个网络卷、
哪个容器沙箱被接到了目录树的哪个节点上。在多用户系统里，这属于安全敏感信息，
所以 Linux 把它放进 `/proc/mounts`（人人可读），macOS/BSD 却把 `getmntinfo()`
的完整视图留给了特权进程。**同一个需求，四个平台四种答案**——这正是本章的主题。

顺带一个经典的"移植性彩蛋"：开机自动挂载的配置文件，
Linux 和 BSD 叫 `/etc/fstab`，Solaris/illumos 却叫 **`/etc/vfstab`**——
多了个 `v`（volume），列也更多（多了 fsck 设备和检查顺序）。
而 Solaris 的"当前挂载表" `/etc/mnttab` 是一份**纯文本**，
Linux 的对应物 `/proc/mounts` 则是伪文件。名字像，读法全不一样。

## 本章适合谁

- 写过 `df`、好奇它背后怎么拿到数字的人
- 要把代码从 Linux 移植到 macOS/Solaris、被 `#include <mntent.h>` 卡住的人
- 想理解"一切皆文件"之上那层——**虚拟文件系统（VFS）**——的人
- 准备写磁盘工具（空间监控、挂载巡检、备份脚本）的人

## 你会学到什么

- **挂载点枚举**的平台四分叉：`getmntinfo()` / `setmntent+getmntent` / `/etc/mnttab`
- **`statvfs()`** —— POSIX 侧统一拿空间使用量（`f_frsize * f_bavail`）
- 文件系统树遍历的现成工具：`nftw()` / `fts()`（点到为止）
- 挂载语义：`fstab` vs `vfstab`，挂载点为什么"只是个普通目录"

## 前置要求

- 理解目录树与路径概念（上一章 [文件与目录操作](../system/file.md)）
- 熟悉 `#if defined(...)` 条件编译
- 知道 `errno` 错误码模式

## 平台对照表

| 需求 | macOS / BSD | Linux | Solaris / illumos |
|------|-------------|-------|-------------------|
| 枚举挂载点 | `getmntinfo()` 快照式 | `setmntent()` + `getmntent()` 逐行 | 直接读 `/etc/mnttab` 纯文本 |
| 头文件 | `<sys/param.h>` + `<sys/mount.h>` | `<mntent.h>` | 无（`fopen` 即可） |
| 数据源 | 内核挂载表（一次拷贝到用户态） | `/proc/mounts` 伪文件 | `/etc/mnttab` 文本文件 |
| 结构体 | `struct statfs` | `struct mntent` | `strtok` 切出的三列字符串 |
| 空间查询 | `statvfs()` / `statfs()` | `statvfs()` / `statfs()` | `statvfs()` / `statfs()` |
| 开机挂载表 | `/etc/fstab` | `/etc/fstab` | **`/etc/vfstab`** |
| 当前挂载表 | `getmntinfo()` | `/proc/mounts` | `/etc/mnttab` |

> **记忆钩子**：BSD 家族"一次快照"，Linux"逐行读伪文件"，Solaris"读普通文本"。
> 空间查询则三家共享同一份 POSIX 代码——`statvfs()`。

## 代码讲解

源码：[`src/advance/vfs_sample.c`](../../../src/advance/vfs_sample.c)

### 1. 挂载点枚举 —— 平台四分叉

```c
#if defined(__APPLE__) || defined(BSD)
  #include <sys/mount.h>
  #include <sys/param.h>
#elif defined(__linux__)
  #include <mntent.h>
#elif defined(__sun__)
  /* Solaris: /etc/mnttab 是纯文本，无需专用头文件 */
#endif
```

**Include 也要条件包裹**——`<mntent.h>` 在 macOS 上根本不存在，
不包起来连编译都过不了。这是移植代码的第一道坎。

#### macOS/BSD 分叉：`getmntinfo()` 快照式

```c
struct statfs *mntbuf = NULL;
int count = getmntinfo(&mntbuf, MNT_NOWAIT);   /* 一次调用，拿回整张表 */
for (int i = 0; i < count && i < 5; i++) {
    printf("%s -> %s (%s)\n",
           mntbuf[i].f_mntfromname,   /* 设备: /dev/disk3s1s1 */
           mntbuf[i].f_mntonname,     /* 挂载点: / */
           mntbuf[i].f_fstypename);   /* 类型: apfs */
}
```

`getmntinfo()` 是**快照式**：一次系统调用把内核挂载表整份拷到用户态，
返回条数，`mntbuf` 指向一块静态缓冲区（不要 free，下次调用会覆盖）。
`MNT_NOWAIT` 表示"别等网络文件系统回应，现在就要"。

#### Linux 分叉：`setmntent` / `getmntent` / `endmntent`

```c
FILE *fp = setmntent("/proc/mounts", "r");     /* 打开伪文件 */
struct mntent *ent;
while ((ent = getmntent(fp)) != NULL) {        /* 逐行解析 */
    printf("%s -> %s (%s)\n",
           ent->mnt_fsname, ent->mnt_dir, ent->mnt_type);
}
endmntent(fp);                                 /* 配对关闭 */
```

三件套和 `fopen/fgets/fclose` 一个套路：`setmntent` 打开，
`getmntent` 每次返回一个填好的 `struct mntent`（内部静态缓冲，要复制就自己 `strdup`），
`endmntent` 关闭。数据源 `/proc/mounts` 是内核生成的伪文件——
**一切皆文件**的又一例证。

#### Solaris 分叉：`/etc/mnttab` 纯文本

```c
FILE *fp = fopen("/etc/mnttab", "r");
char line[1024];
while (fgets(line, sizeof(line), fp) != NULL) {
    /* 格式: device mountpoint fstype options dump pass */
    char *dev  = strtok(line, " \t\n");
    char *mnt  = strtok(NULL, " \t\n");
    char *type = strtok(NULL, " \t\n");
    if (dev == NULL || mnt == NULL || type == NULL) continue;
    printf("%s -> %s (%s)\n", dev, mnt, type);
}
fclose(fp);
```

Solaris 的 `/etc/mnttab` 就是一份普通文本，用 `strtok` 切前三列即可——
连专用 API 都省了。这是"光彩的平台"（illumos）留给移植者的兼容彩蛋：
**同一份信息，Linux 用伪文件 + 专用解析器，Solaris 用真文件 + 字符串切割。**

#### 兜底分叉

```c
#else
  printf("  [平台暂不支持] 本示例只覆盖 macOS/BSD、Linux、Solaris 三分叉\n");
#endif
```

`#else` 保证任何未知平台都"有输出、退出码 0"——教程代码的兜底纪律。

### 2. `statvfs()` —— POSIX 统一空间查询

```c
#include <sys/statvfs.h>

struct statvfs sv;
if (statvfs("/", &sv) < 0) {
    perror("statvfs");
    return;
}

unsigned long long avail = (unsigned long long)sv.f_frsize
                         * (unsigned long long)sv.f_bavail;
printf("可用: %.2f GiB\n", (double)avail / (1024.0 * 1024.0 * 1024.0));
```

**这是全章唯一没有任何 `#if` 的代码**——POSIX 保证四个平台同一份源码。

| 字段 | 含义 |
|------|------|
| `f_frsize` | 传输块大小（统计用，可能 ≠ `f_bsize`） |
| `f_blocks` | 总块数 |
| `f_bfree` | 空闲块数（**含** root 保留块） |
| `f_bavail` | 普通用户**可用**块数 |
| `f_files` / `f_ffree` | inode 总数 / 空闲数 |

**关键公式**：可用字节 = `f_frsize * f_bavail`。

> **常见坑**：`df` 显示的是 `f_bavail`（你能用的），`f_bfree` 是磁盘上真正空闲的。
> 两者差值是给 root 预留的应急空间（`f_bresvd`）。写监控告警时用错字段，
> 会在"磁盘明明有空间"时误报。

### 3. 文件系统树遍历回顾

上一章的 `opendir/readdir` 只能扫**一层**。要递归整棵树，POSIX 有两个现成工具：

```c
#include <ftw.h>

/* nftw: 对 path 下每个条目调用 fn，FTW_PHYS 表示不跟随符号链接 */
nftw("/", callback, /*depth=*/16, FTW_PHYS);
```

- **`nftw()`** —— POSIX 标准，回调式，一行遍历全树
- **`fts_open()/fts_read()`** —— BSD/Solaris 家族出身，可剪枝、可排序，更可控
- **`scandir()`** —— 单层过滤（配合选择器回调）

选择建议：简单递归用 `nftw`，需要跳过/剪枝用 `fts`，单层用 `readdir`。
本章点到为止——`readdir` 的完整讲解见 [文件与目录操作](../system/file.md)。

### 4. 挂载语义

- **`statfs` 结构在 macOS 家族同构**：Darwin 与 BSD 的 `struct statfs`
  字段名一致（`f_blocks`/`f_bavail`/`f_fstypename`），代码基本零改动。
- **挂载点只是普通目录**：`mount /dev/sdb1 /mnt/data` 之后，`/mnt/data`
  的内容被新文件系统接管；`umount` 后又露出下层目录的原有内容——
  像叠罗汉，上面的盘盖住下面的目录。
- **fstab 家族**：`/etc/fstab`（Linux/BSD）vs **`/etc/vfstab`**（Solaris，
  多出 fsck 设备与检查顺序两列）。改错文件名是 Solaris 移植的经典第一坑。

## 常见错误

### ❌ 错误 1: 不条件包裹 include

```c
// ❌ 在 macOS 上编译直接失败: 'mntent.h' file not found
#include <mntent.h>

// ✅ include 和实现一起分叉
#if defined(__linux__)
  #include <mntent.h>
#endif
```

### ❌ 错误 2: 用 f_bfree 当"用户可用空间"

```c
// ❌ f_bfree 含 root 保留块，普通用户写不进去
unsigned long long bad = sv.f_frsize * sv.f_bfree;

// ✅ 用户可用 = f_frsize * f_bavail
unsigned long long good = (unsigned long long)sv.f_frsize * sv.f_bavail;
```

### ❌ 错误 3: 忘记配对关闭

```c
// ❌ Linux: setmntent 了却没 endmntent（fd 泄漏）
FILE *fp = setmntent("/proc/mounts", "r");
getmntent(fp);

// ✅ 配对关闭，和 open/close 一个纪律
FILE *fp = setmntent("/proc/mounts", "r");
/* ... */
endmntent(fp);
```

### ❌ 错误 4: 假设 mntbuf 可以 free

```c
// ❌ getmntinfo 返回的是静态缓冲区，free 它是未定义行为
int n = getmntinfo(&buf, MNT_NOWAIT);
free(buf);

// ✅ 不要 free；下次调用 getmntinfo 会覆盖同一块内存
//    需要长期保留就自己 memcpy 一份
```

## 动手练习

### 🟢 练习 1: 打印完整挂载表

把示例的 `i < 5` 限制去掉，打印**全部**挂载点，并统计各 fstype 出现次数
（apfs 几个、devfs 几个……）。

<details>
<summary>点击查看答案</summary>

```c
int total = getmntinfo(&mntbuf, MNT_NOWAIT);
int apfs = 0, other = 0;
for (int i = 0; i < total; i++) {
    printf("%-30s %-30s %s\n",
           mntbuf[i].f_mntfromname, mntbuf[i].f_mntonname,
           mntbuf[i].f_fstypename);
    if (strcmp(mntbuf[i].f_fstypename, "apfs") == 0) apfs++;
    else other++;
}
printf("apfs: %d, 其他: %d\n", apfs, other);
```
</details>

### 🟡 练习 2: df 风格空间报告

写一个 `my_df(const char *path)`，用 `statvfs(path)` 打印该路径所在文件系统的
总容量、可用容量、使用率百分比（`1 - f_bavail/f_blocks`）。

<details>
<summary>点击查看答案</summary>

```c
void my_df(const char *path) {
    struct statvfs sv;
    if (statvfs(path, &sv) < 0) {
        perror("statvfs");
        return;
    }
    double total = (double)sv.f_frsize * sv.f_blocks;
    double avail = (double)sv.f_frsize * sv.f_bavail;
    printf("%-20s %8.1fG %8.1fG %5.1f%%\n", path,
           total / 1e9, avail / 1e9,
           100.0 * (1.0 - (double)sv.f_bavail / (double)sv.f_blocks));
}
```
</details>

### 🔴 练习 3: 跨平台挂载巡检器

写一个 `count_mounts()`，在 macOS/BSD 返回 `getmntinfo` 的条数、
Linux 返回 `/proc/mounts` 行数、Solaris 返回 `/etc/mnttab` 行数，
未知平台返回 -1。要求三个分支各自配对关闭资源。

<details>
<summary>点击查看答案</summary>

```c
int count_mounts(void) {
#if defined(__APPLE__) || defined(BSD)
    struct statfs *buf;
    return getmntinfo(&buf, MNT_NOWAIT);
#elif defined(__linux__)
    FILE *fp = setmntent("/proc/mounts", "r");
    if (!fp) return -1;
    int n = 0;
    while (getmntent(fp) != NULL) n++;
    endmntent(fp);
    return n;
#elif defined(__sun__)
    FILE *fp = fopen("/etc/mnttab", "r");
    if (!fp) return -1;
    int n = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp) != NULL) n++;
    fclose(fp);
    return n;
#else
    return -1;
#endif
}
```
</details>

## 故障排查

### Q: `getmntinfo()` 返回 0？

`errno` 会给出原因。常见于沙箱环境（macOS App Sandbox 禁止读挂载表）。
退化方案：直接 `statvfs("/")` 拿空间信息，放弃枚举。

### Q: Linux 上 `setmntent("/proc/mounts")` 失败？

确认 `/proc` 已挂载（容器/Chroot 里经常没挂）。备选数据源：
`/etc/mtab`（现代发行版是指向 `/proc/mounts` 的符号链接）。

### Q: `statvfs` 和 `statfs` 选哪个？

`statfs` 是历史事实标准（BSD 出身，字段名各平台略有差异）；
`statvfs` 是 POSIX 标准化后的版本（字段名统一）。**新代码用 `statvfs`**，
要和 `df` 输出逐字对齐时再考虑 `statfs`。

### Q: 为什么 macOS 上 `df` 显示的可用空间比 `statvfs` 少？

APFS 多个卷**共享**同一块容器空间。`statvfs` 报的是本卷配额视角，
`df` 可能按容器合并计算——两边都"对"，口径不同。

## 小结

- **VFS** 是内核里的文件系统总线：上层统一 `open/read/stat`，下层 apfs/ext4/ufs 各自实现
- **挂载点枚举是平台四分叉**：`getmntinfo()`（macOS/BSD 快照式）、
  `setmntent/getmntent`（Linux 读 `/proc/mounts`）、`strtok` 读 `/etc/mnttab`（Solaris）、`#else` 兜底
- **`statvfs()` 是纯 POSIX**：可用字节 = `f_frsize * f_bavail`，全平台同一份代码
- **include 也要条件包裹**——`<mntent.h>` 在 macOS 上不存在
- **fstab 家族**：`/etc/fstab`（Linux/BSD）vs `/etc/vfstab`（Solaris）
- 树遍历有现成工具：`nftw()`（POSIX）/ `fts()`（BSD 系），单层仍是 `readdir`

> **我的教训**：第一次写磁盘监控时我用了 `f_bfree`，结果在 root 保留块 5% 的
> 服务器上疯狂误报"磁盘将满"。记住：**给用户看的用 `f_bavail`**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 虚拟文件系统（VFS, Virtual File System） | 内核抽象层，统一多种具体文件系统的接口 |
| 挂载点（Mount Point） | 目录树上被某个文件系统接管的目录 |
| 挂载表（Mount Table） | 当前所有挂载记录：设备 → 挂载点 → 类型 |
| 快照式（Snapshot） | 一次调用拿回整张表（`getmntinfo`） |
| 伪文件（Pseudo File） | 内核生成、读它等于问内核（`/proc/mounts`） |
| 保留块（Reserved Blocks） | 留给 root 的应急空间，`f_bfree - f_bavail` |

## 延伸阅读

- [POSIX statvfs(3p)](https://pubs.opengroup.org/onlinepubs/9699919799/functions/statvfs.html) — 官方规范
- [mount(8) 手册](https://man7.org/linux/man-pages/man8/mount.8.html) — Linux 挂载机制
- [APUE, Chapter 4](https://en.wikipedia.org/wiki/Advanced_Programming_in_the_UNIX_Anvironments) — 文件与目录（含 nftw/fts）
- [illumos mnttab(4)](https://illumos.org/man/4/mnttab) — Solaris 挂载表格式

## 继续学习

你已经看懂了 VFS 这层"总目录"——挂载表怎么读、空间怎么查、
同一需求在四个平台的四种答案。接下来，我们将深入**内核接口探测**：
sysctl、kstat、`/proc` 与 `/sys`——不写驱动，也能从 userland 读内核状态。

> 💡 **提示**：运行 `src/advance/vfs_sample.c` 查看所有演示。`make build && make run`。

← [Unix 环境与内核](./unix.md) | [下一章：内核接口探测 →](./kernel-interface.md)
