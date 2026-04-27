# 枚举与联合体（Enums & Unions）

> "枚举是给整数起了名字，联合体是让多种类型共享同一块内存。" —— 我发现

## 开篇故事

我写第一个状态机的时候，用的是 `#define` 来定义状态：

```c
#define STATE_IDLE    0
#define STATE_RUNNING 1
#define STATE_PAUSED  2
#define STATE_STOPPED 3
```

代码里到处是 `if (state == STATE_RUNNING)`，看起来很清晰。直到有一天，函数签名变成了 `void set_state(int state)`——这意味着可以传入任何整数，`set_state(999)` 编译器完全不会报错！我调了一个下午才发现，某个地方传了未初始化的 `int` 当状态值。

后来我学到了 `enum`（枚举），才明白**枚举不是宏的替代品，而是一种独立的类型**。编译器能检查枚举的使用，虽然它仍然基于整数，但语义上已经截然不同了。

更让我惊喜的是，枚举和 `union`（联合体）组合后，可以实现类似 Rust 的"tagged union"（带标签联合体）——这是 C 语言里实现类型安全变体（variant）的标准模式。

本章我就带你搞懂 C 的枚举与联合体，让你的代码更安全、更高效。

## 本章适合谁

- 用过 `#define` 定义状态码，但踩过类型安全坑的人
- 想知道 C 语言怎么实现"多种类型之一"的数据结构
- 对 Rust `enum`、Python `Enum` 有了解，想对比 C 的枚举
- 想掌握 tagged union 模式的 C 学习者

## 你会学到什么

- `enum` 的定义、使用与底层原理
- 枚举与 `#define` 常量的对比与选择
- 枚举值的显式赋值与自动递增
- 将枚举作为函数参数和返回值
- Tagged Union 模式（枚举 + 联合体的组合）
- 枚举的边界验证与错误处理
- 实际应用：状态机、错误码、配置选项

## 前置要求

- 了解基本的 `int`、`float`、结构体（struct）概念
- 理解函数的参数传递与返回值
- 能编译运行 `.c` 文件

> 如果还没学结构体，建议先看「数据类型」章节。

## 第一个例子：用枚举定义星期

```c
#include <stdio.h>

enum Weekday { MON, TUE, WED, THU, FRI, SAT, SUN };

int main(void) {
    enum Weekday today = MON;
    printf("Today is day %d\n", today);  /* 输出: 0 */

    today = FRI;
    printf("Friday is day %d\n", today);  /* 输出: 4 */
    return 0;
}
```

运行结果：

```
Today is day 0
Friday is day 4
```

看起来枚举就是把整数起了个好看的名字？不完全是。让我深入解释。

## 原理解析

### 1. 枚举的本质：命名的整数常量

`enum` 定义了一个**枚举类型**，它里面的每个成员（enumerator）都被编译器分配了一个整数值，默认从 0 开始递增：

```c
enum Weekday { MON, TUE, WED, THU, FRI, SAT, SUN };
/*  等价于:
    MON = 0, TUE = 1, ..., SUN = 6
*/
```

**我的理解**：枚举是"带自我文档的整数"——`FRI` 比 `4` 更能表达意图，但底层仍然是整数运算。

### 2. 显式赋值

枚举成员可以显式指定值：

```c
enum Permission {
    READ    = 1,   /* 0b001 */
    WRITE   = 2,   /* 0b010 */
    EXECUTE = 4    /* 0b100 */
};
```

也可以省略后续值，编译器自动递增：

```c
enum ErrorCode {
    OK = 0,
    ERR_INVALID_ARG,   /* = 1 */
    ERR_NULL_PTR,      /* = 2 */
    ERR_TIMEOUT = 100, /* 重新指定 */
    ERR_IO           /* = 101 */
};
```

### 3. ASCII 内存布局

枚举在内存中的存储大小与 `int` 相同（通常是 4 字节）：

```
enum Color { RED, GREEN, BLUE };

变量: enum Color c = GREEN;

内存布局:
┌────────────────────────────────┐
│  0x00000001  (4 字节, int)    │
└────────────────────────────────┘
    enum 底层就是 int，RED=0, GREEN=1, BLUE=2

sizeof(enum Color) == sizeof(int)  ← 通常如此
```

**注意**：C 标准不强制规定枚举的大小，只要求它至少能容纳所有枚举值。实际实现中，大多数编译器用 `int`。

### 4. 与 Python / JavaScript 对比

| 特性 | C `enum` | Python `enum.Enum` | JavaScript 常量 |
|------|----------|-------------------|-----------------|
| 类型安全检查 | 弱（可隐式转 int） | 强（`Color.RED` 不是 int） | 无（就是普通变量） |
| 内存大小 | `sizeof(int)` | 对象实例（几百字节） | 普通数字/字符串 |
| 编译期检查 | 部分（赋值时检查） | 运行期检查 | 无 |
| 自增赋值 | ✅ 自动递增 | ❌ 需手动或用 `auto()` | ❌ 手动 |
| 位运算 | ✅ (`\|`、`&`) | ❌ | ❌ |

C 的枚举**偏底层**，它的设计哲学是"枚举是整数的语法糖"，但通过类型名称提供了**一定程度的语义约束**。

### 5. 枚举作为函数参数

```c
typedef enum Loglevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
} LogLevel;

void logger(LogLevel level, const char *msg) {
    const char *prefix;
    switch (level) {
        case LOG_DEBUG: prefix = "DEBUG"; break;
        case LOG_INFO:  prefix = "INFO";  break;
        case LOG_WARN:  prefix = "WARN";  break;
        case LOG_ERROR: prefix = "ERROR"; break;
    }
    printf("[%s] %s\n", prefix, msg);
}
```

**优势**：调用时只能传递枚举成员，比传递裸 `int` 更安全——虽然技术上仍然可以强转，但编译器会发出警告。

## 常见错误（Error-First）

### ❌ 错误 1：给枚举赋任意整数值

```c
enum Status { STATUS_OK, STATUS_FAIL };

enum Status s = (enum Status)99;  /* ❌ 编译可能不报错！但语义错误 */
/* 99 不是合法的 Status 值，但 C 允许这种隐式转换 */
```

这是 C 枚举的"弱点"——它基于 int，所以你可以赋任何 int 给它。

✅ **修复**：在关键位置添加验证：

```c
#include <stdbool.h>

bool is_valid_status(enum Status s) {
    return s == STATUS_OK || s == STATUS_FAIL;
}

void handle_status(enum Status s) {
    if (!is_valid_status(s)) {
        printf("Error: invalid status code %d\n", s);
        return;
    }
    /* 安全处理 */
}
```

**我的模式**：所有枚举参数在进入核心逻辑前，先用 `switch` + `default` 验证。

```c
void safe_handle(enum Status s) {
    switch (s) {
        case STATUS_OK:  do_ok(); break;
        case STATUS_FAIL: do_fail(); break;
        default:
            /* ❌ 捕获非法值 */
            fprintf(stderr, "Unknown status: %d\n", s);
            return;
    }
}
```

### ❌ 错误 2：枚举未覆盖所有值，switch 缺少 default

编译器 `-Wswitch` 可以提醒，但不是 `-Werror`：

```c
enum Color { RED, GREEN, BLUE };

void print_color(enum Color c) {
    switch (c) {
        case RED: printf("red\n"); break;
        case GREEN: printf("green\n"); break;
        /* 缺少 BLUE 和 default! */
    }
}
```

✅ **修复**：永远在 `switch` 枚举时加 `default`：

```c
void print_color(enum Color c) {
    switch (c) {
        case RED:   printf("red\n"); break;
        case GREEN: printf("green\n"); break;
        case BLUE:  printf("blue\n"); break;
        default:    printf("unknown(%d)\n", c); break;
    }
}
```

## 动手练习

### 🟢 练习 1：定义月份枚举并打印

```c
/* 定义 enum Month { JAN=1, FEB, ..., DEC }
   用 switch 打印中文月份名 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

enum Month { JAN = 1, FEB, MAR, APR, MAY, JUN,
             JUL, AUG, SEP, OCT, NOV, DEC };

void print_month(enum Month m) {
    switch (m) {
        case JAN: printf("一月"); break;
        case FEB: printf("二月"); break;
        case MAR: printf("三月"); break;
        case APR: printf("四月"); break;
        case MAY: printf("五月"); break;
        case JUN: printf("六月"); break;
        case JUL: printf("七月"); break;
        case AUG: printf("八月"); break;
        case SEP: printf("九月"); break;
        case OCT: printf("十月"); break;
        case NOV: printf("十一月"); break;
        case DEC: printf("十二月"); break;
        default: printf("未知"); break;
    }
}
```
</details>

### 🟡 练习 2：模拟简易状态机

```c
/* 定义灯的状态：OFF、DIM、BRIGHT
   实现 next_state() 函数：OFF → DIM → BRIGHT → OFF
   用 enum 参数和返回值 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

typedef enum LightState { LIGHT_OFF, LIGHT_DIM, LIGHT_BRIGHT } LightState;

LightState next_state(LightState current) {
    switch (current) {
        case LIGHT_OFF:   return LIGHT_DIM;
        case LIGHT_DIM:   return LIGHT_BRIGHT;
        case LIGHT_BRIGHT: return LIGHT_OFF;
        default:          return LIGHT_OFF;  /* safety */
    }
}

const char *state_name(LightState s) {
    switch (s) {
        case LIGHT_OFF:    return "OFF";
        case LIGHT_DIM:    return "DIM";
        case LIGHT_BRIGHT: return "BRIGHT";
        default:           return "UNKNOWN";
    }
}
```
</details>

## 联合体的奥秘

### 6. Union（联合体）基础

`union` 是一种特殊的数据类型，它的所有成员**共享同一块内存**。大小等于最大的成员：

```c
union Data {
    int i;
    double d;
    char str[16];
};

sizeof(union Data);  /* = 16 字节（str 最大）*/
```

```
内存布局:
┌────────────────────────────────────┐
│ i (4 bytes)                        │
│ d (8 bytes, overlaps with i)       │
│ str[16] (16 bytes, overlaps all)   │
│ ←── 同一块内存, 你写哪个就读哪个 ──→  │
└────────────────────────────────────┘
```

**我的理解**：结构体是"并排摆放的柜子"，联合体是"同一个柜子，什么都能往里放，但一次只能放一种东西"。

### 7. 危险：Union 类型不安全

```c
union Data u;
u.d = 3.14;    /* 写入 double */
printf("%d\n", u.i);  /* ❌ 以 int 读取 double → 垃圾值！*/
```

编译器**不会检查**你读写的是否是同一种类型——这就是联合体最大的陷阱。

## Tagged Union 模式

### 8. 枚举 + 联合体 = 类型安全的变体

要安全地使用 union，必须搭配一个枚举来标记当前存储的类型，这就是**Tagged Union**（带标签联合体）模式：

```c
#include <stdio.h>
#include <stdint.h>

enum ValueKind { VALUE_INT, VALUE_DOUBLE, VALUE_STRING };

struct Variant {
    enum ValueKind tag;        /* 标签：当前存储什么类型 */
    union {
        int32_t int_val;
        double  double_val;
        const char *str_val;
    } data;
};
```

```
struct Variant 内存布局:
┌──────────────┬────────────────────────┐
│ tag (enum)   │ data (union)            │
│ = 4 字节     │ = 最大成员大小 (如 8)   │
│ 记录类型     │ 实际存储的数据          │
└──────────────┴────────────────────────┘
```

现在可以安全地构造和访问：

```c
struct Variant make_int(int32_t v) {
    struct Variant var = { .tag = VALUE_INT, .data.int_val = v };
    return var;
}

struct Variant make_double(double v) {
    struct Variant var = { .tag = VALUE_DOUBLE, .data.double_val = v };
    return var;
}

void print_variant(const struct Variant *v) {
    switch (v->tag) {
        case VALUE_INT:
            printf("int: %d\n", v->data.int_val);
            break;
        case VALUE_DOUBLE:
            printf("double: %.2f\n", v->data.double_val);
            break;
        case VALUE_STRING:
            printf("string: %s\n", v->data.str_val);
            break;
        default:
            printf("unknown type\n");
            break;
    }
}
```

### 9. 实际应用：错误码模式

C 语言中常见的另一种枚举用法是错误码，配合返回值做 Error-First 风格：

```c
typedef enum Result {
    RESULT_OK = 0,
    RESULT_ERR_NULL,
    RESULT_ERR_IO,
    RESULT_ERR_TIMEOUT
} Result;

Result open_file(const char *path) {
    if (path == NULL) return RESULT_ERR_NULL;
    /* ... 模拟文件操作 ... */
    return RESULT_OK;
}
```

## 故障排查（FAQ）

### Q: `enum` 和 `#define` 到底该用哪个？

| 对比 | `enum` | `#define` |
|------|--------|-----------|
| 类型系统 | 属于枚举类型 | 无类型（文本替换） |
| 自动递增 | ✅ | ❌ 手动 |
| 调试器可见 | ✅（符号表中有名） | ❌（已被替换） |
| 取值范围 | 可以取任意 int（不严格） | 无限制 |
| 适用场景 | 状态码、选项列表、错误码 | 编译期常量、条件编译、宏替换 |

**我的建议**：定义一组相关常量时优先用 `enum`。只在需要"文本替换"（如宏函数）或条件编译（`#ifdef`）时用 `#define`。

### Q: enum 的大小一定是 sizeof(int) 吗？

**大多数时候是**，但 C 标准允许编译器优化。GCC 可以用 `-fshort-enums` 让枚举使用 1 字节或 2 字节（如果能容纳所有值）。跨平台编程时应使用 `sizeof()` 而非假设。

### Q: 可以用 enum 做位标志（bit flags）吗？

**可以**。用 2 的幂次赋值：

```c
typedef enum Permission {
    PERM_READ    = 1 << 0,   /* 0b001 */
    PERM_WRITE   = 1 << 1,   /* 0b010 */
    PERM_EXEC    = 1 << 2,   /* 0b100 */
} Permission;

Permission p = PERM_READ | PERM_WRITE;  /* 组合 */
/* 检查: if (p & PERM_WRITE) */
```

## 知识扩展（选学）

### X-Macro：枚举与字符串同步

X-Macro 是一种高级模式，让枚举值与字符串数组始终保持同步：

```c
#define COLOR_LIST \
    X(RED,   0)    \
    X(GREEN, 1)    \
    X(BLUE,  2)

/* 生成枚举 */
#define X(name, val) COLOR_##name = val,
enum ColorName { COLOR_LIST };
#undef X

/* 生成名称数组 */
#define X(name, val) #name,
const char *color_names[] = { COLOR_LIST };
#undef X

/* color_names[COLOR_RED] == "RED" */
```

这是 C 语言中一种"编译期元编程"技巧。

## 小结

祝贺！你已经掌握了 C 语言的枚举与联合体。让我总结一下——

- **`enum`** 是命名的整数常量，有类型但底层是 `int`
- **枚举 vs `#define`**：枚举更安全、可调试、支持自动递增
- **`union`** 的所有成员共享同一块内存，大小 = 最大成员
- **Tagged Union**（枚举 + 联合体）是 C 中实现类型安全变体的标准模式
- **枚举验证**：永远用 `switch` + `default` 覆盖所有枚举值，防止非法值
- **位标志**：枚举可以用位移值做组合操作
- 枚举的**边界**：C 允许给枚举赋任意 `int`，运行时需验证

> **我的理解**：枚举不是银弹——它提供了更好的表达力，但不像 Rust 的 `enum` 那样严格。理解它既是"命名的整数"也是"带标签的类型"，你就能在安全和效率之间找到最佳平衡。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 枚举（Enum） | 一组命名的整数常量 |
| 枚举成员（Enumerator） | 枚举中的每个命名值 |
| 联合体（Union） | 所有成员共享内存的数据类型 |
| Tagged Union | 枚举标签 + 联合体的组合模式 |
| Variant | 变体类型，可以存储多种类型之一 |
| Bit Flags | 用位运算组合枚举值 |
| Error-First | 用返回值传递错误码的编程风格 |
| X-Macro | 枚举与字符串同步的宏技巧 |

## 延伸阅读

- [cppreference: Enum types (C)](https://en.cppreference.com/w/c/language/enum)
- [cppreference: Union types (C)](https://en.cppreference.com/w/c/language/union)
- K&R《C 程序设计语言》第 1.6、6.5 章
- 《C Primer Plus》第 11 章：结构体和枚举

## 继续学习

枚举和联合体让你掌握了 C 语言中标识多种状态和共享内存的基础。下一章我们将深入学习**作用域与生命周期**——理解变量在哪里可见、什么时候创建、什么时候销毁，这将让你写出更安全的代码。

> 💡 **提示**：检查你的代码里所有 `#define` 定义的状态码，尝试替换为 `enum`。你会发现代码的可读性和安全性都提升了！

[← 上一章：指针运算](./pointer_arith.md) | [下一章：作用域与生命周期 →](./scope.md)
