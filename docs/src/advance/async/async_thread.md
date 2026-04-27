# 线程创建与生命周期 (Thread Creation & Lifecycle)

> "线程就像餐馆里的新厨师——老板叫一声'开火'，新厨师就开始独立工作，但他做的菜你得到时候去验收。"——我发现

---

## 开篇故事

想象你经营一家餐厅厨房。只有一个厨师的时候，他一个人炒菜、切菜、端盘子——每件事都是**顺序**完成的。客人多了，你雇了第二个厨师。现在问题来了：

- 两个厨师能不能同时用同一个灶台？需要协调。
- 你怎么告诉第二个厨师"菜炒好了，可以上菜了"？需要**信号机制**。
- 如果两个厨师同时去冰箱拿最后一块牛肉怎么办？需要**锁**。

这就是多线程编程的第一课：**如何请厨师（创建线程）以及厨师做完后怎么验收（等待线程）**。

```
主线程（老板）                    子线程（新厨师）
  │                                │
  ├─ "你去切菜！" ────create───────>│
  │                                ├─ 开始工作...
  ├─ 继续处理其他事情              │
  │                                ├─ 切完了！
  ├─ "你做完没？我等你"───join─────┤
  │<──────── 厨师回来了 ───────────┤
```

## 本章适合谁

- 学过单线程 C，想知道"怎么让程序同时做多件事"
- 听说过「多线程」但不知道从哪里开始
- 面试被问过"线程是怎么创建的"
- 需要理解 C 标准库之外的多线程实现

## 你会学到什么

1. **什么是线程**——与进程的区别，共享内存的利与弊
2. **pthread_create**——创建线程的 4 个参数
3. **pthread_join**——等待线程结束、回收资源
4. **void\* 数据传递**——向线程传参的正确姿势
5. **pthread_exit + 返回值**——线程如何"交作业"
6. **pthread_detach**——"不用等，做完自己走"模式
7. **生命周期管理**——创建 → 运行 → 回收的完整流程

## 前置要求

- 指针基础（尤其是 `void *` 的强制类型转换）
- `struct` 自定义类型
- 函数指针的基本概念

## 第一个例子

```c
#include <stdio.h>
#include <pthread.h>

void *worker(void *arg) {
    int id = *(int *)arg;
    printf("线程 %d: 我在运行！\n", id);
    return NULL;
}

int main(void) {
    pthread_t t;
    int id = 42;
    pthread_create(&t, NULL, worker, &id);
    pthread_join(t, NULL);
    printf("线程执行完毕。\n");
    return 0;
}
```

编译时加 `-pthread`：
```bash
gcc -Wall -Wextra -std=c17 -pthread -o thread_demo thread_demo.c
./thread_demo
```

输出：
```
线程 42: 我在运行！
线程执行完毕。
```

## 原理解析

### pthread_create 详解

```c
int pthread_create(pthread_t *thread,
                   const pthread_attr_t *attr,
                   void *(*start_routine)(void *),
                   void *arg);
```

| 参数 | 含义 | 常用值 |
|------|------|--------|
| `thread` | 输出：线程 ID | `&t`（局部变量） |
| `attr` | 线程属性（栈大小等） | `NULL`（默认） |
| `start_routine` | 线程入口函数 | `worker` |
| `arg` | 传给入口的参数 | `&data` 或 `(void*)42` |

**为什么函数签名必须是 `void *(*)(void *)`？**——因为 POSIX 需要统一接口，什么类型都能塞进来。你传 `int *`、`struct *` 都行，但在函数内部要转回来。

### pthread_join 详解

```c
int pthread_join(pthread_t thread, void **retval);
```

- **作用**：阻塞调用者，直到 `thread` 结束
- **类比**：老板在门口等厨师做完菜才敢下班
- `retval`：如果非 NULL，收到线程的返回值（`pthread_exit()` 的参数）

**不要漏掉 pthread_join！** 主线程在子线程完成前就 `return`，整个进程终止，子线程被迫死亡。

### pthread_detach — Fire-and-forget

```c
pthread_detach(t);
// ❌ 之后不能调用 pthread_join(t, NULL)
```

- 告诉系统：这个线程结束后自动回收，不需要别人等它
- 适用场景：后台日志、心跳检测、"创建并忘记"的任务

### void\* 传递数据的三种姿势

```c
/* ✅ 姿势 1: 传数组元素的地址（栈安全） */
int32_t ids[3] = {0, 1, 2};
for (int i = 0; i < 3; i++) {
    pthread_create(&t[i], NULL, worker, &ids[i]);
}
// main 会在 join 之后才返回，ids 数组生命周期足够

/* ✅ 姿势 2: 传值（不传地址，直接传整数） */
for (int i = 0; i < 3; i++) {
    pthread_create(&t[i], NULL, worker, (void *)(intptr_t)i);
}
// 在线程内部: int id = (intptr_t)arg;

/* ❌ 危险: 传循环变量的地址 */
for (int i = 0; i < 3; i++) {
    pthread_create(&t[i], NULL, worker, &i);  // ❌ 所有线程拿到同一个地址！
}
```

## 常见错误

### ❌ 错误 1: 忘记 pthread_join

```c
int main(void) {
    pthread_t t;
    pthread_create(&t, NULL, worker, NULL);
    return 0;   // ← 进程退出，子线程被 kill
    // ✅ 修复: 加 pthread_join(t, NULL);
}
```

### ❌ 错误 2: 循环内传同一个地址

```c
for (int i = 0; i < N; i++) {
    pthread_create(&t, NULL, worker, &i);  // ❌ i 是同一个变量
}

/* ✅ 修复: 每个线程独立的存储空间 */
int vals[N];
for (int i = 0; i < N; i++) {
    vals[i] = i;
    pthread_create(&t, NULL, worker, &vals[i]);
}
```

### ❌ 错误 3: join 已经 detach 的线程

```c
pthread_detach(t);
pthread_join(t, NULL);  // ❌ 未定义行为！
// ✅ 已 detach 的线程不能 join
```

## 动手练习

### 🟢 入门：创建 3 个线程

创建 3 个线程，每个线程打印自己的 ID（通过 `arg` 传入数组元素），`join` 回收所有线程。

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
    int32_t ids[3] = {10, 20, 30};
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

### 🟡 中级：线程返回求和结果

创建一个线程计算 1 到 100 的和，通过 `pthread_exit` 返回结果，主线程通过 `pthread_join` 的第二参数接收结果并打印。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct { int result; } SumResult;

static void *calc_sum(void *arg) {
    int n = *(int *)arg;
    SumResult *s = malloc(sizeof(SumResult));
    s->result = 0;
    for (int i = 1; i <= n; i++) s->result += i;
    pthread_exit(s);
}

int main(void) {
    int n = 100;
    pthread_t t;
    SumResult *res;
    pthread_create(&t, NULL, calc_sum, &n);
    pthread_join(t, (void **)&res);
    printf("sum(1..%d) = %d\n", n, res->result);
    free(res);
    return 0;
}
```

</details>

### 🔴 挑战：实现"线程超时"

创建线程执行耗时操作，主线程最多等 3 秒。超过 3 秒后，主线程继续执行（提示：`pthread_tryjoin_np` 不是 POSIX 标准，需要用 `pthread_detach` + 异步通知的方式模拟）。

<details><summary>查看答案提示</summary>

思路：1) 用 `pthread_detach` 让线程自动回收；2) 主线程等 3 秒后不管结果继续走；3) 线程内部定期检查退出标志。这不是 C 标准方式——生产环境推荐用条件变量或线程池。

</details>

## 故障排查

**Q：编译报错 `undefined reference to pthread_create`**

A：编译时没有加 `-pthread`：
```bash
gcc -Wall -Wextra -std=c17 -pthread -o demo demo.c
```

**Q：线程的输出顺序不确定？**

正常。线程调度由操作系统决定，你可能看到 `1 2 3` 也可能看到 `3 1 2`。这就是并发的本质——**你不该依赖线程的执行顺序**。

**Q：程序跑了但没有输出**

A：主线程可能太快 exit 了，子线程还没来得及打印。检查是否调用了 `pthread_join`。

## 知识扩展

### Python 对比

```python
# Python threading — 对比 pthread
import threading

def worker(name):
    print(f"我是 {name}")

threads = [threading.Thread(target=worker, args=(f"厨师{i}",)) for i in range(3)]
for t in threads: t.start()
for t in threads: t.join()
```

Python 的 `threading.Thread` 本质就是调用 pthread，但加了更友好的封装。

### Rust 对比

```rust
// Rust — 编译期就阻止了数据竞争
use std::thread;
use std::sync::Arc;

let data = Arc::new(vec![1, 2, 3]);
let handles: Vec<_> = (0..3).map(|i| {
    let data = Arc::clone(&data);
    thread::spawn(move || {
        println!("线程 {} 看到数据: {:?}", i, data);
    })
}).collect();
for h in handles { h.join().unwrap(); }
```

Rust 用所有权系统保证安全，C 则需要手动管理。

## 小结

- **pthread_create** 创建线程，4 个参数：输出 ID、属性、入口函数、参数
- **pthread_join** 等待线程结束，回收资源
- **void\*** 传参时记住「每个线程的栈地址要独立」
- **pthread_detach** = Fire-and-forget，之后不能再 join
- 线程的退出顺序不可预测——这就是并发

## 术语表

| 英文 | 中文 |
|------|------|
| Thread | 线程 |
| pthread_create | 创建线程 |
| pthread_join | 等待线程结束 |
| pthread_detach | 分离线程（自动回收） |
| pthread_exit | 线程主动退出并返回值 |
| Thread ID | 线程标识符 |
| Entry function | 线程入口函数 |
| Fire-and-forget | 创建并忘记 |
| Thread lifecycle | 线程生命周期 |
| Stack (per-thread) | 每个线程的私有栈 |

## 延伸阅读

- [POSIX Threads 官方文档](https://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_create.html)
- [pthread(7) man page](https://man7.org/linux/man-pages/man7/pthreads.7.html)
- [The Little Book of Semaphores](https://greenteapress.com/wp-content/uploads/2011/12/LittleBookOfSemaphores.pdf)

## 继续学习

你已经掌握了线程的生命周期——如何创建、如何等待、如何"放生"。下一步：当多个线程同时读写同一个变量，会发生什么？这就是下一章要讲的——同步原语（Mutex、Condvar、Atomic）。

- [上一章](../error-handling.md)：错误处理
- [下一章](./async_sync.md)：同步原语
