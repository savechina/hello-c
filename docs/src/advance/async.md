# 异步与线程 (Async & POSIX Threads)

> "把一份工作交给一个人做——他做完一份再继续下一份。交给十个人——你需要协调他们，别让两个人同时抢同一把铲子。"——我发现

---

## 开篇故事

想象你经营一家餐厅厨房。只有一个厨师的时候，他一个人炒菜、切菜、端盘子——每件事都是**顺序**完成的。客人多了，你雇了第二个厨师。现在问题来了：

- 两个人能不能同时用**同一个灶台**？不能——需要协调。
- 你怎么告诉第二个厨师"菜炒好了，可以上菜了"？需要**信号机制**。
- 如果两个厨师同时去冰箱拿最后一块牛肉怎么办？需要**锁**（门锁，一次只能一个人进）。

这就是多线程编程的本质。一个程序里有多个「厨师」（线程），他们共享同一个厨房（内存空间），必须通过 mutex（互斥锁）、条件变量（信号）等同步工具来协调工作。

```
单线程厨房                     多线程厨房
  ┌─────────┐                   ┌─────────┐
  │ 厨师 A   │  → 切菜 → 炒菜 → │ 厨师 A   │  → 切菜 ─┐
  │ 一个人   │                   │         │          ├→ 抢灶台？← 需要互斥锁！
  │ 全包了   │                   │ 厨师 B   │  → 炒菜 ─┘
  └─────────┘                   │         │  → 端盘 → 需要条件变量通知！
                                └─────────┘
```

## 本章适合谁

- 写过单线程 C 程序，想知道"怎么让程序同时做多件事"
- 听说过「多线程」但觉得是黑魔法，怕踩坑
- 遇到过「程序有时候对、有时候错」的幽灵 bug
- 想了解操作系统调度的基本原理

## 你会学到什么

1. **什么是线程 (Thread)**——与进程的区别，共享内存的利与弊
2. **pthread_create / pthread_join**——创建和等待线程的生命周期
3. **竞态条件 (Race Condition)**——为什么多线程会导致「有时候对、有时候错」
4. **互斥锁 (Mutex)**——用 `pthread_mutex_t` 保护共享资源
5. **条件变量 (Condition Variable)**——线程间「发消息」的机制
6. **线程局部存储 (TLS)**——每个线程的「私人储物柜」
7. **平台检测**——`#ifdef PTHREAD` 在支持/不支持 pthread 的平台上优雅降级

## 前置要求

- 已掌握：[指针](../basic/pointers.md)、[函数指针](../basic/function_pointers.md)、[内存管理](../basic/memory_mgmt.md)
- 已掌握：[结构体](../basic/structs.md) (`struct`) 和[类型别名](../basic/typedef.md) (`typedef`)
- 了解操作系统「进程」的基本概念（程序的一次执行）

## 第一个例子：创建线程

这是最简短的多线程程序——创建两个线程，各自打印自己的 ID：

```c
#include <stdio.h>
#include <pthread.h>

void *worker(void *arg) {
    long id = (long)arg;
    printf("线程 %ld 报告：我在运行！\n", id);
    return NULL;
}

int main(void) {
    pthread_t t1, t2;

    /* 创建两个线程，各自执行 worker() */
    pthread_create(&t1, NULL, worker, (void *)1L);
    pthread_create(&t2, NULL, worker, (void *)2L);

    /* 等待两个线程完成 */
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("所有线程执行完毕。\n");
    return 0;
}
```

编译时需要加 `-pthread` 标志：
```bash
gcc -Wall -Wextra -std=c17 -pthread -o thread_demo thread_demo.c
./thread_demo
```

输出（顺序可能不同）：
```
线程 2 报告：我在运行！
线程 1 报告：我在运行！
所有线程执行完毕。
```

> **注意**：输出顺序不固定！线程调度由操作系统决定，你可能看到 `1 2` 也可能看到 `2 1`。这就是并发的本质。

## 原理解析

### 1. 线程 vs 进程

| 特性 | 进程 (Process) | 线程 (Thread) |
|------|----------------|---------------|
| 内存空间 | 独立的地址空间 | **共享**同一进程的内存 |
| 创建开销 | 重（`fork` 复制整个地址空间） | 轻（只创建独立的栈和寄存器） |
| 通信方式 | IPC（管道、socket、共享内存） | **直接读写共享变量** |
| 隔离性 | 强（一个崩溃不影响其他） | 弱（一个崩溃 = 整个进程崩溃） |
| 类比 | 两个独立厨房，各自有食材 | 一个厨房里的多个厨师，共享灶台 |

```
进程内存隔离                        线程共享内存
┌────────────┐  ┌────────────┐     ┌──────────────────────────────┐
│ 进程 A      │  │ 进程 B      │     │           进程               │
│            │  │            │     │  ┌────────┬────────────────┐  │
│ 代码段      │  │ 代码段      │     │  │ 代码段 │ 代码段(共享)    │  │
│ 数据段      │  │ 数据段      │     │  ├────────┼────────────────┤  │
│ 堆(独立)    │  │ 堆(独立)    │     │  │ 堆(共享)              │  │
│ 栈(独立)    │  │ 栈(独立)    │     │  ├────────┼────────────────┤  │
└────────────┘  └────────────┘     │  │线程A栈 │  线程B栈(独立)  │  │
                                  └──┴────────┴─────────────────┴──┘
IPC 管道 → 跨进程通信很慢              共享变量 → 跨线程通信很快但有风险
```

### 2. pthread_create 详解

```c
int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,  /* 通常传 NULL 用默认属性 */
                   void *(*start_routine)(void *),
                   void *arg);
```

| 参数 | 说明 |
|------|------|
| `thread` | 输出参数，线程创建后写入线程 ID |
| `attr` | 线程属性（栈大小、调度策略等），通常 `NULL` |
| `start_routine` | 线程入口函数，签名必须是 `void *(*func)(void *)` |
| `arg` | 传给入口函数的参数，`void *` 类型可传任意数据 |

**关键点**：线程函数签名 `void *(*)(void *)` 是 POSIX 的约定。你可以传入 `int *`、`struct *`，但需要强制类型转换。

### 3. pthread_join 详解

```c
int pthread_join(pthread_t thread, void **retval);
```

- **作用**：阻塞当前线程，直到 `thread` 线程执行完毕
- **类比**：你在等另一位厨师做完菜，等完了才能继续下一步
- `retval`：如果非 NULL，会收到线程函数的返回值

**不要漏掉 `pthread_join`**！如果主线程在子线程完成前就 `exit()`，整个进程会终止，子线程被迫死亡。

## 典型模式：线程创建与等待

下面是一个完整的模式——创建 N 个线程，每个线程处理一段数据：

```c
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

typedef struct {
    int32_t start;
    int32_t end;
    int32_t thread_id;
} Task;

static void *sum_worker(void *arg) {
    Task *t = (Task *)arg;
    int64_t sum = 0;
    for (int32_t i = t->start; i <= t->end; i++) {
        sum += i;
    }
    printf("线程 %d: 计算 sum(%d..%d) = %" PRId64 "\n",
           t->thread_id, t->start, t->end, sum);
    return NULL;
}

int main(void) {
    pthread_t threads[4];
    Task tasks[4];
    int32_t range = 100;
    int32_t chunk = range / 4;

    for (int32_t i = 0; i < 4; i++) {
        tasks[i].start = i * chunk + 1;
        tasks[i].end = (i == 3) ? range : (i + 1) * chunk;
        tasks[i].thread_id = i;
        pthread_create(&threads[i], NULL, sum_worker, &tasks[i]);
    }

    /* 等待所有线程 */
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("全部计算完成！\n");
    return 0;
}
```

**注意数据传递方式**：我们用了**栈上的数组 `tasks[4]`**，每个线程拿到不同元素的指针。这是安全的，因为 `main()` 在 `pthread_join` 之后才返回，保证了栈上的 `tasks` 在整个线程生命周期内有效。

## 竞态条件 (Race Condition) — 错误驱动学习

**这是我学多线程踩的第一个大坑。** 来看一个「看起来没问题」的程序：

```c
/* ❌ 有竞态条件的代码 — 结果不确定 */
#include <stdio.h>
#include <pthread.h>

int64_t counter = 0;       /* 全局共享变量 */

static void *increment(void *arg) {
    long n = *(long *)arg;
    for (long i = 0; i < n; i++) {
        counter++;         /* ❌ 问题！counter++ 不是原子的！ */
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    long n = 100000;
    pthread_create(&t1, NULL, increment, &n);
    pthread_create(&t2, NULL, increment, &n);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("counter = %" PRId64 " (期望值: %ld)\n", counter, n * 2);
    return 0;
}
```

**你可能期望**输出 `counter = 200000`。但你跑十次有八次答案不对。

### 为什么？

`counter++` 在 CPU 层面**不是单条指令**，而是分成三步：

```
1. LOAD  → 从内存读 counter 到寄存器
2. ADD   → 寄存器 +1
3. STORE → 从寄存器写回内存
```

如果两个线程**交错执行**：

```
线程 A: LOAD(counter=0)     线程 B: LOAD(counter=0)    ← 两个都读到 0
线程 A: ADD → 1             线程 B: ADD → 1            ← 各加各的
线程 A: STORE(counter=1)    线程 B: STORE(counter=1)   ← 覆盖！结果 =1，不是 2！
```

两次 `counter++`，结果却只加了 1。这就是**竞态条件**——结果取决于线程调度的时序。

## 互斥锁 (Mutex) — 修复竞态条件

互斥锁是保护共享数据的第一个武器。核心思想：**一次只有一个线程能进入「临界区」(Critical Section)**。

```c
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

int64_t counter = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  /* 初始化互斥锁 */

static void *increment_safe(void *arg) {
    long n = *(long *)arg;
    for (long i = 0; i < n; i++) {
        pthread_mutex_lock(&mutex);      /* 🔒 加锁 — 如果别人锁了，等！ */
        counter++;                        /* ← 临界区：安全了 */
        pthread_mutex_unlock(&mutex);     /* 🔓 解锁 — 让别人进来 */
    }
    return NULL;
}
```

运行结果：`counter = 200000` ✅ **每次都正确。**

### Mutex 工作流程

```
线程 A                         线程 B
  │                              │
  ├─ 尝试 lock()                │
  ├─ ✅ 获得锁                   │
  ├─ 修改 counter++             │
  │                              ├─ 尝试 lock()
  │                              ├─ ❌ 锁已被占用 → 阻塞等待
  ├─ unlock()                   │
  ├─ 🔓 释放锁                  │
  │                              ├─ ✅ 获得锁 (现在轮到 B)
  │                              ├─ 修改 counter++
  │                              ├─ unlock()
```

### 关键规则

1. **每个共享数据对应一个 mutex**——不要一个锁保护所有东西（会降低并发）
2. **lock / unlock 必须成对出现**——忘了解锁 = 死锁 (Deadlock)
3. **临界区越小越好**——只锁「读写共享数据」的那几行
4. **不要在同一线程上对同一个 mutex 调两次 lock**——死锁！

## 条件变量 (Condition Variable) — 线程间的消息

互斥锁解决「谁能访问」，条件变量解决「什么时候该动手」。

**场景**：一个厨师切菜（生产者），另一个厨师炒菜（消费者）。炒菜的不能一直盯着案板看「切好了没」——得有个机制告诉炒菜的：「菜切好了，可以炒了！」

```c
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
int data_ready = 0;          /* 共享状态标志 */

/* 消费者线程：等数据 */
static void *consumer(void *arg) {
    pthread_mutex_lock(&mutex);
    while (data_ready == 0) {                         /* ✅ 必须用 while 循环！ */
        pthread_cond_wait(&cond, &mutex);             /* 🔓 自动释放锁 + 等待 */
    }                                                 /* 被唤醒后自动重新加锁 */
    printf("消费者: 数据已收到！\n");
    data_ready = 0;                                   /* 消费完毕 */
    pthread_mutex_unlock(&mutex);
    return NULL;
}

/* 生产者线程：生产数据 */
static void *producer(void *arg) {
    pthread_mutex_lock(&mutex);
    data_ready = 1;                                    /* 数据就绪 */
    pthread_cond_signal(&cond);                        /* 📢 通知等待的消费者 */
    pthread_mutex_unlock(&mutex);
    return NULL;
}
```

### `pthread_cond_wait` 的三秒理解

这一步看起来简单但其实很关键——调用 `cond_wait` 时，**原子地**做了两件事：

1. **释放 mutex**（让其他线程可以修改状态）
2. **把当前线程挂起**（进入睡眠，不占 CPU）

当另一个线程调用 `cond_signal` 时：

1. **唤醒等待的线程**
2. **重新获取 mutex**（在 `cond_wait` 返回前）

```
消费者调用 cond_wait():
  ┌──────────────────────────────────┐
  │ 1. 释放 mutex   ← 允许生产者加锁   │
  │ 2. 线程进入休眠 ← 不浪费 CPU       │
  │    ... 等待 ...                  │
  │ 3. 被 signal 唤醒                │
  │ 4. 重新获取 mutex ← 安全返回       │
  └──────────────────────────────────┘
```

> **为什么 while 循环而不是 if？** 因为可能存在「虚假唤醒」(Spurious Wakeup)——线程被唤醒但条件实际上没满足。`while` 循环确保条件真的满足才继续。

## 线程安全的数据传递技巧

### 传值还是传址？

```c
/* ❌ 危险：传循环变量的地址 */
for (int i = 0; i < 4; i++) {
    pthread_create(&threads[i], NULL, worker, &i);  /* ❌ i 会变化！ */
}

/* ✅ 安全：每个线程不同的数据 */
for (int i = 0; i < 4; i++) {
    pthread_create(&threads[i], NULL, worker, &tasks[i]);  /* ✅ */
}

/* ✅ 安全：传整数（利用指针值本身） */
for (int i = 0; i < 4; i++) {
    pthread_create(&threads[i], NULL, worker, (void *)(intptr_t)i);  /* ✅ */
}
```

**陷阱**：在循环里传 `&i`——所有线程拿到的都是同一个地址，`i` 的值在循环结束时已经变成了终值，所有线程看到的都是同一个数字。

## 线程局部存储 (Thread-Local Storage, TLS)

如果你需要「每个线程有一份独立的副本」，用 `__thread` 关键字：

```c
#include <pthread.h>
#include <stdint.h>

static __thread int32_t thread_counter = 0;   /* 每个线程一份 */

static void *tls_worker(void *arg) {
    long id = (long)arg;
    for (int i = 0; i < 5; i++) {
        thread_counter++;                     /* 只影响当前线程的副本 */
    }
    printf("线程 %ld: thread_counter = %" PRId32 "\n", id, thread_counter);
    return NULL;
}
```

每个线程的 `thread_counter` 互不相干，不需要 mutex 保护。

```
TLS 内存布局:

主线程:  thread_counter = 5
线程 A:  thread_counter = 5    ← 独立的副本
线程 B:  thread_counter = 5    ← 独立的副本
```

## 常见错误

### 错误 1：忘记 `pthread_join`

```c
/* ❌ 主线程不等子线程就返回了 */
int main(void) {
    pthread_t t;
    pthread_create(&t, NULL, worker, NULL);
    /* 漏了 pthread_join(t, NULL) */
    return 0;   /* ← 进程退出，子线程被 kill */
}
```

**修复**：始终 `pthread_join` 你创建的所有线程，或者用 `pthread_detach` 告诉系统「这个线程自己会回收」。

### 错误 2：互斥锁忘记 unlock

```c
/* ❌ 忘了解锁 — 其他线程永远阻塞 */
pthread_mutex_lock(&mutex);
counter++;
/* 忘了 pthread_mutex_unlock(&mutex); */
```

**修复**：成对使用 lock/unlock，或者用 `goto` 做错误清理（C 的惯用模式）。

### 错误 3：用 `if` 而不是 `while` 检查条件变量

```c
/* ❌ 可能被虚假唤醒骗过 */
pthread_cond_wait(&cond, &mutex);
if (data_ready == 0) { /* 不够安全 */ }

/* ✅ 始终用 while */
while (data_ready == 0) {
    pthread_cond_wait(&cond, &mutex);
}
```

### 错误 4：竞态条件（条件变量 + 标志没有用 mutex 保护）

```c
/* ❌ 没有 mutex 保护共享标志 */
while (!data_ready) {
    /* busy wait — 浪费 CPU! */
}

/* ✅ 需要 mutex + condition variable 配合 */
```

### 错误 5：在循环内传同一个地址

```c
/* ❌ 所有线程拿到同一个 i 的地址 */
for (int i = 0; i < N; i++) {
    pthread_create(&t, NULL, worker, &i);
} pthread_join(t, NULL);
```

**修复**：每个线程一个独立数据（数组、结构体）或传值 `(void *)(intptr_t)i`。

## 动手练习

### 🟢 入门：打印线程 ID

创建 3 个线程，每个线程打印自己的 ID（通过 `arg` 传入），然后 `join` 回收所有线程。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

static void *print_id(void *arg) {
    int32_t id = *(int32_t *)arg;
    printf("我是线程 %d\n", id);
    return NULL;
}

int main(void) {
    pthread_t t[3];
    int32_t ids[3] = {1, 2, 3};
    for (int i = 0; i < 3; i++) {
        pthread_create(&t[i], NULL, print_id, &ids[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(t[i], NULL);
    }
    return 0;
}
```

</details>

### 🟡 中级：线程安全计数器

用 mutex 保护一个全局计数器，3 个线程各加 100,000 次，最终打印结果（应该是 300,000）。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>

static int64_t counter = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void *increment(void *arg) {
    long n = *(long *)arg;
    for (long i = 0; i < n; i++) {
        pthread_mutex_lock(&mtx);
        counter++;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

int main(void) {
    pthread_t t[3];
    long n = 100000;
    for (int i = 0; i < 3; i++) {
        pthread_create(&t[i], NULL, increment, &n);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(t[i], NULL);
    }
    printf("counter = %" PRId64 " (期望: 300000)\n", counter);
    return 0;
}
```

</details>

### 🔴 挑战：生产者-消费者

用一个条件变量实现生产者-消费者模式：生产者每秒产生一个数字放入缓冲区，消费者消费它。3 次后停止。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

static int32_t buffer = 0;
static int ready = 0;
static int done = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static void *producer(void *arg) {
    (void)arg;
    for (int i = 1; i <= 3; i++) {
        sleep(1);  /* 模拟生产耗时 */
        pthread_mutex_lock(&mtx);
        buffer = i;
        ready = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mtx);
    }
    pthread_mutex_lock(&mtx);
    done = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
    return NULL;
}

static void *consumer(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&mtx);
        while (!ready && !done) {
            pthread_cond_wait(&cond, &mtx);
        }
        if (done && !ready) {
            pthread_mutex_unlock(&mtx);
            break;
        }
        printf("消费者收到: %d\n", buffer);
        ready = 0;
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}
```

</details>

## 故障排查 (FAQ)

**Q：线程和协程 (Coroutine) 有什么区别？**

A：线程由操作系统调度（抢占式），协程由用户代码调度（协作式）。C 标准库没有协程，但可以通过 `setjmp/longjmp` 或 ucontext 库模拟。

**Q：什么时候用多线程而不是多进程？**

A：需要频繁共享大量数据时用线程；需要强隔离时用进程。线程通信快（直接读写内存），但一个线程崩溃 = 整个进程挂。

**Q：`pthread_mutex_lock` 失败会怎样？**

A：正常情况下不会失败。但你应该检查返回值——可能因为死锁检测（`EDEADLK`）或不合法参数（`EINVAL`）而失败。

**Q：我的程序编译报错 `undefined reference to pthread_create` ？**

A：编译时没有加 `-pthread` 标志：
```bash
gcc -Wall -Wextra -std=c17 -pthread -o thread_demo thread_demo.c
```

**Q：多个 mutex 会死锁吗？**

A：会！如果线程 A 锁住了 M1 再等 M2，线程 B 锁住了 M2 再等 M1——互相等待，谁也动不了。规则：**所有线程以相同顺序获取 mutex**。

## 知识扩展 (选学)

### pthread_detach — 自动回收

```c
pthread_t t;
pthread_create(&t, NULL, worker, NULL);
pthread_detach(t);  /* 不要 pthread_join — 线程结束后自动回收 */
```

适用场景：「创建并忘记」(Fire-and-forget) 的后台任务。

### Read-Write Lock — 多读单写

```c
pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

pthread_rwlock_rdlock(&rwlock);   /* 多个线程可同时读 */
/* ... 读共享数据 ... */
pthread_rwlock_unlock(&rwlock);

pthread_rwlock_wrlock(&rwlock);   /* 只有 1 个线程能写，且写时不能读 */
/* ... 写共享数据 ... */
pthread_rwlock_unlock(&rwlock);
```

适合「读多写少」的场景。

### 原子操作 (Atomic) — 不用锁的计数

```c
#include <stdatomic.h>

atomic_int64_t counter = 0;
atomic_fetch_add(&counter, 1);    /* 原子 +1，不需要 mutex */
```

适合简单的计数器场景，比 mutex 更高效。

## 小结

- **线程**是轻量级的并发单元，共享进程的内存空间
- **pthread_create** 创建线程，**pthread_join** 等待线程结束
- **竞态条件**——多个线程同时修改共享数据时，结果取决于调度顺序
- **互斥锁 (Mutex)**——一次只允许一个线程进入临界区，解决竞态条件
- **条件变量**——线程间「我完成了/数据就绪了」的通知机制
- **TLS (Thread-Local Storage)**——每个线程独立的数据副本，不需要同步
- **平台检测**——用 `#ifdef PTHREAD` 实现优雅降级

## 术语表

| 英文 | 中文 |
|------|------|
| Thread | 线程 |
| Process | 进程 |
| pthread | POSIX Threads |
| Mutex (Mutual Exclusion) | 互斥锁 |
| Race Condition | 竞态条件 |
| Critical Section | 临界区 |
| Deadlock | 死锁 |
| Condition Variable | 条件变量 |
| Spurious Wakeup | 虚假唤醒 |
| Thread-Local Storage (TLS) | 线程局部存储 |
| pthread_create | 创建线程 |
| pthread_join | 等待线程结束 |
| pthread_detach | 分离线程（自动回收） |
| Semaphore | 信号量 |
| Atomic Operation | 原子操作 |

## 延伸阅读

- [POSIX Threads 官方文档](https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html) — 标准规范
- [pthread(7) man page](https://man7.org/linux/man-pages/man7/pthreads.7.html) — Linux 手册
- [The Little Book of Semaphores](https://greenteapress.com/wp-content/uploads/2011/12/LittleBookOfSemaphores.pdf) — 并发性经典教材

## 继续学习

本章你理解了多线程的核心概念——线程、竞态、互斥锁和条件变量。下一步，我们将学习如何用条件变量和信号量实现更复杂的同步模式，以及如何用原子操作替代简单的 mutex。

- [上一章](./tools.md)：工具链
- [下一章](./system.md)：系统调用
