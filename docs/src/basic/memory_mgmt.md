# 内存管理（Memory Management: malloc/free）

> "C 语言不替你保管任何东西——它给你钥匙，但不帮你锁门。" —— 我发现

## 开篇故事

想象你在一家共享办公空间租了一个工位。你向管理员申请（`malloc`）了一个位置，坐下工作，使用这张桌子。

关键是：**你用完之后必须归还**（`free`）。如果你忘了退租，那张桌子就永远被占着。下一位同事来时，管理员告诉他「没有空位了」——不是因为真的没有，而是有人占了不退。

这就是 C 语言中的内存管理。`malloc` 是你申请空间，`free` 是你归还空间。每一次申请都对应一次归还，否则内存就像那些占着不走的工位，迟早会用完。

## 本章适合谁

- 已经写过 C 代码，但 malloc/free 总是手忙脚乱的人
- 在 Python/Java 等自动管理内存的语言里长大的开发者，想理解 C 的手动管理
- 遇到过程序占用内存越来越大，怀疑"内存泄漏"的人
- 被段错误（Segmentation fault）折磨，想知道"为什么不能解引用那个指针"的人

## 你会学到什么

- `malloc`、`calloc`、`realloc`、`free` 四个函数的正确用法
- **堆（Heap）与栈（Stack）**的本质区别，ASCII 内存布局图
- 三种常见分配模式：单个变量、数组、结构体
- 为什么 `malloc` 的返回值**必须检查 NULL**
- 三种致命错误：内存泄漏（Memory Leak）、悬垂指针（Dangling Pointer）、使用已释放内存（Use-After-Free）
- 用 `valgrind` 检测内存泄漏的基本流程
- **安全分配模式**：每次 malloc 都配对 free，每次解引用前都检查 NULL

## 前置要求

- 理解 C 语言作用域与生命周期（已完成「作用域」章节）
- 掌握指针基本概念（`*` 解引用、`&` 取地址、`NULL` 检查）
- 了解 `sizeof` 运算符——它决定 `malloc` 分配多少字节

## 第一个例子：malloc 与 free 配对

```c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(void) {
    /* 1. 分配：向操作系统申请 sizeof(int32_t) 字节 */
    int32_t *p = malloc(sizeof(int32_t));

    /* 2. 安全检查：malloc 可能失败，返回 NULL */
    if (p == NULL) {
        fprintf(stderr, "Error: malloc failed!\n");
        return 1;
    }

    /* 3. 使用：写入、读取 */
    *p = 42;
    printf("value = %" PRId32 "\n", *p);  /* 42 */

    /* 4. 释放：归还给操作系统 */
    free(p);
    p = NULL;  /* 防止悬垂指针 */

    return 0;
}
```

四步走：**分配 → 检查 → 使用 → 释放 + 置 NULL**。

> 📌 **回顾之前学的**: 栈（Stack）上的变量在函数返回时自动回收，堆（Heap）上的变量需要手动 `free`。详见 [数据类型](./datatype.md) 中关于栈变量的生命周期。

## 原理解析

### 1. 堆 vs 栈：ASCII 内存布局

C 程序运行时的内存分为两个主要区域：

```
                    高地址
┌──────────────────────────────┐
│   栈 (Stack)                 │  ← 自动生长方向：高地址 → 低地址
│   ┌──────────────────────┐   │
│   │  main 的局部变量       │   │     int x = 10;     ← 栈变量
│   │  x = 10               │   │     char buf[64];   ← 栈数组
│   ├──────────────────────┤   │
│   │  func() 的局部变量     │   │     int y;          ← 函数调用时压入
│   │  y = 20               │   │
│   └──────────────────────┘   │     ← 函数返回时弹出，自动释放
├──────────────────────────────┤
│                              │     空洞（未使用）
│         ~~~~~~~~~~~~~~~       │
│                              │
├──────────────────────────────┤
│   堆 (Heap)                  │  ←  malloc 分配区域
│   ┌──────────────────────┐   │
│   │  malloc(4) → p1      │   │     int *p = malloc(sizeof(int));
│   │  malloc(256) → buf   │   │     char *b = malloc(256);
│   └──────────────────────┘   │     ← free(p) 后归还
├──────────────────────────────┤
│   数据段 (.data / .bss)      │     全局变量、static 变量
├──────────────────────────────┤
│   代码段 (.text)             │     程序指令
└──────────────────────────────┘
                    低地址
```

**我的理解**：栈是"自动门"——你走进来（进入作用域），门就开；你走出去，门就关（变量销毁）。堆是"租房子"——用 `malloc` 拿钥匙开门，用 `free` 退租还钥匙，不退就永远占着。

### 2. malloc：最基本的动态分配

```c
#include <stdlib.h>

void *malloc(size_t size);
```

- **参数**：需要分配的字节数
- **返回值**：`void*`（任意类型指针），失败时返回 `NULL`
- **特点**：分配的内存**未初始化**（内容是垃圾值）

```c
int32_t *p = malloc(sizeof(int32_t));   /* 分配 4 字节 */
if (p == NULL) { /* 处理错误 */ }
*p = 42;                                /* 必须先赋值再读取 */
```

### 3. calloc：清零分配

```c
void *calloc(size_t count, size_t size);
```

- **参数**：元素数量 × 每个元素大小
- **返回值**：同 malloc，失败返回 `NULL`
- **特点**：分配的内存**全部初始化为 0**

```c
int32_t *arr = calloc(100, sizeof(int32_t));  /* 100 个 int32_t，全为 0 */
if (arr == NULL) { /* 处理错误 */ }
/* arr[0] ... arr[99] 都是 0，不需要手动初始化 */
```

**malloc vs calloc 对比**：

| 特性 | `malloc(n)` | `calloc(count, size)` |
|------|-------------|----------------------|
| 大小计算 | 手动 `sizeof` | 自动 `count * size` |
| 初始值 | 垃圾值（未初始化） | 全部归零 |
| 性能 | 稍快（不清零） | 稍慢（要清零） |
| 适用场景 | 你准备立刻填充数据 | 初始值应为 0（如计数器、指针数组） |

### 4. realloc：调整已有分配的大小

```c
void *realloc(void *ptr, size_t new_size);
```

- **参数**：原指针（由 malloc/calloc/realloc 返回） + 新大小
- **返回值**：新地址（可能与原址相同或不同），失败返回 `NULL`（原内存**未释放**）
- **特点**：缩小不丢数据；增长时可能搬数据

```c
int32_t *buf = malloc(10 * sizeof(int32_t));
/* ... 使用 buf[0]..buf[9] ... */

/* 需要更多空间 */
int32_t *tmp = realloc(buf, 20 * sizeof(int32_t));
if (tmp == NULL) {
    /* realloc 失败！buf 仍然有效，先 free(buf) 再退出 */
    free(buf);
    return 1;
}
buf = tmp;  /* 更新指针（realloc 可能搬了数据） */
/* 现在可以使用 buf[0]..buf[19]，前 10 个值保留 */
```

**关键教训**：`realloc` 的返回值**必须用临时变量保存**，永远不要 `ptr = realloc(ptr, ...)`——如果失败，`ptr` 变成 `NULL`，原内存地址丢失，造成泄漏。

### 5. free：释放内存

```c
void free(void *ptr);
```

- **参数**：`malloc`/`calloc`/`realloc` 返回的指针（或 `NULL`——对 `NULL` 调用 `free` 是安全的）
- **注意**：
  - 释放后指针不会自动变 `NULL`——**你必须手动 `ptr = NULL`**
  - 对同一指针调用 **两次 `free`** = undefined behavior（double-free，严重错误）

### 6. 内存生命周期

```
┌─── malloc ────┐    ┌─── free ────┐
│               │    │             │
v               v    v             v
NULL ──→ 有效内存 ──→ 已释放(悬垂) ──→ NULL
         (可读写)    (不可再访问!)

生命周期三阶段：
1. 未分配：ptr == NULL
2. 已分配：ptr != NULL，可以读写 *ptr 或 ptr[i]
3. 已释放：free 后，ptr 成为悬垂指针，必须 ptr = NULL
```

## 常见分配模式

### 模式 1：单个变量分配

```c
int32_t *p = malloc(sizeof(int32_t));
if (p == NULL) return;
*p = 99;
printf("%" PRId32 "\n", *p);
free(p); p = NULL;
```

### 模式 2：数组分配

```c
size_t n = 10;
int32_t *arr = calloc(n, sizeof(int32_t));
if (arr == NULL) return;
for (size_t i = 0; i < n; i++) {
    arr[i] = (int32_t)(i * 2);
}
free(arr); arr = NULL;
```

### 模式 3：结构体分配

```c
typedef struct {
    char name[32];
    int32_t age;
} Person;

Person *p = malloc(sizeof(Person));
if (p == NULL) return;
strncpy(p->name, "Alice", sizeof(p->name) - 1);
p->name[sizeof(p->name) - 1] = '\0';
p->age = 25;
printf("name=%s, age=%" PRId32 "\n", p->name, p->age);
free(p); p = NULL;
```

## 常见错误（Error-First）

### ❌ 错误 1：malloc 后忘记 free —— 内存泄漏（Memory Leak）

```c
/* ❌ 泄漏示例 */
void leaky_function(void) {
    char *buf = malloc(256);
    if (buf == NULL) return;
    strncpy(buf, "important data", 255);
    buf[255] = '\0';
    /* ... 使用 buf ... */
    /* 忘记 free(buf)！ */
    /* buf 离开作用域，但堆内存没有归还 → 泄漏 */
}
/* 每次调用这个函数，泄漏 256 字节 */
```

**valgrind 检测报告**：
```
==12345== 256 bytes in 1 blocks are definitely lost
==12345==    at malloc() → leaky_function()
```

✅ **修复**：每条执行路径都必须有 `free`：

```c
/* ✅ 正确示例 */
void safe_function(void) {
    char *buf = malloc(256);
    if (buf == NULL) return;
    strncpy(buf, "important data", 255);
    buf[255] = '\0';
    /* ... 使用 buf ... */
    free(buf);
    buf = NULL;  /* 防止悬垂 */
}
```

### ❌ 错误 2：malloc 返回 NULL 时解引用 —— 段错误

```c
/* ❌ 没有检查 NULL */
int32_t *p = malloc(sizeof(int32_t));
*p = 42;       /* ❌ 如果 malloc 失败 → 对 NULL 解引用 → Segmentation fault */
```

✅ **修复**：永远在使用前检查：

```c
/* ✅ 安全检查 */
int32_t *p = malloc(sizeof(int32_t));
if (p == NULL) {
    fprintf(stderr, "Error: not enough memory\n");
    return;
}
*p = 42;       /* ✅ 安全 */
```

### ❌ 错误 3：Use-After-Free（使用已释放内存）

```c
/* ❌ 先释放，后使用 */
int32_t *p = malloc(sizeof(int32_t));
*p = 42;
free(p);
printf("%d\n", *p);  /* ❌ UB: 读取已释放的内存 */
*p = 100;            /* ❌ UB: 写入已释放的内存 */
```

✅ **修复**：释放后立即置 `NULL`：

```c
free(p);
p = NULL;
/* printf("%d\n", *p); → 现在会立刻崩溃（段错误），比静默损坏易调试 */
```

### ❌ 错误 4：悬垂指针（Dangling Pointer）

```c
/* ❌ 指向局部变量的指针 */
int32_t *get_local(void) {
    int x = 42;
    return &x;  /* ❌ x 是栈变量，函数返回后 x 不存在 */
}
int32_t *p = get_local();
printf("%" PRId32 "\n", *p);  /* ❌ p 是悬垂指针 → UB */
```

✅ **修复**：三种方案——

```c
/* 方案 A：返回堆分配 */
int32_t *safe_a(void) {
    int32_t *x = malloc(sizeof(int32_t));
    if (x) *x = 42;
    return x;
}
/* 调用者负责 free */

/* 方案 B：让调用者分配 */
void safe_b(int32_t *result) {
    *result = 42;
}
/* 调用方: int32_t val; safe_b(&val); */

/* 方案 C：用 static（但非线程安全） */
int32_t *safe_c(void) {
    static int x = 42;
    return &x;
}
```

### ❌ 错误 5：realloc 没有用临时变量

```c
/* ❌ 如果 realloc 失败，原指针丢失 */
int32_t *buf = malloc(100);
buf = realloc(buf, 200);  /* 失败: buf = NULL, 原 100 字节泄漏 */
```

✅ **修复**：

```c
int32_t *buf = malloc(100);
if (buf == NULL) return;

int32_t *tmp = realloc(buf, 200);
if (tmp == NULL) {
    free(buf);  /* 保留的 100 字节仍然安全，手动释放 */
    return;
}
buf = tmp;      /* 成功: 更新指针 */
```

## 动手练习

### 🟢 练习 1：malloc + free 配对

写一个函数 `print_sum()`，用 `malloc` 分配两个 `int32_t`，赋值为 10 和 20，打印它们的和，然后正确 `free`。

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void print_sum(void) {
    int32_t *a = malloc(sizeof(int32_t));
    int32_t *b = malloc(sizeof(int32_t));
    if (a == NULL || b == NULL) {
        free(a);
        free(b);
        fprintf(stderr, "malloc failed\n");
        return;
    }

    *a = 10;
    *b = 20;
    printf("sum = %" PRId32 "\n", *a + *b);

    free(a); a = NULL;
    free(b); b = NULL;
}
```

</details>

### 🟡 练习 2：realloc 动态增长

初始化一个 `int32_t` 数组（3 个元素），填入 10, 20, 30。用 `realloc` 扩展到 6 个元素，追加 40, 50, 60，打印所有元素。

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void grow_array_demo(void) {
    int32_t *arr = malloc(3 * sizeof(int32_t));
    if (arr == NULL) return;
    arr[0] = 10; arr[1] = 20; arr[2] = 30;

    int32_t *tmp = realloc(arr, 6 * sizeof(int32_t));
    if (tmp == NULL) {
        free(arr);
        fprintf(stderr, "realloc failed\n");
        return;
    }
    arr = tmp;
    arr[3] = 40; arr[4] = 50; arr[5] = 60;

    for (int32_t i = 0; i < 6; i++) {
        printf("arr[%" PRId32 "] = %" PRId32 "\n", i, arr[i]);
    }
    free(arr); arr = NULL;
}
```

</details>

### 🔴 练习 3：安全分配模式（完整防御）

写一个函数 `create_person(const char *name, int32_t age)`，返回一个堆分配的 `Person` 结构体。必须满足：
1. malloc 返回值检查 NULL
2. `strncpy` 安全复制字符串（非 `strcpy`）
3. 调用方负责 `free`，且有 `free` 代码

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef struct {
    char name[32];
    int32_t age;
} Person;

Person *create_person(const char *name, int32_t age) {
    Person *p = malloc(sizeof(Person));
    if (p == NULL) return NULL;

    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    p->age = age;
    return p;
}

int main(void) {
    Person *p = create_person("Bob", 30);
    if (p == NULL) {
        fprintf(stderr, "create_person failed\n");
        return 1;
    }
    printf("name=%s, age=%" PRId32 "\n", p->name, p->age);
    free(p);
    p = NULL;
    return 0;
}
```

</details>

## 故障排查（FAQ）

### Q：什么时候应该用 `malloc` 而不是栈变量？

当你需要以下能力时：
- **跨函数使用**：数据在函数返回后仍然有效（栈变量会在函数返回时销毁）
- **大数组**：几个 MB 的数组放在栈上会栈溢出（通常栈只有几 MB），`malloc` 用堆空间
- **动态大小**：在运行时才知道需要多大空间

```c
/* ❌ 栈上大数组 → 可能栈溢出 */
double big_array[1000000];  /* 8 MB! 通常栈只有 8 MB */

/* ✅ 堆上大数组 */
double *big_array = malloc(1000000 * sizeof(double));
/* ... */
free(big_array);
```

### Q：`free(NULL)` 安全吗？

是的！C 标准规定：`free(NULL)` 是一个 no-op（什么都不做）。你可以放心地写：

```c
free(p);  /* 即使 p 是 NULL 也安全 */
p = NULL;
```

### Q：什么是 valgrind？怎么用？

`valgrind` 是一个 C 内存调试工具。安装后，这样使用：

```bash
# 编译时保留调试信息（-g）
gcc -g -Wall -Wextra -std=c17 -o myprogram myprogram.c

# 用 valgrind 运行
valgrind --leak-check=full ./myprogram
```

输出示例：
```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 0 bytes in 0 blocks
==12345==   total heap usage: 5 allocs, 5 frees, 1,234 bytes allocated
==12345==
==12345== All heap blocks were freed -- no leaks are possible
```

如果看到 `in use at exit: 0 bytes`，恭喜你，你的代码 **valgrind 清洁**！

### Q：`malloc` 和 `calloc` 性能差多少？

`calloc` 需要把内存清零，所以比 `malloc` 稍慢。但差距通常很小（现代操作系统有优化）。如果你本来就要把内存清零（例如初始化计数器），`calloc` 可能更快——因为操作系统级别的 zero-page 填充比你自己用 `memset` 快。

### Q：可以多次 `free` 同一个指针吗？

**绝对不行**。第二次 `free` 同一个非 `NULL` 指针 = double-free = undefined behavior（通常崩溃或安全漏洞）。这也是为什么 `free` 后立即 `p = NULL` 很重要。

## 与 Python 对比：自动 vs 手动内存管理

| 特性 | C (`malloc/free`) | Python (自动 GC) |
|------|-------------------|-----------------|
| 分配方式 | `ptr = malloc(size)` | `obj = bytearray(size)` |
| 释放方式 | 手动 `free(ptr)` | 引用计数归零后自动 GC |
| 泄漏风险 | 高（忘记 free） | 低（但循环引用可能泄漏） |
| 悬垂指针 | 可能（free 后不置 NULL） | 不可能（GC 回收后不会保留引用） |
| 性能 | 高（精确控制，无 GC 开销） | 低（GC 暂停、额外元数据） |
| 调试工具 | valgrind | `tracemalloc`、`gc` 模块 |

**我的理解**：Python 像一个贴心的保姆，自动帮你收拾玩具（释放内存）。C 像一个严格的工具——它不帮你收拾，但你可以精确控制每一个字节的去向。哪种更好取决于场景：游戏引擎需要 C 的精确控制，而 web 后端可以用 Python 的便利。

## 知识扩展（选学）

### malloc 的底层原理：sbrk 和 mmap

在 Linux 上，`malloc` 的底层实现依赖两个系统调用：

- **小分配**（一般 < 128KB）：通过 `sbrk()` 扩展程序的数据段（brk）
- **大分配**（≥ 128KB）：通过 `mmap()` 创建新的内存映射区域

```
         sbrk 增长方向 →
┌─────────────────┐  ┌──────┐  ┌──────┐
│    brk (堆)      │  │ mmap │  │ mmap │
│  malloc 的小块    │  │ 大文件 │  │ 大数据 │
└─────────────────┘  └──────┘  └──────┘
```

### 内存对齐（Alignment）

`malloc` 返回的地址**总是对齐到 8 字节或 16 字节**——无论你要多少。这是因为现代 CPU 访问对齐内存比非对齐内存快得多。

```c
char *a = malloc(1);   /* 分配 1 字节，实际可能占用 8/16 字节 */
char *b = malloc(1);   /* malloc 返回的地址间隔 ≥ alignment */
```

### 内存池（Memory Pool）

频繁 `malloc`/`free` 小块内存会导致**碎片化**。高性能程序常用"内存池"——一次性分配一大块，然后自己管理内部的小块分配：

```c
/* 简化版内存池示意 */
#define POOL_SIZE (1024 * 1024)  /* 1 MB */
static char pool[POOL_SIZE];
static size_t pool_offset = 0;

void *pool_alloc(size_t size) {
    if (pool_offset + size > POOL_SIZE) return NULL;
    void *p = &pool[pool_offset];
    pool_offset += size;
    return p;
}
/* 一次 pool_free 释放所有（简化版无单个 free） */
```

## 小结

恭喜你闯过了 C 语言最棘手的章节——内存管理。让我总结核心要点——

- **四步走原则**：`malloc` → `检查 NULL` → 使用 → `free` + `p = NULL`
- **`malloc` vs `calloc` vs `realloc`**：基本分配、清零分配、调整大小
- **安全 realloc**：永远用临时变量保存返回值
- **三大致命错误**：内存泄漏（忘记 free）、悬垂指针（指向已销毁内存）、Use-After-Free（free 后继续使用）
- **valgrind**：你的 C 记忆守护神——每次写 C 程序都跑一遍
- **堆 vs 栈**：栈是自动门（作用域控制），堆是租房子（你控制）

> **我的教训是**：C 语言的内存管理就像骑自行车——开始觉得可怕，但一旦学会，你会获得其他地方学不到的「自由感」。关键就是两个字：**配对**。每一笔 `malloc` 都要有对应的一笔 `free`，清清楚楚，一笔不漏。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 内存分配（Memory Allocation） | 向操作系统申请内存 |
| 手动管理（Manual Management） | 程序员自己负责 malloc/free |
| 内存泄漏（Memory Leak） | 分配的内存没有释放，持续占用 |
| 悬垂指针（Dangling Pointer） | 指向已释放或失效内存的指针 |
| Use-After-Free | 释放内存后仍然读写它 |
| Double-Free | 对同一块内存调用两次 free |
| 堆（Heap） | 动态分配的内存区域 |
| 栈（Stack） | 自动管理的作用域内存 |
| 碎片化（Fragmentation） | 堆上出现大量无法使用的零散空洞 |
| 内存对齐（Alignment） | 地址对齐到特定边界（通常 8/16 字节） |
| 内存池（Memory Pool） | 一次性分配大块，内部自行管理小块 |
| Valgrind | C 程序内存调试工具 |

## 延伸阅读

- [cppreference: Dynamic memory allocation (C)](https://en.cppreference.com/w/c/memory) — C 标准库的 malloc/free 参考
- [Valgrind 官方文档](http://valgrind.org/docs/manual/quickstart.html) — 快速上手指南
- [Beej's Guide to C: Dynamic Memory](https://beej.us/guide/bgc/) — 图解 malloc/free
- K&R《C 程序设计语言》第 8.7 章 — malloc 的经典描述
- [Memory Safety in C](https://owasp.org/www-community/vulnerabilities/Memory_Leak) — OWASP 安全备忘录

## 继续学习

你现在已经掌握了 C 语言中最核心的能力——手动内存管理。这是 C 的强大之处，也是它可怕的原因。

在下一章节中，我们将学习如何编写更复杂的程序结构：**文件输入输出**，学会读写文件、处理错误、以及缓冲区管理的进阶技巧。

> 💡 **提示**：检查你写过的所有使用 `malloc` 的代码——每个是否都有 `free`？每个 malloc 返回值是否都检查了 NULL？每次 free 后是否都 `ptr = NULL`？

<!-- 下一步: 文件 I/O (file_io.md) -->

[← 上一章：作用域与生命周期](./scope.md) | [下一章 → 文件 I/O](./file_io.md)
