# 字符串操作 (String Operations)

上一章我学到了 C 字符串的基石——`char` 数组必须以 `'\0'` 结尾。但知道了结构还不够，我还要知道怎么用这些砖块"搭建"东西。`<string.h>` 就是 C 语言提供给我的工具箱，里面有量尺（`strlen`）、铲子（`strcpy`）、胶水（`strcat`）和镜子（`strcmp`）。

## 开篇故事

我第一次真正理解 `strlen` 的工作原理，是写了一个 bug：我把两个字符串"拼"在一起，但拼接后的结果比预期长了不少。后来我才发现——`strcat` 找到第一个 `'\0'` 就开始写，如果我没正确终止第一个字符串，它就会从错误的位置继续贴。

```c
char buf[20];
buf[0] = 'H'; buf[1] = 'i';  /* 忘记 \0! */
strcat(buf, ", World");       /* 💣 从随机位置开始拼! */
```

这就像贴瓷砖——如果第一块瓷砖没对齐，后面的全都歪了。C 的字符串操作函数**信任**你已经正确终止了字符串，它们不会检查。这份信任换来的是速度，也意味着你必须自己做好收尾工作。

## 本章适合谁

- 已完成 [字符串基础](./string_basics.md)，理解 `'\0'` 的作用
- 想系统掌握 `<string.h>` 核心函数
- 想知道 `strcpy` 为什么"危险"以及 `strncpy` 怎么用
- 希望写出正确、无 bug 的字符串操作代码

## 你会学到什么

1. `strlen()` — 遍历找 `'\0'` 计数
2. `strcpy()` vs `strncpy()` — 复制字符串的安全对比
3. `strcat()` / `strncat()` — 拼接字符串
4. `strcmp()` — 字典序比较（永远不要用 `==`！）
5. `strchr()` — 查找单个字符
6. 实战：安全拼接文件路径

## 前置要求

- 已完成 [字符串基础](./string_basics.md) 章节
- 理解 `'\0'` 终止符和 `char[]` vs `const char*`

## 第一个例子

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char greeting[64];

    /* 复制 */
    strncpy(greeting, "Hello", sizeof(greeting) - 1);
    greeting[sizeof(greeting) - 1] = '\0';

    /* 拼接 */
    strncat(greeting, ", World!", sizeof(greeting) - strlen(greeting) - 1);

    /* 比较 */
    if (strcmp(greeting, "Hello, World!") == 0) {
        printf("匹配成功!\n");
    }

    /* 查找 */
    char *comma = strchr(greeting, ',');
    if (comma) {
        printf("逗号在第 %ld 个位置\n", comma - greeting);
    }

    printf("结果: \"%s\" (长度 %zu)\n", greeting, strlen(greeting));
    return 0;
}
```

完整源码在 [`src/basic/string_operations_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/string_operations_sample.c)。

## 原理解析

### 1. `strlen()` — 数到 `\0` 停

```c
size_t len = strlen("Hello");  /* 返回 5 */
```

`strlen` 内部实现——从首地址开始逐字节读取，遇到 `'\0'` 返回计数器：

```c
/* strlen 的手动实现 */
size_t my_strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}
```

**关键认知**：

- `strlen()` 是 **O(n)**——它必须遍历整个字符串
- 返回的是**字节数**，不是 Unicode 字符数
- 空串 `""` 的 `strlen` 返回 0

```
"Hello" 在内存中:
['H']['e']['l']['l']['o']['\0']
 ↑    ↑    ↑    ↑    ↑    ← 在这里停
 len: 1    2    3    4    5
```

### 2. `strcpy()` vs `strncpy()` — 复制

**`strcpy`（危险！无边界检查）：**

```c
char small[5];
strcpy(small, "Hello World!");
/* 💣 将 13 字节写入 5 字节缓冲区 → 缓冲区溢出! */
```

**`strncpy`（安全，但有陷阱）：**

```c
char small[5];
strncpy(small, "Hello", sizeof(small) - 1);
small[sizeof(small) - 1] = '\0';  /* ← 必须手动做! */
```

`strncpy` 的行为：

| 情况 | `strncpy` 会做什么 |
|------|-------------------|
| src 长度 < n | 复制 + 用 `'\0'` 填充剩余字节 |
| src 长度 >= n | 复制 n 字节 + **不自动 `'\0'`** |

这就是为什么**永远要手动保证 `'\0'`**：
```c
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';
```

### 3. `strcat()` / `strncat()` — 拼接

`strcat` 找到目标串的 `'\0'`，然后把源串从那个位置开始粘贴。

```c
char greeting[64] = "Hello";
strcat(greeting, ", World");   /* greeting = "Hello, World" */
```

**安全版本 `strncat`**：

```c
strncat(buf, addition, sizeof(buf) - strlen(buf) - 1);
/*                      ↑ 关键: 剩余空间，不是总大小! */
```

```
拼接前:
[ 'H' ][ 'e' ][ 'l' ][ 'l' ][ 'o' ][ '\0' ][ ? ][ ? ]...
                                          ↑
                                      strncat 从这开始写

拼接后:
[ 'H' ][ 'e' ][ 'l' ][ 'l' ][ 'o' ][ ',' ][ ' ' ][ 'W' ][ 'o' ][ 'r' ][ 'l' ][ 'd' ][ '\0' ]
```

**常见错误**：第三个参数传 `sizeof(buf)` 而不是 `sizeof(buf) - strlen(buf) - 1`。`strncat` 的 `n` 是**剩余可用字节数**，不是缓冲区总大小。

### 4. `strcmp()` — 字典序比较

```c
int result = strcmp("apple", "banana");
/* result < 0 ("apple" 字典序更小) */

result = strcmp("hello", "hello");
/* result == 0 (完全相等) */

result = strcmp("world", "hello");
/* result > 0 ("world" 字典序更大) */
```

**为什么不能用 `==`？**

```c
const char *a = "hello", *b = "hello";
if (a == b) { /* ❌ 比较的是指针地址! */ }
if (strcmp(a, b) == 0) { /* ✅ 比较字符串内容 */ }
```

`strcmp` 逐字节比较 ASCII 值，返回第一对不同字符的差：

```
'a' = 97, 'b' = 98
strcmp("apple", "banana") → 比较 'a' vs 'b' → 97 - 98 = -1
```

### 5. `strchr()` — 单字符查找

```c
char *found = strchr("Hello, World!", 'W');
if (found) {
    printf("找到! 偏移: %ld\n", found - "Hello, World!");
    /* → 剩余部分: "World!" */
}
```

返回指向找到位置的指针，找不到返回 `NULL`。

### 6. Python 对比

| 操作 | Python | C |
|------|--------|---|
| 长度 | `len(s)` O(1) | `strlen(s)` O(n) |
| 复制 | `s2 = s1` (浅拷贝) | `strncpy(dest, src, n)` |
| 拼接 | `s1 + s2` | `strncat(s1, s2, n)` |
| 比较 | `s1 == s2` | `strcmp(s1, s2) == 0` |
| 查找 | `s.find('x')` | `strchr(s, 'x')` |
| 边界检查 | 自动 | 手动（或用 `_n` 版本） |

## 常见错误

### ❌ 错误 1：目标缓冲区太小

```c
char small[5];
strcpy(small, "Hello");  /* "Hello" 需 6 字节(含 \0) */
/* 💣 溢出! */

/* ✅ 修复 */
char small[6];
strncpy(small, "Hello", sizeof(small) - 1);
small[sizeof(small) - 1] = '\0';
```

### ❌ 错误 2：`strcat` 的第三个参数算错

```c
char buf[20] = "Hello";
strncat(buf, " World!", 20);
/* ❌ 20 是缓冲区总大小，不是剩余空间! */

/* ✅ 修复: 用剩余空间 */
strncat(buf, " World!", sizeof(buf) - strlen(buf) - 1);
```

### ❌ 错误 3：用 `==` 比较字符串

```c
char a[] = "hello", b[] = "hello";
if (a == b) { /* ❌ 永远 false, 比较地址 */ }
/* ✅ 修复 */
if (strcmp(a, b) == 0) { /* ✅ 比较内容 */ }
```

### ❌ 错误 4：`strncat` 忘了 `\0`

```c
char buf[5];
strncpy(buf, "ABCD", 4);
strncat(buf, "E", 1);
/* buf 没有 '\0'! */

/* ✅ 修复: 每次操作后确保 \0 */
strncpy(buf, "ABCD", 4);
buf[4] = '\0';
strncat(buf, "E", sizeof(buf) - strlen(buf) - 1);
```

## 动手练习

### 🟢 入门：`strlen` 实践

不用 `strlen`，手动实现一个函数计算字符串长度。测试 `"Hello"`, `""`, `"C Programming"`。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>

size_t my_strlen(const char *s) {
    size_t n = 0;
    while (s[n] != '\0') n++;
    return n;
}

int main(void) {
    printf("%zu\n", my_strlen("Hello"));
    printf("%zu\n", my_strlen(""));
    printf("%zu\n", my_strlen("C Programming"));
    return 0;
}
```

</details>

### 🟡 中级：安全拼接

写一个函数 `void safe_cat(char *dest, size_t dest_size, const char *src)`，确保：
1. 拼接后总长 < `dest_size`
2. 始终有 `'\0'` 终止

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>

void safe_cat(char *dest, size_t dest_size, const char *src) {
    size_t cur_len = strlen(dest);
    if (cur_len < dest_size - 1) {
        strncat(dest, src, dest_size - cur_len - 1);
    }
}

int main(void) {
    char buf[20] = "Hello";
    safe_cat(buf, sizeof(buf), ", World!");
    printf("%s\n", buf);  /* 输出: Hello, World! */
    return 0;
}
```

</details>

### 🔴 挑战：自己实现 `strcmp`

不使用 `<string.h>` 中的 `strcmp`，手动比较两个字符串，返回 `<0`、`=0` 或 `>0`。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>

int my_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int main(void) {
    printf("%d\n", my_strcmp("abc", "abc"));  /* 0 */
    printf("%d\n", my_strcmp("abc", "abd"));  /* <0 */
    printf("%d\n", my_strcmp("bcd", "abc"));  /* >0 */
    return 0;
}
```

</details>

## 故障排查

**Q：`strncpy` 复制后字符串乱码？**

A：`strncpy` 在源串长度 ≥ n 时不会自动加 `'\0'`。修复：

```c
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';  /* 始终手动补充 */
```

**Q：`strncat` 的第三个参数为什么是 `sizeof - strlen - 1`？**

A：`strncat` 已经在末尾找到 `'\0'` 并开始写。所以：
- `sizeof` — 缓冲区总大小
- 减去 `strlen` — 已有内容占用的空间
- 减 `1` — 留一个字节给 `'\0'`

**Q：`strcmp` 返回值是精确的差值吗？**

A：标准只保证返回正数、负数、零。不同实现可能返回差值，也可能只返回 1、-1、0。**永远不要假设返回值是具体的差值**。

**Q：为什么 `strlen` 是 O(n) 而 Python 的 `len()` 是 O(1)？**

A：C 字符串没有存储长度，必须遍历到 `'\0'` 才能知道。Python 对象内部存储了长度字段，直接返回。这就是"控制权换安全责任"的代价。

## 知识扩展

### `memcpy` vs `strcpy` vs `strncpy`

| 函数 | 用途 | 停止条件 | 边界安全 |
|------|------|----------|----------|
| `strcpy` | 复制字符串 | 遇到 `'\0'` | ❌ 无 |
| `strncpy` | 复制字符串(安全) | n 字节 或 `'\0'` | ⚠️ 需手动 `\0` |
| `memcpy` | 复制任意内存 | n 字节 | ✅ 完全可控 |

如果你需要复制的数据不是字符串（比如有 null 字节在中间），用 `memcpy`。

### Linux 的 `strlcpy` / `strlcat`

OpenBSD 发明了更安全、更易用的替代函数（已被 Linux glibc 部分采用）：

```c
strlcpy(dest, src, sizeof(dest));
strlcat(dest, src, sizeof(dest));
/* 好处: 始终保证 '\0' 终止, 返回完整需要的长度 */
```

但注意它们不是 POSIX 标准。跨平台项目还是用 `strncpy` + 手动 `'\0'` 更稳妥。

## 小结

- **`strlen`** = O(n) 遍历找 `'\0'`，返回字节数
- **`strcpy`** 不安全，**`strncpy`** + 手动 `'\0'` 是标准做法
- **`strcat`** 追加到 `'\0'` 位置，`strncat` 的第三个参数是**剩余空间**
- **`strcmp`** 逐字节比较，永远不要用 `==` 比较字符串内容
- **`strchr`** 返回指针或 NULL，减去原指针得到偏移量
- Python 自动处理的事，C 都要你手动确认——但换来的是速度和灵活性

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 字符串复制 | String Copy | 将源字符串内容复制到目标缓冲区 |
| 字符串拼接 | String Concatenation | 将一个字符串追加到另一个末尾 |
| 字符串比较 | String Comparison | 按字典序逐字节比较内容 |
| 字符查找 | Character Search | 在字符串中查找指定字符 |
| 边界检查 | Bounds Checking | 验证操作是否超出缓冲区范围 |
| 溢出 | Overflow | 写入超出缓冲区边界 |

## 延伸阅读

- [cppreference — strcpy](https://en.cppreference.com/w/c/string/byte/strcpy) — 字符串复制参考
- [cppreference — strcat](https://en.cppreference.com/w/c/string/byte/strcat) — 字符串拼接参考
- [cppreference — strcmp](https://en.cppreference.com/w/c/string/byte/strcmp) — 字符串比较参考

## 继续学习

- [上一章](./string_basics.md)：字符串基础（char 数组、'\0'）
- [下一章](./safe_strings.md)：安全字符串（strncpy、snprintf、溢出预防）
