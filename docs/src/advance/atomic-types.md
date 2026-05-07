# 原子类型 (Atomic Types)

## 开篇故事

想象两个厨师共用一个切菜板。厨师 A 拿起一颗菜，切成两半，放回板子。同时厨师 B 也拿起同一颗菜——但 A 还在切的过程中,板子上的状态是「半切半原」。B 拿走了一个不完整的东西, 而两个人都在同时操作同一个地方, 结果就会混乱。

原子操作 (atomic operation) 就是给切菜板加了一把锁: 当一个厨师在用板子时, 其他人必须等着, 直到当前操作完全完成, 才能轮到下一个厨师。整个过程中, 板子上的状态要么「还没切」, 要么「已切好」, 永远不会出现「切了一半」的中间态被其他人看到。

> 「在并发编程的世界里, 原子操作是你的安全绳。没有它, 多线程就是盲人摸象。」
> —— 我在调试第一个竞态条件 bug 时领悟的

## 本章适合谁

- 已经掌握 C 语言指针、多线程基础 (POSIX pthreads)
- 听说过「竞态条件 (race condition)」但不知道怎么解决
- 用过 `volatile` 但不知道它和 `atomic` 的区别
- 想了解无锁编程 (lock-free programming) 的初学者

## 你会学到什么

1. `stdatomic.h` 是什么 —— C11 标准的原子类型
2. `atomic_int`、`atomic_flag` 的基本用法
3. `memory_order` 内存顺序模型 (relaxed → seq_cst)
4. `volatile` 和 `atomic` 的核心区别
5. CAS (Compare-And-Swap) 无锁编程模式
6. 竞态条件的成因与原子操作修复

## 前置要求

- 已掌握 [void\* 泛型编程](../basic/void_generic.md) —— 指针与类型系统
- 理解多线程基本模式 (pthread_create/join)
- 了解 race condition 的基本概念

## 第一个例子

下面是最简短的原子操作演示。它用 `atomic_int` 替换普通 `int`, 然后两个线程同时增加同一个计数器:

```c
#include <stdio.h>
#include <stdatomic.h>

int main(void) {
    atomic_int counter = ATOMIC_VAR_INIT(0);

    /* 两个线程同时执行 atomic_fetch_add */
    /* ... pthread 代码省略 ... */

    int32_t final = atomic_load(&counter);
    printf("counter = %" PRId32 "\n", final);  /* 总是 200000 */
    return 0;
}
```

这段代码做了几件事:

- 声明了一个 `atomic_int` (原子整型), 初始值为 0
- 两个线程各自对计数器执行 `atomic_fetch_add(1)` 十万次
- 无论线程如何交替执行, 最终结果 **总是** 200000

普通 `int` 的 `++` 操作会被编译器编译成三条机器指令 (read → modify → write), 中间可以被另一个线程打断。`atomic_int` 的 `atomic_fetch_add` 是一条 CPU 原子指令, 不会被打断。

## 原理解析

### 1. 竞态条件: 先犯错 (Error-First)

先看不用原子操作的版本:

```c
int counter = 0;  /* 普通整型 */

/* 线程 A 执行: */ counter++;
/* 线程 B 执行: */ counter++;
```

编译后 (伪代码):

```
线程 A:
  R1 = load counter    ; 读到 5
  R1 = R1 + 1          ; 变成 6
  store R1 → counter   ; 写回 6

线程 B:
  R2 = load counter    ; 读到 5 (在 A 写回之前!)
  R2 = R2 + 1          ; 变成 6
  store R2 → counter   ; 写回 6 (覆盖了 A 的结果!)
```

**结果**: 两次自增, 只得到 6 (应该是 7)。这叫 **lost update**, 是最常见的竞态条件。

### 2. atomic_int —— C11 原子整型

`<stdatomic.h>` 提供了 `atomic_int` 类型:

```c
#include <stdatomic.h>

atomic_int counter = ATOMIC_VAR_INIT(0);

int32_t val  = atomic_load(&counter);       // 原子读取
atomic_store(&counter, 42);                  // 原子写入
int32_t old  = atomic_fetch_add(&counter, 1); // 原子加 1, 返回旧值
int32_t prev = atomic_exchange(&counter, 99); // 原子替换, 返回旧值
```

| 函数 | 作用 | 返回值 |
|------|------|--------|
| `atomic_load(ptr)` | 原子读取 | 当前值 |
| `atomic_store(ptr, val)` | 原子写入 | void |
| `atomic_fetch_add(ptr, n)` | 原子加 | 旧值 |
| `atomic_fetch_sub(ptr, n)` | 原子减 | 旧值 |
| `atomic_exchange(ptr, val)` | 原子替换 | 旧值 |

### 3. 线程安全的原子计数器 (修复竞态)

```c
atomic_int counter = ATOMIC_VAR_INIT(0);

static void *thread_func(void *arg) {
    for (int i = 0; i < 100000; i++) {
        atomic_fetch_add(&counter, 1);  // 原子操作!
    }
    return NULL;
}

// 启动两个线程...
pthread_create(&t1, NULL, thread_func, NULL);
pthread_create(&t2, NULL, thread_func, NULL);
// 等待...

int32_t result = atomic_load(&counter);
printf("%" PRId32 "\n", result);  /* 总是 200000 ✅ */
```

### 4. atomic_flag —— 最简单的原子类型

`atomic_flag` 只有两种状态: set (1) 和 clear (0)。它是唯一保证无锁 (lock-free) 的原子类型:

```c
atomic_flag lock = ATOMIC_FLAG_INIT;  // 初始为 clear

// 自旋锁: 循环尝试获取锁
while (atomic_flag_test_and_set(&lock)) {
    // 别人持有锁, 继续转
}
// --- 临界区 (Critical Section) ---
// 你的代码在这里安全执行
// --- 临界区结束 ---

atomic_flag_clear(&lock);  // 释放锁
```

**CPU 级实现**: x86 使用 `LOCK` 指令前缀, ARM 使用 `LDREX/STREX` 指令对。

### 5. memory_order —— 内存顺序模型

编译器为了提高性能会对指令进行**重排序 (reorder)**。`memory_order` 控制原子操作的排序约束:

```
memory_order 层级 (从弱到强):

  relaxed      仅保证原子性, 不限制排序
  consume      依赖链操作保持顺序 (极少使用)
  acquire      后续内存操作不会被重排到此操作之前
  release      前面的内存操作不会被重排到此操作之后
  acq_rel      acquire + release (用于读-改-写操作)
  seq_cst      全序一致性 (sequential consistency, 最强, 也是默认值)
```

**实际选择指南**:

```c
// 默认使用 seq_cst (安全)
atomic_fetch_add(&x, 1);  // 等价于 memory_order_seq_cst

// 高性能场景用 relaxed (需要仔细分析)
atomic_store_explicit(&x, 1, memory_order_relaxed);
atomic_load_explicit(&x, memory_order_relaxed);

// 生产者 - 消费者模式用 acquire/release
atomic_store_explicit(&data, 42, memory_order_release);  // 生产者
int d = atomic_load_explicit(&data, memory_order_acquire);  // 消费者
```

**简单规则**: 先用 `seq_cst`, 性能分析确认为瓶颈后再降级。

### 6. volatile vs atomic —— 最常被混淆的概念

| 特性 | `volatile` | `atomic` |
|------|-----------|----------|
| 阻止编译器优化 | ✅ | ✅ |
| 硬件级原子操作 | ❌ | ✅ |
| 阻止 CPU 重排序 | ❌ | ✅ (取决于 memory_order) |
| 多线程安全 | ❌ | ✅ |
| 适用场景 | 信号处理 (signal handler)、MMIO (内存映射 I/O) | 多线程共享变量 |

```c
volatile int flag = 0;   // 告诉编译器: 别优化 flag 的读写
atomic_int aflag = ATOMIC_VAR_INIT(0);  // 告诉 CPU: 这是原子操作
```

**核心区别**:
- `volatile` 只管编译器, 不管 CPU 和内存子系统
- `atomic` 同时管编译器 + CPU 硬件, 保证真正的线程安全

> 「用 `volatile` 做线程同步 —— 看起来像是答案, 实际上不是。」
> —— Herb Sutter (C++ Expert)

## 常见错误

### 错误 1: 对普通变量做并发修改

```c
/* ❌ 竞态条件 */
int shared = 0;
/* 线程 A 和 B 同时执行 */ shared++;

/* ✅ 修复: 使用 atomic */
atomic_int shared = ATOMIC_VAR_INIT(0);
atomic_fetch_add(&shared, 1);
```

### 错误 2: 用 volatile 代替 atomic

```c
/* ❌ volatile 不保证线程安全 */
volatile int counter = 0;
/* 两个线程同时执行 */ counter++;  /* 仍然有 lost update */

/* ✅ 使用 atomic */
atomic_int counter = ATOMIC_VAR_INIT(0);
atomic_fetch_add(&counter, 1);  /* 线程安全 */
```

### 错误 3: 忘记初始化 atomic 变量

```c
/* ❌ 未初始化 */
atomic_int x;  /* g 包含垃圾值 */

/* ✅ 显式初始化 */
atomic_int x = ATOMIC_VAR_INIT(0);
/* 或 */
atomic_init(&x, 42);
```

### 错误 4: CAS 循环中不更新 expected

```c
atomic_int val = ATOMIC_VAR_INIT(10);
int32_t expected = 10;

/* ❌ 只试一次就放弃 */
atomic_compare_exchange_strong(&val, &expected, 20);
/* 如果失败, expected 已更新为当前值, 不再重试 */

/* ✅ CAS 自旋模式 */
int32_t cur = atomic_load(&val);
while (!atomic_compare_exchange_weak(&val, &cur, cur + 1)) {
    /* cur 自动更新为最新值, 继续重试 */
}
```

## 动手练习

### 🟢 练习 1: 原子计数器

声明 `atomic_int counter`, 初始为 0, 执行 5 次 `atomic_fetch_add`, 每次加 10。打印最终值。

<details><summary>点击查看答案</summary>

```c
#include <stdatomic.h>

atomic_int counter = ATOMIC_VAR_INIT(0);
for (int i = 0; i < 5; i++) {
    atomic_fetch_add(&counter, 10);
}
printf("%" PRId32 "\n", (int32_t)atomic_load(&counter));
/* 输出: 50 */
```

</details>

### 🟡 练习 2: 无锁最大值更新

用 CAS 实现 `atomic_max`, 将 `atomic_int *target` 更新为 `max(*target, new_val)`。

<details><summary>点击查看答案</summary>

```c
void atomic_max(atomic_int *target, int32_t new_val) {
    int32_t cur = atomic_load(target);
    while (new_val > cur) {
        if (atomic_compare_exchange_weak(target, &cur, new_val)) {
            return;  /* 成功更新 */
        }
        /* cur 自动更新为最新值, 重试 */
    }
}
```

</details>

### 🔴 练习 3: 多线程原子求和

创建 4 个线程, 每个线程对 `atomic_int sum` 执行 10 万次 `atomic_fetch_add(1)`, 验证最终值是否为 400000。

<details><summary>点击查看答案</summary>

```c
#include <pthread.h>

static atomic_int g_sum;

static void *add_worker(void *arg) {
    (void)arg;
    for (int i = 0; i < 100000; i++) {
        atomic_fetch_add(&g_sum, 1);
    }
    return NULL;
}

int main(void) {
    atomic_init(&g_sum, 0);
    pthread_t threads[4];
    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, add_worker, NULL);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("%" PRId32 "\n", (int32_t)atomic_load(&g_sum));
    /* 输出: 400000 */
    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q: `atomic_int` 和 `int` 能隐式转换吗？**

A: 不能。你需要显式用 `atomic_load`/`atomic_store` 或编译器扩展的 `+=` 运算符来读写 `atomic_int`。直接赋值会产生编译警告 (取决于编译器)。

**Q: CAS (compare_exchange) 的 `strong` 和 `weak` 版本有什么区别？**

A: `weak` 版本允许「假失败」——即使值匹配也可能返回 false (在 ARM 等架构上更高效的实现)。`strong` 版本保证「值匹配就一定成功」。在循环中两者等价 (因为都会重试); 如果只调用一次, 用 `strong`。

**Q: `volatile` 真的不能用于多线程同步吗？**

A: 不能。`volatile` 只保证编译器不把变量存在寄存器里, 每次读写都经过内存。但现代 CPU 有**多级缓存和指令重排序**, `volatile` 不能阻止 CPU 级别的重排——只有 `atomic` (配合适当的 memory_order) 能做到这一点。

**Q: atomic 操作有性能损失吗？**

A: 有, 但很小。`seq_cst` 是最强的内存序, 会插入完整的内存屏障 (memory barrier), 性能损失较大 (~10-50 倍于普通操作)。如果你的场景只需要原子性而不关心排序, 可以用 `memory_order_relaxed`, 性能接近普通操作。

## 知识扩展 (选学)

### 无锁队列 (Lock-Free Queue)

基于 CAS 可以构建完全无需互斥锁的数据结构:

```
传统锁:      线程 A ──[获取锁]→ 操作数据 → [释放锁] → 线程 B 进入
无锁算法:    线程 A ───[CAS 尝试]──→ 成功则继续, 失败则重试
             线程 B ───[CAS 尝试]──→ 与 A 同时竞争, 只有一个成功
```

无锁数据结构的优势:
- **无阻塞**: 一个线程被 OS 调度暂停, 不会阻塞其他线程
- **无死锁**: 没有锁就没有死锁
- **高并发**: 多个线程可以同时尝试操作

### 内存屏障 (Memory Barrier)

内存屏障是一种 CPU 指令, 保证屏障前后的内存操作不会重排:

```
普通执行:  A → B → C → D  (可能被 CPU 重排序)
加屏障后:  [A → B] —barrier— [C → D]  (两组内部可重排, 组间不可)
```

`memory_order_acquire` ≈ 读屏障, `memory_order_release` ≈ 写屏障。

### C11 atomic 支持的类型

| 类型 | 别名 |
|------|------|
| `atomic_bool` | `_Atomic bool` |
| `atomic_char` | `_Atomic char` |
| `atomic_int` | `_Atomic int` |
| `atomic_long` | `_Atomic long` |
| `atomic_llong` | `_Atomic long long` |
| `atomic_ptr` | `atomic(T*)` (任意指针) |

## 小结

本章的核心要点:

- **竞态条件** (race condition) 由 read-modify-write 操作的非原子性导致
- **`atomic_int`** 是所有线程安全操作的起点, 替代 `int` + 手动锁
- **`atomic_flag`** 是最简原子类型, 适合实现自旋锁
- **`memory_order`** 从 relaxed (最弱, 无排序) 到 seq_cst (最强, 全序), 默认用 seq_cst
- **`volatile ≠ atomic`**: volatile 只管编译器, atomic 管 CPU + 编译器
- **CAS** 是无锁编程的基石, 用于实现无锁数据结构和并发算法

> 「原子操作不是银弹, 但它是并发世界中第一道安全门。学会用 atomic, 你就迈出了多线程编程最关键的一步。」

## 术语表

| 英文 | 中文 |
|------|------|
| Atomic operation | 原子操作 |
| Race condition | 竞态条件 |
| Lost update | 丢失更新 |
| Compare-And-Swap (CAS) | 比较并交换 |
| Lock-free | 无锁 |
| Spinlock | 自旋锁 |
| Memory order | 内存顺序 |
| Memory barrier | 内存屏障 |
| Sequential consistency | 全序一致性 |
| Critical section | 临界区 |
| Volatile | 易变 (阻止编译器优化) |
| Reorder | 重排序 |

## 延伸阅读

- [cppreference: stdatomic.h](https://en.cppreference.com/w/c/atomic) — C11 原子操作标准参考
- [Anthony Williams: C++ Concurrency in Action](https://www.manning.com/books/c-plus-plus-concurrency-in-action-second-edition) — 并发编程圣经 (C++, 但原理通用)
- [Maged Michael: Lock-Free Data Structures](http://www.cs.rochester.edu/u/michael/PODC-2002.html) — 无锁数据结构经典论文
- [Bartosz: Atomics and Memory Order](https://bartoszmilewski.com/2008/11/05/what-every-programmer-should-know-about-memory/) — 内存序深度解析

## 继续学习

你已经掌握了 C 语言中多线程安全的核心工具 —— 原子类型。它是并发世界的安全基石, 但 C 的高级能力远不止于此。

下一章, 我们将探索 **透明指针 (Opaque Pointers)** —— 用 `void*` 实现信息隐藏 (information hiding)、工厂模式和 RAII 风格资源管理的 C 语言惯用法。

- [上一章](./error-handling.md): 错误处理与 errno 系统
- [下一章 →](./opaque-pointers.md): 不透明指针 (opaque-pointers)
