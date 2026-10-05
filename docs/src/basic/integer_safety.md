# 整数安全 (Integer Safety)

```admonish note
**本章讲什么** — CERT INT30-C / INT31-C / INT33-C 的入门版:有符号溢出绝不执行、无符号回绕才可依赖、移位先查宽度、窄化先验证再转换。全部带可测的 checked 助手。
**对应源码** — [`src/basic/integer_safety_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/integer_safety_sample.c)
```

## 开篇故事

几乎所有 C 程序员都写过这一行:

```c
int total = count * unit_price;   /* 如果 100000 * 30000 呢? */
```

乘积超出 `int` 范围时,有符号溢出是**未定义行为(UB)**——编译器可以假设它永不发生,于是把你的「溢出检查」优化掉。这不是理论洁癖:Heartbleed 之外的无数 CVE,根因就是一行没设防的整数运算。

C 给了你三样工具:标准定义的**无符号回绕**、`limits.h` 的**边界常量**、以及本章的手艺——**在做危险运算之前先证明它安全**。原则一句话:**永远别让危险运算发生,而不是祈祷它不发生。**

## 本章适合谁

- 写过 `count * size`、`len + 1`、`x << n` 却从没想过溢出的初学者
- 被「检查本身也会溢出」绕晕过的程序员
- 准备读 CERT C、想先有点手感的读者

## 你会学到什么

1. 无符号回绕是**良定义**的(可依赖);有符号溢出是 UB(绝不执行)
2. `checked_add_int`:检查不溢出的加法(CERT INT30-C)
3. 安全窄化:宽类型里验证范围 → 显式转换(CERT INT31-C)
4. 移位宽度检查:先查后移(CERT INT33-C)
5. `inttypes.h` 的 `PRIi32` 系列格式宏

## 错例:先算再检查(以及检查本身溢出)

```c
/* ❌ 错例 1: 直接算 — 有符号溢出是 UB */
int sum = a + b;
if (sum < a) { /* "溢出检测"? 编译器可以把这个判断优化掉 */ }

/* ❌ 错例 2: 检查式写错了 — INT_MAX - b 自己溢出 */
if (a > INT_MAX - b) { /* b 为负数时, INT_MAX - b 超出范围, 检查本身是 UB */ }
```

### 为什么错

**错例 1**:标准允许编译器假设「有符号溢出永不发生」,`sum < a` 这种「利用回绕痕迹」的推断会被 `-O2` 直接删除。你看到的检查只是幻觉。

**错例 2** 更阴险——它看起来就是教科书写法,但只对 `b > 0` 成立:

```
b = -1 时:  INT_MAX - (-1) = INT_MAX + 1  ← 检查自己先溢出
```

**检查代码本身也必须是安全的**——这是整数安全里最容易被忽略的一条。

### 正确写法:按 b 的符号分两支检查

```c
int checked_add_int(int a, int b, int *out)
{
    if (out == NULL) {
        return 0;
    }
    /* 检查本身不能溢出: b > 0 时用 INT_MAX - b (合法), b < 0 时用 INT_MIN - b
     * (减负数 = 加正量, 不会越过边界) — 这就是不能只写 a > INT_MAX - b 的原因 */
    if (b > 0 && a > INT_MAX - b) {
        return 0;
    }
    if (b < 0 && a < INT_MIN - b) {
        return 0;
    }
    *out = a + b;
    return 1;
}
```

为什么两支都安全:

| 分支 | 守卫表达式 | 为什么不会溢出 |
|------|-----------|----------------|
| `b > 0` | `INT_MAX - b` | 减正数,只会变小 |
| `b < 0` | `INT_MIN - b` | 减负数 = 加正量;`b = INT_MIN` 时 `INT_MIN - INT_MIN = 0`,恰好不越界 |

两个守卫都成立时,`a + b` **数学上必然**在 `[INT_MIN, INT_MAX]` 内,于是 `*out = a + b` 执行时绝不可能溢出。返回 `1/0` 让调用方显式处理失败——**失败是返回值,不是灾难**。

## 原理解析

### 1. 无符号回绕:标准给你兜底

```c
uint32_t u = UINT32_MAX;
u = u + 1;
printf("UINT32_MAX + 1 = %" PRIu32, u);   /* 输出 0 */
```

C17 6.5p5:无符号运算的结果按 `mod 2^N` 归约——**回绕是语言定义的行为,不是 UB**,sanitizer 也不会报警。所以无符号计数器可以放心依赖回绕(环形缓冲区、哈希混合都这么写)。

对照记忆:

| 运算 | 溢出时 | 能依赖吗 |
|------|--------|---------|
| `unsigned` 加/乘/减 | 模 2^N 回绕,良定义 | ✅ 可依赖 |
| `signed` 加/乘 | **UB** | ❌ 绝不执行 |
| 移位 `x << n`,`n >= 宽度` | **UB** | ❌ 先查宽度 |
| `INT_MIN / -1` | **UB**(商不可表示) | ❌ 单独判 |
| 显式转换到窄类型 | 截断,实现定义 | ⚠️ 不 UB 但结果变了 |

### 2. 安全窄化:先验证,再转换(CERT INT31-C)

```c
/* 先在宽类型里验证范围, 再显式转换 — 顺序不能反过来 (CERT INT31-C) */
static int narrow_to_int(long long v, int *out)
{
    if (v < (long long)INT_MIN || v > (long long)INT_MAX) {
        return 0;
    }
    *out = (int)v;
    return 1;
}
```

- **比较发生在 `long long` 里**——`long long` 比 `int` 宽,两个边界判断本身不可能溢出
- 验证通过后,`(int)` 转换保证值不变,显式写出转换是给读者的信号:「这里我知道自己在做什么」
- 对照反面:

```c
long long big = 5000000000LL;
printf("%d", (int)big);   /* 不报错、不 UB — 但打印的是截断后的 705032704 */
```

实现定义的截断(C17 6.3.1.3p3)**不会被 `-fsanitize=undefined` 抓住**——它安静地给出错误值。这就是为什么窄化必须「先验证」:验证是唯一能发现它的地方。

### 3. 移位宽度检查:先查后移(CERT INT33-C)

```c
int checked_shl_uint(uint32_t value, unsigned shift, uint32_t *out)
{
    if (out == NULL) {
        return 0;
    }
    /* 宽度检查必须发生在移位之前: shift >= 32 的移位是 UB (C17 6.5.7p3) */
    if (shift >= 32u) {
        return 0;
    }
    *out = value << shift;
    return 1;
}
```

- C17 6.5.7p3:移位宽度**大于等于类型宽度是 UB**——不是回绕,不是饱和,是未定义
- 检查必须在移位**之前**:`if (shift < 32) ... else 移位` 的顺序写反一次就是 UB
- 用 `uint32_t` 承载移位结果:无符号左移丢位是良定义的(31 位处 `1u << 31` 得到 `0x80000000`,合法)

### 4. `inttypes.h`:格式宏跨平台打印

```c
int32_t s = -42;
uint32_t u = 0;
printf("s = %" PRId32 "\n", s);           /* int32_t 的正确格式 */
printf("u = %08" PRIX32 "\n", u);         /* 十六进制,可拼接宽度 */
```

`int32_t` 的底层类型**在不同平台可能是 `int` 也可能是 `long`**,写 `%d` 碰运气。`PRId32`/`PRIu32`/`PRIx32` 由 `<inttypes.h>` 按平台展开成正确格式串,拼在 `printf` 格式里即可。`uint32_t` 同理走 `PRIu32`。

### 5. 测试即安全网

```c
TEST_ASSERT_FALSE(checked_add_int(INT_MAX, 1, &out));
TEST_ASSERT_FALSE(checked_add_int(INT_MIN, -1, &out));
TEST_ASSERT_FALSE(checked_shl_uint(1u, 32, &out));
```

`make test-asan` 用 UBSan 跑这些用例:如果哪天有人把守卫「简化」掉,`INT_MAX + 1` 会当场触发 `signed integer overflow` 报警,测试变红。**危险路径既被拒绝、又被测试钉死。**

## 常见错误

### ❌ 错误 1:用 `unsigned` 掩盖有符号问题

```c
int a = -1;
unsigned ua = (unsigned)a;
ua + 1;    /* 良定义, 但语义已不是你想算的那个和 */
```

绕道 `unsigned` 再绕回来,值可能悄悄变了。**该检查就检查**,别靠类型体操。

### ❌ 错误 2:检查顺序写反

```c
if (a < INT_MIN - b) ...   /* b > 0 时, INT_MIN - b 自己下溢! */
```

和错例 2 是同一类病:守卫表达式也要按符号分支,或证明它对所有输入都安全。

### ❌ 错误 3:移位宽度来自运行期却没检查

```c
unsigned n = read_user_input();   /* 0..255 随便什么 */
x << n;                            /* n >= 32 → UB */
```

宽度是运行期值时,**每次移位前都要过 `checked_shl_uint` 这道门**。

### ❌ 错误 4:除法忘了 `INT_MIN / -1`

```c
int q = a / b;    /* b = -1 且 a = INT_MIN → UB (商 2^31 装不下) */
```

本章样例刻意不执行任何这种除法;真实代码里除法前要同时防 `b == 0` 和 `(a, b) == (INT_MIN, -1)`。

## 动手练习

### 🟢 入门:识别危险表达式

下面哪些在某个输入下是 UB?

```c
(1) unsigned u;  u + 1
(2) int a;       a + 1
(3) int x, n;    x << n
(4) uint32_t v;  v << 5
```

<details><summary>点击查看答案</summary>

```
(1) ✅ 安全 — 无符号回绕良定义
(2) ⚠️ 当 a == INT_MAX 时 UB — 有符号溢出
(3) ⚠️ 当 n >= 32 或 n < 0 时 UB — 未检查的移位宽度
(4) ✅ 安全 — 5 < 32, 且无符号丢位良定义
```

</details>

### 🟡 中级:实现 checked 乘法

仿照 `checked_add_int` 写 `checked_mul_int(int a, int b, int *out)`(提示:除法分支处理,`b != 0` 时用 `a` 与 `INT_MAX / b` 比较;`b == 0` 直接放行)。

<details><summary>点击查看答案</summary>

```c
#include <limits.h>

int checked_mul_int(int a, int b, int *out)
{
    if (out == NULL) return 0;
    if (a == 0 || b == 0) {
        *out = 0;
        return 1;
    }
    if (a > 0 && b > 0 && a > INT_MAX / b) return 0;
    if (a > 0 && b < 0 && b < INT_MIN / a) return 0;
    if (a < 0 && b > 0 && a < INT_MIN / b) return 0;
    if (a < 0 && b < 0 && a < INT_MAX / b) return 0;
    *out = a * b;
    return 1;
}
```

每个除法的除数都已非 0,且 `INT_MIN / -1` 的组合被符号分支排除——**守卫自身无 UB**。

</details>

### 🔴 挑战:环形缓冲区索引

用无符号回绕实现 `(idx + 1) % CAP` 的替代写法:要求 `idx` 始终是 `unsigned`,证明 `idx + 1` 何时回绕、回绕后为何仍是合法下标(设 `CAP` 是 2 的幂)。

<details><summary>点击查看答案</summary>

```c
#define CAP 8u   /* 必须是 2 的幂 */

unsigned next_index(unsigned idx)
{
    return (idx + 1u) & (CAP - 1u);
}
/* idx == CAP-1 时: idx+1 == CAP (不回绕), 掩码后 = 0 — 回到起点。
   依赖两点: 无符号加法良定义; CAP 为 2 的幂时 & (CAP-1) 等价 % CAP。 */
```

</details>

## 故障排查 (FAQ)

**Q:为什么我的溢出检查没生效?**

A:九成是检查在**有符号溢出发生之后**执行(`sum = a + b; if (sum < a)`)。检查必须在运算**之前**用 `limits.h` 边界推导,像 `checked_add_int` 那样。

**Q:`size_t` 加法会溢出吗?**

A:`size_t` 是无符号,回绕良定义——但回绕后的长度分配、拷贝会酿成缓冲区灾难(分配小、写入多)。**语义上**仍是危险的,分配路径要用 `checked` 思路先证明 `n + header` 不越界。

**Q:`-fwrapv` 能让有符号溢出可依赖吗?**

A:它把有符号溢出定义为回绕——但这是 GCC/Clang 的**编译器扩展**,不可移植,且只覆盖加减乘。本教程的标准姿势是:不依赖它,永远让溢出不发生。

**Q:为什么不用饱和运算(saturating add)?**

A:饱和(到边界就停)是**业务语义**,标准没有;要不要饱和取决于调用方。`checked_add_int` 返回失败让调用方决定,比默认饱和更诚实。

## 小结

- 有符号溢出 = **UB**,检查必须在运算之前,且**检查本身不能溢出**
- `checked_add_int`:按 `b` 符号分两支守卫(`INT_MAX - b` / `INT_MIN - b`),证明安全后再加
- 无符号回绕是**良定义**的,可依赖;`mod 2^N` 是语言承诺
- 窄化:**宽类型里验证范围 → 显式转换**;实现定义的截断 sanitizer 抓不到
- 移位:**先查 `shift < 宽度` 再移**,`shift >= 32` 的移位是 UB
- `INT_MIN / -1` 单独防;`int32_t` 打印用 `PRId32` 族格式宏
- 危险运算的正确策略是**让它不发生**,并用 `make test-asan` 钉死守卫

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| Signed overflow | 有符号溢出 | 超出有符号范围,UB |
| Unsigned wraparound | 无符号回绕 | 模 2^N 归约,良定义 |
| Checked arithmetic | 检查式算术 | 运算前验证边界,失败即返回错误 |
| Narrowing conversion | 窄化转换 | 宽类型 → 窄类型,可能截断 |
| Shift width | 移位宽度 | 移位位数,必须小于类型位宽 |
| Implementation-defined | 实现定义 | 标准留给实现选择、但必须文档化的行为 |

## 延伸阅读

- [CERT INT30-C](https://wiki.sei.cmu.edu/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap) — 无符号回绕的合规要求
- [CERT INT31-C](https://wiki.sei.cmu.edu/confluence/display/c/INT31-C.+Ensure+that+conversions+to+smaller+integer+types+cannot+lose+data) — 窄化转换
- [CERT INT33-C](https://wiki.sei.cmu.edu/confluence/display/c/INT33-C.+Ensure+that+shift+counts+are+less+than+the+precision+of+the+promoted+left+operand) — 移位宽度
- [cppreference - integer overflow](https://en.cppreference.com/w/c/language/integer_overflow) — 溢出与回绕的语义差别

## 继续学习

- [上一章](./designated_init.md):指定初始化与复合字面量
- [下一章](./bit_ops.md):位运算

---

> 本章代码位于 [`src/basic/integer_safety_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/integer_safety_sample.c),测试位于 [`test/basic/test_integer_safety_sample.c`](https://github.com/savechina/hello-c/blob/main/test/basic/test_integer_safety_sample.c)。
