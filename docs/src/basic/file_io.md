# 文件 I/O (File I/O)

## 开篇故事

我第一次用 C 语言读写文件时，以为它和 Python 差不多。在 Python 中，我这样写：

```python
with open("hello.txt", "w") as f:
    f.write("Hello, World!")
```

三行搞定，文件自动关闭，安全优雅。后来我尝试在 C 中写文件：

```c
FILE *fp = fopen("hello.txt", "w");
fprintf(fp, "Hello, C!");
/* 我忘了 fclose(fp)，然后 wonders 为什么文件是空的 😱 */
```

文件是空的——因为输出还在缓冲区里，没有刷到磁盘。我还有一次忘了检查 `fopen` 的返回值，程序打开一个不存在的文件直接崩溃了。

那天我明白了：**在 C 语言中，文件 I/O 不仅是"读写数据"，更是"管理操作系统资源"**。每一个 `FILE*` 都是一个文件描述符，每一次忘记 `fclose` 都是一次资源泄漏。C 不替你操心——但这也意味着你对整个 I/O 过程有完全的控制权。

## 本章适合谁

- 学过 Python/JavaScript 的文件操作，想知道 C 语言中怎么做
- 听说过"缓冲区"但不清楚它如何影响文件写入
- 想理解文本模式和二进制模式的区别
- 希望写出安全的文件 I/O 代码，处理所有错误情况

## 你会学到什么

1. `FILE*` 的本质：文件指针是什么，它如何连接到操作系统
2. `fopen` 与文件模式：`"r"`, `"w"`, `"rb"`, `"wb"` 的区别
3. `fclose` 与资源管理：为什么不关闭文件会导致数据丢失
4. `fprintf`/`fscanf`：格式化的文件读写（类似 `printf`/`scanf`）
5. **安全核心**：`fgets` vs `gets`（为什么 `gets()` 是致命缺陷）
6. `fread`/`fwrite`：二进制 I/O，直接读写 struct 到文件
7. **文本模式 vs 二进制模式**：跨平台差异详解
8. `fseek`/`ftell`：文件定位（随机访问，跳到任意位置）
9. 错误处理：`ferror`、`feof`、`clearerr`
10. 常见错误模式与修复

## 前置要求

- 已完成 [字符串深度](./strings) 章节（`fprintf`、`fgets` 的基础）
- 理解指针概念（`FILE*` 是指针类型）
- 理解 `struct` 基础（二进制 I/O 部分需要）
- 已配置 C 编译环境（`gcc` 或 `clang`）

> **💡 编译命令**：本章代码使用 `-Wall -Wextra -Werror -std=c17` 编译，所有警告视为错误。

## 第一个例子

最简短的文件读写示例——写一行文本然后读回来：

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    /* 1. 写入文件 */
    FILE *fp = fopen("hello.txt", "w");
    if (fp == NULL) {
        fprintf(stderr, "无法打开文件!\n");
        return 1;
    }
    fprintf(fp, "Hello, File I/O!\n");
    fclose(fp);  /* 不关闭 → 数据可能在缓冲区，未写入磁盘! */

    /* 2. 读取文件 */
    fp = fopen("hello.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "无法打开文件!\n");
        return 1;
    }
    char buf[256];
    if (fgets(buf, sizeof(buf), fp) != NULL) {
        printf("读取到: %s", buf);  /* buf 已包含 \n */
    }
    fclose(fp);

    return 0;
}
```

编译并运行：

```bash
gcc -Wall -Wextra -Werror -std=c17 -o demo demo.c
./demo
```

输出：

```
读取到: Hello, File I/O!
```

完整源码在仓库 [`src/basic/file_io_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/file_io_sample.c)。

## 原理解析

### 1. FILE* 是什么？

`FILE*` 不是"文件指针"的字面意思——它不是指向磁盘上文件的指针。`FILE` 是一个结构体，包含：

```
  ┌── FILE* 的内部结构（简化） ──────────────────────────┐
  │                                                       │
  │  struct FILE {                                        │
  │      int          fd;         ← 底层文件描述符         │
  │      char        *buffer;     ← I/O 缓冲区（约 4KB）   │
  │      size_t       buf_size;   ← 缓冲区大小             │
  │      size_t       buf_pos;    ← 当前位置               │
  │      int          flags;      ← 读/写/错误标志         │
  │  };                                                 │
  │                                                       │
  │  数据流:                                              │
  │    fprintf(fp, "Hello")                              │
  │      → 先写入 fp->buffer（内存）                      │
  │      → 缓冲区满 / fclose 时 → 刷到磁盘                │
  │                                                       │
  │  ┌── 程序内存 ──┐   ┌── 内核 ──┐   ┌── 磁盘 ──┐      │
  │  │ FILE* buf   │ → │ 页缓存   │ → │ 文件系统  │     │
  │  │             │ ← │          │ ← │          │     │
  │  └─────────────┘   └──────────┘   └──────────┘      │
  └───────────────────────────────────────────────────────┘
```

**关键理解**：`fprintf` 不会立即写入磁盘，而是先写到 `FILE*` 的缓冲区。缓冲区满了（通常 4KB）或 `fclose` 时，才一次性刷到磁盘。这就是为什么忘记 `fclose` 会导致数据丢失。

### 2. fopen — 打开文件

```c
FILE *fp = fopen("data.txt", "w");
```

`fopen` 的两个参数：文件名、模式字符串。

**常见模式**：

| 模式 | 含义 | 文件不存在 | 文件存在 |
|------|------|-----------|---------|
| `"r"` | 只读 | 返回 NULL | 读取 |
| `"w"` | 写入（覆盖） | 创建新文件 | **清空原内容** |
| `"a"` | 追加 | 创建新文件 | 从末尾追加 |
| `"r+"` | 读写 | 返回 NULL | 读取+写入 |
| `"w+"` | 读写（新建） | 创建新文件 | **清空原内容** |
| 加 `"b"` | 二进制模式 | 同左 | 同左 |

### 3. Python open() vs C FILE*

| 特性 | Python `open()` | C `fopen()` |
|------|-----------------|-------------|
| 返回值 | 文件对象 | `FILE*` 指针 |
| 自动关闭 | `with` 语句自动关闭 | **必须手动 `fclose()`** |
| 缓冲区 | 自动管理 | 自动缓冲区，但需 `fclose` 刷新 |
| 错误处理 | 抛出异常 | 返回 `NULL`，设 `errno` |
| 二进制模式 | `"rb"`/`"wb"` | `"rb"`/`"wb"` |

```python
# Python — with 自动关闭文件
with open("data.txt", "w") as f:
    f.write("Hello")
# ← 这里自动调用了 f.close()
```

```c
// C — 必须手动关闭
FILE *fp = fopen("data.txt", "w");
fprintf(fp, "Hello");
fclose(fp);  // ← 没有这行 = 数据可能在缓冲区里
```

**我的理解**：C 的 `FILE*` 哲学是"给程序员一切控制权，但不帮你擦屁股"。你需要自己管理打开和关闭，但这也意味着你可以精确控制何时刷缓冲区、何时复用文件句柄。

### 4. fprintf + fscanf — 格式化 I/O

和 `printf`/`scanf` 几乎一样，只不过多了第一个参数 `FILE*`：

```c
/* 写入 */
FILE *fp = fopen("data.txt", "w");
fprintf(fp, "%-10s %5d %8.2f\n", "Alice", 20, 95.50);
fprintf(fp, "%-10s %5d %8.2f\n", "Bob",   22, 87.75);
fclose(fp);

/* 读取 */
fp = fopen("data.txt", "r");
char name[32];
int age;
double score;
while (fscanf(fp, "%31s %d %lf", name, &age, &score) == 3) {
    printf("Read: %s, age %d, score %.2f\n", name, age, score);
}
fclose(fp);
```

**关键点**：`fscanf` 返回成功匹配的字段数。用 `== 3` 检查确保完整读到一行三个字段。

### 5. fgets 与 gets — 安全对比

```c
/* ❌ gets() — 极度危险！C11 标准已彻底删除 */
char buf[10];
gets(buf);  /* 用户输入 100 个字符 → 栈溢出 → 崩溃或安全漏洞! 💥 */

/* ✅ fgets() — 指定缓冲区大小，安全 */
char buf[10];
fgets(buf, sizeof(buf), stdin);  /* 最多读 9 字符 + '\0' */
```

`fgets(buf, size, stream)` 的行为：
1. 最多读 `size - 1` 个字符
2. 遇到 `\n` 或 `EOF` 停止（`\n` 也会被存入）
3. **总是**在末尾加 `\0`

```c
fgets(buf, sizeof(buf), stdin);  // 从键盘读
fgets(buf, sizeof(buf), fp);     // 从文件读
```

**记住**：永远不要使用 `gets()`。如果你在任何代码中看到它，立即替换为 `fgets()`。

### 6. fwrite + fread — 二进制 I/O

与 `fprintf`/`fscanf`（人类可读的文本）不同，`fwrite`/`fread` 直接按内存布局读写二进制数据：

```c
typedef struct {
    int32_t  id;
    char     name[32];
    double   score;
} Student;

Student s = {1001, "Alice", 95.5};

/* 写入 — 直接把 struct 的内存布局写进文件 */
FILE *fp = fopen("students.bin", "wb");
fwrite(&s, sizeof(Student), 1, fp);
fclose(fp);

/* 读取 — 直接从文件恢复到 struct */
fp = fopen("students.bin", "rb");
Student loaded;
fread(&loaded, sizeof(Student), 1, fp);
fclose(fp);
```

`fwrite(ptr, size, count, fp)` 的参数：
- `ptr`：要写入的数据指针
- `size`：每个元素的字节数
- `count`：元素个数
- 返回值：实际写入的元素数

`fread` 的参数完全相同，返回实际读取的元素数。

**⚠️ 警告**：二进制文件与平台/编译器有关。不同的 padding、字节序（endianness）会导致跨平台不兼容。如果需要在不同系统间传输数据，用文本格式（fprintf/fscanf）或显式序列化。

### 7. 文本模式 vs 二进制模式

```
  ┌── 文本模式 vs 二进制模式 ──────────────────────────┐
  │                                                     │
  │  macOS/Linux:                                       │
  │    "w" 和 "wb" 完全相同，没有换行转换               │
  │                                                     │
  │  Windows:                                           │
  │    "w"  → \n 自动转换为 \r\n                       │
  │    "wb" → 不转换，原样写入                          │
  │                                                     │
  │  示例: fprintf(fp, "Hello\n");                      │
  │    "w" 模式写入 6 字节: H e l l o \r \n             │
  │    "wb" 模式写入 6 字节: H e l l o \n               │
  │    （Windows 上 "w" 模式变成 7 字节!）              │
  │                                                     │
  │  ✅ 通用策略:                                        │
  │     - 文本/日志/配置 → 用 "w"/"r"                   │
  │     - 图片/音频/struct → 用 "wb"/"rb"              │
  └─────────────────────────────────────────────────────┘
```

### 8. fseek + ftell — 文件定位

```c
FILE *fp = fopen("data.txt", "r");

/* ftell: 获取当前位置 */
long pos = ftell(fp);  /* 初始为 0 */

/* fseek: 移动文件位置指针 */
fseek(fp, 10, SEEK_SET);   /* 跳到第 10 字节 */
fseek(fp, 5, SEEK_CUR);    /* 从当前位置再前进 5 字节 */
fseek(fp, -3, SEEK_CUR);   /* 回退 3 字节 */
fseek(fp, 0, SEEK_END);    /* 跳到文件末尾 */
pos = ftell(fp);           /* pos = 文件大小 */

fclose(fp);
```

三个定位基点：

| 常量 | 含义 |
|------|------|
| `SEEK_SET` | 文件开头（偏移 0） |
| `SEEK_CUR` | 当前位置 |
| `SEEK_END` | 文件末尾 |

`fseek`/`ftell` 的典型用途：获取文件大小、跳过文件头、随机访问记录、计算进度等。

### 9. ferror — 错误处理

```c
FILE *fp = fopen("data.txt", "r");

/* 读操作后检查错误 */
char buf[100];
fgets(buf, sizeof(buf), fp);
if (ferror(fp)) {
    fprintf(stderr, "读取失败!\n");
    clearerr(fp);  /* 清除错误标志 */
}

/* 检查是否到达文件末尾 */
if (feof(fp)) {
    printf("文件已读完\n");
}

fclose(fp);
```

文件 I/O 的错误处理层次：

```
  ┌── 文件 I/O 错误检测 ───────────────────┐
  │                                         │
  │  fopen  → 检查返回 NULL                 │
  │  fread  → 检查返回值 < 预期数量        │
  │  ferror → 检测读写错误                  │
  │  feof   → 检测是否到达 EOF              │
  │  clearerr → 清除错误标志               │
  │                                         │
  │  标准流程:                               │
  │    fp = fopen(...);                     │
  │    if (!fp) handle error                │
  │    ... perform I/O ...                  │
  │    if (ferror(fp)) handle error         │
  │    fclose(fp);                          │
  └─────────────────────────────────────────┘
```

### 10. fclose — 资源管理

```c
FILE *fp = fopen("data.txt", "w");
fprintf(fp, "Hello");
fclose(fp);  /* ← 必须调用！ */
```

`fclose` 做的事情：
1. **刷新缓冲区**：把未写入的数据强制刷到磁盘
2. **释放文件描述符**：归还给操作系统
3. **释放 FILE 结构体**：释放内存

```c
int result = fclose(fp);
/* result == 0 成功 */
/* result == EOF 失败（检查 errno） */
```

**忘记 fclose 的后果**：
- 🔸 短期程序：数据可能还在缓冲区，文件不完整
- 🔸 长期运行的程序（服务器）：文件描述符泄漏，最终无法打开新文件（每个进程有限额：通常 1024 个）

## 常见错误

### ❌ 错误 1：不检查 fopen 返回值

```c
/* ❌ 错误 — 崩溃! */
FILE *fp = fopen("missing.txt", "r");
fgets(buf, 100, fp);  /* fp == NULL → Segmentation Fault */

/* ✅ 修复 */
FILE *fp = fopen("missing.txt", "r");
if (fp == NULL) {
    fprintf(stderr, "无法打开文件: %s\n", strerror(errno));
    return 1;
}
```

### ❌ 错误 2：忘记 fclose

```c
/* ❌ 数据可能丢失 */
FILE *fp = fopen("output.txt", "w");
fprintf(fp, "important data\n");
/* 没调用 fclose → 缓冲区未刷新 → 文件可能为空! */

/* ✅ 修复 */
FILE *fp = fopen("output.txt", "w");
fprintf(fp, "important data\n");
fclose(fp);  /* ← 刷新缓冲区 + 释放资源 */
```

### ❌ 错误 3：使用 gets()

```c
/* ❌ C11 已删除 */
char buf[10];
gets(buf);  /* 溢出! 💥 */

/* ✅ 修复 */
char buf[10];
fgets(buf, sizeof(buf), stdin);  /* 最多 9 字符 + '\0' */
```

### ❌ 错误 4：二进制读写不检查返回值

```c
/* ❌ 可能文件比预期短 */
fread(&data, sizeof(data), 1, fp);
printf("id = %d\n", data.id);  /* 如果读取失败，data 是垃圾! */

/* ✅ 修复 */
if (fread(&data, sizeof(data), 1, fp) != 1) {
    fprintf(stderr, "读取失败或文件不完整\n");
}
```

## 动手练习

### 🟢 入门：写入 hello.txt

写一个程序，用 `fprintf` 将 `"Hello, File I/O!"` 写入 `hello.txt`，然后读出来打印。确保：
1. 每次 `fopen` 后检查 `NULL`
2. 最后调用 `fclose`

<details><summary>查看答案</summary>

```c
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(void) {
    const char *filename = "hello.txt";

    /* 写入 */
    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "写入失败: %s\n", strerror(errno));
        return 1;
    }
    fprintf(fp, "Hello, File I/O!\n");
    fclose(fp);

    /* 读取 */
    fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "读取失败: %s\n", strerror(errno));
        return 1;
    }
    char buf[256];
    if (fgets(buf, sizeof(buf), fp) != NULL) {
        printf("读取到: %s", buf);
    }
    fclose(fp);

    return 0;
}
```

</details>

### 🟡 中级：逐行统计文件行数

打开一个文本文件，用 `fgets` 逐行读取并统计行数。要求处理空文件（0 行）和多行文件。

<details><summary>查看答案</summary>

```c
#include <stdio.h>

int main(void) {
    FILE *fp = fopen("input.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "无法打开文件\n");
        return 1;
    }

    int line_count = 0;
    char buf[256];
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        line_count++;
    }

    printf("总行数: %d\n", line_count);
    fclose(fp);
    return 0;
}
```

</details>

### 🔴 挑战：序列化 Struct 到二进制文件

定义一个 `Student` struct（包含 id、name、score），写入 3 条记录到 `students.bin`，然后读取并打印。测试跨不同编译器的兼容性（思考题：padding 会影响结果吗？）。

<details><summary>查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    int32_t  id;
    char     name[32];
    double   score;
} Student;

int main(void) {
    Student students[] = {
        {1001, "Alice", 95.5},
        {1002, "Bob",   87.75},
        {1003, "Charlie", 92.0},
    };
    int count = 3;

    /* 写入 */
    FILE *fp = fopen("students.bin", "wb");
    if (fp == NULL) {
        fprintf(stderr, "fopen 失败\n");
        return 1;
    }
    fwrite(students, sizeof(Student), count, fp);
    fclose(fp);

    /* 读取 */
    fp = fopen("students.bin", "rb");
    if (fp == NULL) {
        fprintf(stderr, "fopen 失败\n");
        return 1;
    }

    Student s;
    while (fread(&s, sizeof(Student), 1, fp) == 1) {
        printf("id=%d name=%-10s score=%.2f\n", s.id, s.name, s.score);
    }
    fclose(fp);
    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q：我写了文件但打开是空的，为什么？**

A：忘记 `fclose(fp)` 了。`fprintf` 先写到缓冲区，`fclose` 时才刷新到磁盘。或者调用 `fflush(fp)` 手动刷新。

**Q：fread 返回 0，文件不是空的啊？**

A：检查模式是否正确。用 `"r"` 打开二进制文件 → 在 Windows 上可能提前遇到 `\r\n` 转换问题。尝试 `"rb"`。

**Q：`fgets` 读到的字符串末尾有 `\n`，怎么去掉？**

A：手动检查并替换：

```c
size_t len = strlen(buf);
if (len > 0 && buf[len - 1] == '\n') {
    buf[len - 1] = '\0';
}
```

**Q：`fseek` 之后 `ftell` 返回 -1？**

A：在某些系统上，对某些文件类型（如管道、终端）不能使用 `fseek`/`ftell`。检查文件的 `fp` 是否支持定位操作。

**Q：文件大小用 ftell 怎么算？**

A：跳到末尾再 `ftell`：

```c
fseek(fp, 0, SEEK_END);
long size = ftell(fp);
```

## 知识扩展 (选学)

### 缓冲区与性能

I/O 缓冲是操作系统优化磁盘读写的关键技术：

```
  无缓冲 I/O:
    每次 write → 系统调用 → 磁盘 IO (慢!)
    写 1000 次 = 1000 次磁盘操作

  有缓冲 I/O (FILE*):
    1000 次 fprintf → 写进缓冲区（内存，极快）
    缓冲区满 → 1 次系统调用 → 1 次磁盘操作
```

`setvbuf` 可以自定义缓冲区大小和模式：

```c
char mybuf[8192];
setvbuf(fp, mybuf, _IOFBF, sizeof(mybuf));  /* 8KB 全缓冲 */
```

### 低级 I/O vs 标准 I/O

C 有两层 I/O API：

| 层级 | 函数 | 特点 |
|------|------|------|
| 高级 (标准 I/O) | `fopen`/`fclose`/`fprintf`/`fgets` | 有缓冲，跨平台，易用 |
| 低级 (系统调用) | `open`/`close`/`write`/`read` | 无缓冲，POSIX 专属，更底层 |

对于大多数应用，标准 I/O (`FILE*`) 足够。需要极致性能或特殊操作（如 `mmap`、`epoll`）时才需要低级 I/O。

### 文件权限与安全性

```c
/* Linux 上可以指定文件权限 */
FILE *fp = fopen("/tmp/secret.txt", "w");
/* 权限取决于 umask，通常创建后为 0644 */
```

避免在 `/tmp` 中创建可预测文件名的文件——可能被符号链接攻击。使用 `mkstemp()` 创建唯一的临时文件。

## 小结

本章深入学习了 C 语言的文件 I/O：

- **FILE\* 的本质**：带缓冲区的文件描述符封装，不是直接指向磁盘
- **fopen/fclose**：打开文件、检查 NULL、用完必关、刷新缓冲区
- **fprintf/fscanf**：格式化文件 I/O，类似 printf/scanf
- **fgets vs gets**：gets 已删除，永远用 fgets 指定缓冲区大小
- **fread/fwrite**：二进制读写 struct，速度快但不跨平台
- **文本 vs 二进制**：macOS/Linux 无区别；Windows 上文本模式转换 `\n ↔ \r\n`
- **fseek/ftell**：随机访问文件，定位到任意位置
- **ferror/feof**：错误检测与 EOF 判断

**核心术语**：
- File pointer (FILE*) / 文件指针
- I/O buffer / I/O 缓冲区
- Flush / 刷新（缓冲区数据写入磁盘）
- File descriptor / 文件描述符
- Binary mode / 二进制模式
- Random access / 随机访问

## 术语表

| 英文 | 中文 | 说明 |
|------|------|------|
| `FILE*` | 文件指针 | 标准 I/O 文件句柄，带缓冲区 |
| `fopen` / `fclose` | 打开/关闭文件 | 获取/释放文件资源 |
| `fprintf` / `fscanf` | 格式化文件 I/O | 类似 printf/scanf，操作 FILE* |
| `fgets` / `fputs` | 行 I/O | 安全地读取/写入一行 |
| `fread` / `fwrite` | 二进制 I/O | 按内存块读写 |
| `fseek` / `ftell` | 文件定位 | 移动/获取当前位置 |
| I/O buffer | I/O 缓冲区 | 内存中的临时数据区，批量刷入磁盘 |
| Flush | 刷新 | 将缓冲区数据强制写入磁盘 |
| File descriptor | 文件描述符 | 操作系统层面的文件编号 |
| Endian (字节序) | 大端/小端 | 多字节数据在内存中的存储顺序 |

## 延伸阅读

- **C 标准库**：[cppreference — stdio.h](https://en.cppreference.com/w/c/header/stdio) — 文件 I/O 函数完整参考
- **缓冲机制**：[POSIX — setbuf](https://pubs.opengroup.org/onlinepubs/9699919799/functions/setvbuf.html) — 控制 I/O 缓冲区
- **文件权限**：OWASP — [File Upload Vulnerabilities](https://owasp.org/www-community/vulnerabilities/Unrestricted_File_Upload) — 文件安全
- **低级 I/O**：[GNU libc — Low-Level I/O](https://www.gnu.org/software/libc/manual/html_node/Low-Level-I-O.html) — open/read/write 系统调用

## 继续学习

- 上一章：[字符串深度](./strings)
- 下一章：[命令行参数与 I/O 重定向](./cli_args)

---

> 本章代码位于仓库 [`src/basic/file_io_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/file_io_sample.c)。
> 运行 `make build && make run` 查看完整演示输出。
