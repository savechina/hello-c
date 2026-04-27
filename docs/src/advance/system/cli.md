# CLI 开发模式 (Command-Line Interface)

> "CLI 像餐馆的点单系统—— argv[1] 是主菜，-v 是加辣，--help 是菜单。好的 CLI 让客人轻松点单，差的 CLI 让客人不知所措。"

## 开篇故事

你去一家餐厅，拿到菜单（--help）点了一份牛排（主菜参数），要了三分熟（-c medium），加黑胡椒（--extra pepper）。服务员（CLI 解析器）准确理解了你的需求并传递给厨房。如果少点了一份——服务员会告诉你"缺少必需的食材"。

一个设计良好的 CLI 程序应该有：
- 清晰的帮助信息（菜单）
- 合理的参数顺序
- 有意义的退出码（点单成功/失败/参数错误）
- 参数验证（不能点 5kg 牛排）

## 本章适合谁

- 写了命令行程序但参数处理乱七八糟的人
- 好奇 `ls -la --color` 是怎么解析的的人
- 想编写 Unix 风格工具的人
- 经常被 getopt 搞得头晕的人

## 你会学到什么

- `argc` / `argv` — 命令行参数基础
- `getopt()` — 短选项解析 (`-v`, `-o file`)
- 长选项手动解析 — (`--verbose`, `--output`)
- 退出码约定 — `EXIT_SUCCESS`(0), `EXIT_FAILURE`(1), 自定义
- 参数验证模式 — 必需参数检查、范围校验
- CLI 最佳实践 — stderr、--help、--version

## 前置要求

- 理解 `main(int argc, char **argv)` 签名
- 知道指针和字符串操作
- 会 `printf/fprintf` 基本用法
- 理解 shell 退出码概念

## 第一个例子

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int verbose = 0;
    char *output = NULL;
    int ch;

    while ((ch = getopt(argc, argv, "vo:")) != -1) {
        switch (ch) {
        case 'v':
            verbose = 1;
            break;
        case 'o':
            output = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-v] [-o file] <input>\n", argv[0]);
            return 2;  // 用法错误
        }
    }

    /* 剩余参数 (optind 是非选项参数的索引) */
    if (optind >= argc) {
        fprintf(stderr, "错误: 缺少必需参数 <input>\n");
        fprintf(stderr, "用法: %s [-v] [-o file] <input>\n", argv[0]);
        return 2;
    }

    const char *input = argv[optind];
    if (verbose) {
        fprintf(stderr, "[verbose] input=%s, output=%s\n",
                input, output ? output : "(stdout)");
    }

    /* 主逻辑 ... */
    printf("Processing: %s\n", input);
    return EXIT_SUCCESS;
}
```

`getopt` 循环 → 处理选项 → 检查剩余参数 → 主逻辑。

## 原理解析

### 1. argc / argv 基础

```c
// 运行: ./mytool -v -o output.txt data.txt
int main(int argc, char *argv[]) {
    // argc = 5
    // argv[0] = "./mytool"    (程序名)
    // argv[1] = "-v"          (选项)
    // argv[2] = "-o"          (选项)
    // argv[3] = "output.txt"  (选项参数)
    // argv[4] = "data.txt"    (非选项参数)
    // argv[5] = NULL
}
```

`argc` 是参数数量（含程序名），`argv` 是字符串数组。`argv[0]` 是程序名（用于 help 信息）。

### 2. getopt 短选项

```c
// getopt 字符串: "vo:n:h"
int ch;
while ((ch = getopt(argc, argv, "vo:n:h")) != -1) {
    switch (ch) {
    case 'v':  /* -v: 不需要参数 */
        verbose = 1;
        break;
    case 'o':  /* -o: 需要参数 (冒号) */
        output = optarg;  // optarg = "-o" 后面的参数
        break;
    case 'n':  /* -n: 需要参数 */
        count = atoi(optarg);
        break;
    case 'h':  /* -h: 不需要参数 */
        usage(argv[0]);
        return 0;
    case '?':  /* 未知选项 */
        usage(argv[0]);
        return 2;
    }
}
```

| 字符串 | 含义 |
|--------|------|
| `v` | `v` 选项，不需要参数 |
| `o:` | `o` 选项，需要参数 |
| `n:` | `n` 选项，需要参数 |
| 前面加 `:` | 启用 silent error 模式（不自动打印错误） |

**规则**：`:` 后面需要一个参数，没有 `:` 不需要参数。

### 3. 退出码约定

| 退出码 | 含义 |
|--------|------|
| 0 (EXIT_SUCCESS) | 成功 |
| 1 (EXIT_FAILURE) | 通用失败 |
| 2 | 用法错误 / 缺参数 |
| 2 | 文件不存在 |
| 3 | 权限不足 |
| 4 | 内存不足 |
| 126 | 找到但不能执行 |
| 127 | 未找到 |
| 128+N | 被信号 N 杀死 |

```c
/* 检查文件 */
FILE *f = fopen(path, "r");
if (!f) {
    fprintf(stderr, "错误: 找不到文件 '%s'\n", path);
    return 2;
}

/* 内存分配 */
void *ptr = malloc(size);
if (!ptr) {
    fprintf(stderr, "错误: 内存不足\n");
    return 4;
}
```

### 4. Usage 模板

```c
void usage(const char *prog) {
    fprintf(stderr, "用法: %s [选项] <必需参数> [可选参数]\n", prog);
    fprintf(stderr, "\n选项:\n");
    fprintf(stderr, "  -v, --verbose    显示详细信息\n");
    fprintf(stderr, "  -o, --output F   输出到文件 F\n");
    fprintf(stderr, "  -n, --count N    处理 N 行\n");
    fprintf(stderr, "  -h, --help       显示帮助\n");
    fprintf(stderr, "  -V, --version    显示版本\n");
    fprintf(stderr, "\n示例:\n");
    fprintf(stderr, "  %s -v -o out.txt input.txt\n", prog);
}
```

**规范**：
- `--help` 写到 stderr（Unix 惯例）或直接 stdout 都可以
- 错误信息和用法写到 **stderr**（不是 stdout）
- 正常结果写到 **stdout**（管道友好）

### 5. 参数验证清单

```c
/* 必需参数 */
if (optind >= argc) {
    fprintf(stderr, "错误: 缺少必需参数\n");
    usage(argv[0]);
    return 2;
}

/* 数值范围 */
if (n <= 0) {
    fprintf(stderr, "错误: --count 必须是正整数\n");
    return 2;
}

/* 互斥选项 */
if (use_stdin && input_file) {
    fprintf(stderr, "错误: --stdin 和 -f 不能同时使用\n");
    usage(argv[0]);
    return 2;
}

/* 文件存在性 */
if (access(path, F_OK) != 0) {
    fprintf(stderr, "错误: 文件不存在: %s\n", path);
    return 2;
}

/* 文件可读 */
if (access(path, R_OK) != 0) {
    fprintf(stderr, "错误: 文件不可读: %s\n", path);
    return 3;
}
```

## 常见错误

### ❌ 错误 1: 不检查 optarg

```c
// ❌ -o 后面没参数时 optarg 可能为空或指向下一个 argv
case 'o':
    output = optarg;  // 如果用户只写了 -o → 可能指向 -v 或 NULL!

// ✅ 检查
case 'o':
    if (!optarg) {
        fprintf(stderr, "错误: -o 需要参数\n");
        return 2;
    }
    output = optarg;
    break;
```

### ❌ 错误 2: 正常输出到 stderr

```c
// ❌ 结果输出到 stderr — 管道会丢数据
printf = fprintf(stderr, "result: %d\n", result);
// shell: ./tool | wc -l  → 不统计 stderr!

// ✅ 正常结果输出到 stdout
printf("result: %d\n", result);
// 错误/帮助输出到 stderr
fprintf(stderr, "Usage: ...\n");
```

### ❌ 错误 3: 不处理 optind

```c
// ❌ 忽略 optind → 丢失非选项参数
int ch;
while ((ch = getopt(argc, argv, "v")) != -1) { /* ... */ }
// 用户给的 input file 在哪？忘了检查 optind!

// ✅ getopt 后检查 optind
if (optind >= argc) {
    fprintf(stderr, "错误: 缺少文件参数\n");
    return 2;
}
const char *file = argv[optind];
```

### ❌ 错误 4: exit vs return

```c
// ❌ 在 main 中用 exit() 而不是 return
exit(0);  // 可行但不规范

// ✅ main 中用 return
return EXIT_SUCCESS;

/* exit() 适合从其他函数提前退出 */
void process(const char *path) {
    if (!path) {
        fprintf(stderr, "错误\n");
        exit(1);  // 从非 main 函数退出
    }
}
```

## 动手练习

### 🟢 练习 1: 实现 cat 基础版

写一个 `mycat` 程序，支持参数 `-n` 显示行号，接受文件或 stdin。

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int show_number = 0;
    int ch;
    while ((ch = getopt(argc, argv, "n")) != -1) {
        if (ch == 'n') show_number = 1;
    }
    
    for (int i = optind; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (!f) { perror(argv[i]); continue; }
        
        char buf[1024];
        int line = 1;
        while (fgets(buf, sizeof(buf), f)) {
            if (show_number) printf("%6d  ", line++);
            printf("%s", buf);
        }
        fclose(f);
    }
    return 0;
}
```
</details>

### 🟡 练习 2: 完整的 usage

为你的工具实现 `--help` 和 `--version`，包含示例用法。

<details>
<summary>点击查看答案</summary>

```c
void usage(const char *prog) {
    fprintf(stderr, "%s v1.0 - File Processor\n", prog);
    fprintf(stderr, "用法: %s [选项] <file>\n\n", prog);
    fprintf(stderr, "  -v, --verbose     详细模式\n");
    fprintf(stderr, "  -o, --output F    输出文件\n");
    fprintf(stderr, "  -n, --lines N     处理前 N 行\n");
    fprintf(stderr, "  -h, --help        显示帮助\n");
    fprintf(stderr, "  -V, --version     显示版本\n\n");
    fprintf(stderr, "示例:\n");
    fprintf(stderr, "  %s -v -o out.txt data.csv\n", prog);
    fprintf(stderr, "  %s -n 10 large_log.txt\n", prog);
}
```
</details>

### 🔴 练习 3: 长选项解析

实现长选项解析（`--verbose`、`--output=f`），不用 `getopt_long`。

<details>
<summary>点击查看答案</summary>

```c
for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--verbose") == 0) {
        verbose = 1;
    } else if (strncmp(argv[i], "--output=", 9) == 0) {
        output = argv[i] + 9; // --output=<value>
    } else if (strcmp(argv[i], "--help") == 0) {
        usage(argv[0]);
        return 0;
    } else if (argv[i][0] == '-') {
        fprintf(stderr, "未知选项: %s\n", argv[i]);
        return 2;
    } else {
        // 非选项参数
        inputs[input_count++] = argv[i];
    }
}
```
</details>

## 故障排查

### Q: getopt 不识别选项

检查 getopt 字符串是否正确。`getopt(argc, argv, "vo:n")` 意味着能解析 `-v`, `-o FILE`, `-n NUM`。

### Q: "option requires an argument" 错误

选项后面没跟参数。`./tool -o` → `-o` 需要参数但没给。检查 getopt 字符串中是否误加了 `:`。

### Q: --version 应该写 stdout 还是 stderr？

GNU 惯例：`--version` 和 `--help` 写 **stdout**；错误用法写 **stderr**。

## 知识扩展

### 1. getopt_long — GNU 长选项

```c
#include <getopt.h>

struct option long_opts[] = {
    { "verbose",  no_argument,       0, 'v' },
    { "output",   required_argument, 0, 'o' },
    { "help",     no_argument,       0, 'h' },
    { "version",  no_argument,       0, 'V' },
    { 0, 0, 0, 0 }
};

int opt;
while ((opt = getopt_long(argc, argv, "vo:hV", long_opts, NULL)) != -1) {
    /* 和处理短选项一样 */
}
```

### 2. 环境变量

```c
const char *home = getenv("HOME");
const char *editor = getenv("EDITOR");  // $EDITOR
if (!editor) editor = "vi";             // 默认值
```

### 3. POSIX 选项约定

- 短选项：`-X` 或 `-X value`（GNU 风格，一个字符）
- 长选项：`--word` 或 `--word=value`（GNU 风格，完整单词）
- `--` 表示选项结束：`./tool -- -file.txt`（-file.txt 是文件，不是选项）

## 小结

- **getopt** 解析短选项，字符串中 `:` 表示需要参数
- **退出码**：0=成功，2=用法错误，127=未找到
- **错误写到 stderr**，正常输出到 **stdout**
- **参数验证**：必需参数、数值范围、互斥选项
- **--help** 和 **--version** 是每个 CLI 程序的标配

> **我的教训**：第一次写 CLI 时，我把结果输出到 stderr，导致管道不工作（`./tool | wc` 为空）。记住：**正常输出 stdout，错误输出 stderr**。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 可选参数（Optional Argument） | 方括号可选的参数 [options] |
| 必选参数（Required Argument） | 尖括号必需参数 <file> |
| 退出码（Exit Code） | 程序结束时返回的状态码 |
| stderr | 标准错误 |
| getopt | POSIX 选项解析函数 |

## 延伸阅读

- [POSIX getopt](https://pubs.opengroup.org/onlinepubs/9699919799/functions/getopt.html) — 官方规范
- [GNU 命令行接口惯例](https://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html) — 命名规范
- [The Art of Command Line](https://github.com/jlevy/the-art-of-command-line) — CLI 最佳实践

## 继续学习

你已经掌握了 CLI 开发的核心技能。至此，系统调用章节的 6 个子章节全部完成——文件 I/O、信号、mmap、进程、IPC、CLI，覆盖了 POSIX 系统编程的完整工具箱。

> 💡 **提示**：运行 `src/advance/system_cli_sample.c` 查看演示模式。`make build && make run`。

[← 上一章：管道与 IPC](./ipc.md) | [系统调用总览 →](../system.md)
