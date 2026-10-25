# 内核接口探测 (Kernel Interface Probing)

> "想写 LED 驱动之前，先学会走路——在 userland 里把内核的三扇服务窗口各敲一遍。"

## 开篇故事

你买了一块开发板，雄心勃勃想写一个点亮 LED 的内核驱动。打开教程第一页：「请先配置内核源码树、交叉编译工具链、独立的驱动构建系统……」——然后你意识到，驱动跑在**内核态**，一次野指针不是 segfault，而是整机 panic。

本章不教你写驱动，而是教你**从 userland 观察内核**：内核虽然不对外开放，却留了三扇「服务窗口」——

- **macOS / FreeBSD** 的 `sysctl`：报出门牌号（oid），内核把数据递出来；
- **illumos / Solaris** 的 `kstat`：像仪表盘，打开 → 找表 → 读数 → 关上；
- **Linux** 的 `/proc`（procfs）：**文件即 API**，`fopen` 就是敲窗。

同样的内核，零 panic 风险。学会走路，再学跑。

## 本章适合谁

- 想写驱动但还没准备好掉进内核构建系统的人
- 用过 `sysctl -a` / `cat /proc/loadavg` 但不知道背后 C API 的人
- 想理解「为什么 Linux 把内核状态导出成文件树」的人
- 好奇 macOS、Solaris、Linux 三家如何各讲各的「内核方言」的人

## 你会学到什么

- `sysctlbyname()` — 两段式读取内核 oid（先查长度，再读值）
- `kstat_open → kstat_lookup → kstat_read → kstat_close` 四步走
- `KSTAT_type_named`（`KSTAT_TYPE_NAMED`）— named 型 kstat 的解读约定
- `fopen("/proc/...")` + `fscanf` — 用标准 C 文件 API 读内核
- `#ifdef` 平台四分叉 — 与 `src/sysinfo.c` 同款的条件编译迷宫

## 前置要求

- 理解 `#ifdef` / `#elif` / `#else` 条件编译
- 熟悉 `fopen` / `fgets` / `fscanf` 文件操作
- 了解 `errno` 与错误处理习惯
- 读过 [sysinfo.c](https://github.com/savechina/hello-c/blob/main/src/sysinfo.c) 的平台检测更佳

## 三方对照表

| 维度 | `sysctl(3)` | `kstat(3K)` | `procfs` (`/proc`) |
|------|-------------|-------------|--------------------|
| 平台 | macOS, FreeBSD | illumos / Solaris | Linux |
| 寻址方式 | 命名 oid（`"kern.ostype"`） | module + instance + name | 文件路径（`/proc/loadavg`） |
| 读取方式 | `sysctlbyname()` 两段式 | open → lookup → read → close | `fopen` / `fscanf` / `fgets` |
| 数据形态 | 有类型的值（string / u32 / u64） | `kstat_named_t` 命名项 | 文本行 |
| 写入 | 部分 oid 可写（`sysctl -w`） | 基本只读 | 部分文件可写（`echo >`） |
| 类比 | 点菜窗口 | 仪表盘 | 文件即 API |
| C 头文件 | `<sys/sysctl.h>` | `<kstat.h>` | 无需（标准 `<stdio.h>`） |

## 代码讲解

完整代码见 `src/advance/kernel_iface_sample.c`。入口是 `main_kernel_iface_sample()`，内部一个**平台四分叉**：

```c
#if defined(__APPLE__) || defined(__FreeBSD__)
  /* sysctl 分支 */
#elif defined(__sun__)
  /* kstat 分支 */
#elif defined(__linux__)
  /* procfs 分支 */
#else
  /* 兜底: 打印说明, 仍然 exit 0 */
#endif
```

每个 demo 函数自带平台守卫：**只对当平台生效，其余平台打印 `[跳过]` fallback**——所以无论在哪个系统上 `make run`，三大接口都会各报一次到。

### 1. `sysctlbyname()` — 两段式读取

sysctl 的惯用法是**先问长度、再读值**：

```c
size_t len = 0;
sysctlbyname(oid, NULL, &len, NULL, 0);   /* 第一段: 查询值长度 */
sysctlbyname(oid, buf, &len, NULL, 0);    /* 第二段: 按长度读取 */
```

值的类型由**内核**决定：字符串（含结尾 NUL）、`uint32_t` 还是 `uint64_t`？按返回长度分派，不要硬假设：

```c
if (looks_like_string(buf, len))      /* 结尾 NUL 且全可打印 → string */
    printf("\"%s\"", (char *)buf);
else if (len == sizeof(uint32_t)) {   /* 4 字节 → u32 */
    uint32_t v; memcpy(&v, buf, 4);   /* memcpy: 避免未对齐与别名问题 */
} else if (len == sizeof(uint64_t)) { /* 8 字节 → u64 */
    uint64_t v; memcpy(&v, buf, 8);
}
```

> 💡 `memcpy` 不是多余的——`buf` 是 `unsigned char[]`，直接强转成 `uint32_t *` 可能未对齐，还踩 strict aliasing。本仓的 [strict aliasing 章节](../strict-aliasing.md) 讲的就是这个。

### 2. `kern.usrstack` — 潜伏的进程栈基址

```c
kernel_iface_sysctl_show("kern.ostype");     /* "Darwin" */
kernel_iface_sysctl_show("kern.osrelease");  /* "24.6.0" */
kernel_iface_sysctl_show("kern.usrstack");   /* 进程栈基址! */
kernel_iface_sysctl_show("kern.usrstack64"); /* 完整 64 位值 */
```

`kern.usrstack` 是本章最有趣的 oid：**进程栈的基址就潜伏在内核里**。在 Apple Silicon 上实测它只有 4 字节（`0x6dbc8000`）——高地址被 32 位截断了；`kern.usrstack64` 才是完整的 64 位值（`0x16dbc8000`）。这正是「两段式 + 按长度分派」存在的理由：**类型是内核说了算**。

### 3. 错误处理 — 未知 oid 不崩溃

```c
if (sysctlbyname("kern.this_oid_does_not_exist", NULL, &len, NULL, 0) != 0) {
    printf("[预期失败] %s (已优雅跳过)\n", strerror(errno));  /* ENOENT */
}
```

未知 oid 返回 `-1` 且 `errno = ENOENT`。**打印跳过，绝不崩溃，永远 exit 0**——这是探测未知内核版本时的生存法则。

### 4. `kstat()` — illumos 四步走

```c
kstat_ctl_t *kc = kstat_open();                          /* 1) 打开框架 */
kstat_t *ksp = kstat_lookup(kc, "unix", 0, "system_misc"); /* 2) 定位表盘 */
if (ksp->ks_type != KSTAT_TYPE_NAMED) { ... }            /* 3) 类型检查 */
kstat_read(kc, ksp, NULL);                               /* 4) 读快照 */
kstat_named_t *kn = kstat_data_lookup(ksp, "nproc");     /* 5) 查命名项 */
printf("nproc = %u\n", kn->value.ui32);
kstat_close(kc);                                         /* 6) 必须关闭 */
```

三个要点：

- **`KSTAT_type_named` 约定**：只有 `ks_type == KSTAT_TYPE_NAMED` 的 kstat 才能按 `kstat_named_t` 解读——kstat 还有 `KSTAT_TYPE_RAW`、`KSTAT_TYPE_INTR` 等类型，类型错了就是未定义行为。
- **`kstat_read` 才拿到最新值**：kstat 是内核周期性刷新的统计，lookup 到的只是元数据。
- **每条错误路径都要 `kstat_close`**：上面每个 `return` 前都先关句柄——泄漏内核句柄比泄漏堆内存更难排查。

### 5. `/proc` — 文件即 API

```c
FILE *fp = fopen("/proc/sys/kernel/osrelease", "r");
fgets(line, sizeof(line), fp);          /* 单行文本 = 内核版本 */

fp = fopen("/proc/loadavg", "r");
fscanf(fp, "%lf %lf %lf", &l1, &l5, &l15);  /* 三个 double: 1/5/15 分钟负载 */
```

不需要任何平台专用头文件——**标准 `<stdio.h>` 就是全部**。`/proc` 自身是 procfs：Linux 把内核状态导出成文件树，读文件 = 读内核。这是三家之中最「Unix 哲学」的设计，也是 `/proc/sys` 能被 `sysctl -w` 包装、`/sys`（sysfs）能成为设备模型入口的原因。

### 6. `#ifdef` 平台迷宫

本章的四分叉与 [sysinfo.c](https://github.com/savechina/hello-c/blob/main/src/sysinfo.c) 同款：**include 也要按平台守卫**——`<sys/sysctl.h>` 只在 macOS/FreeBSD 出现，`<kstat.h>` 只在 illumos 出现，Linux 分支一个额外头都不需要。编译期就决定了哪段代码存在，运行期零开销。

## 驱动为什么不在本仓

> **⚠️ 说明块**
>
> 你可能会问：既然是「driver 替代教程」，为什么不直接写一个内核驱动？
>
> - **macOS**：KEXT/KPI 需要内核头文件、独立的签名与构建流程；Apple 正把驱动推向用户态的 **DriverKit**。
> - **FreeBSD**：kernel modules 需要完整的 `src` 树和 `sys/conf` 构建系统。
> - **illumos**：drivers 需要 DDK 和专门的驱动构建模板。
> - **本项目 Makefile** 只能编译 userland 二进制（`-std=c17 -Wall -Wextra -Werror`），**无法产出内核态模块**——内核模块有自己的 ABI、加载器（`kldload` / `kextload` / `modload`）和符号解析规则。
>
> 因此本章用「userland 内核接口」代替 driver 教学：**同样的内核，零 panic 风险**。想走驱动这条路，指路：
>
> - FreeBSD Handbook → *Kernel Modules and Tuning*
> - Apple Developer Documentation → *DriverKit*
> - illumos Documentation → *Writing Device Drivers*

## 常见错误

### ❌ 错误 1: 假设 sysctl 值的类型和长度

```c
// ❌ 假设 kern.usrstack 是 8 字节 — 实际 macOS 上是 4 字节, 读失败
uint64_t v;
size_t len = sizeof(v);
sysctlbyname("kern.usrstack", &v, &len, NULL, 0);

// ✅ 先查长度, 按长度分派
size_t len = 0;
sysctlbyname(oid, NULL, &len, NULL, 0);   /* 内核告诉你多大 */
```

### ❌ 错误 2: kstat 路径上忘记 `kstat_close`

```c
// ❌ 中途 return 泄漏内核句柄
ksp = kstat_lookup(kc, "unix", 0, "system_misc");
if (ksp == NULL) return;              /* kc 泄漏! */

// ✅ 每条错误路径都关闭
if (ksp == NULL) { kstat_close(kc); return; }
```

### ❌ 错误 3: 不检查 `fopen` 返回值

```c
// ❌ /proc 在非 Linux 或受限容器里可能不存在
FILE *fp = fopen("/proc/loadavg", "r");
fgets(line, sizeof(line), fp);        /* fp == NULL → 崩溃 */

// ✅ 判空 + 打印跳过
if (fp != NULL) { ... fclose(fp); }
else printf("[跳过] %s\n", strerror(errno));
```

## 动手练习

### 🟢 练习 1: 探测你自己的机器

写一个程序，用两段式 `sysctlbyname` 打印 `hw.memsize`（macOS，总内存字节数）和 `hw.ncpu`（CPU 核数），类型按长度自动分派。

<details>
<summary>点击查看答案</summary>

```c
size_t len = 0;
unsigned char buf[16];

sysctlbyname("hw.ncpu", NULL, &len, NULL, 0);
memset(buf, 0, sizeof(buf));
sysctlbyname("hw.ncpu", buf, &len, NULL, 0);

if (len == 4) {
    uint32_t n; memcpy(&n, buf, 4);
    printf("ncpu = %u\n", n);
}
```
</details>

### 🟡 练习 2: 读取 `/proc/meminfo` 的前两行

用 `fopen` + `fgets` 打印 `/proc/meminfo` 的 `MemTotal` 和 `MemFree` 两行（Linux），并对 `fopen` 失败做优雅跳过。

<details>
<summary>点击查看答案</summary>

```c
FILE *fp = fopen("/proc/meminfo", "r");
if (!fp) { printf("[跳过] %s\n", strerror(errno)); return; }

char line[128];
int shown = 0;
while (shown < 2 && fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "MemTotal", 8) == 0 ||
        strncmp(line, "MemFree", 7) == 0) {
        printf("%s", line);
        shown++;
    }
}
fclose(fp);
```
</details>

### 🔴 练习 3: 写一个跨平台 `kernel_version()` 函数

实现 `int kernel_version(char *buf, size_t n)`：macOS/FreeBSD 走 `kern.osrelease`，Linux 读 `/proc/sys/kernel/osrelease`，illumos 用 `uname()` 兜底；任何失败返回 `-1` 且不写脏 `buf`。

<details>
<summary>点击查看答案</summary>

```c
int kernel_version(char *buf, size_t n) {
    if (!buf || n == 0) return -1;
#if defined(__APPLE__) || defined(__FreeBSD__)
    size_t len = n;
    if (sysctlbyname("kern.osrelease", buf, &len, NULL, 0) != 0) return -1;
    buf[n - 1] = '\0';
    return 0;
#elif defined(__linux__)
    FILE *fp = fopen("/proc/sys/kernel/osrelease", "r");
    if (!fp) return -1;
    if (!fgets(buf, (int)n, fp)) { fclose(fp); return -1; }
    fclose(fp);
    buf[strcspn(buf, "\r\n")] = '\0';
    return 0;
#else
    struct utsname u;
    if (uname(&u) != 0) return -1;
    snprintf(buf, n, "%s", u.release);
    return 0;
#endif
}
```
</details>

## 小结

- **三扇服务窗口**：macOS/FreeBSD 的 `sysctl`、illumos 的 `kstat`、Linux 的 `/proc`——同一内核，三种方言
- **`sysctlbyname` 是两段式**：先查长度再读值，类型由内核决定，`memcpy` 解读避免未对齐/别名问题
- **`kern.usrstack` 潜伏着进程栈基址**——32 位截断的教训：永远别假设类型
- **kstat 四步走**：open → lookup → read → close；`KSTAT_type_named` 才能按 `kstat_named_t` 解读；每条错误路径都 `kstat_close`
- **文件即 API**：procfs 用标准 `<stdio.h>` 就能读内核，这是最 Unix 的设计
- **探测代码的生存法则**：所有失败都打印跳过，永远 exit 0
- **驱动不在本仓**：内核模块需要内核头 + 独立构建系统，本 Makefile 只产 userland 二进制

> **我的教训是**：第一次写探测代码时，我直接把 `sysctl` 的返回值强转成 `uint64_t *`，在 Apple Silicon 上因为 4 字节值 + 未对齐访问被 UBSan 抓个正着。记住：**类型是内核说了算，不是你**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 内核接口（Kernel Interface） | 内核暴露给 userland 的受控入口 |
| sysctl | BSD 系的命名 oid 内核参数接口 |
| oid（Object ID） | sysctl 中标识一个内核值的字符串路径 |
| kstat（Kernel Statistics） | illumos/Solaris 的内核统计框架 |
| KSTAT_type_named | kstat 的命名键值类型（`KSTAT_TYPE_NAMED`） |
| procfs（/proc） | 把内核/进程状态导出为文件树的虚拟文件系统 |
| sysfs（/sys） | Linux 导出设备模型的虚拟文件系统 |
| KEXT / KPI | macOS 内核扩展及其编程接口（正被 DriverKit 取代） |
| 内核态（Kernel Mode） | 特权执行级别；错误直接威胁整机稳定性 |

## 延伸阅读

- [sysctl(3) — FreeBSD Manual Pages](https://www.freebsd.org/cgi/man.cgi?query=sysctl) — 两段式 API 官方文档
- [kstat(3K) — illumos man pages](https://illumos.org/man/3k/kstat) — kstat 四步走规范
- [proc(5) — Linux man pages](https://man7.org/linux/man-pages/man5/proc.5.html) — `/proc` 文件系统全表
- [FreeBSD Handbook — Kernel Modules](https://docs.freebsd.org/en/books/handbook/kernelconfig/) — 想走驱动之路的第一站
- [Apple Developer — DriverKit](https://developer.apple.com/documentation/driverkit) — macOS 用户态驱动框架
- [illumos — Writing Device Drivers](https://illumos.org/books/) — illumos 驱动教程
- 本仓 [sysinfo.c](https://github.com/savechina/hello-c/blob/main/src/sysinfo.c) — 同款 `#ifdef` 迷宫的 canonical 参考

## 继续学习

你已经从 userland 把内核的三扇窗各敲了一遍——知道内核愿意说什么、怎么说、以及**为什么本仓不带你进那扇门**。接下来可以回到 [Advance 总览](../overview.md) 继续系统编程之旅，或者翻开 [系统编程 · 文件与目录](../system/file.md) 深入虚拟文件系统。

> 💡 **提示**：运行 `src/advance/kernel_iface_sample.c` 查看所有演示。`make build && make run`。

[← 上一章：Advance 总览](../overview.md)
