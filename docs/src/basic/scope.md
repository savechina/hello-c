# 作用域与生命周期（Scope & Lifetime）

> "变量就像人一样：出生在某些地方，活在特定的范围内，然后消失在某个时刻。" —— 我发现

## 开篇故事

想象你走进一栋大楼。每扇门进去都是一个独立的房间——客厅的家具不在卧室里，厨房的刀叉不在书房里。你在 A 房间里声明的东西，B 房间看不到，也碰不到。当你离开一个房间，里面的一切就自动归还给大楼，不再属于你。

作用域就是 C 语言里的「房间」。用 `{ }` 划出一片空间，在里面声明的变量只在这个空间里可见。出了 `{ }`，变量就消失了——像离开房间时门被锁上，里面的东西你再也拿不到。

```c
{
    int x = 10;      // x 在「这个房间」里
    printf("%d\n", x);  // ✅ 看得到
}
// printf("%d\n", x); // ❌ 门已经关上了
```

最常见的陷阱，就是试图从门外拿走房间里的东西——比如返回一个局部变量的地址。变量已经「离开房间」了，指针却还指着那片空间。编译器不一定拦你，但结果可能是随机的。

> "变量在哪个房间声明，就在哪个房间可见。门关了，东西就没了。"

## 本章适合谁

- 遇到过"偶尔正常偶尔崩溃"的 bug，怀疑是内存问题的人
- 用过 `static` 但说不清它到底做了什么的人
- 想理解 `extern` 关键字在跨文件编程中作用的人
- 对 Python/Rust/Go 等有垃圾回收的语言有了解，想对比 C 的手动内存管理

## 你会学到什么

- 作用域的四个层次：块作用域、函数作用域、文件作用域、全局作用域
- `static` 关键字的双重身份：函数内 vs 文件级
- `extern` 跨文件链接的机制
- 局部变量与全局变量的生命周期
- Dangling pointer（悬垂指针）与 use-after-free
- 为什么"返回局部变量地址"是未定义行为
- 安全的作用域管理模式

## 前置要求

- 了解变量的声明与初始化
- 理解函数调用与返回
- 掌握指针的基本概念（`*` 取内容、`&` 取地址）

> 如果还没学指针，建议先看「指针运算」章节。

## 第一个例子：块作用域

```c
#include <stdio.h>

int main(void) {
    int x = 10;
    printf("外层: x = %d\n", x);

    {
        int x = 20;   /* 新的 x，遮蔽外层的 x */
        printf("内层: x = %d\n", x);
    }
    /* 内层的 x 已经消失了 */
    printf("回到外层: x = %d\n", x);
    return 0;
}
```

运行结果：

```
外层: x = 10
内层: x = 20
回到外层: x = 10
```

内层的 `x` 在 `{ }` 结束后就"死"了，外层的 `x` 不受影响。这正是作用域的力量。

## 原理解析

### 1. 块作用域（Block Scope）

在 C 语言中，任何一个 `{ }` 代码块都创建了一个新的作用域。在块内声明的变量只在该块内可见：

```c
{
    int a = 1;
    if (a > 0) {
        int b = 2;  /* b 只在 if 块内可见 */
    }
    /* printf("%d", b); ❌ b 不存在了！ */
}
```

**ASCII 栈图——变量的进出**：

```
栈帧（Stack Frame）

内存地址 ↑
┌────────────────┐
│ main 的局部变量 │ ← main 作用域开始
│   x = 10       │
│                │
├────────────────┤
│ { } 块的局部变量 │ ← 进入 { }，新的"架子"
│   x = 20       │   这个 x 在栈帧的较高位置
│                │
├────────────────┤ ← } 关闭，架子抽走
│                │   内部 x 被销毁
│   x = 10       │ ← 恢复外层的 x
│                │
└────────────────┘ ← main 作用域结束，整个栈帧弹出
```

**我的理解**：把栈帧想象成一层层抽屉——`{ }` 打开时放一个新抽屉，`}` 关闭时把抽屉抽走。抽屉里的东西随抽屉一起消失。

### 2. 变量遮蔽（Shadowing）

内层可以声明与外层同名的变量，这叫"遮蔽"（Shadowing）：

```c
int count = 10;
{
    int count = 5;  /* 遮蔽外层的 count */
    printf("%d\n", count);  /* 输出 5 */
}
printf("%d\n", count);  /* 输出 10 */
```

**我的建议**：虽然 C 允许遮蔽，但大多数风格指南**不推荐**这样做——它容易让人混淆，降低代码可读性。

### 3. 函数作用域（Function Scope）

函数内的参数和局部变量只在函数体内可见：

```c
void greet(const char *name) {  /* name 是参数，作用域在函数内 */
    int len = 0;                /* len 也是函数局部变量 */
    /* ... */
}
/* name 和 len 在这里都不存在 */
```

函数作用域还有一个特殊的"标签作用域"（Label Scope）——`goto` 的标签在整个函数内可见：

```c
void example(void) {
    goto end;
    int x = 10;  /* 注意：跳过初始化是合法的，但不推荐 */
    end:
    printf("done\n");
}
```

### 4. 文件作用域（File Scope）

在函数外部声明的变量/函数，作用域是整个当前源文件：

```c
/* file.c */
int file_counter = 0;  /* 文件级变量，本文件所有函数可见 */

static int secret = 42; /* 文件级 + static：只有本文件可见 */

void func_a(void) {
    file_counter++;  /* 可以访问 */
    secret++;        /* 可以访问 */
}

void func_b(void) {
    file_counter++;  /* 可以访问 */
}
```

### 5. 全局作用域（External Linkage）

在函数外声明且**没有** `static` 的变量/函数，具有**外部链接**（external linkage），其他源文件可以通过 `extern` 声明来访问：

```c
/* module_a.c */
int shared_data = 100;  /* 外部链接：其他文件可访问 */

/* module_b.c */
extern int shared_data;  /* 声明：shared_data 在别处定义 */

void use_shared(void) {
    printf("shared_data = %d\n", shared_data);
}
```

**对比表**：

| 链接类型 | 关键字 | 可见范围 | 可被其他文件引用 |
|---------|--------|---------|----------------|
| 无链接（None） | 函数内局部变量 | 当前 `{ }` | ❌ |
| 内部链接（Internal） | `static` | 当前文件 | ❌ |
| 外部链接（External） | 无 / `extern` | 整个程序 | ✅ |

### 6. static 的双重身份

`static` 在 C 语言中有两种完全不同的含义，取决于它出现在哪里：

**6a. 函数内部的 static：延长生命周期**

```c
void counter(void) {
    static int count = 0;  /* 只初始化一次，程序结束后才销毁 */
    count++;
    printf("called %d times\n", count);
}

/* 第1次调用: called 1 times
   第2次调用: called 2 times
   ... */
```

对比普通局部变量：

```
┌──────────────────────┐  时间线
│ 局部变量: count = 0  │ ← 函数返回 → 销毁
│                      │ ← 下次调用 → 重新创建为 0
├──────────────────────┤
│ static 变量: count   │ ← 函数返回 → 保留值
│         ↑ 存活       │ ← 下次调用 → 保持上次值
└──────────────────────┘
```

**我的理解**：函数内的 `static` = "活到程序结束，但只能在这个函数里看到"。

**6b. 文件级的 static：限制可见性**

```c
/* utils.c */
static void helper(void) {  /* 只有 utils.c 内部能调用 */
    /* ... */
}

void public_api(void) {     /* 其他文件可以通过 extern 调用 */
    helper();
}
```

这是模块化的基础——用 `static` 隐藏实现细节。

### 7. ASCII 内存布局全览

C 程序在内存中的分布：

```
内存地址 ↑

┌──────────────────────────┐
│  栈 (Stack)              │ ← 局部变量、函数参数
│  auto 变量               │    生命周期 = 块/函数
│                          │
├──────────────────────────┤
│                          │
│  堆 (Heap)               │ ← malloc/calloc 分配
│  动态内存                │    生命周期 = 手动控制
│                          │
├──────────────────────────┤
│  已初始化数据段          │ ← 全局变量、static 变量
│  .data                   │    (有初值的)
├──────────────────────────┤
│  未初始化数据段          │ ← 全局 = 0 的变量
│  .bss                    │    (零初始化的)
├──────────────────────────┤
│  只读数据段              │ ← 字符串字面量
│  .rodata                 │    ("hello")
├──────────────────────────┤
│  代码段                  │ ← 函数指令
│  .text                   │    (只读, 可执行)
└──────────────────────────┘
```

**生命周期对照**：
- 栈变量 → 离开作用域即销毁
- 堆变量 → `free()` 才销毁
- `.data`/`.bss` 变量 → 程序结束时销毁

## 常见错误（Error-First）

### ❌ 错误 1：返回局部变量的地址

```c
int *get_pointer(void) {
    int x = 42;
    return &x;  /* ❌ x 是局部变量，函数返回后 x 不存在 */
}

int main(void) {
    int *p = get_pointer();
    printf("%d\n", *p);  /* ❌ 未定义行为！（Undefined Behavior） */
    return 0;
}
```

`x` 存储在栈上，函数返回后这片栈空间被回收。`p` 指向的内存可能已被其他数据覆盖。

✅ **修复方式 1**：用 `static` 局部变量

```c
int *get_pointer(void) {
    static int x = 42;  /* static: 存储在 .data 段，生命周期 = 整个程序 */
    return &x;
}
```

✅ **修复方式 2**：用 `malloc` 动态分配

```c
int *get_pointer(void) {
    int *x = malloc(sizeof(int));
    *x = 42;
    return x;  /* 堆内存，函数返回后仍然有效 */
}
/* 注意：调用者必须 free() */
```

✅ **修复方式 3**：让调用者分配

```c
void get_value(int *result) {
    *result = 42;  /* 直接写入调用者提供的空间 */
}
```

### ❌ 错误 2：Dangling Pointer（悬垂指针）

```c
int *dangling = NULL;

void create_array(void) {
    int arr[10] = {0};
    dangling = arr;  /* 记录指针 */
}
/* arr 已在函数返回时销毁，dangling 成为"悬垂指针" */

void use_it(void) {
    printf("%d\n", dangling[0]);  /* ❌ 未定义行为！可能崩溃 */
}
```

✅ **修复**：永远不要持有指向已销毁对象的指针。如果必须跨函数传递，用堆分配或让调用者管理内存。

### ❌ 错误 3：Use-After-Free

```c
#include <stdlib.h>

int main(void) {
    int *p = malloc(sizeof(int) * 10);
    p[0] = 42;

    free(p);        /* p 已被释放 */
    p[0] = 99;      /* ❌ 写入已释放的内存！ */
    printf("%d\n", p[0]);  /* ❌ 读取已释放的内存！ */

    return 0;
}
```

✅ **修复**：`free()` 后立即将指针置为 `NULL`：

```c
free(p);
p = NULL;  /* 防止意外再次使用 */
/* p[0] = 99; ← 现在会立刻崩溃（段错误），比静默损坏好 */
```

### ❌ 错误 4：`extern` 声明与实际类型不一致

```c
/* a.c */
int data = 100;

/* b.c */
extern double data;  /* ❌ 声明为 double，实际是 int */

void wrong(void) {
    printf("%f\n", data);  /* ❌ 以 double 方式读取 int → 垃圾值！ */
}
```

✅ **修复**：用头文件统一管理 `extern` 声明，不要在各处重复写。

## 动手练习

### 🟢 练习 1：观察变量生命周期

```c
/* 写一个函数，声明一个 static int 计数器
   每次调用加 1 并打印。在另一个函数中调用它 5 次
   验证计数器的值在调用间保持 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

void tick(void) {
    static int count = 0;
    count++;
    printf("tick #%d\n", count);
}

int main(void) {
    for (int i = 0; i < 5; i++) {
        tick();
    }
    return 0;
}
/* 输出: tick #1, tick #2, ..., tick #5 */
```
</details>

### 🟡 练习 2：文件级 static 的模块化

```c
/* 写一个文件 util.c：
   - static 函数 helper() 只做内部辅助
   - 公开函数 public_func() 调用 helper()
   另一个文件 main.c 只能调用 public_func()，不能调用 helper() */
```

<details>
<summary>点击查看答案</summary>

```c
/* util.c */
static int helper(int x) {
    return x * 2;
}
int public_func(int x) {
    return helper(x) + 1;
}

/* main.c */
#include <stdio.h>
extern int public_func(int x);
/* extern int helper(int x); ← 链接错误：helper 是 static */

int main(void) {
    printf("%d\n", public_func(10));  /* 输出 21 */
    return 0;
}
```
</details>

## 与 Python/Rust 对比

| 特性 | C | Python | Rust |
|------|---|--------|------|
| 管理方式 | 手动（程序员） | 自动（GC） | 编译期（所有权系统） |
| 离开作用域后 | 栈变量自动销毁 | 引用计数归零后 GC 回收 | 自动调用 `drop` |
| 跨函数返回指针 | ⚠️ 必须用堆或 static | ✅ 对象始终在堆上 | 必须用 `Box` 或引用+生命周期 |
| Use-After-Free | ✅ 可能发生（undefined behavior） | ❌ 不可能（GC 保护） | ❌ 不可能（编译器拒绝） |
| 全局变量 | `extern` 或单文件 `static` | `global` 关键字 | `static` + 内部可变性 |

**我的理解**：C 把内存管理的权力交给程序员——这意味着更大的灵活性和性能，但也意味着更大的责任。Python/Rust 用各自的机制把这类错误消灭在摇篮里，C 则需要你自己做安全检查。

## 故障排查（FAQ）

### Q: 什么时候该用 `static` 局部变量？

当你希望在函数调用间保持状态，但又不想让外部直接访问这个变量时。典型场景：计数器、缓存、单例模式。但注意：`static` 局部变量**不是线程安全的**。

### Q: 全局变量和 `static` 文件级变量有什么区别？

| 对比 | 全局变量（无 static） | 文件级 static 变量 |
|------|----------------------|-------------------|
| 可见范围 | 整个程序（其他文件可 `extern`） | 仅当前文件 |
| 链接方式 | 外部链接（external） | 内部链接（internal） |
| 命名冲突 | 可能与其它文件冲突 | 不会冲突 |

**我的建议**：尽量用 `static` 限制全局变量的可见性，只在确实需要跨文件共享时才用 `extern`。

### Q: `free()` 后为什么还要 `p = NULL`？

`free(p)` 只释放了 `p` 指向的内存，但 `p` 变量本身仍然保存着那个地址。如果不置 `NULL`，`p` 就变成了悬垂指针，下次不小心 `*p` 写入或读取就触发 undefined behavior。置 `NULL` 后，`*p` 会立即触发段错误（crash），这比"静默损坏内存，在很后面才暴露出来"好调试得多。

### Q: 可以返回 `const char *` 字面量吗？

```c
const char *get_name(void) {
    return "hello";  /* ✅ 安全！字符串字面量存储在 .rodata 段 */
}
```

字符串字面量（"hello"）存储在只读数据段（`.rodata`），生命周期 = 整个程序。所以返回它完全安全。但返回指向栈上局部变量的指针就不行。

## 知识扩展（选学）

### C 标准的作用域规则

C11/C17 定义了四种作用域：
1. **块作用域**（Block scope）：从声明处到包含它的 `{ }` 结束
2. **函数作用域**（Function scope）：仅适用于 `goto` 标签（整个函数内）
3. **文件作用域**（File scope）：从声明处到文件结尾
4. **原型作用域**（Prototype scope）：函数原型中的参数名，仅在原型中有效

### TLS（Thread-Local Storage）C11 新增

`_Thread_local` 关键字：每个线程有自己的副本：

```c
#include <threads.h>

_Thread_local int thread_counter = 0;
/* 每个线程有自己的 thread_counter，互不影响 */
```

### `register` 关键字（过时）

C99 前曾用 `register int x` 建议编译器把变量放寄存器，但现代编译器比人更擅长寄存器分配。C17 中 `register` 已标记过期，仅作兼容性保留。

## 小结

祝贺！你已经掌握了 C 语言的作用域与生命周期。让我总结一下——

- **块作用域**：`{ }` 创建新作用域，变量在离块时销毁
- **变量遮蔽**：内层可以重复定义同名变量（但不推荐）
- **`static` 局部变量**：生命周期 = 整个程序，作用域 = 当前函数
- **`static` 文件级**：内部链接，防止跨文件暴露
- **`extern`**：声明在其他文件中定义的全局变量/函数
- **返回局部变量地址** = 未定义行为 → 用 `static`、`malloc` 或调用者分配
- **悬垂指针**：指向已销毁内存的指针 → 永远不要持有
- **Use-After-Free**：`free()` 后继续使用 → `free` 后立即 `p = NULL`
- **字符串字面量**安全返回 → 存储在 `.rodata` 段

> **我的理解**：C 的作用域规则可以浓缩成一句话——变量在哪里声明，就在哪里可见；变量在哪里创建，就在哪里销毁。理解了这个原则，90% 的内存安全问题都可以提前预见。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 作用域（Scope） | 变量可见的代码范围 |
| 生命周期（Lifetime） | 变量存在的时间范围 |
| 块作用域（Block Scope） | `{ }` 内的作用域 |
| 变量遮蔽（Shadowing） | 内层变量隐藏外层同名变量 |
| 链接（Linkage） | 符号在不同文件间的可见性 |
| 外部链接（External Linkage） | 可被其他文件 `extern` 引用 |
| 内部链接（Internal Linkage） | 仅当前文件可见（`static`） |
| 悬垂指针（Dangling Pointer） | 指向已销毁内存的指针 |
| Use-After-Free | 释放后仍访问该内存 |
| 自动变量（Auto Variable） | 栈上分配，离开作用域自动销毁 |
| 静态变量（Static Variable） | 程序运行期间始终存在 |
| 堆（Heap） | 动态分配的内存区域 |
| 栈（Stack） | 自动管理的作用域内存区域 |

## 延伸阅读

- [cppreference: Scope and Linkage (C)](https://en.cppreference.com/w/c/language/scope)
- [cppreference: Lifetime (C)](https://en.cppreference.com/w/c/language/lifetime)
- [Beej's Guide to C: Static and Global Variables](https://beej.us/guide/bgc/)
- K&R《C 程序设计语言》第 4.4、4.5 章

## 继续学习

你现在已经理解了 C 语言中变量的"生死循环"。在后续章节中，我们将进入更高级的话题——**动态内存管理**，学习如何精准控制堆上每一块内存的分配与释放。

> 💡 **提示**：检查你的代码里所有返回指针的函数，确保没有返回局部变量的地址。如果使用了 `malloc`，确认每条路径都有对应的 `free`。

[← 上一章：枚举与联合体](./enums.md) | [下一章 →](./next_chapter.md)
