# C11 存储类说明符 (_Alignas / _Noreturn / _Thread_local)

```admonish note
**本章讲什么** — C11 的三个存储类说明符:`_Alignas` 请求更强对齐、`_Noreturn` 声明绝不返回、`_Thread_local` 每线程一份副本。错误先行:先看注释里的约束违规错例,再看 pthread worker 实测的线程隔离。
**对应源码** — [`src/basic/c11_storage_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/c11_storage_sample.c)
```

## 开篇故事

CPU 访问未对齐的地址可能极慢(甚至在某些架构上直接触发异常);库函数"绝不返回"是个承诺,编译器想据此优化;多线程里"人人有份"的计数器和"大家挤一间"的全局变量,结果天差地别。C11 把这三件事分别交给了 `_Alignas`、`_Noreturn`、`_Thread_local` —— 它们不改变数据,只改变编译器和运行时**看待数据的方式**。

## 本章适合谁

- 学过变量/结构体,想了解 C11 存储类的新读者
- 写过多线程、被"线程私有数据"坑过的程序员
- 好奇"对齐"和"绝不返回"如何影响代码生成

## 你会学到什么

1. `_Alignas(N)` 如何请求比自然对齐更强的对齐,以及哪些写法是约束违规
2. `_Alignof` 的查询用法(深挖在[结构体内存布局](./struct_memory_layout.md)章)
3. `_Noreturn` 的契约语义,以及它如何消除"控制流到达函数末尾"的警告
4. `_Thread_local` 与 pthread worker:线程私有计数器的正确姿势

## 错例:_Alignas 写错就是约束违规

```c
/* ❌ 错例 (仅示意, 编译不过, C17 §6.7.5) */
_Alignas(1) int x;    /* 小于 int 的自然对齐 → 约束违规 */
_Alignas(3) char c;   /* 不是 2 的幂 → 约束违规 */
```

### 为什么错

`_Alignas(N)` 的 `N` 必须是 2 的幂,且**不小于**该类型的自然对齐。降低对齐会让 CPU 访问错位、产生未定义行为,所以标准直接禁止。反过来,**请求更强的对齐是允许的**:

```c
_Alignas(64) static unsigned char cache_line[64];  /* ✅ cache-line 对齐 */
```

## 正确写法:_Alignas 请求更强对齐

```c
_Alignas(64) static unsigned char c11s_cache_line[64];

int c11s_cache_line_mod64(void)
{
    return (int)((uintptr_t)c11s_cache_line % 64u);  /* 必为 0 */
}
```

`_Alignof(T)` 查询类型/对象的对齐需求;`struct_memory_layout_sample.c` 有完整对照表,本章只交叉引用。

## 原理解析

### 1. _Noreturn:一份"绝不返回"的契约

```c
static _Noreturn void c11s_die(const char *reason)
{
    fprintf(stderr, "  [fatal] %s\n", reason);
    exit(1);            /* exit 终止进程 — _Noreturn 契约成立 */
}
```

`_Noreturn` 告诉编译器:这个函数**绝不返回调用点**。收益有两个:

1. 调用点之后的代码被标记为不可达
2. 消除 "control reaches end of non-void function" 这类警告

```c
static int c11s_checked_div(int a, int b, int *out)
{
    if (b == 0) {
        c11s_die("division by zero");   /* 编译器知道它不返回 */
    }
    *out = a / b;
    return 1;   /* 不会再有"所有路径都返回了吗"的警告 */
}
```

> ❌ 在 `_Noreturn` 函数里写 `return` 是**约束违规**,clang 直接编译错误,不是警告。

### 2. _Thread_local:每线程一份副本

```c
static _Thread_local long c11s_tls_partial_sum;   /* 每线程独立 */

static void *c11s_tls_worker(void *arg)
{
    int k = (int)(intptr_t)arg;      /* 按值传参,见 async_thread 章铁律 */
    c11s_tls_partial_sum = 0;
    for (int i = 1; i <= k; i++) {
        c11s_tls_partial_sum += i;   /* 只改自己的副本 — TSan 眼里无竞争 */
    }
    return (void *)(intptr_t)c11s_tls_partial_sum;   /* pthread_join 取回 */
}
```

对照:把 `_Thread_local` 换成普通 `static`,3 个线程同时读改写**同一块内存**,结果取决于调度 —— 那是数据竞争(未定义行为)。

### 3. 传参铁律(复用 async_thread 章)

线程参数用 `(void *)(intptr_t)N` **按值**传递,**绝不**传 `&stack_local`:线程的栈帧可能比父函数活得久,传地址就是悬空指针。

## 动手练习

### 🟢 入门:查询对齐

用 `_Alignof` 打印 `int`、`double`、`void *` 的对齐需求,与 `sizeof` 对比。

<details><summary>点击查看答案</summary>

```c
printf("_Alignof(int)     = %zu\n", _Alignof(int));
printf("_Alignof(double)  = %zu\n", _Alignof(double));
printf("_Alignof(void *)  = %zu\n", _Alignof(void *));
```

</details>

### 🟡 中级:_Noreturn 收束

写一个 `_Noreturn void oom_die(void)`,在 `malloc` 失败时调用,验证"调用点之后的 return 不再触发警告"。

<details><summary>点击查看答案</summary>

```c
static _Noreturn void oom_die(void)
{
    fprintf(stderr, "out of memory\n");
    exit(1);
}

int *p = malloc(n * sizeof *p);
if (p == NULL) {
    oom_die();     /* 之后的代码不可达, 编译器据此免警告 */
}
```

</details>

### 🔴 挑战:线程私有计数器

写 4 个线程,每个线程把自己的 `_Thread_local` 计数器加到自己的编号(1..4),主线程用 `pthread_join` 求和验证结果为 10。

<details><summary>点击查看答案</summary>

```c
static _Thread_local int tls_c;

static void *w(void *arg) {
    int k = (int)(intptr_t)arg;
    tls_c = k;
    return (void *)(intptr_t)tls_c;
}
/* 主线程 join 4 个线程, 累加返回值 = 1+2+3+4 = 10 */
```

</details>

## 故障排查 (FAQ)

**Q:`_Alignas(128)` 会浪费内存吗?**

A:会多占对齐空间,但不会破坏正确性。cache-line / DMA buffer 是典型合理用途;给普通 `int` 请求 128 对齐纯属浪费。

**Q:`_Thread_local` 能用在函数局部变量吗?**

A:可以,`_Thread_local` 可修饰局部 `static` 变量(但不修饰普通自动变量 —— 自动变量本来就每调用一份)。

**Q:`_Noreturn` 和 `[[noreturn]]`(C23)什么关系?**

A:C23 引入属性式 `[[noreturn]]`,`_Noreturn` 仍是标准关键字;语义相同。

## 小结

- `_Alignas(N)`:N 必须是 2 的幂且 ≥ 自然对齐;请求**更强**对齐是允许的
- `_Alignof(T)`:查询对齐;深挖见[结构体内存布局](./struct_memory_layout.md)
- `_Noreturn`:绝不返回的契约,调用点之后不可达;写 return 是约束违规
- `_Thread_local`:每线程一份副本;配合 pthread worker 用 `(void *)(intptr_t)N` 传值

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| Alignment | 对齐 | 数据地址必须是某数(通常 2 的幂)的倍数 |
| Storage class | 存储类 | `static`/`extern`/`_Thread_local` 等关键字 |
| Thread-local storage | 线程局部存储 | 每线程独立副本(TLS) |
| Constraint violation | 约束违规 | 违反标准硬性规则,编译必须拒绝 |

## 延伸阅读

- [cppreference - _Alignas](https://en.cppreference.com/w/c/language/align)
- [cppreference - _Thread_local](https://en.cppreference.com/w/c/language/_Thread_local)
- [C17 standard §6.7](https://port70.net/~nsz/c/c17/n1570.html#6.7)

## 继续学习

- [上一章](./struct_memory_layout.md):结构体内存布局
- [下一章](./unions.md):联合体

---

> 本章代码位于 [`src/basic/c11_storage_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/c11_storage_sample.c),测试位于 [`test/basic/test_c11_storage_sample.c`](https://github.com/savechina/hello-c/blob/main/test/basic/test_c11_storage_sample.c)。
