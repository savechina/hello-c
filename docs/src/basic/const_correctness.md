# const 正确性 (Const Correctness)

## 开篇故事

想象你在博物馆里看展品。展品后面贴着标签：「请勿触碰」。这不是建议，是规则。如果每个人都遵守这个规则，展品就能完好保存；如果有人违反，可能会造成不可逆的损坏。

`const` 就是 C 语言里的「请勿触碰」标签。它告诉编译器（和所有读代码的人）：**这块数据不应被修改**。编译器会强制这个规则——如果你试图通过 `const` 指针修改数据，编译直接报错。

```c
const int32_t pi = 314;
int32_t *bad = &pi;    /* ❌ 编译错误: 丢弃 const 限定符 */
const int32_t *good = &pi;  /* ✅ 承认数据的 const 身份 */
```

`const` 不是可选项——它是你和编译器之间的契约。签了约，编译器帮你执行；不签约，所有安全责任都靠手动。

## 本章适合谁

- 已掌握指针基础（`&`、`*`、NULL）
- 看到 `const int *p` 和 `int *const p` 感到混乱的初学者
- 好奇 `const` 在函数参数中起什么作用
- 想写出更安全的 C 代码的程序员

## 你会学到什么

1. `const int *p` — 指向 const 的指针（不能修改所指向的值）
2. `int *const p` — const 指针（不能修改指针自身指向）
3. `const int *const p` — 双重 const（值不可改 + 指向不可改）
4. `const` 在函数签名中的意义：API 的契约
5. 记忆口诀：看 `const` 在 `*` 的哪一侧
6. Python 可变/不可变类型 vs C const 对比

## 前置要求

- 已完成 [指针基础](./pointer_basics.md) 章节
- 理解指针声明和赋值的基本语法

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    const int32_t pi = 314;

    /* ❌ 不能通过普通指针指向 const 数据 */
    /* int32_t *p = &pi;  // 编译错误 */

    /* ✅ 用 const 指针访问 const 数据 */
    const int32_t *p = &pi;
    printf("pi = %" PRId32 "\n", *p);

    /* *p = 999;  // ❌ 编译错误: 不能修改 const 数据 */

    int32_t other = 42;
    p = &other;   /* ✅ 可以改指向（ptr 本身不是 const） */
    printf("other = %" PRId32 "\n", *p);

    return 0;
}
```

核心规则：`const` 在 `*`**左边**，保护的是**所指向的数据**；`const` 在 `*`**右边**，保护的是**指针自身**。

## 原理解析

### 1. 指向 const 的指针——`const T *p`

```c
const int32_t pi = 314;
const int32_t *ptr = &pi;
```

```
  ┌──────────────────────┐
  │  const int32_t *ptr  │
  │  「我只读，不改数据」 │
  └──────────────────────┘

  pi  (const, 只读)
  ▲
  │
  ptr → 可以改指向 (ptr = &other)
  *ptr → 不能修改 (*ptr = 999 ❌)
```

**什么可以**：`ptr` 本身可以重新指向其他地址。
**什么不可以**：通过 `*ptr` 修改所指向的数据。

```c
ptr = &other;   /* ✅ ptr 可以改指向 */
*ptr = 999;     /* ❌ 不能通过 ptr 修改数据 */
```

### 2. const 指针——`T *const p`

```c
int32_t vals[2] = {10, 20};
int32_t *const ptr = &vals[0];
```

```
  ┌──────────────────────┐
  │  int32_t *const ptr  │
  │  「我不能换目标，    │
  │    但可以改数据」     │
  └──────────────────────┘

  ptr → 不能改指向 (ptr = &vals[1] ❌)
  *ptr → 可以修改 (*ptr = 99 ✅)
```

**什么可以**：通过 `*ptr` 修改所指向的数据。
**什么不可以**：`ptr` 本身不能重新指向。

```c
*ptr = 99;           /* ✅ 可以修改值 */
ptr = &vals[1];      /* ❌ const 指针不能改指向 */
```

### 3. 双重 const——`const T *const p`

```c
const int32_t secret = 777;
const int32_t *const ptr = &secret;
```

```
  ┌──────────────────────────┐
  │  const int32_t *const ptr│
  │  「我只读，且不能换目标」 │
  └──────────────────────────┘

  ptr  → 不能改指向
  *ptr → 不能修改值
  完全只读 = 最高级别保护
```

### 4. 记忆口诀——看 `const` 的位置

最可靠的方法：**从右往左读指针声明**。

| 声明 | 读法 | 数据可改？ | 指向可改？ |
|------|------|----------|----------|
| `const int *p` | `p` is a pointer to `const int` | ❌ | ✅ |
| `int const *p` | 同上（const 在类型名左边） | ❌ | ✅ |
| `int *const p` | `p` is a `const` pointer to `int` | ✅ | ❌ |
| `const int *const p` | `p` is a `const` pointer to `const int` | ❌ | ❌ |

**速记法**：`const` 在 `*`**左边**保护**数据**，在 `*`**右边**保护**指针**。

### 5. const 在函数签名中的力量

`const` 是函数的「契约」——告诉调用者函数会/不会修改什么：

```c
/* 契约: 我不会修改你传入的字符串 */
size_t my_strlen(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') len++;
    return len;
}

/* 契约: dest 可写, src 只读 */
void copy_string(char *dest, const char *src) {
    while (*dest++ = *src++);
}
```

```c
/* 调用时 */
const char msg[] = "hello";
size_t len = my_strlen(msg);  /* ✅ my_strlen 承诺不修改 msg */

char msg2[] = "hello";
size_t len2 = my_strlen(msg2);  /* ✅ 即使是非 const 也能传给 const 参数 */
```

### 6. Python 可变/不可变 vs C const 对比

| 特性 | Python | C |
|------|--------|---|
| 不可变数据 | `tuple`, `str` 内置不变 | `const T` 编译器强制 |
| 可变数据 | `list`, `dict` 可改 | 裸 `T*` 可改 |
| 只读传递 | 传 tuple，对方无法改 | 传 `const T*`，编译器拦截 |
| 强制执行 | 运行时 `TypeError` | 编译期错误 |

```python
# Python — 运行时保护
x = (1, 2, 3)     # tuple 不可变
x[0] = 99         # TypeError (运行时)
```

```c
// C — 编译时保护
const int32_t arr[] = {1, 2, 3};
const int32_t *p = arr;
*p = 99;          /* ❌ 编译错误 — 根本过不了编译 */
```

C 的 `const` 更严格——在代码运行前就阻止了错误。

## 常见错误

### ❌ 错误 1：隐式丢弃 const

```c
const int32_t pi = 314;
int32_t *p = &pi;   /* ❌ 编译错误: discards 'const' qualifier */
*p = 999;           /* 试图去除 const 保护修改数据 */
```

```c
/* ✅ 修正: 使用 const 指针 */
const int32_t *p = &pi;
/* *p = 999; 仍然编译错误 — 这就是 const 的意义 */
```

### ❌ 错误 2：双重 const 误用

```c
int32_t val = 10;
const int32_t *const p = &val;

*p = 20;    /* ❌ 不能通过双重 const 改值 */
p = &other; /* ❌ 不能改指向 */
```

```c
/* 如果只需要保护数据，用单层 const */
const int32_t *p = &val;  /* ✅ 值不可改，指向可改 */
```

### ❌ 错误 3：const 指针指向非 const 数据

```c
/* 这本身合法，但要注意语义 */
int32_t val = 10;
const int32_t *p = &val;
/* *p = 20; ❌ 但 val 本身不是 const! */
val = 20;   /* ✅ val 可以通过自身名字修改 */
```

通过 `const` 指针读数据是一种「承诺不通过该指针修改」，不代表数据本身不可变。

## 动手练习

### 🟢 入门：识别 const 类型

判断以下声明中哪些可以改数据，哪些可以改指向：

```c
const int32_t *a;    // ?
int32_t *const b;    // ?
const int32_t *const c;  // ?
```

<details><summary>点击查看答案</summary>

```
a: 数据❌, 指向✅   (指向 const 的指针)
b: 数据✅, 指向❌   (const 指针)
c: 数据❌, 指向❌   (双重 const)
```

</details>

### 🟡 中级：安全读取函数

编写 `int read_value(const int32_t *ptr, int32_t *out)`，当 ptr 为 NULL 时返回 0，否则通过 out 返回 `*ptr`。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

int read_value(const int32_t *ptr, int32_t *out)
{
    if (ptr == NULL || out == NULL) return 0;
    *out = *ptr;
    return 1;
}

int main(void) {
    const int32_t secret = 42;
    int32_t result = 0;
    if (read_value(&secret, &result)) {
        printf("value = %d\n", result);
    }
    return 0;
}
```

</details>

### 🔴 挑战：const 正确的字符串复制

编写 `void safe_copy(char *dest, size_t dest_size, const char *src, size_t count)`，确保 dest 可写、src 只读，复制不超过 count 字节且始终 null 终止。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void safe_copy(char *dest, size_t dest_size, const char *src, size_t count)
{
    size_t max = dest_size - 1;
    if (count < max) max = count;
    memcpy(dest, src, max);
    dest[max] = '\0';
}

int main(void) {
    const char msg[] = "hello world";
    char buf[6];
    safe_copy(buf, sizeof(buf), msg, 100);
    printf("'%s' (len %zu)\n", buf, strlen(buf));
    return 0;
}
/* 输出: 'hello' (len 5) — 安全截断 */
```

</details>

## 故障排查 (FAQ)

**Q：`const int *p` 和 `int const *p` 有什么区别？**

A：**没有区别**。它们在 C 标准中等价。但 `const int *p` 更常见，因为「const 修饰类型」的直觉更清晰。

**Q：`const` 和 `#define` 宏常量有什么不同？**

A：`#define PI 3.14` 是文本替换，没有类型，没有地址。`const double pi = 3.14;` 是真正的变量，有类型、有地址、受编译器检查。优先使用 `const`。

**Q：函数参数用 `const` 会影响性能吗？**

A：不会。`const` 是编译期语义，不生成额外代码。它只影响编译检查，不影响运行时行为。现代编译器甚至可能利用 `const` 做更好的优化（因为知道数据不会变）。

## 知识扩展

### const 的 `volatile` 搭档

```c
const volatile int32_t *reg = (const volatile int32_t *)0x40000000;
```

`const volatile` 一起使用——告诉编译器「数据不可通过本程序修改，但可能被硬件改变，不要优化掉每次读取」。常用于嵌入式系统的寄存器访问。

### const 与字符串字面量

```c
const char *s = "hello";
/* s[0] = 'H';  ← ❌ 字符串字面量存储在 .rodata 段（只读） */

char s2[] = "hello";
/* s2[0] = 'H'; ← ✅ char[] 在栈上，可修改 */
```

## 小结

- `const T *p` — **指向 const**：不能通过 `p` 修改数据，`p` 可以改指向
- `T *const p` — **const 指针**：`p` 不能改指向，可以通过 `*p` 修改数据
- `const T *const p` — **双重 const**：值和指向都不可改
- 速记：`const` 在 `*` 左边保护数据，在 `*` 右边保护指针
- `const` 是编译期契约——在运行前就阻止错误

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| Const pointer | const 指针 | 指针自身不可修改 |
| Pointer to const | 指向 const 的指针 | 所指向的数据不可修改 |
| Discard const | 丢弃 const | 隐式将 const 转为非 const（编译错误） |
| Qualifier | 限定符 | const / volatile 等类型修饰 |
| API contract | API 契约 | 函数签名对调用方的承诺 |

## 延伸阅读

- [cppreference - Type qualifiers](https://en.cppreference.com/w/c/language/type) — const/volatile 定义
- [const correctness](https://en.wikipedia.org/wiki/Const_correctness) — 维基百科详解

## 继续学习

- [上一章](./void_pointers.md)：void* 指针
- [下一章](./memory_mgmt.md)：内存管理——栈 vs 堆，malloc/free

---

> 本章代码位于 [`src/basic/const_correctness_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/const_correctness_sample.c)。
