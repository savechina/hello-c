# 字符串处理 (String Processing)

如果说 `strcpy` 和 `strcmp` 是字符串世界的砖块和水泥，那么 `strtok`、`strstr`、`strchr` 就是工具箱里的锯子和刨子——它们帮你把原始字符串**加工**成有意义的片段。本章我将学习如何拆分、搜索和解析字符串。

## 开篇故事

我第一次用 `strtok` 解析配置文件的时候，写了一段看起来天衣无缝的代码：

```c
char config[] = "key=value;flag=true";
char *token = strtok(config, ";");
while (token) {
    /* 处理 token */
    token = strtok(NULL, ";");
}
```

它跑得很好——直到我的配置文件里多了个嵌套的分号，或者我想同时解析两行配置。然后我发现：**`strtok` 会修改原字符串**，而且**不是线程安全的**。

这就像用一把会吃木头的锯子——每次锯完，木头就变了样。如果你需要保留原始数据，得先复印一份。

## 本章适合谁

- 已完成 [安全字符串](./safe_strings.md)，掌握 `strncpy` 和 `snprintf`
- 想学习如何拆分、搜索和处理字符串
- 有 CSV 解析、日志处理等实际需求
- 想知道 `strtok` 的陷阱和替代方案

## 你会学到什么

1. `strtok()` — 按分隔符拆分字符串（及其陷阱）
2. `strstr()` — 查找子串
3. `strchr()` — 查找单个字符
4. `strspn()` / `strcspn()` — 匹配/跳过字符集合
5. `strtol()` — 安全地将字符串转为数字
6. 实战：安全 CSV 解析、路径提取、空白去除

## 前置要求

- 已完成 [安全字符串](./safe_strings.md)
- 理解字符串在内存中以 `'\0'` 结尾

## 第一个例子

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    /* 拆分: 用 strtok 按逗号分隔 */
    char data[] = "apple,banana,cherry";
    char *token = strtok(data, ",");

    printf("tokens: ");
    while (token != NULL) {
        printf("[%s] ", token);
        token = strtok(NULL, ",");
    }
    printf("\n");

    /* 查找: 用 strstr 找子串 */
    const char *text = "Hello, C Programming!";
    char *found = strstr(text, "C");
    if (found) {
        printf("找到 'C' 在偏移 %ld 处\n", found - text);
    }

    /* 搜索字符: 用 strchr */
    char *dot = strchr(text, '!');
    if (dot) {
        printf("'!' 在偏移 %ld 处\n", dot - text);
    }

    return 0;
}
```

完整源码在 [`src/basic/string_processing_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/string_processing_sample.c)。

## 原理解析

### 1. `strtok()` — 拆分字符串

```c
char text[] = "apple,banana,cherry";
char *token = strtok(text, ",");  /* 第一次: 传入字符串 */
while (token != NULL) {
    printf("%s\n", token);
    token = strtok(NULL, ",");    /* 后续: 传入 NULL */
}
/* → apple
     banana
     cherry */
```

**工作原理**：`strtok` 在分隔符位置插入 `'\0'`，把原字符串分割成多个子串。每次返回下一个子串的指针。

```
┌─────────────────────────────────────────────────────────┐
│                strtok 拆分过程可视化                       │
│                                                         │
│  原始字符串: "apple,banana,cherry"                        │
│                                                         │
│  "apple"    "banana"      "cherry"                        │
│  ┌───────┐  ┌──────────┐  ┌────────┐                     │
│  │ a ppl │e │\0 b anan │a │\0 cherry│\0                  │
│  └───┬───┘  └─────┬────┘  └───┬────┘                     │
│      ↑            ↑           ↑                         │
│   token 1      token 2     token 3                      │
│      ↓            ↓           ↓                         │
│  "apple"      "banana"    "cherry"                      │
│                                                         │
│  返回值: 指向每个子串首字符的指针                           │
│  修改:   原串被修改（',' 变成 '\0'）                       │
└─────────────────────────────────────────────────────────┘
```

**三个重要警告**：

1. **修改原字符串**——`strtok` 会在分隔符处写入 `'\0'`。不要传给字符串字面量或需要保留的字符串。
2. **不是线程安全**——内部用静态变量保存状态。多线程使用 `strtok_r`（POSIX）。
3. **连续分隔符被跳过**——`"a,,b"` 用 `","` 分隔得到 `"a"` 和 `"b"`。

```c
/* 如果需要保留原字符串，先复制 */
char work_buf[128];
strncpy(work_buf, original, sizeof(work_buf) - 1);
work_buf[sizeof(work_buf) - 1] = '\0';
char *token = strtok(work_buf, ",");
```

### 2. `strstr()` — 查找子串

```c
const char *text = "The quick brown fox";
char *found = strstr(text, "brown");

if (found) {
    printf("找到! 偏移: %ld\n", found - text);  /* 10 */
    printf("剩余: %s\n", found);                  /* "brown fox" */
} else {
    printf("未找到\n");
}
```

返回指向子串首次出现位置的指针，找不到返回 `NULL`。区分大小写。

**查找所有出现位置**：

```c
const char *text = "abcabcabc";
const char *pos = text;
int count = 0;

while ((pos = strstr(pos, "abc")) != NULL) {
    count++;
    pos++;  /* 从下一个字符继续找 */
}
printf("找到 %d 次\n", count);  /* 3 */
```

### 3. `strchr()` — 查找单个字符

```c
const char *path = "/home/user/docs/file.txt";
char *slash = strchr(path, '/');

if (slash) {
    printf("第一个 '/': 偏移 %ld\n", slash - path);  /* 0 */
}

/* 找最后一个 '/'——循环搜索 */
char *last_slash = NULL;
const char *p = path;
while ((p = strchr(p, '/')) != NULL) {
    last_slash = (char *)p;
    p++;
}
if (last_slash) {
    printf("文件名: %s\n", last_slash + 1);  /* "file.txt" */
}
```

### 4. `strspn()` / `strcspn()` — 字符集合匹配

```c
/* strspn: 前缀中全部由集合内字符组成的最大长度 */
strspn("12345abc", "0123456789")  /* → 5 (前5个都是数字) */

/* strcspn: 到第一个集合内字符的位置 */
strcspn("hello123world", "0123456789")  /* → 5 (到第一个数字前) */
```

```
"12345abc"
 ↑    ↑
 │    └── strspn 在这里停 (遇到 'a')
 └─── 5 个字符都匹配 "0-9"

"hello123world"
     ↑
     └── strcspn 在这里停 (遇到 '1')
 前 5 个不是数字
```

### 5. `strtol()` — 安全的字符串转数字

`atoi()` 看起来方便但不能检测错误。`strtol()` 是正确的方式：

```c
#include <stdlib.h>

const char *input = "42";
char *endptr = NULL;
long value = strtol(input, &endptr, 10);

if (endptr == input) {
    printf("没有数字\n");
} else if (*endptr != '\0') {
    printf("部分解析: %ld, 剩余: \"%s\"\n", value, endptr);
} else {
    printf("完整解析: %ld\n", value);
}
```

**`base` 参数**：
- `10` — 十进制
- `16` — 十六进制
- `0` — 自动检测（`0x` 前缀 = 十六进制，`0` 前缀 = 八进制）

### 6. Python vs C 对比

| 操作 | Python | C |
|------|--------|---|
| 拆分 | `s.split(",")` | `strtok(s, ",")` |
| 查找 | `s.find("abc")` | `strstr(s, "abc")` |
| 字符查找 | `s.index('x')` | `strchr(s, 'x')` |
| 前缀匹配 | `s.startswith()` | `strspn()` |
| 转数字 | `int("42")` | `strtol("42", &end, 10)` |
| 去除空白 | `s.strip()` | 手动移动指针 |

## 常见错误

### ❌ 错误 1：`strtok` 修改了不应该修改的字符串

```c
const char *config = "key=value";
strtok(config, "=");  /* 💣 config 是字面量, 写入 \0 会崩溃! */

/* ✅ 修复: 先复制 */
char buf[64];
strncpy(buf, config, sizeof(buf) - 1);
buf[sizeof(buf) - 1] = '\0';
strtok(buf, "=");
```

### ❌ 错误 2：忘记 `strtok` 后续调用传入 `NULL`

```c
char data[] = "a,b,c";
char *tok1 = strtok(data, ",");
char *tok2 = strtok(data, ",");  /* ❌ 应该传 NULL! */

/* ✅ 修复 */
char *tok1 = strtok(data, ",");
char *tok2 = strtok(NULL, ",");  /* NULL = 继续上一次的位置 */
```

### ❌ 错误 3：`strstr` 区分大小写却期望大小写不敏感

```c
const char *text = "Hello World";
strstr(text, "hello");  /* → NULL! 大小写不匹配 */

/* ✅ 如果需要不敏感匹配, 先转小写或用 strcasestr (GNU) */
```

### ❌ 错误 4：`strtol` 不检查 `endptr`

```c
char *endptr = NULL;
long val = strtol("abc", &endptr, 10);
/* val = 0, endptr = "abc" (没有解析任何字符) */
if (endptr == input) {
    /* 输入不包含数字 */
}
```

## 动手练习

### 🟢 入门：拆分空格分隔的单词

用 `strtok` 拆分 `"Hello C World"` 为三个单词。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char text[] = "Hello C World";
    char *token = strtok(text, " ");
    while (token) {
        printf("[%s]\n", token);
        token = strtok(NULL, " ");
    }
    return 0;
}
```

输出：
```
[Hello]
[C]
[World]
```

</details>

### 🟡 中级：查找文件名

给定路径 `"/home/user/docs/readme.txt"`，找到最后 `'/'` 之后的内容（文件名）。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    const char *path = "/home/user/docs/readme.txt";
    const char *last = NULL;
    const char *p = path;
    while ((p = strchr(p, '/')) != NULL) {
        last = p;
        p++;
    }
    if (last) {
        printf("文件名: %s\n", last + 1);
    } else {
        printf("无 '/'，整个路径即文件名\n");
    }
    return 0;
}
```

</details>

### 🔴 挑战：简易 CSV 解析器

实现 `int parse_csv(const char *csv, char fields[][32], int max_fields)`，安全解析逗号分隔的字段（先复制再 `strtok`），返回解析到的字段数。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

int parse_csv(const char *csv, char fields[][32], int max_fields) {
    char buf[256];
    strncpy(buf, csv, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    int count = 0;
    char *token = strtok(buf, ",");
    while (token && count < max_fields) {
        strncpy(fields[count], token, 31);
        fields[count][31] = '\0';
        count++;
        token = strtok(NULL, ",");
    }
    return count;
}

int main(void) {
    char f[4][32];
    int n = parse_csv("John,25,Engineer,NYC", f, 4);
    for (int i = 0; i < n; i++) {
        printf("  [%d] %s\n", i, f[i]);
    }
    return 0;
}
```

</details>

## 故障排查

**Q：为什么 `strtok` 会"吞掉"空字段？**

A：`strtok` 将**连续的分隔符**视为一个分隔符。`"a,,b"` → `"a"`, `"b"`（中间的 `""` 被跳过）。如果需要保留空字段，用其他方法（如手动查找分隔符）。

**Q：为什么 `strtok` 不能用于同时解析多个字符串？**

A：`strtok` 内部用一个静态变量记录上次的位置。如果你在嵌套循环中使用：

```c
char outer[] = "a-b";
char inner[] = "1,2";
char *t1 = strtok(outer, "-");
while (t1) {
    char *t2 = strtok(inner, ",");   /* 💣 覆盖了 strtok 的内部状态! */
    while (t2) {
        printf("%s %s\n", t1, t2);
        t2 = strtok(NULL, ",");
    }
    t1 = strtok(NULL, "-");          /* 💣 内部状态已被破坏 */
}
```

修复：使用 `strtok_r`（POSIX, 线程安全版）或手动实现。

**Q：`strstr` 返回的指针能用来修改原字符串吗？**

A：如果原字符串是 `char[]`（非字面量），可以。如果指向字面量（`const char*`），不行。

## 知识扩展

### 手动实现安全拆分（不修改原串）

```c
/* 使用 strcspn 找到分隔符位置，复制子串 */
void safe_split(const char *input, const char *delim) {
    const char *start = input;
    while (*start) {
        size_t len = strcspn(start, delim);
        if (len > 0) {
            /* 复制 len 字节到新缓冲区 */
            char token[64];
            strncpy(token, start, len < 63 ? len : 63);
            token[len < 63 ? len : 63] = '\0';
            printf("[%s]\n", token);
        }
        start += len;
        if (*start) start++;  /* 跳过分隔符 */
    }
}
```

### `strsep` — Linux 上的 `strtok` 替代

`strsep` 是 Linux 上更安全的 `strtok` 替代：

```c
char *ptr = buffer;
char *token;
while ((token = strsep(&ptr, ",")) != NULL) {
    printf("[%s]\n", token);
}
/* 与 strtok 不同: strsep 不会跳过空字段 */
```

### 从字符串中提取数字

```c
const char *line = "User count: 42 active";
char *endptr = NULL;
const char *num_start = line;

/* 跳过非数字字符找到第一个数字 */
while (*num_start && !isdigit((unsigned char)*num_start)) num_start++;
long count = strtol(num_start, &endptr, 10);
printf("count = %ld\n", count);  /* 42 */
```

## 小结

- **`strtok`** 拆分字符串，但**修改原串**、**非线程安全**
- **`strstr`** 查找子串，区分大小写，返回指针或 NULL
- **`strchr`** 查找单个字符，循环搜索可找到最后一次出现
- **`strspn`/`strcspn`** 用于匹配/跳过字符集合
- **`strtol`** 安全地转数字：比 `atoi` 能检测错误
- 实战 CSV 解析 = 复制原串 → `strtok` → `strncpy` 到字段缓冲区
- Python 的 `split()` / `find()` 一行搞定，C 需要多步手动组合

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| Token | Token | `strtok` 拆分出的子串 |
| Delimiter | Delimiter | 分隔符（如 `","`, `" "`) |
| Tokenization | Tokenization | 将字符串拆分为 token 的过程 |
| Substring | Substring | 字符串中的连续子序列 |
| Character Set | Character Set | 一组字符，用于 `strspn`/`strcspn` 匹配 |
| End Pointer | End Pointer | `strtol` 输出的 `endptr`，指向停止解析的位置 |

## 延伸阅读

- [cppreference — strtok](https://en.cppreference.com/w/c/string/byte/strtok) — 字符串令牌化参考
- [cppreference — strstr](https://en.cppreference.com/w/c/string/byte/strstr) — 子串查找参考
- [OpenBSD strsep manual](https://man.openbsd.org/strsep.3) — 比 strtok 更安全的替代

## 继续学习

- [上一章](./safe_strings.md)：安全字符串（strncpy、snprintf、溢出预防）
- [下一章](./structs.md)：结构体
