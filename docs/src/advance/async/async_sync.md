# 同步原语 (Synchronization — Mutex, Condvar, Atomic)

> "同步原语像十字路口的红绿灯——没有它，线程就会撞车；有了它，即使交通拥挤也不会出事故。"——我发现

---

## 开篇故事

两个厨师同时冲向冰箱拿最后一块牛肉。如果没人协调，两个人都会伸手——结果可能是：

1. 两个人各抢到一半（**数据损坏**）
2. 一个人抢到了，另一个人一无所获（**不可预测的结果**）

在编程里，这种「两个人同时抢同一份数据」的情况叫**竞态条件 (Race Condition)**。我们需要红绿灯：**互斥锁 (Mutex)** 一次只允许一个人进冰箱，**条件变量 (Condvar)** 告诉另一个人"牛肉准备好了，你可以拿了"。

```
没有同步:                        有同步:
线程 A ─┐   抢 counter++        线程 A ───🔒lock───counter++───🔓unlock───
线程 B ─┘   抢 counter++        线程 B ───⏳等待...🔒lock───counter++───🔓
结果: 只加了 1                    结果: counter = 2 ✅
```

## 本章适合谁

- 已经会创建线程，但跑程序发现「有时候对、有时候错」
- 听说过「线程安全」但不知道具体怎么保证
- 想在面试中解释「什么是竞态条件」

## 你会学到什么

1. **竞态条件 (Race Condition)**——为什么 `counter++` 会少加
2. **mutex (互斥锁)**——一次只允许一个线程进入「临界区」
3. **条件变量 (Condvar)**——线程间「我完成了/你开始」的通知
4. **C11 atomic**——不需要锁的简单计数器
5. **生产 vs 消费 (Producer-Consumer)**——条件变量的经典模式

## 前置要求

- 已掌握：线程创建与 join
- 已掌握：`struct`、`void *` 传参
- 理解「共享变量」的概念

## 第一个例子

```c
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>

static int64_t counter = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void *inc(void *arg) {
    long n = *(long *)arg;
    for (long i = 0; i < n; i++) {
        pthread_mutex_lock(&mtx);      /* 加锁 */
        counter++;                      /* ← 只有 1 个线程能到这里 */
        pthread_mutex_unlock(&mtx);    /* 解锁 */
    }
    return NULL;
}

int main(void) {
    long n = 100000;
    pthread_t t[2];
    pthread_create(&t[0], NULL, inc, &n);
    pthread_create(&t[1], NULL, inc, &n);
    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);
    printf("counter = %" PRId64 " (期望: %ld)\n", counter, n * 2);
    return 0;
}
```

编译：`gcc -Wall -Wextra -std=c17 -pthread -o sync demo.c`
输出：`counter = 200000 (期望: 200000)` ✅

## 原理解析

### 竞态条件：counter++ 的真相

`counter++` 看起来是一行代码，在 CPU 层面分成三步：

```
线程 A: LOAD(r1, counter=5)    线程 B: LOAD(r2, counter=5)   ← 两个都读到 5
线程 A: ADD(r1, r1, 1)         线程 B: ADD(r2, r2, 1)        ← 各加各的，结果都是 6
线程 A: STORE(counter, r1=6)   线程 B: STORE(counter, r2=6)  ← 都写 6！少了 1
```

两次 self-increment，结果只加了 1。这就是**竞态条件**——结果取决于线程调度时机。

### Mutex 工作流程

```
线程 A                         线程 B
  │                              │
  ├─ lock()                     │
  ├─ 🔒 获得锁                   │
  ├─ counter++                  │
  ├─ unlock()                   │
  ├─ 🔓 释放锁                  │
  │                              ├─ lock()
  │                              ├─ 🔒 获得锁 (轮到 B 了)
  │                              ├─ counter++
  │                              ├─ unlock()
```

### Condition Variable 工作流程

```
生产者线程                      消费者线程
  │                               │
  ├─ lock()                      │
  ├─ buffer = 数据               │
  ├─ ready = 1                   │
  ├─ signal(cond)                │
  ├─ unlock()                    │
  │                               ├─ lock()
  │                               ├─ cond_wait()
  │                               │   ├─ 释放锁 + 休眠
  │                               │   └─ (被 signal 唤醒)
  │                               ├─ 重新获得锁
  │                               ├─ 读取 buffer
  │                               ├─ unlock()
```

### C11 Atomic vs Mutex

| 特性 | Mutex | C11 Atomic |
|------|-------|------------|
| 适用场景 | 保护多步操作 | 保护单步操作 |
| 开销 | 较高（系统调用） | 较低（CPU 指令） |
| 用法 | lock → 操作 → unlock | `atomic_fetch_add()` |
| 限制 | 什么都可以保护 | 只保护特定类型 |

## 常见错误

### ❌ 错误 1: 临界区太大

```c
/* ❌ 锁范围太宽 —— 降低了并发度 */
pthread_mutex_lock(&mtx);
process_large_dataset();  /* ← 这个耗时的操作不需要锁 */
counter++;                /* ← 实际需要锁的只有这一行 */
pthread_mutex_unlock(&mtx);

/* ✅ 只锁需要的部分 */
process_large_dataset();
pthread_mutex_lock(&mtx);
counter++;
pthread_mutex_unlock(&mtx);
```

### ❌ 错误 2: 忘记 unlock

```c
pthread_mutex_lock(&mtx);
counter++;
// 忘记 pthread_mutex_unlock(&mtx);  ← 其他线程永远阻塞
```

### ❌ 错误 3: while vs if (条件变量)

```c
/* ❌ 可能被虚假唤醒骗过 */
pthread_cond_wait(&cond, &mtx);
if (!ready) { /* 不够安全 */ }

/* ✅ 必须用 while 循环 */
while (!ready) {
    pthread_cond_wait(&cond, &mtx);
}
```

### ❌ 错误 4: 用 busy-wait 代替 condvar

```c
/* ❌ 浪费 CPU —— 不停地检查 */
while (!ready) { }

/* ✅ 用条件变量让线程休眠 */
while (!ready) {
    pthread_cond_wait(&cond, &mtx);
}
```

## 动手练习

### 🟢 入门：mutex 保护计数器

3 个线程各对全局计数器加 100,000 次，用 mutex 保护，最终结果应该是 300,000。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>

static int64_t counter = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void *inc(void *arg) {
    long n = *(long *)arg;
    for (long i = 0; i < n; i++) {
        pthread_mutex_lock(&mtx);
        counter++;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

int main(void) {
    long n = 100000;
    pthread_t t[3];
    for (int i = 0; i < 3; i++) pthread_create(&t[i], NULL, inc, &n);
    for (int i = 0; i < 3; i++) pthread_join(t[i], NULL);
    printf("counter = %" PRId64 " (期望: 300000)\n", counter);
    return 0;
}
```

</details>

### 🟡 中级：生产-消费者 (Condvar)

用条件变量实现生产者-消费者：生产者产生 3 个数字放入缓冲区，消费者消费并打印。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <pthread.h>

static int buffer, ready, done;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static void *producer(void *arg) {
    (void)arg;
    for (int i = 1; i <= 3; i++) {
        pthread_mutex_lock(&mtx);
        buffer = i; ready = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mtx);
    }
    pthread_mutex_lock(&mtx); done = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
    return NULL;
}

static void *consumer(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&mtx);
        while (!ready && !done)
            pthread_cond_wait(&cond, &mtx);
        if (done && !ready) { pthread_mutex_unlock(&mtx); break; }
        printf("received: %d\n", buffer);
        ready = 0;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}
```

</details>

### 🔴 挑战：多个互斥锁避免死锁

两个线程各自需要锁 A 和 B 才能完成任务。如果线程 1 锁 A 等 B，线程 2 锁 B 等 A → **死锁**。修复方案：所有线程**以相同顺序**获取锁。

<details><summary>查看答案提示</summary>

```c
/* ❌ 死锁 */
// 线程 1: lock(A); lock(B);
// 线程 2: lock(B); lock(A);

/* ✅ 同顺序 */
// 线程 1: lock(A); lock(B);
// 线程 2: lock(A); lock(B);  /* 也先锁 A */
```

</details>

## 故障排查

**Q：死锁 (Deadlock) 怎么排查？**

A：两个 mutex，以不同顺序获取 → 互相等待。规则：**所有线程以相同顺序获取 mutex**。

**Q：atomic 比 mutex 快多少？**

对于简单计数器，atomic 通常快 5-10 倍。但一旦操作超过「读取-修改-写入」三步，就需要 mutex。

**Q：条件变量被唤醒了但数据不对？**

A：检查是否用了 `while` 而不是 `if` 来检查条件。虚假唤醒是 POSIX 的已知行为。

## 知识扩展

### Read-Write Lock (多读单写)

```c
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_rdlock(&rwlock);   /* 多个读者可以同时读 */
/* ... 读操作 ... */
pthread_rwlock_unlock(&rwlock);

pthread_rwlock_wrlock(&rwlock);   /* 只能一个作者写，且写时不能读 */
/* ... 写操作 ... */
pthread_rwlock_unlock(&rwlock);
```

### Spin Lock (自旋锁)

```c
pthread_spinlock_t spin;
pthread_spin_init(&spin, 0);
pthread_spin_lock(&spin);
/* 临界区 */
pthread_spin_unlock(&spin);
```

适合临界区极短的场景（CPU 空转等待，比 sleep 更快）。

## 小结

- **竞态条件**——`counter++` 不是原子的，多线程会丢失更新
- **Mutex**——一次只允许一个线程进入临界区
- **Condvar**——线程间「数据就绪/我完成了」的通知机制
- **Atomic**——简单计数器用 C11 atomic 更高效
- **生产者/消费者**——condvar 最经典的模式

## 术语表

| 英文 | 中文 |
|------|------|
| Race Condition | 竞态条件 |
| Mutex | 互斥锁 |
| Critical Section | 临界区 |
| Condition Variable | 条件变量 |
| Spurious Wakeup | 虚假唤醒 |
| Deadlock | 死锁 |
| Producer-Consumer | 生产者-消费者模式 |
| Atomic Operation | 原子操作 |
| Memory Order | 内存序 |
| Busy-wait | 忙等待 |

## 延伸阅读

- [POSIX Mutex 文档](https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_mutex_lock.html)
- [C11 Atomics](https://en.cppreference.com/w/c/atomic)
- [The Little Book of Semaphores](https://greenteapress.com/wp-content/uploads/2011/12/LittleBookOfSemaphores.pdf)

## 继续学习

你已经掌握了同步的核心——让多个线程安全地共享数据。但每次都手动开闭锁太麻烦了，有没有更好的方式？下一章介绍**线程池**——创建好一组线程，任务排队等执行，自动回收。

- [上一章](./async_thread.md)：线程创建与生命周期
- [下一章](./async_pool.md)：线程池模式
