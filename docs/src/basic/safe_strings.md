# 安全字符串 (Safe Strings)

`strcpy` 在 C 语言中的地位，就像一把没有手套的电锯——功能强大，但一旦松手就会切到自己。本章我将学习如何给电锯装上保护罩：`strncpy`、`snprintf`、边界检查、以及溢出预防。

## 开篇故事

我写的第一段"像样"的 C 代码，用了 `strcpy` 来复制用户输入的用户名。它在我的笔记本上跑得很好——直到某个天真的日子，有人输入了一个超长字符串，程序直接段错误崩溃。

```c
/* ❌ 我当时的代码 */
char username[16];
strcpy(username, user_input);  /* 如果 user_input 超过 15 字符... 💥 */
```

后来我查了一下 OWASP（开放式 Web 应用程序安全项目），缓冲区溢出攻击已经连续十多年位居 Top 10。攻击者只需要输入精心构造的超长数据，覆盖栈上的返回地址，就能控制程序的执行流程。

**C 语言的 `strcpy` 就是这一切的起点**——它不检查边界，永远写入。`strncpy` 和 `snprintf` 就是给我们的代码穿的防弹衣。

> "安全不是功能，而是底线。"

## 本章适合谁

- 已完成 [字符串操作](./string_operations.md)，知道 `strcpy` 怎么用
- 听说过"缓冲区溢出"，想知道具体怎么防御
- 希望写出生产级安全的字符串处理代码
- 被 `Werror` 标志下编译报错折磨过的人

## 你会学到什么

1. `strncpy` 安全复制模式：始终以 `'\0'` 终止
2. `snprintf` 安全格式化：比 `sprintf` 安全三个等级
3. 边界检查模式：预判缓冲区大小、检测截断
4. 溢出防御三件套：够大、剩余、返回值
5. 组合使用：分段安全构建、格式化安全
6. **本章铁律**：工作代码中绝不出现 `strcpy` / `sprintf`

## 前置要求

- 已完成 [字符串操作](./string_operations.md)
- 理解 `strncpy` 的基本用法

## 第一个例子

```c
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 32

int main(void) {
    char buf[BUF_SIZE];
    const char *name = "Alice";
    int32_t score = 95;

    /* ✅ snprintf 安全格式化 */
    int ret = snprintf(buf, sizeof(buf), "Name: %s, Score: %d",
                       name, score);

    printf("结果: \"%s\"\n", buf);
    printf("需要: %d 字符, 缓冲区: %zu 字节\n",
           ret, sizeof(buf));

    if (ret >= 0 && ret < (int)sizeof(buf)) {
        printf("✅ 未发生截断\n");
    } else {
        printf("⚠️ 发生了截断\n");
    }

    return 0;
}
```

完整源码在 [`src/basic/safe_strings_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/safe_strings_sample.c)。

## 原理解析

### 1. `strncpy` 安全复制模式

```c
char dest[16];
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';  /* ← 这行就是安全线 */
```

`strncpy` 的三个参数：

| 参数 | 含义 | 推荐值 |
|------|------|--------|
| `dest` | 目标缓冲区 | 数组变量 |
| `src` | 源字符串 | 字符指针 |
| `n` | 最大写入字节 | `sizeof(dest) - 1` |

**`strncpy` 的陷阱**：当 `strlen(src) >= n` 时，`strncpy` 不会自动添加 `'\0'`。

```c
char buf[5];
strncpy(buf, "ABCDEFGH", 5);  /* 写满了5个字符, 但没有 \0! */
printf("%s\n", buf);           /* 💣 越界读取 */

/* ✅ 修复 */
strncpy(buf, "ABCDEFGH", sizeof(buf) - 1);
buf[sizeof(buf) - 1] = '\0';  /* 保证终止 */
```

```text
┌────────── strcpy 溢出对比 strncpy 安全 ──────────┐
│                                                    │
│  char buf[8];                                      │
│  strcpy(buf, "HelloWorld!");  // 12 字符           │
│                                                    │
│  栈内存: ┌─buf[8]──┬─相邻数据/返回地址─┐            │
│          │H│e│l│l│o│W│o│r│l│d│!│\0│               │
│          └─────────┘ ← 越界溢出! → ┘               │
│  💥 数据被覆盖 → 段错误或安全漏洞                   │
│                                                    │
│  char buf[8];                                      │
│  strncpy(buf, src, sizeof(buf)-1);                 │
│  buf[sizeof(buf)-1] = '\0';                        │
│                                                    │
│  栈内存: ┌─buf[8]─────┐                            │
│          │H│e│l│l│o│W│o│\0│                        │
│          └─────────────┘                            │
│          └── 总在边界内 ──┘                         │
│  ✅ 截断为 "HelloWo"，绝不越界                      │
└─────────────────────────────────────────────────────┘
```

### 2. `snprintf` 安全格式化

`snprintf` 是 `sprintf` 的安全替代品——它增加了一个 `size` 参数：

```c
/* ❌ 危险 */
char buf[10];
sprintf(buf, "User: %s, Score: %d", "AliceWonderland", 95);
/* 💣 "User: AliceWonderland, Score: 95" 共 30 字节 > 10 → 溢出! */

/* ✅ 安全 */
char buf[10];
int ret = snprintf(buf, sizeof(buf), "User: %s, Score: %d",
                   "AliceWonderland", 95);
/* buf = "User: Alic" (截断), ret = 30 (完整需要 30 字符) */
```

**`snprintf` 的返回值是关键**：

```c
int ret = snprintf(buf, sizeof(buf), "fmt", args...);

/* ret < 0 → 编码错误 */
/* ret >= sizeof(buf) → 发生了截断 */
/* ret < sizeof(buf) → 成功, ret 是实际写入的字符数 */
```

**预判缓冲区大小**：

```c
/* 先调用 snprintf(NULL, 0, ...) 预判需要多少空间 */
int needed = snprintf(NULL, 0, "User: %s, Age: %d, City: %s",
                      "Alice", 25, "Shanghai");
/* needed = 34 → malloc 35 字节就够 */
char *buf = malloc(needed + 1);
snprintf(buf, needed + 1, "User: %s, Age: %d, City: %s",
         "Alice", 25, "Shanghai");
```

### 3. 边界检查模式

无论用什么函数，字符串操作之前都应该检查。三种常见模式：

**模式 A: 预判长度**

```c
size_t needed = strlen(src) + 1;  /* +1 给 \0 */
if (needed > sizeof(dest)) {
    /* 截断或报错 */
    strncpy(dest, src, sizeof(dest) - 1);
    dest[sizeof(dest) - 1] = '\0';
} else {
    strncpy(dest, src, sizeof(dest) - 1);
    dest[sizeof(dest) - 1] = '\0';
}
```

**模式 B: 拼接时检查剩余空间**

```c
size_t remaining = sizeof(buf) - strlen(buf) - 1;
if (strlen(addition) > remaining) {
    /* 拼接会溢出 */
}
```

**模式 C: 用返回值校验**

```c
int ret = snprintf(buf, sizeof(buf), "fmt", args...);
if (ret < 0 || ret >= (int)sizeof(buf)) {
    /* 发生错误或截断 */
}
```

### 4. 溢出防御三件套

每次字符串操作，问自己三个问题：

```
1️⃣ 目标缓冲区够大吗？
   needed = strlen(src) + 1
   if (needed > sizeof(dest)) → 截断

2️⃣ 拼接时剩余空间够吗？
   remaining = sizeof(buf) - strlen(buf) - 1
   if (strlen(addition) > remaining) → 截断

3️⃣ 格式化后总长度超标吗？
   ret = snprintf(buf, sizeof(buf), "fmt", ...)
   if (ret >= sizeof(buf)) → 截断
```

### 5. 分段安全构建

```c
char safe[64];
int pos = 0;

pos += snprintf(safe + pos, sizeof(safe) - pos,
                "[%s] ", author);
pos += snprintf(safe + pos, sizeof(safe) - pos,
                "%s", content);
```

`snprintf(safe + pos, sizeof(safe) - pos, ...)` 每次都从上次的位置继续写，确保永远不会超出边界。

### 6. Python vs C 安全感对比

| 场景 | Python | C (无保护) | C (有保护) |
|------|--------|-----------|-----------|
| 超长字符串 | 自动扩展 | 💥 溢出 | 截断 |
| 拼接溢出 | 自动扩展 | 💥 溢出 | 检测 |
| 格式化溢出 | 自动扩展 | 💥 溢出 | snprintf 截断 |
| 字符串不可变 | 是 | 看情况 | `const char*` |

## 常见错误

### ❌ 错误 1：`strncpy` 之后忘记 `'\0'`

```c
char buf[8];
strncpy(buf, "ABCDEFGHIJ", 7);
/* buf = "ABCDEFG" 后面没有 '\0'! */

/* ✅ 修复 */
strncpy(buf, "ABCDEFGHIJ", sizeof(buf) - 1);
buf[sizeof(buf) - 1] = '\0';
```

### ❌ 错误 2：`snprintf` 不检查返回值

```c
char small[10];
snprintf(small, sizeof(small), "This is a very long string");
/* small 被截断，但你不知道 */

/* ✅ 修复 */
int ret = snprintf(small, sizeof(small), "This is a very long string");
if (ret >= (int)sizeof(small)) {
    printf("⚠️ 截断! 需要 %d 字节\n", ret);
}
```

### ❌ 错误 3：`strncat` 用错第三个参数

```c
char buf[20] = "Hello";
strncat(buf, " World", 20);
/* ❌ 20 是总大小，不是剩余空间! */

/* ✅ 修复 */
strncat(buf, " World", sizeof(buf) - strlen(buf) - 1);
```

### ❌ 错误 4：误用 `sprintf`

```c
char buf[32];
sprintf(buf, "Name: %s, Score: %d", "A Very Long Name", 100);
/* 💣 如果组合后超过 32 字节 → 溢出 */

/* ✅ 修复 */
snprintf(buf, sizeof(buf), "Name: %s, Score: %d", "A Very Long Name", 100);
```

## 动手练习

### 🟢 入门：安全的用户名复制

写一个函数 `void copy_username(char *dest, size_t dest_size, const char *input)`，始终安全复制。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

void copy_username(char *dest, size_t dest_size, const char *input) {
    strncpy(dest, input, dest_size - 1);
    dest[dest_size - 1] = '\0';
}

int main(void) {
    char short_buf[10];
    copy_username(short_buf, sizeof(short_buf), "A Very Long Username");
    printf("'%s' (%zu chars)\n", short_buf, strlen(short_buf));
    return 0;
}
```

</details>

### 🟡 中级：用 `snprintf` 检测截断

实现一个函数，尝试格式化字符串，如果发生截断则返回负数；成功则返回使用的字节数。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>
#include <stdint.h>

int safe_format(char *buf, size_t size, const char *fmt,
                const char *name, int32_t score) {
    int ret = snprintf(buf, size, "User: %s, Score: %d", name, score);
    if (ret < 0 || ret >= (int)size) {
        return -1;  /* 截断或错误 */
    }
    return ret;
}

int main(void) {
    char buf[20];
    int result = safe_format(buf, sizeof(buf), "x", "Alice", 95);
    printf("ret=%d, buf='%s'\n", result, buf);
    return 0;
}
```

</details>

### 🔴 挑战：安全拼接多段内容

实现 `int multi_snprintf(char *buf, size_t size, const char *parts[], int count)`，依次拼接每个部分，返回实际需要的字符数（如果没截断的话）。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

int multi_snprintf(char *buf, size_t size, const char *parts[], int count) {
    int total = 0;
    size_t pos = 0;
    for (int i = 0; i < count; i++) {
        int ret = snprintf(buf + pos, size - pos, "%s", parts[i]);
        total += ret;
        if (ret > 0 && pos + (size_t)ret < size) {
            pos += ret;
        }
    }
    return total;
}

int main(void) {
    const char *parts[] = {"Hello", ", ", "World", "!"};
    char buf[10];
    int total = multi_snprintf(buf, sizeof(buf), parts, 4);
    printf("'%s' (需要 %d 字符)\n", buf, total);
    return 0;
}
```

</details>

## 故障排查

**Q：`strncpy` 和 `snprintf` 我该用哪个？**

A：简单复制用 `strncpy`，需要格式化用 `snprintf`。`snprintf` 其实更安全——它始终保证 `'\0'` 终止，返回值告诉你是否截断。很多项目选择**统一用 `snprintf`** 做所有字符串操作。

**Q：`snprintf` 的 `size` 参数应该是 `sizeof(buf)` 还是 `sizeof(buf) - 1`？**

A：`sizeof(buf)`。`snprintf` 会自动留一个字节给 `'\0'`。它会写入**最多** `size - 1` 个字符 + `'\0'`。

**Q：如果缓冲区太小，是截断还是报错？**

A：取决于业务。配置类场景：截断并记录日志。安全敏感场景：拒绝请求并返回错误码。本章演示代码统一截断（`snprintf` 自动处理）。

**Q：`strncpy` 在 src 短于 n 时会填充多余的 `\0`，性能如何？**

A：是的，`strncpy` 会用 `'\0'` 把剩余空间补满。如果你的缓冲区很大但 src 很短，这有性能开销。如果你不在乎性能（大多数情况），`strncpy` 足够好。追求极致性能的场景可用 `snprintf` 替代。

## 知识扩展

### C11 Annex K: `strcpy_s` / `strcat_s` / `sprintf_s`

C11 标准引入了可选的"边界检查"扩展：

```c
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>

strcpy_s(dest, dest_size, src);   /* 自动 \0 终止 */
strcat_s(dest, dest_size, src);
sprintf_s(buf, buf_size, "%s", str);
```

**优点**：始终保证 `'\0'` 终止，溢出时调用约束处理函数。

**缺点**：glibc 默认不启用（需要 `__STDC_WANT_LIB_EXT1__`），跨平台兼容性差。

**实际建议**：`strncpy` + 手动 `'\0'` 仍然是最广泛的跨平台方案。

### `strlcpy` / `strlcat` — OpenBSD 的好公民

```c
strlcpy(dest, src, sizeof(dest));
/* 好处: 始终 \0 终止, 返回完整需要的长度 */
strlcat(dest, src, sizeof(dest));
/* 好处: 自动处理剩余空间 */

/* 检测截断: */
size_t needed = strlcpy(dest, src, sizeof(dest));
if (needed >= sizeof(dest)) { /* 截断发生了 */ }
```

这些函数已被 Linux 采用但非 POSIX 标准。如果你在 BSD/Linux 环境下开发，它们比 `strncpy` 更优雅。

### 为什么 `strcpy` 被称为"最危险函数"

```
缓冲区溢出攻击的典型模式:

栈布局:
[ char buf[16] ] [ saved RBP ] [ return address ]

攻击者输入 100 个 'A':
strcpy(buf, "AAAA...AAAA(100个)");
→ 'A' 覆盖了 return address
→ 函数返回时跳转到 0x41414141
→ 💥 崩溃 或被劫持执行恶意代码

这就是 strcpy 在安全编码规范中被禁止的原因。
```

## 小结

- **永远不要用 `strcpy` — 用 `strncpy` + 手动 `'\0'`**
- **永远不要用 `sprintf` — 用 `snprintf`**
- `snprintf` 返回值告诉你是否需要更大缓冲区
- 边界检查三件套：预判长度、剩余空间、返回值校验
- 分段构建：`snprintf(buf + pos, sizeof - pos, ...)` 是安全范式
- C11 Annex K 和 `strlcpy` 是更好的替代，但跨平台兼容性有限

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 边界检查 | Bounds Checking | 验证写入不会超出缓冲区 |
| 缓冲区溢出 | Buffer Overflow | 写入超出缓冲区边界 |
| 截断 | Truncation | 因缓冲区不足而丢失部分数据 |
| 安全函数 | Safe Function | 带边界检查的函数 (如 snprintf) |
| 不安全函数 | Unsafe Function | 无边界检查的函数 (如 strcpy) |
| 约束处理 | Constraint Handling | C11 Annex K 的溢出处理机制 |

## 延伸阅读

- [OWASP Buffer Overflow](https://owasp.org/www-community/vulnerabilities/Buffer_Overflow) — 安全编码标准
- [C11 Annex K](https://en.cppreference.com/w/c/types/limits) — 边界检查扩展
- [Strlcpy and Strlcat — Consistently Correct](https://www.freebsd.org/cgi/man.cgi?query=strlcpy) — OpenBSD 文档

## 继续学习

- [上一章](./string_operations.md)：字符串操作（strlen, strcpy, strcat, strcmp）
- [下一章](./string_processing.md)：字符串处理（strtok, strstr, 解析）
