# 线程池模式 (Thread Pool Pattern)

> "线程池像出租车调度——车在站里等着，有客单时分配一辆，送完客回站待命。"——我发现

---

## 开篇故事

你开了一个餐厅。客人点单时，如果每次都去招聘并培训一个新厨师——太慢了。更好的方式是：

1. **提前雇佣好 N 个厨师**（创建 N 个线程）
2. 客人来了，把菜谱放到出菜口（提交任务）
3. 厨师做完一个菜，回来等下一个菜谱（从队列取任务）
4. 打烊时，告诉厨师「做完手上这个就下班」（优雅关闭）

这就是**线程池**。创建线程有成本（几微秒到几十微秒），频繁创建/销毁线程会浪费资源。线程池复用线程，适合「大量短任务」的场景。

```
线程池架构:

┌───────────── 主线程 (老板) ──────────────────┐
│                                              │
│        ┌── 任务 ──┐                          │
│  submit──────────→  待办队列 (Task Queue)     │
│        └── 任务 ──┘                          │
│                                              │
│   ┌────────┐  取任务  ┌────────┐  取任务     │
│   │Worker A├─────────>│Worker B├─────────    │
│   │(线程1) │          │(线程2) │             │
│   └────────┘          └────────┘             │
└──────────────────────────────────────────────┘
```

## 本章适合谁

- 已经会手动创建/销毁线程，但觉得太麻烦
- 需要处理大量并发任务（如网络请求）
- 想了解服务器/框架「幕后是怎么管理线程的」

## 你会学到什么

1. **线程池的组成**——Worker 线程 + 任务队列 + 同步
2. **环形队列 (Ring Buffer)**——固定大小的循环任务队列
3. **任务提交**——`pool_submit(func, arg)` 入队
4. **优雅关闭**——标记 shutdown → worker 清空队列 → join 回收
5. **实际应用**——用线程池批量处理数组数据

## 前置要求

- 已掌握：mutex + condvar 的基本用法
- 已掌握：函数指针 (`void (*)(void *)`)
- 理解回调函数的概念

## 第一个例子

```c
/* 简化版线程池 */
typedef void (*TaskFunc)(void *);
typedef struct { TaskFunc func; void *arg; } Task;

/* 提交任务 */
pool_submit(pool, my_task, &data);
// 某个空闲 worker 会自动执行: my_task(&data)
```

编译：`gcc -Wall -Wextra -std=c17 -pthread -o pool_demo pool_demo.c`

## 原理解析

### 环形缓冲区 (Ring Buffer)

任务队列用**数组**实现，头和尾两个指针，满了就绕回开头：

```
队列大小 = 4:

  [task0] [task1] [task2] [task3]
    ↑head            ↑tail

head == tail 且 count == 0 → 空
head == tail 且 count == MAX → 满
```

### Worker 线程循环

```c
void *worker_loop(void *arg) {
    ThreadPool *pool = arg;
    while (1) {
        pthread_mutex_lock(&pool->mutex);
        while (count == 0 && !shutdown)
            pthread_cond_wait(&not_empty, &mutex);
        if (shutdown && count == 0) break;
        Task task = queue[head];  // 取任务
        head = (head+1) % max;    // 环形前进
        count--;
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        task.func(task.arg);  // 执行任务
    }
    return NULL;
}
```

### 优雅关闭流程

```
pool_shutdown(pool):
  1. 设置 shutdown = 1
  2. cond_broadcast 唤醒所有 worker
  3. worker: 执行完队列剩余任务
  4. worker: 检查 shutdown 标记 → 退出循环
  5. pool_shutdown: join 回收所有 worker 线程
```

## 常见错误

### ❌ 错误 1: shutdown 后还 submit

```c
pool_shutdown(pool, 4);
pool_submit(pool, task, &data);  // ❌ pool 已经关了！
// ✅ 先提交所有任务，再调用 shutdown
```

### ❌ 错误 2: 队列满时不等待

```c
if (count == max_queue) {
    // ❌ 直接丢弃任务？还是阻塞？
}
// ✅ 用 cond_wait 等 not_full 信号
```

### ❌ 错误 3: worker 执行长任务导致队列积压

```c
/* ❌ 一个任务耗时 10 秒 → 其他 worker 空闲 */
pool_submit(pool, long_task, &data);   // 10 秒
pool_submit(pool, quick_task, &data);  // 等 10 秒才被执行
// ✅ 拆分长任务，或限制队列深度做背压 (backpressure)
```

## 动手练习

### 🟢 入门：提交 3 个打印任务

创建 2 个 worker 的线程池，提交 3 个任务（每个打印自己的 id），优雅关闭。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <pthread.h>

typedef void (*TaskFunc)(void *);
typedef struct { TaskFunc func; void *arg; } Task;

static void print_task(void *arg) {
    int id = *(int *)arg;
    printf("执行任务 %d\n", id);
}

/* ... thread pool 实现 ... */
int main(void) {
    int ids[] = {1, 2, 3};
    /* pool 创建后: */
    pool_submit(pool, print_task, &ids[0]);
    pool_submit(pool, print_task, &ids[1]);
    pool_submit(pool, print_task, &ids[2]);
    pool_shutdown(pool, 2);
    return 0;
}
```

</details>

### 🟡 中级：批量平方计算

提交 10 个任务，每个计算一个数字的平方，结果存回原 struct。提交完关闭线程池，打印所有结果。

<details><summary>点击查看答案</summary>

思路：每个任务接收一个 `IntTask {id, value, result}`，任务函数里 `result = value * value`。提交 10 个后 shutdown，shutdown 保证所有任务执行完才返回。

</details>

### 🔴 挑战：实现动态线程池

当队列持续积压时，自动创建更多 worker 线程（上限 N）。当队列持续为空时，减少多余的 worker。

<details><summary>查看答案提示</summary>

思路：维护一个活跃 worker 计数和空闲 worker 计数。定时检查队列深度，超过阈值则创建新 worker；低于阈值一定时间则释放多余 worker。需要 `pthread_detach` 自动回收空闲 worker。

</details>

## 故障排查

**Q：pool_shutdown 卡住了？**

A：某个 worker 在执行的任务永远不返回。用 `sleep` 模拟超时、或者检查是否有死锁。

**Q：任务被执行了两次？**

A：环形队列的「满」和「空」条件判断有误。`head == tail` 在空和满时都成立，需要用 `count` 变量来区分。

**Q：任务丢失了？**

A：submit 时没有检查队列是否满了，直接覆盖。正确做法：等待 `not_full` 信号或返回错误码。

## 知识扩展

### 实际项目中的线程池

- **Nginx**: 多进程 + listen fd 共享
- **Redis**: 单线程事件循环 + background worker
- **gRPC**: C++ ThreadPool 实现，支持动态扩缩
- **Linux 内核**: `workqueue` 子系统，内核态线程池

### 任务类型分类

| 类型 | 特征 | 推荐线程池大小 |
|------|------|-----------------|
| CPU 密集型 | 大量计算 | CPU 核心数 |
| I/O 密集型 | 等待网络/磁盘 | CPU 核心数 × 2~4 |
| 混合型 | 计算+等待 | 根据测量调整 |

## 小结

- **线程池** = N 个 worker 线程 + 任务队列 + mutex + condvar
- **提交任务**：放入队列，cond_signal 唤醒空闲 worker
- **优雅关闭**：标记 shutdown → worker 清空队列 → join 回收
- **适用场景**：大量短任务，避免频繁创建/销毁线程
- **核心数据结构**：环形缓冲区

## 术语表

| 英文 | 中文 |
|------|------|
| Thread Pool | 线程池 |
| Worker Thread | 工作线程 |
| Task Queue | 任务队列 |
| Ring Buffer | 环形缓冲区 |
| Graceful Shutdown | 优雅关闭 |
| Backpressure | 背压（限流） |
| Dynamic Scaling | 动态扩缩 |
| CPU-bound | CPU 密集型 |
| I/O-bound | I/O 密集型 |
| Fire-and-forget | 投递即忘 |

## 延伸阅读

- [POSIX Thread Pool 模式](https://computing.llnl.gov/tutorials/pthreads/)
- [The Art of Multiprocessor Programming](https://dl.acm.org/doi/10.1145/2385452)

## 继续学习

你已经学会了如何高效复用线程。但还有一个场景：服务器同时有 1000 个客户端连接等着读数据——每个连接开一个线程？不行。下一章介绍 **I/O 多路复用**——一个线程监控所有文件描述符，哪个有数据就处理哪个。

- [上一章](./async_sync.md)：同步原语
- [下一章](./async_iomux.md)：I/O 多路复用
