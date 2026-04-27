# 回调函数与多态 (Callbacks & Polymorphism)

## 开篇故事

想象你点了一份外卖。你留下手机号，然后去做别的事。厨师做好了之后会**回电给你**——你不需要站在柜台干等。

这就是回调（Callback）：你把一个「联系方式」（函数指针）交给别人，等对方完成任务后主动调用它。`qsort` 把比较函数当回调、GUI 框架把 `onClick` 当回调、网络库把数据到达通知当回调——核心道理完全一样：你不用等，对方会来找你。

函数指针的真正力量不在于「调用一个已知函数」，而在于**把函数交给别人去调用**。

## 本章适合谁

- 已经掌握 [函数指针](./function_pointers.md) 的基本语法
- 用过 `qsort` 但想理解它为什么需要回调函数
- 好奇 C 语言如何实现面向对象的多态效果
- 想在 C 中实现事件驱动 / 观察者模式的开发者

## 你会学到什么

1. 回调函数的本质：把函数指针作为参数传入
2. `qsort` 回调示例——标准库如何设计回调接口
3. `void*` 泛型数据传递——回调的通用参数模式
4. 函数指针表实现多态——手动 vtable
5. 事件驱动回调模拟——发布-订阅模式

## 前置要求

- 完成 [函数指针](./function_pointers.md) 章节
- 理解 `void*`（无类型指针）的含义

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* 比较函数: 回调给 qsort 使用 */
int cmp_int(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);  /* 安全的三态比较 */
}

int main(void)
{
    int arr[] = { 5, 2, 8, 1, 9, 3 };
    size_t n = sizeof(arr) / sizeof(arr[0]);

    /* qsort 接受回调函数 */
    qsort(arr, n, sizeof(int), cmp_int);

    printf("排序后: ");
    for (size_t i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    return 0;
}
```

输出：
```
排序后: 1 2 3 5 8 9
```

这就是回调——你把 `cmp_int` 函数传给 `qsort`，`qsort` 在内部需要比较两个元素时调用它。

## 原理解析

### 1. 回调函数（Callback）

回调的本质就是：**函数 A 把函数指针传给函数 B，B 在需要时调用 A 的函数**。

```
调用者 (Caller)          被调者 (Callee)
    │                       │
    │  apply(3, 5, my_add) │
    ├─────────────────────────►│
    │                       │  内部: op(a, b)
    │                       │  └► 调用 my_add(3, 5)
    │  ◄── 8 ────────────────┤
    │                       │
```

```c
/* callee: 接受回调函数 */
int32_t compute(int32_t a, int32_t b, int32_t (*callback)(int32_t, int32_t))
{
    return callback(a, b);  /* 回调调用者的函数 */
}

int32_t add(int32_t a, int32_t b) { return a + b; }
int32_t mul(int32_t a, int32_t b) { return a * b; }

printf("%d\n", compute(3, 5, add));  /* 8 */
printf("%d\n", compute(3, 5, mul));  /* 15 */
```

### 2. qsort 回调详解

C 标准库的 `qsort` 是回调的经典示例：

```c
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
```

- `base`: 数组起始地址
- `nmemb`: 元素数量
- `size`: 每个元素的大小
- `compar`: **回调函数指针**，比较两个元素

比较函数约定：
- 返回 `< 0`：a < b（a 排前面）
- 返回 `0`：a == b
- 返回 `> 0`：a > b（b 排前面）

### 3. void* 泛型数据传递

回调函数经常需要接收额外的用户数据。C 的模式是 `void*`：

```c
/* 回调函数签名: 接受 void* 用户数据 */
typedef void (*visitor_t)(int32_t value, void *user_data);

/* 遍历数组并调用回调 */
void visit_array(const int32_t arr[], int32_t len,
                 visitor_t visit, void *user_data)
{
    for (int32_t i = 0; i < len; i++) {
        visit(arr[i], user_data);
    }
}

/* 回调实现: 统计总和 */
void sum_visitor(int32_t value, void *user_data)
{
    int32_t *sum = (int32_t *)user_data;
    *sum += value;
}
```

### 4. 函数指针表实现多态

C 没有虚拟函数，但可以用函数指针表模拟：

```
Python 多态              C 函数指针多态
─────────────────       ──────────────────────
class Shape:            struct Shape {
    def area(self): ...     double (*area)(Shape*);
                          }
class Circle(Shape):    struct Circle {
    def area(self): ...       Shape base;
                              double radius;
                          }
```

### 5. 事件驱动回调模拟

事件驱动模式：注册回调 → 事件触发 → 调用所有注册的回调

```
  ┌───────────┐     register(on_click)     ┌───────────┐
  │  Listener ├────────────────────────────►│  Emitter  │
  └───────────┘                              │           │
  ┌───────────┐     register(on_hover)       │  callbacks│
  │  Handler  ├────────────────────────────►│  [0]: on_ │
  └───────────┘         ...                  │       click│
                                            │  [1]: on_  │
                                             │       hover│
                                            └─────┬──────┘
                                                  │ emit("click")
                                            ┌─────▼──────┐
                                            │ 调用 callbacks│
                                            └────────────┘
```

### Python / JavaScript 回调对比

| 特性 | Python | JavaScript | C |
|------|--------|------------|---|
| 函数作参数 | `map(f, lst)` | `arr.map(f)` | `apply(..., f)` |
| Lambda | `lambda x: x*2` | `x => x*2` | 无，需具名函数 |
| 闭包 | ✅ 捕获外部变量 | ✅ 捕获外部变量 | ❌ 需 `void*` |
| 多态 | 鸭子类型 | 原型链 | 函数指针表 |
| 事件系统 | asyncio/globals | DOM Events | 手动回调表 |

C 回调的**劣势**：不支持闭包，无法捕获外部变量。必须通过 `void* user_data` 显式传递。
C 回调的**优势**：零开销，编译时类型安全，无需运行时虚拟机或垃圾回收。

## 常见错误

### ❌ 错误 1：回调函数签名不匹配

```c
/* ❌ qsort 要求: int (*)(const void *, const void *) */
int cmp(int a, int b) { return a - b; }  /* 类型不对! */

qsort(arr, n, sizeof(int), cmp);  /* 编译警告! */
```

编译器报错：
```
warning: incompatible pointer types passing 'int (int, int)' to parameter
  of type 'int (*)(const void *, const void *)'
```

修复：使用正确的签名 + `typedef`：

```c
typedef int (*comparator_t)(const void *, const void *);

int cmp_int(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

comparator_t cmp = cmp_int;
qsort(arr, n, sizeof(int), cmp);  /* ✅ 正确 */
```

### ❌ 错误 2：用减法做三态比较（危险!）

```c
/* ❌ 危险: 整数溢出可能导致错误结果 */
int cmp_bad(const void *a, const void *b)
{
    return *(int *)a - *(int *)b;  /* 如果 a=INT_MAX, b=-1, 溢出! */
}
```

修复：用分支安全的三态比较：
```c
/* ✅ 安全 */
int cmp_good(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}
```

### ❌ 错误 3：忘记 void* 强转

```c
/* ❌ C 中 void* 不能隐式解引用 */
int sum = *(void *)user_data;  /* 编译错误! */

/* ✅ 必须强转到具体类型 */
int sum = *(int *)user_data;
```

## 动手练习

### 🟢 入门：用 qsort 排序字符串数组

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmp_str(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

int main(void)
{
    const char *words[] = {"banana", "apple", "cherry", "date"};
    size_t n = sizeof(words) / sizeof(words[0]);

    qsort(words, n, sizeof(const char *), cmp_str);

    for (size_t i = 0; i < n; i++) {
        printf("%s\n", words[i]);
    }
    return 0;
}
```

</details>

### 🟡 中级：用 void* 实现泛型遍历器

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

typedef void (*element_visitor_t)(const void *elem, void *user_data);

void foreach(const void *arr, int32_t len, size_t elem_size,
             element_visitor_t visit, void *user_data)
{
    const char *p = (const char *)arr;
    for (int32_t i = 0; i < len; i++) {
        visit(p + i * elem_size, user_data);
    }
}

void print_elem(const void *elem, void *user_data)
{
    printf("%d ", *(const int *)elem);
}

void double_elem(const void *elem, void *user_data)
{
    int32_t *target = (int32_t *)elem;
    *target *= 2;
    int32_t *total = (int32_t *)user_data;
    *total += *target;
}

int main(void)
{
    int arr[] = { 1, 2, 3, 4, 5 };

    printf("原始: ");
    foreach(arr, 5, sizeof(int), print_elem, NULL);
    printf("\n");

    int32_t total = 0;
    foreach(arr, 5, sizeof(int), double_elem, &total);
    printf("翻倍后总和: %d\n", total);
    return 0;
}
```

</details>

### 🔴 挑战：实现简易观察者模式

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

#define MAX_OBSERVERS 8

typedef void (*observer_func_t)(int32_t event_code, void *user_data);

typedef struct {
    observer_func_t func;
    void *user_data;
} Observer;

typedef struct {
    Observer observers[MAX_OBSERVERS];
    int32_t count;
} EventManager;

void em_init(EventManager *em)
{
    em->count = 0;
}

int32_t em_register(EventManager *em, observer_func_t func,
                    void *user_data)
{
    if (em->count >= MAX_OBSERVERS) return -1;
    em->observers[em->count].func = func;
    em->observers[em->count].user_data = user_data;
    em->count++;
    return 0;
}

void em_notify(EventManager *em, int32_t event_code)
{
    for (int32_t i = 0; i < em->count; i++) {
        em->observers[i].func(event_code, em->observers[i].user_data);
    }
}

void on_click(int32_t code, void *data)
{
    printf("  [ClickHandler] received event %d\n", (int)code);
}

void on_log(int32_t code, void *data)
{
    printf("  [Logger] event %d logged\n", (int)code);
}

int main(void)
{
    EventManager em;
    em_init(&em);

    em_register(&em, on_click, NULL);
    em_register(&em, on_log, NULL);

    printf("触发 CLICK 事件:\n");
    em_notify(&em, 100);

    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q：为什么 C 标准库回调都用 `void*`？不安全吗？**

A：`void*` 是 C 实现泛型的唯一方式。它确实放弃了类型安全，但换来了通用性。调用时你必须手动 cast 到正确的类型——这就是「信任但验证」模式。

**Q：回调函数能修改原始数据吗？**

A：可以。如果回调接收的是 `void*`（不是 `const void*`），它可以修改指向的数据。`qsort` 的比较函数接收 `const void*` 因为它不应修改数组；我们的 `foreach` 接收 `void*` 因为它允许修改。

**Q：C 没有闭包, 回调怎么捕获外部变量？**

A：通过 `void* user_data` 参数手动传递。你需要把需要捕获的变量打包成 `struct`，然后把结构体指针传进去。这就是 C 的「闭包」。

## 知识扩展 (选学)

### C++ lambda vs C void*

```cpp
// C++: lambda + closure
int threshold = 50;
auto filter = [threshold](int x) { return x > threshold; };

// C 等价: 手动 struct 闭包
typedef struct { int threshold; } FilterCtx;
int filter_callback(int x, void *ud) {
    FilterCtx *ctx = (FilterCtx *)ud;
    return x > ctx->threshold;
}
```

### 链表遍历回调

```c
typedef struct Node {
    int data;
    struct Node *next;
} Node;

void foreach_node(Node *head,
                  void (*visit)(int data, void *ud),
                  void *ud)
{
    for (Node *cur = head; cur != NULL; cur = cur->next) {
        visit(cur->data, ud);
    }
}

/* 使用: 求和 */
void sum_visit(int data, void *ud) {
    *(int *)ud += data;
}
```

### Qt 的 Signal/Slot vs C 回调

Qt 的 Signal/Slot 本质上也是回调的变体——只是多了类型检查和连接管理。C 的函数指针回调是它的极简版。

## 小结

- **回调函数**：把函数指针作为参数，由被调用方在需要时 invoke
- `qsort` 是最经典的回调：传入比较函数，由排序算法调用
- `void* user_data` 是 C 的泛型参数模式——手动 cast 到具体类型
- **函数指针表** = C 模拟 OO 多态的方式（手动 vtable）
- **事件管理器** = 用函数指针数组实现观察者模式
- C 回调不支持闭包，需手动传 `void*` 捕获状态
- 安全的三态比较：`(a > b) - (a < b)`，不是 `a - b`

## 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| 回调函数 | Callback Function | 作为参数传入、由他人调用的函数 |
| 发布者-订阅 | Publish-Subscribe | 事件通知的设计模式 |
| void* 泛型 | Generic void* | 用无类型指针传递任意数据 |
| 多态 | Polymorphism | 同一种接口，不同行为实现 |
| vtable | Virtual Function Table | 虚函数表（C++ 中的实现方式） |
| 闭包 | Closure | 捕获外部变量的函数 |
| 观察者模式 | Observer Pattern | 注册监听 → 事件触发 → 通知回调 |
| 三态比较 | Three-way Comparison | 返回 <0 / 0 / >0 的比较 |

## 延伸阅读

- [cppreference: qsort](https://en.cppreference.com/w/c/algorithm/qsort) — 标准库回调
- [Beej's Guide: Callbacks](https://beej.us/guide/bgc/)
- [Design Patterns: Observer](https://refactoring.guru/design-patterns/observer)

**选择建议**：先用 `qsort` 熟悉回调，再尝试自己实现事件管理器。

## 继续学习

回调是 C 语言事件驱动和面向对象的基石。掌握了回调模式后，你可以：
- 深入 [Advance](../advance/) 章节，学习更复杂的 C 设计模式
- 尝试用 C 实现链表、树等数据结构中的回调遍历
- 阅读开源项目中 event loop / reactor 模式的实现

- [上一章](./function_pointers.md)：函数指针
- [下一章](../advance/)：进阶主题
