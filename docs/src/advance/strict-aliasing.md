# 严格别名与有效类型 (Strict Aliasing & Effective Type)

> 💡 本章配套源码是 `src/advance/strict_aliasing_sample.c` (入口 `main_strict_aliasing_sample()`, 辅助函数 `pun_u32_to_bytes()` / `pun_bytes_to_u32()`), 单元测试在 `test/advance/test_strict_aliasing_sample.c`。这是三条被 `-O2` 「用错误答案惩罚你」的 C 契约之一: 本章讲**类型**维度,姊妹篇 [restrict 章](restrict.md) 讲**对象重叠**维度。

## 开篇故事

有一位同事写了个「高效」的浮点/整数转换:

```c
uint32_t bits = 0x3F800000u;
float *pf = (float *)&bits;
printf("%f\n", *pf);
```

`-O0` 下一切正常, `-O2` 下输出变成 `0.000000`。他检查了三遍代码, 逻辑毫无问题——问题出在编译器「知道」`float` 的写不会影响 `uint32_t` 对象, 于是把第二次读直接折叠成了常量。这段代码从写下第一天起就是 UB, 只是 `-O0` 恰好「配合演出」。

严格别名 (strict aliasing) 是 C 编译器最重要的优化契约之一, 也是最容易被一次强转悄悄违反的。它不像空指针解引用那样立刻崩溃——它给你**错误的答案**, 而且只在优化级别上开后才出现。

## 本章适合谁

- 用过 `*(float *)&some_uint32` 这类「技巧」
- 想知道 `memcpy` 为什么是类型双关的官方答案
- 被 `-O2` 「算错过」、想理解 TBAA (type-based alias analysis)
- 已读 [volatile 章](volatile.md), 继续清理内存安全盲区

## 你会学到什么

1. **有效类型 (effective type)**: `malloc` 的内存如何「变成」某种类型 (C11 §6.5 / §6.2.6.1)
2. **合法双关**: `memcpy` 逐字节搬运 —— `uint32_t` ↔ `uint8_t[4]` ↔ `float`
3. **非法双关**: 指针强制转换为什么是 UB (§6.5p7) —— 只讲不跑
4. **union 双关**: 为什么是「实现定义」而不是「合法」 (§6.5.2.3 脚注)
5. **-O2 的误编译机制**: TBAA 如何让违规代码得到错误答案

## 误解先行 (Error-First)

### 错例: 指针强转做类型双关

```c
/* ❌ 反例 (Counter-example — 永不执行): */

uint32_t bits = 0x3F800000u;
float *pf = (float *)&bits;    /* 指针强制转换 */
if (*pf == 1.0f) { ... }       /* 经 float lvalue 读 int 对象 */
```

### 为什么是 UB — C11 §6.5p7

> 通过**不兼容类型 (incompatible type)** 的 lvalue 访问对象的值 → 未定义行为。
> 两个豁免: **字符类型** (char / unsigned char / signed char) 永远可以; **union 成员**见 §6.5.2.3。

`float` 和 `uint32_t` 是不兼容类型。`bits` 的有效类型是 `uint32_t`, 而 `*pf` 是一个 `float` lvalue——用它去读, 就踩中 §6.5p7。

关键认知: **UB 的惩罚由优化器执行**。GCC/Clang 在 `-O2` (默认 `-fstrict-aliasing`) 下做基于类型的别名分析 (Type-Based Alias Analysis, TBAA): 编译器为每种类型建一张「可合法改写的对象」表, 推导出「`float` lvalue 的访问不可能触达 `uint32_t` 对象」, 于是:

```text
x = *p;        // load p → r1
*q = 1.0f;     // float 写 — TBAA 认为与 p 无关
y = *p;        // TBAA: 「没人动过 p」→ 直接复用 r1, 第二次 load 被消除
```

若 `q` 真的指向 `p` 的存储, `y` 就读到陈旧值——**源码的语义被优化破坏**。这不是编译器 bug, 是 UB 把裁量权交给了编译器。

### 正确做法: 用 memcpy 搬运「表示」

```c
/* ✅ 合法双关: 搬运的是对象表示 (object representation), 不是「换个类型读」 */

#include <string.h>

uint32_t v = 0x01020304u;
uint8_t bytes[4];
memcpy(bytes, &v, sizeof v);      /* uint32 → 4 字节 */
uint32_t back;
memcpy(&back, bytes, sizeof back); /* 4 字节 → uint32 */
```

为什么 `memcpy` 不违规? 它把对象表示**逐字节复制**到另一个对象里, 每次访问都是对**目标对象自己**的合法类型访问; 接收方获得的是一个新对象的合法值, 不是「透过不兼容 lvalue 去读源对象」。逐字节视角还正好落在 §6.5p7 的字符类型豁免上——**字符类型访问永远合法**, 这是 C 标准给序列化/字节操作留的官方通道。

## 有效类型 (Effective Type): 内存如何「成为」某类型

`malloc` 返回的块**没有声明类型** (no declared type)。它什么时候变成某种类型? C11 §6.5p6:

> 把一个值存入无声明类型的对象 (通过非字符类型的 lvalue) 后, 该 lvalue 的类型成为这次访问及其后「不修改该值的访问」的**有效类型 (effective type)**。

```c
/* ✅ 合法: 写入确立有效类型, 读取类型一致 */

int *slot = malloc(sizeof *slot);   /* 块尚无有效类型 */
*slot = 42;                          /* 写入 → 有效类型变为 int */
int ok = *slot;                      /* 读 int — 与有效类型一致, 合法 */
free(slot);
```

推论三条 (决定你后续能怎么读它):

1. 经 `int` lvalue 写入后, 这块内存就「是」`int`——再用 `float*` 去读它, 违反 §6.5p7。
2. 想换类型访问? 要么**重新写入** (让新类型成为有效类型), 要么走字符类型 / `memcpy`。
3. 表示层面另有 **C11 §6.2.6.1** (对象的表示): 整数的位级布局、填充位、大小端都是「表示」问题——双关代码搬的就是这层。

## 合法双关的三种姿势 (配套源码 Demo 2)

### ① uint32_t ↔ 字节缓冲

配套源码把这段暴露给了单元测试 (`pun_u32_to_bytes` / `pun_bytes_to_u32`, 非 static):

```c
void pun_u32_to_bytes(uint32_t v, uint8_t out[4])
{
    /* memcpy 不违反别名规则: 逐字符访问是 §6.5p7 明文许可的例外 */
    memcpy(out, &v, sizeof v);
}

uint32_t pun_bytes_to_u32(const uint8_t in[4])
{
    uint32_t v = 0;
    memcpy(&v, in, sizeof v);
    return v;
}
```

单元测试 `test/advance/test_strict_aliasing_sample.c` 断言: 任意值 roundtrip 无损, 且字节布局符合**运行时探测**的宿主端序 (不硬编码小端——硬编码就把大端平台的正确实现测成失败)。

### ② 原对象的逐字节观察

```c
uint32_t magic = 0x01020304u;
const unsigned char *raw = (const unsigned char *)&magic;
/* raw[0..3] 直接看位型 — 字符类型访问永远合法, 结果反映宿主端序 */
```

这是 `ntohl`/协议解析/十六进制 dump 的底层原理。注意它读的是**同一个对象自己的字节**, 不是「换个类型解引用」。

### ③ uint32_t ↔ float

```c
uint32_t bits = 0x3F800000u;   /* IEEE-754 中 1.0f 的编码 */
float f = 0.0f;
memcpy(&f, &bits, sizeof f);   /* 合法: 搬运表示, 不经不兼容 lvalue 读 */
```

两个安全细节 (源码里都有):

- **只用有定义的位型**: `0x3F800000` 是 `1.0f` 的合法编码; 不要凭空造位型, signaling NaN 之类的表示可能在读取时触发实现定义行为。
- **检查 `sizeof(float) == sizeof(uint32_t)`**: 双关隐含「4 字节」假设, 本平台满足; 不满足时跳过而不是硬搬。

## union 双关: 是「实现定义」, 不是「合法」

另一个流传极广的写法:

```c
/* ❌ 不依赖 (讲解用, 本章不执行): */

union { uint32_t u; float f; } pun;
pun.u = 0x3F800000u;          /* 写入成员 u */
printf("%f", pun.f);          /* 改经成员 f 读出 — 换成员了 */
```

C11 §6.5.2.3 的脚注写明: 读取所用成员与上次写入的成员不同时, 行为按「对值的表示是合适的 (appropriate for the representation)」处理——也就是**实现定义 (implementation-defined)**, 标准不强制统一结果。GCC/Clang 确实明确定义了它 (这也是它在工程代码里「能用」的原因), 但:

- 标准不保证 → 换编译器、换架构、换优化级别, 你**没有追责依据**;
- 静态分析器与 lint 依然会标红;
- 可移植代码的唯一标准答案仍是 `memcpy` ——一行, 零假设, 和 union 写法一样快 (优化器都会折叠成寄存器搬运)。

**结论: union 双关不执行、不依赖; 本章只留注释反例与条文。**

## -O2 为什么能把它算「错」— TBAA 机制

完整推理链 (源码 Demo 5 有同款伪代码):

```text
违规源码:  y = *p;  *q = 1.0f;  z = *p;   (p: uint32_t*, q 实际指向同一对象)

1. 编译器按类型建模: float lvalue 的合法目标集合里没有「有效类型为 uint32_t 的对象」
2. 推导: 中间的 *q 不可能修改 *p
3. 优化: 第二次 load 消除 / 提升 (load hoisting), 复用第一次的值
4. 运行: q 真的覆盖了那个存储 → 复用的值已过期 → 错误答案
```

三个必须记住的特征:

- **症状滞后**: `-O0` 正常、`-O2` 出错, 让人误以为是「优化器 bug」;
- **症状随机**: 换个版本、加一行代码, 重排变了, bug 可能「自己消失」然后在生产复现;
- **UB 无诊断**: 编译器没有义务警告, 这类代码靠 review 和 `make asan` 都抓不到 (ASan 不管别名), 只有**代码本身不违反契约**才可靠。

顺带说明与 [volatile 章](volatile.md) 的分野: `volatile` 管「访问必须发生」, 严格别名管「访问必须用兼容类型」——都在和优化器打交道, 但约束的是优化器的不同权限。

## 常见错误速查

| # | 错误 | 后果 | 正确做法 |
|---|------|------|----------|
| 1 | `(float *)&uint32_var` 解引用 | UB (§6.5p7), `-O2` 可算错 | `memcpy` 搬位型 |
| 2 | 写入后换不兼容类型读回 | UB, 症状随优化漂移 | 重新写入有效类型, 或 `memcpy` |
| 3 | `char*` 之外还用 `short*` 探对象表示 | 违反别名, UB | 字符类型或 `memcpy` |
| 4 | 依赖 union 换成员读 | 实现定义, 不可追责 | `memcpy` (标准唯一答案) |
| 5 | 双关野值 float (任意大整数位型) | 可能踩 signaling NaN | 用有定义的位型, 先查 `sizeof` |
| 6 | 以为 ASan/UBSan 能抓别名违规 | 工具抓不到, 别依赖 | 结构上用合法通道 |

## 动手练习

### 🟢 练习 1: 合法还是非法

判断下列写法 (设 `uint32_t u = 0x3F800000u;`):

a. `memcpy(&f, &u, 4);` 读回 `float f`
b. `float *pf = (float *)&u; use(*pf);`
c. `unsigned char *p = (unsigned char *)&u; use(p[0]);`
d. `union { uint32_t a; float b; } x; x.a = u; use(x.b);`

<details><summary>点击查看答案</summary>

**a、c 合法** (memcpy 搬表示; 字符类型豁免 §6.5p7); **b 非法** — UB; **d 实现定义** — 会跑, 但不可依赖 (§6.5.2.3 脚注)。

</details>

### 🟡 练习 2: 写一个端序无关的 roundtrip 断言

不硬编码字节序, 验证 `pun_u32_to_bytes` 与 `pun_bytes_to_u32` 互为逆操作。

<details><summary>点击查看答案</summary>

```c
uint8_t buf[4];
const uint32_t v = 0xCAFEBABEu;
pun_u32_to_bytes(v, buf);
/* roundtrip 不看字节顺序 —— 无论宿主端序如何都必须成立 */
assert(pun_bytes_to_u32(buf) == v);
```

单元测试 `test_strict_aliasing_sample.c` 正是这么写的 (外加运行时探测端序的逐字节断言)。

</details>

### 🔴 练习 3: 给 malloc 块「换类型」

一块 `malloc(4 * sizeof(int))` 的内存, 已经写过 4 个 `int`。现在想把它当 `float` 数组用, 正确的两步是什么?

<details><summary>点击查看答案</summary>

1. 逐元素**重新写入** (经 `float` lvalue 存值) —— 写入建立新的有效类型 (§6.5p6);
   或者整块 `memcpy` 到一个新的 `float` 数组里再用。
2. 绝不把指针强转后直接读 —— 那是 §6.5p7 的 UB。

</details>

## 故障排查 (FAQ)

**Q: `-fno-strict-aliasing` 能不能让违规代码「变安全」?**

A: 它只是让**这个编译器**放弃相关优化, 违规代码仍是 UB——你买到的是「这次没被坑」, 不是正确性。移植、升级工具链、换架构后结论随时改变。别用编译选项为 UB 背书。

**Q: union 双关在我的 GCC/Clang 上一直好用, 为什么说不能用?**

A: 编译器定义 ≠ 标准定义。你失去的是可移植性与追责依据 (§6.5.2.3 脚注是实现定义), 同时静态分析/lint 依然会标红。`memcpy` 一样会被优化成相同的寄存器搬运, 没有性能损失, 没有假设。

**Q: ASan/UBSan 能抓别名违规吗?**

A: 不能。Sanitizer 查的是越界、UAF、空指针这类**内存**错误, 不查**类型契约**。别名违规在 `-O2` 下表现为「正确的地址读到错误的值」, 工具链保持沉默——保证只能来自代码本身合法。

**Q: `char` 数组读 `int` 的字节, 也算双关吗?**

A: 不算——字符类型访问是 §6.5p7 的明文豁免, 永远合法。这正是协议解析、序列化、十六进制 dump 的标准通道。

## 小结

本章的核心要点:

- **有效类型 (effective type)**: `malloc` 块经 lvalue 写入后获得类型 (§6.5p6); 后续访问必须与之兼容。
- **§6.5p7**: 不兼容 lvalue 访问 = UB; 两条豁免是**字符类型**与 **union** (后者实现定义)。
- **`memcpy` 是合法双关的唯一标准答案**: 搬运对象表示, 每次访问都是合法类型访问; `pun_u32_to_bytes` / `pun_bytes_to_u32` 是它的模块化形态。
- **指针强转双关只当反例**: 永不执行; `-O2` 的 TBAA 会用「错误答案」执行 UB 的惩罚。
- **union 双关是实现定义** (§6.5.2.3 脚注): 能跑 ≠ 可移植, 不依赖。
- **工具局限**: ASan/UBSan 不查别名; 保证只能来自不违反契约的代码。

> 「类型双关的正确写法只有一种: 承认你在搬**字节**, 就用字节的工具。」

## 术语表

| 英文 | 中文 |
|------|------|
| Strict aliasing | 严格别名 |
| Effective type | 有效类型 |
| Object representation | 对象表示 |
| Type punning | 类型双关 |
| Incompatible type | 不兼容类型 |
| Type-based alias analysis (TBAA) | 基于类型的别名分析 |
| Load hoisting | 读取提升 (指令外提) |
| Implementation-defined | 实现定义 |
| Signaling NaN | 信号型 NaN |
| Endianness | 字节序 (端序) |
| Undefined behavior (UB) | 未定义行为 |

## 延伸阅读

- [CERT EXP39-C](https://wiki.sei.cmu.edu/confluence/display/c/EXP39-C.+Do+not+break+the+effective+type+rules) — Do not break the effective type rules (本章的规则编号)
- [C11 §6.5p6-p7 / §6.5.2.3 / §6.2.6.1](https://en.cppreference.com/w/c/language/effective_type) — 条文与 cppreference 解读
- [LLVM TBAA metadata](https://llvm.org/docs/TypeMetadata.html) — 编译器如何实现别名分析 (进阶)
- ulfalizer — *A Strict Aliasing Story* (GitHub) — 真实误编译案例集
- 姊妹篇: [restrict 章](restrict.md) — 对象**重叠**维度的另一条优化契约
- 前一篇: [volatile 章](volatile.md) — 访问**必须发生**维度的契约

## 继续学习

你已经拿到 C 与优化器之间最隐蔽的一份契约: 类型不兼容的访问不是「技巧」, 是 UB——而 `-O2` 会替标准执行判决。`memcpy` 是你唯一需要记住的逃生门。

下一章, 我们看契约的另一面: **`restrict`** —— 你向编译器承诺「这些指针指向的存储互不重叠」, 交换来一次自动向量化 (vectorization) 的机会。

- [上一章](./volatile.md): volatile 易变限定符
- [下一章 →](./restrict.md): restrict 指针契约
