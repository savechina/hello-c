# volatile (易变限定符)

> 💡 本章配套源码是 `src/advance/volatile_sample.c` (入口 `main_volatile_sample()`), 单元测试在 `test/advance/test_volatile_sample.c`。`volatile_demo_poll()` 把 signal/raise 演示暴露给测试, 保证可重复断言。本章是错误优先 (Error-First) 叙事: 先看最流行的误解, 再看它为什么是 UB, 最后落到三个真正正确的用法。

## 开篇故事

很多从嵌入式或老代码入门的 C 程序员都听过一句口口相传的咒语:「共享变量加个 `volatile`, 线程就安全了」。于是你在多线程计数器前面加上 `volatile`, 编译警告消失了, 心里也踏实了——直到某天两个线程各加十万次, 结果不是二十万。

`volatile` 是 C 里被误解最深的关键字。它不是并发工具, 而是一条给**编译器**的约束:「这个地址背后可能有你不知道的事情, 每次访问都给我老老实实走内存」。它管不住 CPU, 管不住内存模型, 也管不住另一个线程。

> 「`volatile` is about what the compiler may NOT do; concurrency is about what the hardware and the abstract machine MUST guarantee. The two conversations don't overlap.」

## 本章适合谁

- 听说过「`volatile` 线程安全」但没读过标准条文
- 写过信号处理 (signal handler) 或嵌入式寄存器访问
- 用 `-O2` 优化过循环计数, 怀疑被「优化没了」
- 想弄清 `volatile` 和 `_Atomic` (见 [原子类型](atomic-types.md)) 的边界

## 你会学到什么

1. `volatile` 到底保证什么、**不**保证什么 (C11 §5.1.2.4 的视角)
2. 错误用法: 用 `volatile` 做线程共享计数器 —— 为什么仍是 data race / UB
3. 正确用法 ①: `volatile sig_atomic_t` 信号标志 (标准 C, 可移植)
4. 正确用法 ②: 内存映射 I/O (MMIO) —— 原理与边界
5. 正确用法 ③: `volatile` 计数循环 —— 以及为什么不能依赖优化器行为
6. 什么时候该换成 `atomic`

## 误解先行 (Error-First)

### 错例: 「volatile 共享计数器」

最常见的写法长这样:

```c
/* ❌ 反例 (Counter-example — 不要执行): */

volatile int shared_counter = 0;

/* 线程 A 与线程 B 同时执行: */
shared_counter++;              /* 仍是 read → modify → write 三步 */
```

看起来 `volatile` 已经把变量「保护」起来了。实际发生的是:

```
线程 A:  R1 = load shared_counter   ; 读到 5
线程 B:  R2 = load shared_counter   ; 也读到 5 (volatile 保证每次都走内存,
                                     ; 但走内存 ≠ 别人不同时在走)
线程 A:  store R1+1 → 6
线程 B:  store R2+1 → 6             ; 丢失更新 (lost update)
```

`volatile` 甚至**没有**让这个时序变得更安全: 它只保证读写真的发生, 不保证读-改-写三步之间没有别人插队。

### 为什么是 UB — C11 §5.1.2.4

标准对多线程的规则非常干脆 (C11 §5.1.2.4「Multithreaded executions and data races」):

> 两个执行线程访问同一标量对象、至少一个是写、且没有同步 → **data race → 行为未定义 (UB)**。

`volatile` 在这条判定里**完全不出现**。它不提供三样东西:

| 保证 | `volatile` 给吗 | 说明 |
|------|:---:|------|
| 原子性 (atomicity) | ❌ | `++` 拆成的读/改/写仍可被打断 |
| 内存顺序 (ordering) | ❌ | 编译器与 CPU 都可对访问重排序 |
| 免受其他执行流干扰 (interference) | ❌ | 缓存、乱序执行照样产生陈旧读 |

第三条尤其容易被忽略: 就算每次读都真的走内存, 另一个核心 (core) 通过缓存看到的值仍可能滞后——`volatile` 是编译器关键字, 不是内存屏障 (memory barrier)。

`volatile` 唯一的承诺是 (C11 §6.7.3, 对 volatile lvalue 的访问是实现定义的副作用, 必须按源码发生): **编译器不得把这次访问缓存进寄存器、不得删除、不得合并次数**。就这一条, 与并发正确性无关。

### 正确做法: 用 atomic

线程间共享的读-改-写, 用 C11 原子类型:

```c
/* ✅ 正确: atomic_int + atomic_fetch_add */

#include <stdatomic.h>

atomic_int shared_counter;
atomic_init(&shared_counter, 0);

atomic_fetch_add(&shared_counter, 1);   /* 真正的原子操作 */
```

完整讲解、内存顺序 (memory order) 与 CAS 见同目录章节 [原子类型 (Atomic Types)](atomic-types.md) ——源码 `src/advance/atomic_types_sample.c`。记忆锚点:

> **`volatile` 管编译器, `atomic` 管编译器 + CPU + 内存模型。**

## volatile 到底保证什么

把边界一次说清:

```c
volatile int x = 0;
x = 1;      /* 必须真的写内存 —— 不可被省略、合并 */
int a = x;  /* 必须真的读内存 —— 不可复用上一次的寄存器值 */
int b = x;  /* 第二次读仍是独立的内存访问 —— 不可被 CSE (公共子表达式) 合并 */
```

不保证的: 原子性、顺序、宽度 (读 int 可能编译成两次访存的撕裂读)、以及任何跨线程/跨核的可见性语义。把 `volatile` 想象成「取消编译器的这几项优化豁免权」——仅此而已。

## 正确用法 ①: 信号标志 (signal handler)

标准 C 明确规定 (C11 §7.14.1.1): 信号处理器与主程序之间, 唯一可以放心共享的对象是 **`volatile sig_atomic_t`**。`sig_atomic_t` 保证单次读写的原子性, `volatile` 保证主程序的轮询循环每次都重新读内存。

配套源码里的实现 (摘自 `src/advance/volatile_sample.c`):

```c
static volatile sig_atomic_t g_signal_flag = 0;

static void volatile_signal_handler(int signo)
{
    (void)signo;
    g_signal_flag = 1; /* 处理器只做最小动作: 置标志 */
}

int volatile_demo_poll(void)
{
    g_signal_flag = 0;

    if (signal(SIGINT, volatile_signal_handler) == SIG_ERR) {
        return 0; /* 安装失败按「未收到信号」报告, 保持确定性 */
    }

    (void)raise(SIGINT); /* 标准 C: raise 返回前处理器已执行完毕 */

    /* 有界轮询: 上限 1000 次、不 sleep —— 绝不无限等待 */
    int polls = 0;
    while (g_signal_flag == 0 && polls < 1000) {
        polls++;
    }

    (void)signal(SIGINT, SIG_DFL); /* 恢复默认动作, 不污染调用者环境 */
    return (g_signal_flag != 0) ? 1 : 0;
}
```

要点:

- **为什么需要 `volatile`**: 没有它, 编译器看到循环里 `g_signal_flag` 没被当前代码修改, 可能把读提升到循环外——循环就再也看不到信号了。
- **为什么需要 `sig_atomic_t`**: 普通类型在处理器与主程序之间可能被撕裂 (torn read/write)。
- **处理器里只做最小动作**: `printf`、`malloc`、加锁都不是异步信号安全 (async-signal-safe) 的, 只允许写 `volatile sig_atomic_t` 这种级别。
- **轮询必须有界**: 真实程序里等待外部事件可以无限等, 教学/测试代码必须设上限——否则信号丢失时就挂死了。本例用固定 1000 次上限 + `raise()` 同步触发, 保证确定性终止。

单元测试 `test/advance/test_volatile_sample.c` 对 `volatile_demo_poll()` 连续断言三次返回 1, 证明「安装 → 触发 → 轮询 → 恢复」这个序列可重复、无状态泄漏。

## 正确用法 ②: 内存映射 I/O (MMIO) — 原理占位

嵌入式系统把外设寄存器映射到固定地址:

```c
/* 嵌入式典型写法 (示意 — 本平台无真实 MMIO 设备, 仓库不执行): */

*(volatile uint32_t *)0x40021000U = 0x01U;  /* 写状态寄存器 */
uint32_t st = *(volatile uint32_t *)0x40021000U;  /* 读状态寄存器 */
```

为什么必须 `volatile`:

- 写寄存器有**硬件副作用** (启动转换、清中断标志)。若编译器认为「这个地址没人改, 合并两次写」, 外设行为就错了。
- 读寄存器的值**随时被硬件改变** (状态位翻转)。若编译器复用第一次读的寄存器值, 你永远等不到「转换完成」。

`volatile` 强制每次访问真正发出总线读写, 次数与顺序按源码执行——这正是「编译器不知道地址背后有硬件」的补丁。

边界: `volatile` 只约束**编译器**。多核之间、DMA 与 CPU 之间的顺序仍需要 memory barrier / 设备专属的同步原语。本平台 (macOS 用户态) 没有可映射的真实寄存器, 所以源码里只留讲解、不写会执行的代码。

## 正确用法 ③: volatile 计数循环 — 以及为什么别依赖优化器

对比两个循环 (摘自配套源码):

```c
volatile int vol_counter = 0;
for (int i = 0; i < 5; i++) {
    vol_counter++; /* 语义约束: 每次迭代回内存读-改-写, 不可被省略 */
}

int plain_counter = 0;
for (int i = 0; i < 5; i++) {
    plain_counter++; /* 普通变量: 优化器可整体折叠为一次 plain += 5 */
}
```

- `volatile` 版本: 每次迭代都必须真实地读内存、改、写回。语义上被钉死了。
- 普通版本: `-O2` 下编译器**很可能**把它折叠成一条 `+= 5`——但这不是标准保证的行为, 换个编译器、加个函数调用, 结论可能就变了。

**教学立场 (重要)**: 我们**不**断言「普通循环一定被优化掉」。依赖优化器的输出差异来写逻辑或断言, 是经典的脆弱代码 (fragile code)。本章两个循环都打印出 `5`, 演示的只是 `volatile` 那一侧的**约束**存在, 而非另一侧的**优化**必然发生。

什么时候真的需要这种循环? 只有当计数本身是被别处 (硬件、信号、DMA) 异步观察的对象时——那其实是用法 ①/② 的场景, 而不是「防优化」。

## 常见错误速查

| # | 错误 | 后果 | 正确做法 |
|---|------|------|----------|
| 1 | `volatile` 计数器做线程同步 | data race → UB (C11 §5.1.2.4) | `atomic_int` + `atomic_fetch_add` |
| 2 | `volatile` 当轻量锁 / 自旋标志 | 无顺序保证, 仍 UB | `atomic_flag` 或 mutex |
| 3 | 信号处理器里读写普通 `int` | torn read, 行为未定义 | `volatile sig_atomic_t` |
| 4 | 信号处理器里 `printf`/`malloc` | 异步信号不安全, 可死锁 | 只置标志, 主流程再做事 |
| 5 | 依赖「普通循环被优化掉」写逻辑 | 换编译器/选项即碎 | 写语义明确的代码, 不赌优化 |
| 6 | `volatile` 指针解引用当原子操作 | 原子性/顺序全无 | 见原子类型章 |

## 动手练习

### 🟢 练习 1: 正误判断

下面哪些写法是标准允许的 `volatile` 用法?

a. 信号处理器写 `volatile sig_atomic_t` 标志
b. 两个线程对 `volatile int` 计数器自增
c. 读 `*(volatile uint32_t *)reg` 轮询硬件状态位
d. 主循环轮询 volatile 标志直到置位

<details><summary>点击查看答案</summary>

**a、c、d 正确; b 是 UB**。b 的 data race 判定完全不看 volatile (C11 §5.1.2.4), 正确写法是 `atomic_int` + `atomic_fetch_add`。

</details>

### 🟡 练习 2: 有界轮询改造

把 `volatile_demo_poll()` 的等待改写成「最多轮询 100 次, 每次用空转代替 sleep」, 要求: (1) 不使用任何 sleep/挂起, (2) 无论信号是否到达都必然返回, (3) 结束后恢复 `SIG_DFL`。

<details><summary>点击查看答案</summary>

```c
int polls = 0;
while (g_signal_flag == 0 && polls < 100) {
    for (volatile int spin = 0; spin < 1000; spin++) { /* 空转, 不 sleep */ }
    polls++;
}
(void)signal(SIGINT, SIG_DFL);
return g_signal_flag ? 1 : 0;
```

</details>

### 🔴 练习 3: 一句话划清边界

用一句话分别回答: (1) `volatile` 保证什么? (2) 它不保证什么? (3) 线程共享计数器该用什么?

<details><summary>点击查看答案</summary>

1. 每次访问都真实发生于内存 —— 编译器不得缓存、删除或合并这些访问。
2. 原子性、内存顺序、跨核可见性, 一概不保证; data race 照样是 UB (§5.1.2.4)。
3. `atomic_int` + `atomic_fetch_add` —— 见 [原子类型章](atomic-types.md)。

</details>

## 故障排查 (FAQ)

**Q: `-O2` 下我给计数器加了 volatile, 程序变慢了, 这算「volatile 起作用了」吗?**

A: 变慢只是因为每轮迭代真的访存了。但这不是可以依赖的手段——普通变量的优化结果是实现自由, 逻辑要基于语义写, 不基于优化器的输出写 (Demo 4 的立场)。

**Q: `volatile sig_atomic_t` 能在多线程里当原子变量用吗?**

A: 不能。`sig_atomic_t` 的原子性只覆盖「信号处理器 vs 主程序」这一对场景 (C11 §7.14.1.1), 对线程不作任何承诺。线程同步用 `<stdatomic.h>`。

**Q: 为什么轮询循环有时看不到标志置位?**

A: 两个经典原因: 标志没加 `volatile` 被缓存进寄存器; 或者等待既无触发又无上限。配套源码的组合是: `volatile sig_atomic_t` + `raise()` 同步触发 + 有界轮询。

**Q: 用户态想访问 DMA/设备 buffer, 加 volatile 够吗?**

A: 够用的场景基本只有 MMIO 寄存器 (Demo 2)。DMA buffer 往往还需要 cache 一致性接口 (平台相关), `volatile` 完全不涉及这一层。

## 小结

本章的核心要点:

- **`volatile` 只对编译器说话**: 每次访问必须真实发生于内存; 它不提供原子性、内存顺序、跨核可见性。
- **data race 判定不看 `volatile`** (C11 §5.1.2.4): 线程间无同步读写同一对象 → UB, 加不加 `volatile` 都一样。
- **三个正确用法**: ① `volatile sig_atomic_t` 信号标志 (标准 C 唯一许可的共享方式); ② MMIO 寄存器访问 (编译器看不见硬件); ③ 需要「访问必须发生」的异步可观察对象。
- **信号 demo 的工程约束**: 轮询有界、不 sleep、结束恢复 `SIG_DFL`——教学代码必须确定性终止。
- **线程共享 → atomic**: 正确答案永远先想到 [原子类型章](atomic-types.md) (`src/advance/atomic_types_sample.c`), 它在目录结构上就是本章的兄弟篇。
- **不要依赖优化器行为**: 普通变量的循环是否被折叠是实现自由, 不是你的 API。

> 「`volatile` 不是 `atomic` 的平价替代, 它连 `atomic` 要解决的问题都不在同一个维度上。」

## 术语表

| 英文 | 中文 |
|------|------|
| Volatile qualifier | 易变限定符 |
| Data race | 数据竞争 |
| Undefined behavior (UB) | 未定义行为 |
| Atomicity | 原子性 |
| Memory ordering | 内存顺序 |
| Interference | (其他执行流的) 干扰 |
| Signal handler | 信号处理器 |
| `sig_atomic_t` | 信号安全的原子整型 |
| Asynchronous signal-safe | 异步信号安全 |
| Memory-mapped I/O (MMIO) | 内存映射输入输出 |
| Torn read/write | 撕裂读写 |
| Common subexpression elimination (CSE) | 公共子表达式消除 |
| Type-based alias analysis (TBAA) | 基于类型的别名分析 |

## 延伸阅读

- [CERT EXP32-C](https://wiki.sei.cmu.edu/confluence/display/c/EXP32-C.+Do+not+access+a+variable+using+one+volatile+qualified+type+through+another+volatile+qualified+type) — Do not access a variable using one volatile-qualified type through another (本章误解的权威编号)
- [C11 §5.1.2.4 Multithreaded executions and data races](https://en.cppreference.com/w/c/language/memory_model) — data race = UB 的条文出处
- [cppreference: volatile](https://en.cppreference.com/w/c/language/volatile) — 三类合法用法的标准表述
- Herb Sutter — *Volatiles are Atomic* (Dr. Dobb's): 「用 volatile 做线程同步——看起来像答案, 实际上不是」
- 同章姊妹篇: [原子类型 (Atomic Types)](atomic-types.md) —— `volatile` 误解的正确替代方案

## 继续学习

你已经把 C 里最容易被神化的关键字放回了它该在的位置: `volatile` 是编译器约束, 不是并发原语。

下一章, 我们深入类型系统的另一条优化契约——**严格别名 (Strict Aliasing)**: 为什么 `uint32_t*` 强转 `float*` 是 UB, 而 `memcpy` 却永远合法。

- [上一章](./atomic-types.md): 原子类型 (Atomic Types)
- [下一章 →](./strict-aliasing.md): 严格别名与有效类型 (Strict Aliasing)
