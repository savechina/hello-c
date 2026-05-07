# 命令行参数与 I/O 重定向 (CLI Args & I/O Redirect)

## 开篇故事

想象你在快餐店点餐。你告诉柜台你要什么（参数），柜台把食物递给你（返回值）。你不需要知道厨房怎么操作，也不需要知道食物怎么送到你手上。I/O 重定向更像是一条传送带：你把需求放进去，结果从另一边出来，中间的协作完全透明。

命令行参数的道理完全一样。`argc` / `argv` 就是把用户输入交给程序的「点餐接口」。程序拿到参数，处理完，把结果写回 stdout。至于结果去了终端、文件还是下一个程序的 stdin，程序不需要知道——那是 Shell 操心的事。

这就是 Unix 哲学：程序做一件事，通过命令行参数和 I/O 重定向可以无限组合。

## 本章适合谁

- 写过 Python 脚本，用 `sys.argv` 或 `argparse` 处理过命令行参数
- 在终端用过 `>`、`<`、`|` 但想理解底层原理
- 想写出像 `grep`、`cat`、`wc` 那样好用的命令行工具
- 好奇 C 程序中"标准输入"和"标准输出"到底是什么

## 你会学到什么

1. `main(int argc, char *argv[])`：C 程序如何接收命令行参数
2. **安全核心**：检查 `argc` 再访问 `argv`（防止越界崩溃）
3. 参数解析：识别 `-v`、`--flag=value` 等常见模式
4. `stdin`/`stdout`/`stderr`：三个标准流的本质与区别
5. **认知对比**：Python `sys.argv` vs C `argc/argv`
6. 读取用户输入：`fgets` 从 `stdin` 读取
7. I/O 重定向：`>`、`<`、`>>` 的工作原理
8. 管道编程：`|` 如何连接两个程序的 stdin/stdout
9. `stderr` vs `stdout`：为什么错误信息需要单独输出
10. `getopt` 概念：标准参数解析函数（选学）

## 前置要求

- 已完成 [文件 I/O](./file_io) 章节（`fprintf(stderr, ...)` 的使用）
- 理解指针（`char *argv[]` 是字符串数组）
- 基本终端操作经验（在命令行运行过程）
- 已配置 C 编译环境（`gcc` 或 `clang`）

> **💡 编译命令**：本章代码使用 `-Wall -Wextra -Werror -std=c17` 编译，所有警告视为错误。

## 第一个例子

最简短的命令行参数程序：

```c
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = \"%s\"\n", i, argv[i]);
    }
    return 0;
}
```

编译并运行：

```bash
gcc -Wall -Wextra -Werror -std=c17 -o demo demo.c
./demo hello world
```

输出：

```
argc = 3
argv[0] = "./demo"
argv[1] = "hello"
argv[2] = "world"
```

完整源码在仓库 [`src/basic/cli_args_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/cli_args_sample.c)。

> **注意**：本章节示例代码通过 coordinator 调用，无法接收真正的命令行参数。实际运行时会使用**模拟的 argv 数组**来演示解析逻辑。

## 原理解析

### 1. main(int argc, char *argv[])

每个 C 程序都有一个 `main` 函数。最常见的两个签名：

```c
/* 无参数版本 */
int main(void) { ... }

/* 有命令行参数版本 */
int main(int argc, char *argv[]) { ... }
```

参数含义：

| 参数 | 类型 | 含义 |
|------|------|------|
| `argc` | `int` | Argument Count（参数个数） |
| `argv` | `char*[]` | Argument Values（参数值数组） |

```
  命令行: ./hello -v --mode=fast input.txt

  argc = 4
  argv[0] = "./hello"          ← 程序名（总是存在）
  argv[1] = "-v"               ← 第一个用户参数
  argv[2] = "--mode=fast"      ← 第二个参数
  argv[3] = "input.txt"        ← 第三个参数
  argv[4] = NULL               ← 数组以 NULL 结尾
```

**关键规则**：`argv[0]` 总是程序名。真正用户输入的数从 `argv[1]` 开始。

### 2. 错误优先：检查 argc 再访问 argv

```c
/* ❌ 危险：不检查 argc */
int main(int argc, char *argv[]) {
    printf("%s\n", argv[1]);  /* 如果用户只输入 "./hello" → 越界! 💥 */
}
```

当用户只运行 `./hello`（没有额外参数）时：
- `argc = 1`（只有程序名）
- `argv[1]` 不存在！访问它 = **Segmentation Fault**

```c
/* ✅ 正确：先检查 argc */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <filename>\n", argv[0]);
        return 1;  /* 非零返回值 = 程序出错 */
    }
    printf("%s\n", argv[1]);  /* 现在安全了 */
}
```

**经验法则**：使用 `argv[i]` 之前，先确认 `argc > i`。

### 3. Python sys.argv vs C argc/argv

| 特性 | Python `sys.argv` | C `argc/argv` |
|------|-------------------|---------------|
| 获取方式 | `import sys; sys.argv` | `int main(int argc, char *argv[])` |
| 类型 | `list[str]` | `char*[]`（字符串数组） |
| 越界行为 | `IndexError`（可捕获） | **Segmentation Fault**（程序直接崩溃） |
| 数字解析 | `int(sys.argv[1])` | `atoi(argv[1])` 或 `strtol(argv[1], ...)` |
| 参数个数 | `len(sys.argv)` | `argc` |

```python
# Python
import sys
print(sys.argv[1])    # 或 "world"
print(len(sys.argv))  # 2
```

```c
// C
printf("%s\n", argv[1]);  // 或 "world"
printf("%d\n", argc);     // 2
```

**我的理解**：Python 把 argv 包装成一个安全的列表，越界会抛出异常。C 把它暴露成裸指针数组，越界就是未定义行为。Python 保护你，C 信任你。两者各有取舍。

### 4. 参数解析模式

命令行参数有三种常见格式：

```c
/* ── 简单标志（无值） ── */
if (strcmp(argv[i], "-v") == 0) {
    verbose = 1;
}

/* ── 等号参数（带值） ── */
if (strncmp(argv[i], "--mode=", 7) == 0) {
    mode = argv[i] + 7;  /* "--mode=fast" → "fast" */
}

/* ── 位置参数（文件名等） ── */
if (argv[i][0] != '-') {
    input_file = argv[i];  /* 不以 - 开头的参数 */
}
```

**常见命令行惯例**：

| 格式 | 示例 | 含义 |
|------|------|------|
| `-x` | `-v` | 单字母标志（verbose） |
| `--word` | `--help` | 长选项（help） |
| `--key=value` | `--mode=fast` | 带值的选项 |
| `位置参数` | `input.txt` | 非 `-` 开头的参数 |

> **选学**：实际项目中使用 `getopt()`（POSIX 标准库）或第三方解析库（如 `argparse`），避免手动 `strcmp` 解析。

### 5. stdin / stdout / stderr

每个 C 程序启动时，操作系统自动打开三个文件流：

```
  ┌── 三个标准流 ──────────────────────────┐
  │                                          │
  │  流          描述符  默认方向            │
  │  ──────      ──────  ──────────         │
  │  stdin      0        键盘 → 程序        │
  │  stdout     1        程序 → 终端        │
  │  stderr     2        程序 → 终端        │
  │                                          │
  │  数据流:                                 │
  │  ┌── 键盘 ──┐                           │
  │  │           │ stdin (fd 0)             │
  │  │ [输入] ───→  ┌──────────────┐       │
  │  │           │     │ 程序       │       │
  │  └───────────┘     │            │       │
  │                ┌──→│            │       │
  │                │   └──┬──────┬──┘       │
  │                │      │      │          │
  │          stdout(1)  stderr(2)           │
  │                ↓      ↓                  │
  │             正常输出  错误消息             │
  └──────────────────────────────────────────┘
```

**关键区别**：

| 流 | 函数 | 缓冲模式 | 用途 |
|---|------|---------|------|
| `stdin` | `fgets(buf, n, stdin)` | 行缓冲 | 读取用户输入 |
| `stdout` | `printf("...")` / `fprintf(stdout, ...)` | 行缓冲（终端时） | 正常输出 |
| `stderr` | `fprintf(stderr, "错误!\n")` | **无缓冲** | 错误/诊断信息 |

```c
printf("正常输出\n");               /* → stdout */
fprintf(stderr, "出错了!\n");        /* → stderr */
```

**为什么 stderr 需要独立？** 因为用户可以重定向 stdout 到文件，但仍想在终端看到错误：

```bash
./program > output.txt    # 正常输出进文件
                          # 错误信息仍然显示在终端 ✅
```

如果错误也用 stdout，用户会被 redirect 到文件里，看不到任何反馈。

### 6. 从 stdin 读取输入

```c
char buf[256];
printf("请输入你的名字: ");
fgets(buf, sizeof(buf), stdin);  /* 等待用户输入，最多 255 字符 */
printf("你好, %s", buf);  /* buf 包含末尾的 \n */
```

`fgets(buf, sizeof(buf), stdin)` 的行为：
1. 等待用户输入（程序阻塞）
2. 读到 `\n` 或缓冲区满时停止
3. 始终在末尾加 `\0`
4. 遇到 EOF（Ctrl+D / Ctrl+Z）返回 NULL

**与 `gets()` 对比**：

```c
/* ❌ gets — 永远不要用 */
gets(buf);  /* 不知道缓冲区大小 → 溢出 */

/* ✅ fgets — 安全 */
fgets(buf, sizeof(buf), stdin);  /* 指定大小 → 安全 */
```

### 7. I/O 重定向

Shell 提供了强大的重定向操作符：

```bash
# ── 输出重定向 ──
./program > output.txt      # 覆盖写入文件
./program >> output.txt     # 追加到文件末尾

# ── 输入重定向 ──
./program < input.txt       # 从文件读取 stdin

# ── 错误重定向 ──
./program 2> errors.log     # stderr 到文件

# ── 全重定向 ──
./program > all.log 2>&1    # stdout + stderr 都到文件
```

**工作原理**：重定向在程序**启动前**由操作系统完成。程序照常 `printf` / `fgets`，操作系统把 stdin/stdout 的文件描述符指向了文件而非终端。

```c
/* 程序代码不需要任何改变 */
printf("输出一些数据\n");    /* 终端 or 文件？程序不知道也不需要知道 */
fgets(buf, sizeof(buf), stdin);  /* 键盘 or 文件？程序不知道也不需要知道 */
```

这是 **Unix 哲学** 的核心——程序只做一件事，不关心输入从哪来、输出到哪去。

### 8. 管道编程

管道 `|` 把一个程序的 stdout 连接到另一个程序的 stdin：

```bash
cat log.txt | grep "ERROR" | wc -l
```

```
  ┌── 管道链 ──────────────────────────┐
  │                                     │
  │  cat log.txt ──→ stdout ──┐         │
  │                           ↓         │
  │                  grep "ERR"         │
  │                     ^   ↓           │
  │                     │   stdout ──┐  │
  │  stdin ─────  wc -l │            ↓  │
  │                     │      终端显示   │
  └─────────────────────┴─────────────┘
```

每个程序只负责读 stdin、写 stdout。管道让它们协同工作。

**C 程序无需任何代码改动**就能参与管道：

```c
/* 这个程序可以用作管道中的任何一个环节 */
char buf[256];
while (fgets(buf, sizeof(buf), stdin) != NULL) {
    /* 处理每一行 */
    printf("%s", buf);  /* 输出到 stdout → 下一个程序的 stdin */
}
```

### 9. 交互式输入 vs 批处理

```c
/* ── 交互式模式：等待用户输入 ── */
while (1) {
    printf("> ");
    if (fgets(buf, sizeof(buf), stdin) == NULL) break;  /* EOF */
    /* 处理命令 */
}

/* ── 批处理模式：处理所有可用输入 ── */
while (fgets(buf, sizeof(buf), stdin) != NULL) {
    /* 处理每一行，遇到 EOF 自动退出 */
}
```

两种模式的区别：交互式循环需要显式 quit 命令，批处理遇到 EOF 自动退出。当 stdin 连接到终端时，EOF 需要用户按 Ctrl+D。当 stdin 连接到文件/管道时，读完后自动遇到 EOF。

## 常见错误

### ❌ 错误 1：不检查 argc

```c
/* ❌ 越界! */
int main(int argc, char *argv[]) {
    printf("%s\n", argv[1]);  /* 如果 argc==1 → 崩溃! */
}

/* ✅ 修复 */
int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <filename>\n", argv[0]);
        return 1;
    }
    printf("%s\n", argv[1]);
}
```

### ❌ 错误 2：用 atoi 不检查转换结果

```c
/* ❌ atoi("abc") 返回 0，不报错 */
int n = atoi(argv[1]);
printf("%d\n", n);  /* 如果用户输入 "abc" → 0，但用户以为是 0! */

/* ✅ 用 strtol 检查 */
#include <stdlib.h>
#include <errno.h>

char *endptr;
errno = 0;
long n = strtol(argv[1], &endptr, 10);
if (errno != 0 || *endptr != '\0') {
    fprintf(stderr, "无效数字: %s\n", argv[1]);
    return 1;
}
```

### ❌ 错误 3：混淆 argv 和文件内容

```c
/* ❌ argv[1] 只是文件名，不是文件内容! */
printf("文件内容: %s\n", argv[1]);  /* 只打印 "data.txt" 这几个字! */

/* ✅ 正确：需要先打开文件 */
FILE *fp = fopen(argv[1], "r");
char buf[256];
while (fgets(buf, sizeof(buf), fp) != NULL) {
    printf("%s", buf);  /* 这才是文件内容 */
}
fclose(fp);
```

### ❌ 错误 4：误解 argv 指向的内存

```c
/* ❌ argv 指向的字符串可能位于只读段 */
argv[1][0] = 'A';  /* 某些系统上 → Segmentation Fault */

/* ✅ 需要修改时复制到自己的缓冲区 */
char name[256];
strncpy(name, argv[1], sizeof(name) - 1);
name[sizeof(name) - 1] = '\0';
name[0] = 'A';  /* 安全 */
```

## 动手练习

### 🟢 入门：打印所有参数

写一个程序，打印出传入的 argc 和所有 argv。测试 `./demo a b c` 和 `./demo`（无参数）。

<details><summary>查看答案</summary>

```c
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = \"%s\"\n", i, argv[i]);
    }
    return 0;
}
```

</details>

### 🟡 中级：支持 -v 和 -o 参数

写一个程序，支持 `-v`（verbose 模式）和 `-o <filename>`（输出文件）参数。解析后打印配置。

提示：
- `-v` 后面没有值
- `-o <filename>` 后面紧跟一个参数
- 剩余参数是位置参数

<details><summary>查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int verbose = 0;
    const char *output = "stdout";
    int input_args = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output = argv[++i];  /* 取下一个参数 */
        } else {
            input_args++;
            if (verbose) {
                printf("处理输入: %s\n", argv[i]);
            }
        }
    }

    printf("verbose=%d, output=%s, input_count=%d\n",
           verbose, output, input_args);
    return 0;
}
```

</details>

### 🔴 挑战：实现简易 grep

从 stdin 逐行读取，如果一行包含指定字符串（argv[1]），则将该行打印到 stdout。这样可以使用管道：

```bash
cat log.txt | ./mygrep "ERROR"
```

<details><summary>查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "用法: %s <pattern>\n", argv[0]);
        return 1;
    }

    const char *pattern = argv[1];
    char buf[256];

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (strstr(buf, pattern) != NULL) {
            printf("%s", buf);
        }
    }
    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q：为什么 printf 的输出没有立即显示？**

A：终端环境下 `stdout` 是**行缓冲**的——遇到 `\n` 才刷新。如果 `printf("Hello")` 没有 `\n`，输出可能在缓冲区里。加上 `\n` 或调用 `fflush(stdout)`。`stderr` 是无缓冲的，`fprintf(stderr, ...)` 立即显示。

**Q：从文件重 stdin 读时，fgets 的行为和在终端有什么同？**

A：完全相同！`fgets` 不知道数据来自键盘还是文件。唯一的区别：从终端需要用户按回车，从文件直接读取下一行直到 EOF。

**Q：如何让程序既可以交互又可以管道？**

A：两种模式代码相同。关键在于——如果 stdin 来自终端且用户不输入，程序阻塞等待；如果来自文件/管道，读完 EOF 自动退出。

**Q：`argv[argc]` 是什么？**

A：标准保证 `argv[argc] == NULL`。所以你可以遍历 `for (char **p = argv; *p != NULL; p++)`。

**Q：getopt 和手动解析有什么区别？**

A：`getopt` 自动处理 `-ab`（合并标志）和 `-o value` 等常见模式，减少手动 `strcmp` 代码。但 `getopt` 是 POSIX 扩展，不是 C 标准库。Windows 上需要额外配置。

## 知识扩展 (选学)

### getopt — 标准参数解析

当参数变多时，`getopt` 简化了解析逻辑：

```c
#include <unistd.h>

int opt;
while ((opt = getopt(argc, argv, "vh:o:")) != -1) {
    switch (opt) {
    case 'v':
        verbose = 1;
        break;
    case 'h':
        host = optarg;  /* getopt 自动解析 "host:port" */
        break;
    case 'o':
        output = optarg;
        break;
    case '?':
        fprintf(stderr, "未知选项\n");
        break;
    }
}
/* 处理剩余位置参数: optind 之后 */
```

格式字符串 `"vh:o:"` 含义：
- `v`：无值标志
- `h:`：需要值的标志（`-h <host>`）
- `o:`：需要值的标志（`-o <output>`）

### exit() 返回值

```c
#include <stdlib.h>

exit(0);  /* 正常退出 */
exit(1);  /* 一般错误 */
exit(2);  /* 用法错误 */
```

程序退出后，shell 可以通过 `$?` 获取返回值：

```bash
$ ./program
$ echo $?
0
```

### 环境变量

除了 CLI 参数，程序还可以通过 `getenv()` 读取环境变量：

```c
#include <stdlib.h>

const char *path = getenv("PATH");
const char *home = getenv("HOME");

if (path == NULL) {
    fprintf(stderr, "PATH 未设置\n");
}
```

`getenv` 返回字符串指针，`NULL` 表示环境变量未设置。

## 小结

本章深入学习了 C 语言的命令行参数与 I/O 重定向：

- **argc/argv**：`main` 接收命令行参数，`argv[0]` 是程序名
- **安全的参数访问**：始终检查 `argc` 再访问 `argv[i]`
- **参数解析**：识别 `-v`、`--flag=value`、位置参数
- **三个标准流**：stdin（0）、stdout（1）、stderr（2）
- **Python vs C**：Python 安全的 `sys.argv` vs C 裸 `argc/argv`
- **stdin 读取**：`fgets(buf, n, stdin)` — 安全的交互式输入
- **I/O 重定向**：`>` 覆盖、`>>` 追加、`<` 输入，程序无需改变
- **管道**：`|` 连接 stdout → stdin，组合小工具
- **stderr 独立**：错误信息走 stderr，不受 stdout 重定向影响

**核心术语**：
- Command-line arguments / 命令行参数
- Standard streams / 标准流（stdin, stdout, stderr）
- I/O redirection / I/O 重定向
- Pipe / 管道
- File descriptor / 文件描述符

## 术语表

| 英文 | 中文 | 说明 |
|------|------|------|
| `argc` | 参数计数 (Argument Count) | 传入 main 的参数个数 |
| `argv` | 参数值 (Argument Values) | 参数字符串数组 |
| `stdin` | 标准输入 | 文件描述符 0，默认连接键盘 |
| `stdout` | 标准输出 | 文件描述符 1，默认连接终端 |
| `stderr` | 标准错误 | 文件描述符 2，默认连接终端，无缓冲 |
| File descriptor | 文件描述符 | 操作系统中文件的编号（0, 1, 2, ...） |
| I/O redirection | 输入/输出重定向 | 通过 `>`、`<` 改变 stdin/stdout 的流向 |
| Pipe | 管道 | 通过 `|` 连接前一个 stdout 和后一个 stdin |
| Block | 阻塞 | 程序暂停等待输入（如 fgets 等待用户输入） |
| EOF | 文件末尾 | End Of File，Ctrl+D / Ctrl+Z 触发 |

## 延伸阅读

- **Unix 哲学**：[The Art of Unix Programming](http://www.catb.org/~esr/writings/taoup/) — 管道与重定向的哲学基础
- **getopt**：[GNU libc — Parsing Program Arguments](https://www.gnu.org/software/libc/manual/html_node/Getopt.html) — 标准参数解析
- **标准流**：[POSIX — Standard I/O Streams](https://pubs.opengroup.org/onlinepubs/9699919799/functions/stdin.html) — stdin/stdout/stderr 规范
- **cppreference — stdio.h**：[C 标准 I/O 库](https://en.cppreference.com/w/c/header/stdio)

## 继续学习

- 上一章：[标准库精要](./stdlib.md)
- 下一章：[递归函数](./recursion.md)

---

> 本章代码位于仓库 [`src/basic/cli_args_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/cli_args_sample.c)。
> 运行 `make build && make run` 查看完整演示输出。
