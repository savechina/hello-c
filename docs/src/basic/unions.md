# 联合体（Unions）

所有成员共享同一块内存——写一个，毁所有。

## 开篇故事

想象你有一个万能储物柜。你可以把衣服放里面，也可以把书放里面，也可以把食物放里面。但不能同时放——每次只能放一种东西。你放食物进去，衣服就得拿出来。

联合体（union）就是 C 的万能储物柜。所有成员共享同一块内存空间，大小等于最大的成员。写入一个成员，其他成员的值就废了。

我第一次用 union 是在解析二进制协议——同一个数据包可能包含 4 字节整数、4 字节浮点数、或 8 字节字符串。用 union 可以不用 `memcpy`，直接以不同视角读取同一段内存。但我也因为 union 踩过一个坑——写了 float 用 int 读，输出了完全不对的垃圾值。union 不保护你，你必须自己知道当前存的是什么。

> "union 是'同一个柜子，什么都能往里放，但一次只能放一种东西'。"

## 本章适合谁

- 已经学了结构体，理解结构体各成员独立占有内存
- 想实现"多种类型之一"的数据结构
- 好奇 Rust 的 `enum`、Python 的 `Union` 在 C 中等价是什么

## 你会学到什么

- union 的基本概念和内存布局
- union 的 `sizeof` 和对齐
- Tagged Union（枚举 + 联合体）安全模式
- union 在协议解析中的应用
- union 的类型不安全陷阱

## 前置要求

- 完成 [typedef](./typedef.md) 章节
- 理解 [结构体](./struct_basics.md) 的基本使用

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

union Data {
    int32_t i;
    float   f;
    char    str[8];
};

int main(void) {
    union Data d;
    d.i = 42;
    printf("d.i = %d\n", (int)d.i);
    printf("d.f = %f  ← 垃圾值! (union 只保留最后一次写入)\n", (double)d.f);

    d.f = 3.14f;
    printf("d.f = %f\n", (double)d.f);
    printf("sizeof(union Data) = %zu\n", sizeof(union Data));
    return 0;
}
```

输出：
```
d.i = 42
d.f = 0.000000  ← 垃圾值! (union 只保留最后一次写入)
d.f = 3.140000
sizeof(union Data) = 8
```

关键：`sizeof(union) = 最大成员`（这里是 str[8] = 8 字节）。

## 原理解析

### 1. union 的内存布局

```
union Data { int32_t i; float f; char str[8]; };

内存布局（所有成员从 offset 0 开始）:

偏移:    0    1    2    3    4    5    6    7
        ┌────────────────────────────────────────┐
字段:    │    i (4 bytes, 和 f/str 共享)        │
        │    f (4 bytes, 和 i/str 共享)          │
        │    str (8 bytes)                       │
        └────────────────────────────────────────┘

sizeof = 8 (最大成员 str 的大小)
```

结构体是"并排摆放的柜子"，联合体是"同一个柜子"——所有成员重叠。

### 2. union 的大小和对齐

```
union Small { int8_t a; int16_t b; };  /* sizeof = 2 (最大是 int16_t)，对齐到 2 */
union Big   { int64_t x; char pad[3]; };  /* sizeof = 8 (最大是 int64_t)，对齐到 8 */
```

union 大小 = 最大成员大小，向上对齐到最严格对齐的成员。

### 3. 危险：类型不安全

```c
union Data d;
d.f = 3.14f;    /* 写入 float */
printf("%d\n", d.i);  /* ❌ 以 int 读 float 的 bit 模式 → 垃圾值 */
```

编译器**不检查**你读写是否同一种类型。这是 union 最大的陷阱——它把类型安全责任全交给程序员。

### 4. Tagged Union — 安全模式

```c
typedef enum { VAL_INT, VAL_FLOAT, VAL_STRING } ValueType;

struct TaggedValue {
    ValueType tag;  /* 标签：当前存什么类型 */
    union {
        int32_t i;
        float   f;
        char    str[16];
    } data;
};
```

```
struct TaggedValue 内存布局:
┌──────────┬────────────────────────┐
│ tag: 4B  │ union data: 16B (max)  │
│ (枚举)   │ (int/f/str 共享)       │
└──────────┴────────────────────────┘
总计: 可能需要 padding → 20-24 bytes
```

现在可以通过 `tag` 知道当前 `data` 里存的是什么类型：

```c
void print_val(const struct TaggedValue *tv) {
    switch (tv->tag) {
    case VAL_INT:    printf("%d\n", tv->data.i); break;
    case VAL_FLOAT:  printf("%.2f\n", tv->data.f); break;
    case VAL_STRING: printf("%s\n", tv->data.str); break;
    }
}
```

这正是 Rust `enum` 在底层的实现方式。Tagged Union 是 C 中实现"类型安全变体"的最佳实践。

### 5. Union 在协议解析中的应用

```c
union Payload {
    uint8_t  bytes[8];
    uint16_t words[4];
    uint32_t dwords[2];
};
```

同一个 8 字节内存，可以按字节/字/双字三种方式查看。这在网络协议、文件格式解析中非常常见。

## 常见错误（Error-First）

### ❌ 错误 1: 写 A 读 B

```c
union Data d;
d.i = 42;
printf("%.2f\n", d.f);  /* ❌ float 读出 garbage */
```

这是"类型双关"（Type Punning），C 标准的行为未定义（尽管 GCC/Clang 允许做 bit reinterpretation）。

✅ **修复**: 始终通过 `tag` 判断类型，或用 `memcpy` 做类型双关

```c
float f;
memcpy(&f, &d.i, sizeof(float));  /* 安全的 bit  reinterpretation */
```

### ❌ 错误 2: 忽略 union 大小影响 struct 大小

```c
struct Header {
    uint8_t version;
    union { uint32_t int_val; double dbl_val; uint8_t raw[16]; } payload;
};
/* sizeof = 1 + 7(pad) + 16 = 24 字节 — union 拉大了整体 */
```

## C struct vs C union 对比

| 特性 | C `struct` | C `union` |
|------|-----------|-----------|
| 内存布局 | 各成员独立占有空间 | 所有成员共享同一空间 |
| sizeof | ≥ 字段总大小 (+ padding) | = 最大成员 (+ padding) |
| 同时存多个值 | ✅ 可以 | ❌ 只能存最后一个写入 |
| 安全性 | 天然安全 | 需要 tag 保护 |
| 类比 | "多抽屉柜子" | "万能储物柜" |

## 动手练习

<details>
<summary>🟢 入门: union 写入读回</summary>

定义 `union Mixed { int32_t i; uint8_t bytes[4]; }`，写入 int，逐个打印字节。

</details>

<details>
<summary>🟡 中级: Tagged Union 计算器</summary>

定义 `struct Expr` (tag: INT/FLOAT/STRING)，写构造函数和打印函数。

</details>

## 故障排查（FAQ）

<details>
<summary>Q: 为什么 union 写入一个成员后其他成员的值乱了？</summary>

因为所有成员共享同一块内存。写入 `d.f` 覆盖了整个 union 空间，`d.i` 的 bit 模式随之改变。这是 union 的设计——它就是要共享内存。

</details>

<details>
<summary>Q: union 和 struct 哪个更安全？</summary>

Struct 天然安全——每个字段独立。Union 不安全——你需要外部机制（通常是 tag enum）来跟踪当前存的是什么类型。

</details>

## 知识扩展

### C11 匿名 union

```c
struct Variant {
    int tag;
    union { int i; double d; char *s; };
};
/* 可以直接 Variant.v.i，不需要 Variant.v.data.i */
```

### Type Punning 的替代方案

C 标准允许通过 `unsigned char*` 访问任何类型的字节表示：

```c
int x = 0x12345678;
unsigned char *bytes = (unsigned char *)&x;
/* bytes[0] = 0x78 (小端), bytes[1] = 0x56, ... */
```

## 小结

- union 所有成员共享内存，sizeof = 最大成员
- 写入 union 一个成员，其他成员值作废
- Tagged Union（enum + union）是安全的变体模式
- union 在协议解析中常用（不同粒度查看同一段内存）
- struct 是"多抽屉柜子"，union 是"万能储物柜"

## 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| 联合体 | Union | 所有成员共享内存的数据类型 |
| Tagged Union | Tagged Union | enum 标签 + union 数据 |
| Type Punning | Type Punning | 以不同类型解析同一内存 |
| Variant | Variant | 多种类型之一的数据结构 |
| 共享内存 | Shared Memory | union 的核心特性 |

## 延伸阅读

- [cppreference: Union types](https://en.cppreference.com/w/c/language/union)
- K&R《C 程序设计语言》§6.5: Unions
- [Rust enum vs C tagged union](https://doc.rust-lang.org/book/ch06-01-defining-an-enum.html)

## 继续学习

| 方向 | 链接 |
|------|------|
| 上一章 → | [typedef](./typedef.md) |
| 下一章 → | [作用域与生命周期](./scope.md) — 变量在哪里可见、何时销毁 |
