# 内存安全工程 (Memory Safety Engineering)

## 开篇故事

你写完一个 C 程序, 测试全绿, 上线。然后线上出现一段偶发崩溃——只在某台机器、某个时刻发生, 因为有一个指针在它不该活着的时候还活着。

C 给你三样东西: 指针、`malloc`、以及**完全的自由**。这份自由意味着**没有任何机制会在你忘记 `free` 时提醒你**, 在你读已释放内存时阻止你。C++ 有 `delete`, Rust 有所有权检查器 (ownership checker)。C 有什么? C 有一份 `free()` 列表, 靠人的记忆力维持。

本章不讲"你应该小心一点"这种话。我们讲**工程手段**: 用语言和工具把"不小心"从可能变成不可能。

> 💡 你已经会写 RAII 宏了 (见 [不透明指针](opaque-pointers.md))。本章解决那个宏的根本缺陷, 并补齐五种可落地的替代方案。

## 本章适合谁

- 写过一定量的 C, 被 leak / use-after-free / double-free 咬过
- 想系统了解 C 有哪些"接近内存安全"的手段, 以及每种**真实能做到什么**
- 想知道为什么 Rust 的 `Option<T>` 和 RAII 在 C 里只能"模拟", 不能"获得"

## 你会学到什么

1. `__attribute__((cleanup))` —— 编译器帮你做清理, 比 `goto` 和宏都可靠
2. `DEFER` 宏 —— 作用域退出时按 LIFO 执行任意动作
3. Arena (Region) 分配器 —— 用**一次** `free` 消除一整类 bug
4. `Option<T>` —— 用 tagged union 编码「可能没有值」
5. 定容缓冲区 —— 把容量写进类型, 让溢出**结构上不可能**
6. 工具链: `make asan` / `make test-asan` —— 让 bug 在你发现前先自杀

## 前置要求

- 已掌握 [内存管理](../basic/memory_mgmt.md) —— `malloc`/`free`/`realloc`
- 已掌握 [作用域与生命周期](../basic/scope.md) —— 栈变量什么时候死
- 已了解 [不透明指针](opaque-pointers.md) —— 工厂 + 析构模式

---

## 一、先看清问题: 清理逻辑为什么容易漏

### 错误示范

```c
/* ❌ 手动清理: 每加一个 return 就多一个漏掉的 free */
int process(const char *path)
{
    char *buf = malloc(4096);
    if (!buf) return -1;

    int fd = open(path, O_RDONLY);
    if (fd < 0) { free(buf); return -1; }      /* 要记得写 */

    if (read(fd, buf, 4096) < 0) {
        close(fd);
        free(buf);                            /* 又要记得写 */
        return -1;
    }

    close(fd);
    free(buf);
    return 0;
}
```

这段代码本身是**对的**。问题在于它的正确性依赖三件事同时成立: 没人新增 `return`、没人新增资源、没人重构。代码一改, 静默泄漏。

我们想要的机制是: **把"必须清理"变成语言层面的保证, 而不是人的纪律。**

---

## 二、`__attribute__((cleanup))`: 编译器插入清理

GCC/Clang 扩展 (非 ISO C)。这是 C 里最接近 RAII 的机制。

### 基本用法

```c
static void close_fd(int *fd)          /* 参数是【变量的指针】 */
{
    if (*fd >= 0) { close(*fd); *fd = -1; }
}

static void free_mem(void **p)
{
    if (*p) { free(*p); *p = NULL; }
}

int process(const char *path)
{
    int  fd __attribute__((__cleanup__(close_fd)))  = open(path, O_RDONLY);
    void *buf __attribute__((__cleanup__(free_mem))) = malloc(4096);

    if (fd < 0 || !buf) return -1;

    if (read(fd, buf, 4096) < 0) return -1;   /* 不用写任何清理! */
    return 0;                                  /* 也不用写 */
}
```

三种退出路径 (正常结束 / `return` / `break`) 全都自动清理。**这才是 RAII。**

### 三个必须记住的坑

**坑 1: 属性写在变量上, 不是函数上。**

```c
/* ❌ 这样写会报 -Wignored-attributes */
static void __attribute__((cleanup)) close_fd(int *fd) { }

/* ✅ 属性属于变量声明 */
int fd __attribute__((cleanup(close_fd))) = 3;
```

**坑 2: 清理函数的参数类型必须匹配变量的类型。**

```c
/* 变量是 void*  → 清理函数收 void** */
void *__attribute__((cleanup(free_mem))) p = malloc(64);

/* 变量是 int32_t* → 清理函数必须收 int32_t**, 收 void** 会报错 */
```

**坑 3: `exit()` / `_exit()` / `longjmp()` 会绕过它。**

栈展开 (stack unwinding) 根本不发生, 所以 `cleanup` 一次都不会跑。只有 `atexit` / `__attribute__((destructor))` 会在进程退出时触发。**如果清理函数会解锁, 别在持锁时调用 `exit()`。**

### 对比: 宏写法为什么不够好

[不透明指针章节](opaque-pointers.md) 里的 `WITH_MALLOC` 宏:

```c
#define WITH_MALLOC(ptr, type, count) \
    for (type *ptr = calloc(count, sizeof(type)); \
         ptr != NULL; \
         (free(ptr), ptr = NULL))
```

它在**正常落出循环**时工作, 但循环体里一旦 `return` / `break` / `goto`, 第三个表达式就被跳过 → **泄漏**。这不是使用姿势问题, 是机制本身的缺陷。`cleanup` 属性没有这个洞。

---

## 三、`DEFER` 宏: LIFO 延迟执行

有时你要的不是一个 `free`, 而是"离开作用域时做任意一件事"。

```c
struct defer { void (*fn)(void *); void *arg; };

static void defer_run(struct defer *d) { d->fn(d->arg); }

#define CAT_(a, b) a##b
#define CAT(a, b)  CAT_(a, b)
#define defer_to(fn, arg) \
    struct defer CAT(_d_, __LINE__) \
        __attribute__((__cleanup__(defer_run))) = { (fn), (arg) }
```

用起来:

```c
lock(&mutex);
defer_to(unlock, &mutex);       /* 无论如何都会解锁 */

if (need_abort()) return -1;    /* 自动解锁 */
do_work();
```

输出顺序是**后进先出** (LIFO), 和栈一样。

> ⚠️ 标准 C **没有** `defer`。它目前只是 ISO/DIS TS 25755 草案, 所以这个宏是 GNU/Clang 扩展技巧。别在教程里教「C 有 defer」。

---

## 四、Arena 分配器: 一次分配, 一次释放

这是**收益最大**的一个技巧。

### 想法

传统做法是 N 次 `malloc` 配 N 次 `free`。每一次 `free` 都是一个出错机会: 漏掉、重复、顺序错。**Arena 把 N 次决策压缩成 1 次。**

```c
typedef struct {
    unsigned char *base;
    size_t cap, used;
} arena_t;

static void *arena_alloc(arena_t *a, size_t n)
{
    size_t off = (a->used + 15u) & ~(size_t)15u;      /* 16 字节对齐 */
    if (n > a->cap || off > a->cap - n) return NULL;  /* 容量检查 */
    void *p = a->base + off;
    a->used = off + n;
    return p;
}

static void arena_reset(arena_t *a) { a->used = 0; }  /* O(1) 释放全部 */
```

从 arena 里分配的所有指针, 彼此**在内存上连续**——这对缓存命中率有实际好处:

```c
rec  = 0x615000001980
nums = 0x615000001998     /* 紧挨着 */
msg  = 0x6150000019a8
```

### 什么时候用

- 一次请求的生命周期内创建、销毁大量小对象 (编译器 AST、解析器、每帧游戏对象)
- 你**确定**它们会一起死

### 两个经典坑

**坑 1: 无法单独释放某个对象。** 生命周期不同的数据混在一个 arena 里, 短命对象会被长命对象钉住内存。解法: 分两个 arena (永久 + 每帧/每请求)。

**坑 2: `reset` 让所有指向 arena 内部的指针失效。** 派生指针必须**整体**一起丢弃, 不能留一部分。

---

## 五、`Option<T>`: 用类型表达「可能没有值」

### 不用哨兵值

```c
/* ❌ 用 -1 / NULL 表示"失败" —— 调用方很容易忘查 */
int find_user(int id);
```

问题不在于 `-1` 本身, 而在于**"没找到"和"找到 0"在类型上长得一样**。

### tagged union

```c
typedef enum { RAII_NONE, RAII_SOME } raii_tag_t;

typedef struct {
    raii_tag_t tag;
    int        value;
} raii_opt_int_t;

raii_opt_int_t raii_some(int v);
raii_opt_int_t raii_none(void);

/* 返回 bool, 强迫调用者处理"空" */
bool raii_value(raii_opt_int_t o, int *out)
{
    if (o.tag != RAII_SOME) return false;   /* 不写 *out */
    *out = o.value;
    return true;
}
```

调用处被强制分支:

```c
int v;
if (raii_value(raii_some(42), &v)) {
    printf("有值: %d\n", v);
} else {
    printf("无值\n");                        /* 不能被跳过 */
}
```

### ⚠️ 必须说清的限制

**这不是编译期检查。** 忘记判断 `tag` 依然是合法 C, 依然能编译通过。真正的 Rust `Option<T>` 会在你 `unwrap()` 一个 `None` 时**编译失败**; C 版的只能在运行时发现。

这是 C 和 Rust 在内存安全上的根本差距: **C 可以把"可能出错"编码进类型, 但无法强制你去检查。**

---

## 六、定容缓冲区: 让溢出结构上不可能

```c
#define RAII_BUF_CAP 64

typedef struct {
    unsigned char data[RAII_BUF_CAP];
    size_t        len;
} raii_buf_t;

bool raii_buf_append(raii_buf_t *b, const void *p, size_t n)
{
    if (n > sizeof b->data - b->len) return false;   /* 注意用 sizeof */
    memcpy(b->data + b->len, p, n);
    b->len += n;
    return true;
}
```

**关键在于 `sizeof b->data`, 而不是调用方传进来的长度。** 边界由类型自己保证, 调用方无法绕过。

```c
raii_buf_t b;
raii_buf_init(&b);

raii_buf_append_str(&b, "Hello, world");   /* ok=true, len=12 */

/* 塞 123 字节 → 被拒绝, 且 len 纹丝不动 */
bool ok = raii_buf_append_str(&b, 巨长字符串);
size_t before = raii_buf_len(&b);
/* len 仍是 12, 没有被破坏 */
```

对应的反例是 `sprintf`——边界由**输入内容**决定, 而输入长度在编译期通常未知 (见 [日志与格式化输出](../basic/logging.md))。

---

## 七、工具链: 让 bug 在你发现前先自杀

约定靠不住, 工具靠得住。

### `make asan` — AddressSanitizer + UBSan

```bash
make asan      # 编译整个程序并在 ASan+UBSan 下运行
```

它能抓到: 缓冲区溢出、use-after-free、double-free、无效释放、对齐错误、未定义行为。

> 💡 本章配套源码是 `src/advance/raii_sample.c` —— 五种手法各一个可运行 demo, 注册在 `src/advance/advance.c` 的协调器里。单元测试见 `test/advance/test_raii_safety.c`。

### `make test-asan` — 让测试也带上探针

```bash
make test        # 17 个测试, 但没有任何检测能力
make test-asan   # 同样的测试, 跑在 ASan+UBSan 下
```

`make test` 编译出来的程序**不带任何插桩**, 所以它对内存错误是完全瞎的。这一点很重要: "测试全绿"和"代码没有内存 bug"是两件毫不相干的事。

### `make analyze` — GCC 静态分析

```bash
make analyze                # macOS 上会自动跳过
make analyze CC=gcc-14      # 需要真正的 GCC
```

### 平台限制（重要）

| 工具 | macOS (Apple Silicon) | Linux |
|------|----------------------|-------|
| ASan (溢出/UAF/double-free) | ✅ | ✅ |
| UBSan (未定义行为) | ✅ | ✅ |
| **LeakSanitizer (泄漏)** | ❌ **不支持** | ✅ |
| Valgrind | ❌ 无 arm64 支持 | ✅ |
| `-fanalyzer` | ❌ Apple clang 没有 | ✅ |

**在 macOS 上, ASan 不会报告内存泄漏。** 这是 LeakSanitizer 的平台限制, 不是配置问题。查泄漏要用 Apple 自带工具:

```bash
MallocStackLogging=1 leaks --atExit -- ./build/bin/hello-asan
```

---

## 八、真实案例: 工具抓到了人眼没抓到的东西

本章的代码在接入 `make asan` 时立刻抓出两个 bug, 都不是靠读代码能发现的。

### 案例 1: 分离线程读取已销毁的栈变量

```c
static void async_thread_detached_sample(void)
{
    int32_t id = 42;
    pthread_t t;
    pthread_create(&t, NULL, worker, &id);   /* 传了栈变量的地址 */
    pthread_detach(t);
    /* 函数返回 → id 的生命周期结束 */
    /* 但分离线程可能还在读它 */
}
```

`ASan: stack-use-after-scope`。一个**关于线程安全**的章节里, 藏着一个真实的线程安全 bug。

正确做法是按值传递:

```c
pthread_create(&t, NULL, worker, (void *)(intptr_t)42);
/* worker 内部: int32_t id = (int32_t)(intptr_t)arg; */
```

> ⚠️ **哪个门禁真的抓得住它（mutation 实测）**: 把 `&id` 改回去,
> `make asan` **3/3 次**报 `stack-use-after-scope`; `make test-asan` **0/3 次**报——
> 两个二进制之后执行的代码不同, 栈帧复用时机不同, ASan 是否命中取决于时序。
> `test/advance/test_async_thread_lifecycle.c` 因此只自称冒烟测试, 不自称这个 bug 的门禁。
> **结论: 工具检测是概率性的, 结构上按值传递才是确定的。**

> 📌 **通用规则**: 分离线程的生命周期超过父函数, 所以绝不能把指向父函数栈变量的指针交给它。要么按值传, 要么用生命周期更长的存储 (堆 / `static`)。详见 [线程创建与生命周期](async/async_thread.md) 错误 4。

### 案例 2: 把「未定义行为」教成了「回绕」

原代码:

```c
printf("  INT32_MAX + 1 = %d (overflow! wraps to negative)\n", max + 1);
```

两个问题:

1. **有符号整数溢出在 C 里是未定义行为 (UB), 不是回绕。** 编译器有权假设它永不发生, 并在 `-O2` 下把相关判断整个删掉。
2. **无符号溢出才是标准定义的行为**——按模 2^N 回绕。

正确的写法是同时展示两者, 并说明边界检查必须**提前做**:

```c
printf("  (uint32_t)INT32_MAX + 1 = %" PRIu32 "  ← 无符号: 标准定义, 按模回绕\n",
       (uint32_t)smax + 1u);
printf("  (int32_t) 有符号溢出是【未定义行为】, 不是回绕!\n");
printf("  边界检查必须写成 if (x > INT32_MAX - n) 而不能靠事后判断结果。\n");
```

> 📌 **为什么不用 `#pragma` 屏蔽掉这个警告?** 因为屏蔽会让**错误的教学**留下来。这不是代码 bug, 是知识 bug——比 bug 更难发现。

---

## 九、小结

| 手段 | 能保证什么 | **不能**保证什么 | 标准 |
|------|-----------|----------------|------|
| `cleanup` 属性 | `return`/`break`/`goto` 全覆盖 | `exit()`/`longjmp` | GNU/Clang 扩展 |
| `DEFER` 宏 | LIFO 延迟执行 | 同上 | GNU/Clang 扩展 |
| Arena | 一处决定生死, 无 double-free | 无法单独释放、指针会失效 | 纯 C11 |
| `Option<T>` | 「可能没有值」进类型 | **不强制检查** | 纯 C11 |
| 定容缓冲区 | 溢出结构上不可能 | 容量固定 | 纯 C11 |
| ASan/UBSan | 抓到内存错误和 UB | macOS 上抓不到泄漏 | 工具链 |

**没有银弹。** 真正的做法是**叠加**: 用 `cleanup` 消灭大部分清理代码, 用 Arena 消灭大部分生命周期问题, 用 `Option` 让「可能失败」显式化, 用定容缓冲区让溢出不可能, 最后用 ASan 兜住你没想到的。

工具链是最后一道防线, 也是唯一能抓住**你没想到的情况**的那道。

## 下一章

- [不透明指针](opaque-pointers.md) —— 工厂模式与信息隐藏 (本章的宏写法背景)
- [测试框架](testing.md) —— 怎么给这些安全属性写回归测试