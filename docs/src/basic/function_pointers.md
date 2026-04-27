# 函数指针 (Function Pointers)

## 开篇故事

想象你有一台电视遥控器。遥控器上的每一个按钮，并不「包含」换台的功能——它们只是**指向**电视内部不同的信号处理电路。按下「频道+」，遥控器告诉你：「去调那个函数」。

函数指针就是 C 语言里的遥控器按钮。它不保存代码本身，它保存的是**代码的地址**。当你通过函数指针调用时，程序跳转到那个地址去执行。就像遥控器指向电视内部的电路，函数指针指向程序的「入口」。

## 本章适合谁

- 已经理解普通指针（`int *p`、`char *s`）的基本概念
- 学过函数声明和调用，但对函数名是什么还不清楚
- 想理解「第一等函数」在 C 中如何模拟
- 被 `int (*fp)(int)` 和 `int *fp(int)` 搞混淆的初学者

## 你会学到什么

1. 函数指针的语法——`return_type (*name)(param_types)` 到底怎么读
2. 函数指针与函数名的关系——为什么 `func` 和 `&func` 等价
3. 函数指针数组（Dispatch Table / 分派表）
4. 将函数指针作为参数传递（C 语言中的「高阶函数」）
5. 将函数指针嵌入结构体（C 语言模拟 OOP 方法）
6. `typedef` 让函数指针可读——避免灾难性语法

## 前置要求

- 完成 [函数](./functions.md) 章节
- 完成 [指针基础](./pointers.md) 章节
- 理解类型（type）、声明（declaration）、定义（definition）的概念

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

int32_t add(int32_t a, int32_t b) {
    return a + b;
}

int main(void) {
    /* 声明一个函数指针: 指向 "返回 int32_t、接受两个 int32_t 参数" 的函数 */
    int32_t (*fp)(int32_t, int32_t) = &add;

    /* 通过指针调用函数 */
    int32_t result = fp(3, 5);  /* 等价于 add(3, 5) */
    printf("fp(3, 5) = %" PRId32 "\n", result);

    /* 也可以不用 &——函数名 decay 为指针 */
    fp = add;
    result = (*fp)(10, 20);  /* 显式解引用调用 */
    printf("(*fp)(10, 20) = %" PRId32 "\n", result);

    return 0;
}
```

输出：
```
fp(3, 5) = 8
(*fp)(10, 20) = 30
```

### 分步解析

1. `int32_t (*fp)(int32_t, int32_t)` — 声明 `fp` 是一个函数指针
2. `= &add` — 把 `add` 函数的地址赋给 `fp`（`&` 可以省略）
3. `fp(3, 5)` — 通过指针调用函数，等价于 `add(3, 5)`

## 原理解析

### 1. 函数指针语法：怎么读？

C 的声明语法是「顺时针螺旋规则」（Clockwise/Spiral Rule）。从变量名开始，向外螺旋阅读：

```
          ┌─ param types: (int32_t, int32_t)
          │
int32_t (*fp)(int32_t, int32_t)
  │  │    │
  │  │    └── fp is a
  │  └────── pointer to
  └───────── function returning int32_t
```

读出来就是：「`fp` 是一个指针，指向一个函数，该函数接受两个 `int32_t` 参数，返回 `int32_t`。」

对比迷惑性写法：
```c
int32_t *fp(int32_t, int32_t);   /* ❌ 这是函数声明！fp 返回 int32_t* */
int32_t (*fp)(int32_t, int32_t); /* ✅ 这才是函数指针 */
```

括号的优先级决定了一切。

### 2. 函数名 vs. 函数地址

在 C 语言中，函数名本身就是一个「衰减为指针」的值：

```c
int32_t add(int32_t a, int32_t b) { return a + b; }

/* 以下四种写法完全等价 */
int32_t (*fp1)(int32_t, int32_t) = add;    /* 函数名 decay */
int32_t (*fp2)(int32_t, int32_t) = &add;   /* 显式取地址 */
fp1(3, 5);        /* 隐式解引用调用 */
(*fp1)(3, 5);     /* 显式解引用调用 */
```

这类似于数组名 decay 为 `&arr[0]`。但建议统一用 `&func` 取地址 + 隐式调用风格，语义最清晰。

### 3. ASCII 内存示意图：函数指针存的是什么？

函数指针存储的是**可执行代码的内存地址**（代码段 / text segment）：

```
内存地址空间:
┌─────────────────────────────────────────┐
│  .text (代码段)                          │
│                                         │
│  0x00401000 │ add():          ← fp 指向这里 │
│             │   push rbp      │              │
│             │   mov eax, edi  │              │
│             │   add eax, esi  │              │
│             │   pop rbp       │              │
│             │   ret           │              │
│  0x00401010 │ sub():          │              │
│             │   ...           │              │
├─────────────────────────────────────────┤
│  .data / .bss (栈/数据段)                │
│                                         │
│  fp (在栈上): 0x00401000 ←───────────────┘
│  (8 bytes on 64-bit)                    │
└─────────────────────────────────────────┘
```

### 4. 函数指针数组（Dispatch Table）

函数指针可以组成数组，实现简单的「分派表」模式：

```c
typedef int32_t (*binary_op_t)(int32_t, int32_t);

int32_t add(int32_t a, int32_t b) { return a + b; }
int32_t sub(int32_t a, int32_t b) { return a - b; }
int32_t mul(int32_t a, int32_t b) { return a * b; }

static const binary_op_t ops[] = { add, sub, mul };

/* ops[0](3, 5) → add(3, 5) → 8 */
/* ops[1](10, 3) → sub(10, 3) → 7 */
```

### 5. 函数指针作为参数（高阶函数）

把函数指针传给另一个函数，就是 C 中的「高阶函数」：

```c
/* apply 接受一个二元操作函数指针 */
int32_t apply(int32_t a, int32_t b, int32_t (*op)(int32_t, int32_t)) {
    return op(a, b);  /* 调用传入的函数 */
}

printf("%d\n", apply(3, 5, add));  /* 8 */
printf("%d\n", apply(10, 3, sub)); /* 7 */
```

### 6. Struct + 函数指针（模拟 OOP 方法）

C 没有类和方法，但可以用结构体 + 函数指针模拟：

```c
typedef struct {
    double x, y;
    double (*length)(const struct Point2D *);  /* 方法 */
} Point2D;

static double point_length(const Point2D *p) {
    return sqrt(p->x * p->x + p->y * p->y);
}

Point2D pt = { .x = 3.0, .y = 4.0, .length = point_length };
printf("length = %.1f\n", pt.length(&pt));  /* 5.0 */
```

### Python / JavaScript 对比

| 特性 | Python | JavaScript | C 函数指针 |
|------|--------|------------|-----------|
| 函数是一等公民 | `f = add` | `f = add` | `fp = add` |
| 调用 | `f(3, 5)` | `f(3, 5)` | `fp(3, 5)` |
| 传入函数 | `map(add, list)` | `arr.map(add)` | `apply(3, 5, add)` |
| 类型安全 | 运行时 | 无 | 编译时类型检查 |
| 闭包 | ✅ 支持 | ✅ 支持 | ❌ 需配合 `void*` |
| 内存开销 | 对象包装 | 闭包对象 | 8 bytes（指针） |

C 函数指针的优势是**零开销**——它就是一个 8 字节的函数地址，没有任何运行时包装。劣势是**不支持闭包**（无法捕获外部变量），除非配合 `void* user_data` 传参。

## 常见错误

### ❌ 错误 1：括号优先级搞错

```c
/* ❌ 声明了一个返回 int* 的函数，不是函数指针！ */
int *fp(int);

/* ✅ 正确的函数指针声明 */
int (*fp)(int);
```

编译器报错：
```
warning: incompatible pointer types initializing 'int *(int)' (aka ...) with &func
```

修复：加上括号确保 `*fp` 先绑定：
```c
int (*fp)(int) = &func;  /* ✅ fp 是指针，指向函数 */
```

### ❌ 错误 2：函数指针类型不匹配

```c
int add(int a, int b) { return a + b; }

/* ❌ 指针类型不匹配：返回 double vs 返回 int */
double (*fp)(int, int) = add;  /* 编译警告/错误！ */
```

修复：用 `typedef` 明确类型，避免手写错误：
```c
typedef int (*binary_add_t)(int, int);
binary_add_t fp = add;  /* ✅ 类型一致 */
```

### ❌ 错误 3：解引用语法错误

```c
int32_t (*fp)(int32_t) = &square;

/* ❌ 优先级错误：*(fp(3)) 先调用 fp(3)，再解引用返回值 */
int32_t result = *(fp(3));

/* ✅ 两种都是正确的 */
int32_t result1 = fp(3);       /* 隐式解引用 */
int32_t result2 = (*fp)(3);    /* 显式解引用 */
```

## 动手练习

### 🟢 入门：声明并使用函数指针

定义 `int32_t multiply(int32_t a, int32_t b)`，用函数指针调用它。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

int32_t multiply(int32_t a, int32_t b) {
    return a * b;
}

int main(void) {
    int32_t (*fp)(int32_t, int32_t) = &multiply;
    printf("6 × 7 = %" PRId32 "\n", fp(6, 7));
    return 0;
}
```

</details>

### 🟡 中级：函数指针数组实现简易计算器

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

int32_t add(int32_t a, int32_t b) { return a + b; }
int32_t sub(int32_t a, int32_t b) { return a - b; }
int32_t mul(int32_t a, int32_t b) { return a * b; }

typedef int32_t (*op_func_t)(int32_t, int32_t);

int main(void) {
    const op_func_t ops[3] = { add, sub, mul };
    const char *names[3] = { "+", "-", "*" };

    int32_t x = 10, y = 3;
    for (int32_t i = 0; i < 3; i++) {
        printf("%d %s %d = %" PRId32 "\n",
               (int)x, names[i], (int)y, ops[i](x, y));
    }
    return 0;
}
```

</details>

### 🔴 挑战：用 typedef 简化复杂函数指针

声明一个函数指针：指向 `void (*)(const char *, int)`，即「接受 `const char*` 和 `int`，返回 `void`」的函数。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>

void logger(const char *msg, int level) {
    printf("[%d] %s\n", level, msg);
}

/* typedef 简化 */
typedef void (*log_callback_t)(const char *, int);

int main(void) {
    log_callback_t cb = logger;
    cb("hello", 1);
    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q：`fp(3, 5)` 和 `(*fp)(3, 5)` 有什么区别？**

A：没有区别。C 标准规定函数指针调用时自动解引用，两者完全等价。我推荐 `fp(3, 5)` 更简洁。

**Q：为什么不用 `typedef` 每次都想死？**

A：这正是我推荐的原因。函数指针类型签名极难手写，每次出错。`typedef int32_t (*binary_op_t)(int32_t, int32_t)` 让代码清晰得多。

**Q：函数指针能指向 lambda / 匿名函数吗？**

A：C17 不支持 lambda。GCC 扩展有嵌套函数和 lambda 表达式，但不可移植。如果需要闭包，用 `void* user_data` + 函数指针组合。

**Q：函数指针的大小是多少？**

A：与数据指针相同——32 位平台 4 字节，64 位平台 8 字节。`sizeof(fp)` 即可验证。

## 知识扩展 (选学)

### 函数指针强制转换

C 允许将函数指针转换为 `void*`（非标准但广泛支持），用于动态加载库：

```c
/* 不推荐但在 dlfcn.h 中常见 */
void *handle = dlopen("libfoo.so", RTLD_LAZY);
void *sym = dlsym(handle, "my_function");
/* 必须 cast 回正确的函数指针类型才能调用 */
typedef int (*func_t)(int);
func_t f = (func_t)sym;
```

### Function Pointer vs. Virtual Function (C++)

C++ 的虚函数表（vtable）本质上就是一张函数指针数组。C 结构体 + 函数指针是手动实现 vtable 的方式：

```
vtable (C++ 编译器自动生成):
┌──────────────────┐
│  vtable ptr ──→  │  Draw()     → Circle::Draw
│                  │  Resize()   → Circle::Resize
└──────────────────┘
```

用 C 结构体手动实现的就是本章第 6 个模式。

### Python Callable 对比表

| Python | 等价 C |
|--------|--------|
| `def f(x): return x*2` | `int f(int x) { return x*2; }` |
| `g = f` (赋值函数引用) | `int (*g)(int) = f;` |
| `g(5)` | `g(5)` |
| `map(f, [1,2,3])` | `for (... ) { arr[i] = f(arr[i]); }` |
| `lambda x: x*2` | 需定义具名函数 |

## 小结

- **函数指针**存储的是函数的入口地址，指向可执行代码
- 语法核心：`return_type (*name)(param_types)` — 括号改变优先级
- `func` 和 `&func` 等价（函数名 decay 为函数指针）
- `fp(args)` 和 `(*fp)(args)` 等价（调用时自动解引用）
- **函数指针数组**实现分派表（dispatch table）
- **函数指针作参数** = C 的高阶函数模式
- **结构体 + 函数指针** = C 模拟 OOP 方法的基石
- 始终用 `typedef` 给函数指针类型命名，避免语法灾难

## 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| 函数指针 | Function Pointer | 指向函数入口地址的指针 |
| 函数名衰减 | Function Decay | 函数名自动转换为函数指针 |
| Dispatch Table | 分派表 | 函数指针数组，按索引选择操作 |
| 高阶函数 | Higher-Order Function | 接受或返回函数的函数 |
| typedef | 类型别名 | 为复杂类型创建易读名称 |
| Callback | 回调 | 通过函数指针传入的函数 |
| vtable | 虚函数表 | C++ 虚函数调用的实现机制 |

## 延伸阅读

- [cppreference: Function pointers](https://en.cppreference.com/w/c/language/function)
- [The C Declarator Syntax — Clockwise/Spiral Rule](https://en.wikipedia.org/wiki/Spiral_rule)
- K&R《C 程序设计语言》§5.8 — 函数指针

**选择建议**：先理解语法再深入——cppreference 是权威参考，K&R 的解释更直觉。

## 继续学习

函数指针是回调函数的基础。掌握了函数指针的语法，下一章我们将学习如何用函数指针实现回调模式和模拟多态——这是 C 语言事件驱动和面向对象的根基。

- [上一章](./functions.md)：函数基础
- [下一章](./callbacks.md)：回调函数与多态
