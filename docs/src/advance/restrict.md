# restrict 指针契约 (Pointer Qualifier Contract)

> 💡 本章配套源码是 `src/advance/restrict_sample.c` (入口 `main_restrict_sample()`, 辅助函数 `add_arrays()`), 单元测试在 `test/advance/test_restrict_sample.c`。`restrict` 是编译器优化契约三部曲的第二部: [volatile 章](volatile.md) 管「访问必须发生」, 本章管「存储互不重叠」, [严格别名章](strict-aliasing.md) 管「类型必须兼容」。三者共同点: 违反 = UB, 且 `-O2` 才执行判决。

## 开篇故事

你写了一个向量加法, 跑得比标准库慢三倍。同事看了一眼说: 「加上 `restrict` 试试」。加上之后, 循环真的被自动向量化 (vectorized) 了——同一条循环, 速度翻了几倍。

第二天, 另一个调用点传了重叠的缓冲区, 结果**有时对有时错**, `-O0` 下还总是对的。`restrict` 不是 `const` 那样的只读标记, 也不是 `volatile` 那样的防优化标记——它是一份**你写给编译器的承诺书**。承诺兑现, 编译器回报你性能; 承诺撒谎, 标准授权编译器给你 UB。

## 本章适合谁

- 写过数值/图像/音频处理的逐元素循环
- 听说过「`restrict` 提升性能」但不知道它的违约代价
- 分不清 `restrict`、`const`、`volatile` 三个限定符的分工
- 已读 [严格别名章](strict-aliasing.md), 想补齐另一条优化契约

## 你会学到什么

1. `restrict` 的契约语义 (C11 §6.7.3.1) — 承诺什么、换来什么
2. 合规示例: 分离缓冲区的 `add_arrays` — 编译器可向量化
3. 违规调用为什么是 UB, 为什么症状要到 `-O2` 才出现
4. 何时**不**该用 `restrict`: 重叠语义 (memmove) 的缓冲区
5. `restrict` 与严格别名的关系 — 同族契约的两个维度

## 误解先行 (Error-First)

### 错例: 给可能重叠的参数加 restrict

最常见的误用不是「忘加」, 而是「乱加」——觉得 `restrict` 是性能开关, 于是给所有指针参数都套上:

```c
/* ❌ 反例 (Counter-example — 不要执行): */

void add_arrays(double *restrict dst, const double *restrict a,
                const double *restrict b, size_t n);

double buf[8] = {...};
add_arrays(buf, buf + 1, buf, 7);   /* dst 与 a、b 都重叠! */
```

### 为什么是 UB — C11 §6.7.3.1

`restrict` 的语义: 在其所在块执行期间, 该指针指向的对象**只能通过这一个 lvalue** (及其算术衍生) 被访问/修改。上面的调用让 `dst` 与 `a`、`b` 指向同一存储——同一个对象在块内经多个互不协调的 restrict lvalue 访问, 承诺自相矛盾, **契约被破坏 → 行为未定义**。标准不欠你任何诊断: 编译器完全可以不警告。

### 症状为什么隐蔽 (和严格别名同款)

没有 `restrict`, 编译器必须保守假设「写 `dst` 可能改变 `a[i]`」, 每轮循环都老老实实重读。有了 `restrict`, 它可以批量化:

```text
无 restrict (标量逐个执行):
  读 a[0] 读 b[0] 写 dst[0] | 读 a[1] 读 b[1] 写 dst[1] | ...

有 restrict (展开 + 向量化):
  读 a[0..3] 读 b[0..3] → 算 → 写 dst[0..3] | 下一批 ...
```

重叠时, 第一批的 `写 dst[0..3]` 会污染**还没读的** `a[4..]`——`-O0` 逐个执行时恰好「错得更少」, `-O2` 向量化后结果崩坏。**`-O0` 正常、`-O2` 随机错**, 这是契约类 UB 的经典症状, 详见 [严格别名章](strict-aliasing.md) 的同款分析。

### 正确做法: 先保证分离, 再谈 restrict

```c
/* ✅ 合规调用: 三个缓冲区在栈上各自独立, 契约成立 */

double dst[8];
const double a[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
const double b[8] = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0};

add_arrays(dst, a, b, 8);   /* 分离 → 快, 且编译器可向量化 */
```

## 契约语义: restrict 到底承诺了什么

配套源码里的定义 (摘自 `src/advance/restrict_sample.c`):

```c
void add_arrays(double *restrict dst, const double *restrict a,
                const double *restrict b, size_t n)
{
    /* restrict 契约 (C11 §6.7.3.1): 本函数执行期间, dst/a/b 各自指向的
     * 对象只能通过这三个 lvalue 访问 —— 即三者存储两两不重叠。
     * 编译器可据此假设写 dst 不影响后续读 a/b, 进而展开/向量化循环。 */
    for (size_t i = 0; i < n; i++) {
        dst[i] = a[i] + b[i];
    }
}
```

拆解四条要点:

1. **这是编译期契约, 不是运行时检查**。没有任何代码会去验证重叠; 违约的惩罚由优化器在「某个未来的编译」中兑现。
2. **作用域是「块 (block)」**, 即函数体/语句块的执行期间——不是永久声明。
3. **承诺的是访问路径唯一**: 对象可以通过 restrict 指针的算术衍生 (`p[i]`、`p+1`) 访问, 但不能同时通过另一个指针访问。
4. **与 `const`、`volatile` 正交**: `const` 管只读, `volatile` 管「访问必须发生」, `restrict` 管「存储不重叠」。可以叠着用 (`const double *restrict` = 只读 + 不重叠)。

> 关于声明位置: 头文件 `restrict_sample.h` 给出不带 `restrict` 的兼容声明, 定义处带完整限定——两者按 C11 §6.7.6.3p15 是同一参数类型 (限定/无限定版本在类型判定中等价), 这也让纯 C++ 解析器能读入该头文件。

## 合规示例: 向量加法与向量化

`add_arrays` 是 `restrict` 的教科书形态: 三个参数、三个独立对象、逐元素读写。编译器看到它之后可以安全地:

- **循环展开 (loop unrolling)**: 一次迭代处理 2/4/8 个元素;
- **软件流水 (software pipelining)**: 预取下一批 `a`/`b`, 不怕 `dst` 的写回干扰;
- **SIMD 向量化**: 一条指令加 4 个 `double` (SSE/NEON)。

单元测试 `test/advance/test_restrict_sample.c` 断言数值正确性 (逐元素求和、`n == 0` 不写 `dst`)——契约不检查, 但**结果**永远可以检查。

## 何时不该用 restrict

判断口诀 (源码 Demo 4):

| 场景 | 选择 |
|------|------|
| 调用方**能**保证缓冲区分离, 且要性能 | `restrict` 接口 (如 `add_arrays`) |
| 调用方**可能**传入重叠缓冲区 | 不加 `restrict`, 或内部处理重叠 |
| 语义本身要求支持重叠搬移 | 用 `memmove` 的思路: 先存后写、或双向拷贝 |

标准库给了对照组:

```c
/* 允许重叠: memmove 明确定义了重叠行为 */

char text[16] = "memory";
memmove(text + 2, text, 6);   /* 合法且结果确定 */
```

`memcpy` 的规范里正藏着这条契约的影子——标准允许 `memcpy` 在重叠时行为未定义, 本质就是把「不重叠」的举证责任交给了调用方。给可能重叠的参数加 `restrict`, 等于替调用方签了一张你兑现不了的支票。

**核心原则: `restrict` 必须由「最弱的那个调用方」的使用方式来决定, 不是由「最快的那条路径」的愿望来决定。**

## 与严格别名的关系 — 同族优化契约

两条契约约束的是优化器的不同权限 (源码 Demo 5 的对照表):

| 契约 | 条文 | 约束什么 | 违反时编译器可假设 |
|------|------|----------|--------------------|
| 严格别名 | §6.5p7 | 访问用的**类型**兼容 | 不兼容类型的写不碰此对象 |
| `restrict` | §6.7.3.1 | 指针指向的**存储**不重叠 | 两个指针访问的是不同对象 |

- **共同点**: 都是声明期承诺、违反即 UB、`-O2` 下以错误结果而非崩溃来惩罚、ASan/UBSan 都抓不到。
- **不同点**: 别名看「类型身份」, restrict 看「对象身份」。同一个 `double` 数组, 类型完全兼容, 但传了重叠指针照样违反 restrict; 反之, 类型不兼容的强转与重叠无关, 归 [严格别名章](strict-aliasing.md) 管。

把三部曲放在一起收口:

> **`volatile` 说「每次都访问」, `restrict` 说「只通过我访问」, 别名规则说「用对的类型访问」——三份契约, 一个违约后果: UB。**

## 常见错误速查

| # | 错误 | 后果 | 正确做法 |
|---|------|------|----------|
| 1 | 重叠缓冲区传给 `restrict` 参数 | 契约违反 → UB, `-O2` 结果错 | 保证分离, 或用支持重叠的接口 |
| 2 | 把 `restrict` 当成 `const`/只读标记 | 语义完全错位, 误删契约 | 三限定各管一事, 勿混用 |
| 3 | 给「可能重叠」的公共 API 加 `restrict` | 调用方无法逐个审查, UB 面扩大 | 公共 API 默认不加, 内部热路径再包一层 |
| 4 | 相信运行时会检查重叠 | 永远不会, 无诊断义务 | 正确性靠设计 (分离) 与测试 (数值) |
| 5 | 指望 ASan/UBSan 抓 restrict 违约 | 工具查不了别名/重叠契约 | 结构上不违约才是唯一保证 |
| 6 | 忘了 `n == 0` 时不得解引用任何指针 | 无意义解引用 | 循环边界天然保护, 测试要覆盖 |

## 动手练习

### 🟢 练习 1: 判断能否加 restrict

下列调用场景, `add_arrays(dst, a, b, n)` 能否安全加/用 restrict 接口?

a. `dst`、`a`、`b` 是三个独立的栈数组
b. `add_arrays(buf, buf, buf, n)` —— 三参同一数组
c. `add_arrays(out, in, weight, n)` —— out 与 in 分配自不同 malloc 块
d. `add_arrays(a + 1, a, b, n - 1)` —— dst 与 a 相邻且重叠

<details><summary>点击查看答案</summary>

**a、c 可以** (存储分离, 契约成立); **b、d 不行** —— 重叠违反 restrict 契约, 行为未定义 (C11 §6.7.3.1)。

</details>

### 🟡 练习 2: 实现 restrict 版向量乘加

写 `void axpy(double *restrict y, const double *restrict x, double alpha, size_t n)`, 计算 `y[i] = alpha * x[i] + y[i]`, 并说明契约。

<details><summary>点击查看答案</summary>

```c
void axpy(double *restrict y, const double *restrict x, double alpha, size_t n)
{
    /* 契约: y 与 x 指向的存储不重叠 (y 可被读写, x 只读) */
    for (size_t i = 0; i < n; i++) {
        y[i] = alpha * x[i] + y[i];
    }
}
```

注意: `y` 自己读写同一个对象是允许的——restrict 禁的是**不同** restrict 指针共享对象, 不禁止单指针自读写。

</details>

### 🔴 练习 3: 给重叠场景设计接口

调用方需要 `dst` 与 `src` 可能重叠的「整体右移一位」操作, 你会怎么设计? 为什么不能直接给现有 restrict 版加参数默认值?

<details><summary>点击查看答案</summary>

两条路: (1) 语义上就是 memmove——提供 `shift_right(void *dst, const void *src, size_t n)` **不带** restrict, 内部按「先存后写 / 反向遍历」处理重叠; (2) 要性能就拆成两个函数: 保证分离的 restrict 版 + 处理重叠的保守版。

不能「加个默认值」: restrict 是契约不是开关——同一个函数声明里写了 restrict, 所有调用方都必须兑现, 没有「偶尔重叠也行」的中间态。

</details>

## 故障排查 (FAQ)

**Q: `restrict` 和 `const`、`volatile` 能一起用吗?**

A: 能, 三者正交: `const double *restrict p` = 只读 + 不重叠。`const` 管写权限, `volatile` 管访问必须发生, `restrict` 管对象身份不重叠——互不替代。

**Q: 违反 restrict 时, 为什么编译器不给我一个警告?**

A: 标准没有给编译器任何诊断义务 (契约由调用方兑现, 调用点往往跨编译单元)。而且违规是 UB——编译器有理由假设它不发生, 基于假设生成的代码自然不会「警告假设错了」。

**Q: 我把 restrict 写在定义、没写在声明里, 是 bug 吗?**

A: 不是。C11 §6.7.6.3p15 规定类型判定中参数的限定/无限定版本视为同一类型, 两者兼容。本仓库头文件正是这样处理的 (见 `restrict_sample.h` 注释)——顺带让纯 C++ 解析器也能读头文件。

**Q: 怎么证明我的调用真的没重叠?**

A: 证明只能来自设计 (不同数组/不同分配), 不来自测试。测试能做的是断言**结果数值** (本章 `test_restrict_sample.c` 的做法)——这次对了不代表重叠时也对, 重叠时标准直接不承诺。

## 小结

本章的核心要点:

- **`restrict` 是契约不是检查** (C11 §6.7.3.1): 块执行期间, 指向对象只经该 lvalue 访问; 违反 = UB, 无诊断。
- **换来的是优化权限**: 不重叠假设 → 循环展开、流水、SIMD 向量化——`add_arrays` 就是标准收益形态。
- **症状模式**: `-O0` 正常、`-O2` 随机错——因为惩罚由优化器执行。
- **何时不用**: 重叠语义 (滑动搬移) 用 `memmove` 思路; 公共 API 不轻易加 `restrict`。
- **三部曲定位**: `volatile` (访问必须发生) / `restrict` (存储不重叠) / 别名 (类型兼容) 同族, 全部「违反即 UB」。
- **工具边界**: ASan/UBSan 不查契约, 数值测试只证明这次没重叠——保证来自代码结构。

> 「`restrict` 是你和编译器之间最快也最危险的一份合同: 你拿性能, 它拿走你犯错的权利。」

## 术语表

| 英文 | 中文 |
|------|------|
| Restrict qualifier | restrict 限定符 |
| Pointer contract | 指针契约 |
| Aliasing | 别名 (多指针指向同一对象) |
| Overlap | 重叠 |
| Vectorization | 向量化 |
| Loop unrolling | 循环展开 |
| Software pipelining | 软件流水 |
| SIMD (Single Instruction, Multiple Data) | 单指令多数据 |
| Orthogonal | 正交 (互不干扰) |
| Undefined behavior (UB) | 未定义行为 |
| Object identity | 对象身份 |

## 延伸阅读

- [CERT EXP43-C](https://wiki.sei.cmu.edu/confluence/display/c/EXP43-C.+Do+not+create+incompatible+pointer+types) / [DCL33-C](https://wiki.sei.cmu.edu/confluence/display/c/DCL33-C.+Do+not+use+the+restrict+qualifier) — restrict 使用与误用的规则编号
- [C11 §6.7.3.1 Restrictions on effective type](https://en.cppreference.com/w/c/language/restrict) — 条文原文与解读
- [cppreference: restrict](https://en.cppreference.com/w/c/language/restrict) — 合规/违规示例集
- [IBM XL compiler restrict 文档](https://www.ibm.com/docs/en/xl/16.1.0?topic=restrict-qualifier) — 向量化收益的工程视角 (进阶)
- 姊妹篇: [严格别名章](strict-aliasing.md) — 类型维度的契约; [volatile 章](volatile.md) — 访问维度的契约; [原子类型章](atomic-types.md) — 并发的正确答案

## 继续学习

你已经拿到与优化器签约的方法论: 承诺 (restrict) 换性能, 但只能承诺你**真正能兑现**的那部分——重叠就别加, 公共接口就要替调用方留余地。至此, 内存安全审计发现的三条空白 (volatile 误用、严格别名、restrict) 全部补齐。

- [上一章](./strict-aliasing.md): 严格别名与有效类型
- [下一章 →](./memory-safety.md): 内存安全与 RAII 手法
