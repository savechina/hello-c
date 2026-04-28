# 结构体内存布局（Struct Memory Layout）

`sizeof(struct)` 为什么总比我计算的大？

## 开篇故事

你买了一个书架，把一本书（1cm 厚）、一个花瓶（10cm 宽）、一支笔（1cm 长）依次放上去。结果发现书架用了 16cm 的空间，而不是 12cm。中间多了 3cm 的空隙。

这就是 padding（填充）。编译器在结构体字段之间和末尾插入了空隙，让每个字段都对齐到 CPU 最舒服的地址上。

我第一次遇到这个问题时，以为编译器有 bug。后来才知道，这不是 bug——这是 CPU 的"对齐偏好"。`int32_t` 一定要住在 4 的倍数地址上，`int64_t` 一定要住在 8 的倍数地址上。编译器不会商量的，它直接帮你排好间距。

> "padding 不是浪费，而是 CPU 的效率税。"

## 本章适合谁

- 已经学过 [结构体与函数](./struct_functions.md)，知道传结构体的基本概念
- 被 `sizeof` 搞糊涂了的 C 学习者
- 想理解底层内存布局的系统程序员 / 嵌入式开发者

## 你会学到什么

- 内存对齐（Alignment）规则
- 编译器 padding 的位置和原因
- 字段排列顺序对 `sizeof` 的影响
- `__attribute__((packed))` 消除 padding
- `_Alignof` 检查类型对齐要求
- 嵌套结构体的内存布局推导

## 前置要求

- 完成 [结构体与函数](./struct_functions.md) 章节
- 知道 `int8_t` = 1 字节、`int32_t` = 4 字节、`int64_t` = 8 字节

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

struct Bad { char a; int32_t b; char c; };
struct Good { int32_t b; char a; char c; };

int main(void) {
    printf("Bad:  sizeof = %zu\n", sizeof(struct Bad));   /* 12 */
    printf("Good: sizeof = %zu\n", sizeof(struct Good));  /* 8 */
    return 0;
}
```

明明两组字段的数据之和都是 6 字节（1+4+1），`sizeof` 却不同。Bad 有 6 字节 padding，Good 只有 2 字节。

## 原理解析

### 1. 对齐规则

CPU 读取内存不是按字节读的，而是按"字"（word）读。在 64 位机器上，一个字通常是 8 字节。

**黄金规则**：类型大小为 N 字节 → 地址必须是 N 的倍数。

| 类型 | 大小 | 对齐要求 |
|------|------|---------|
| `int8_t` / `char` | 1 字节 | 任意地址 |
| `int16_t` | 2 字节 | 2 的倍数地址 |
| `int32_t` | 4 字节 | 4 的倍数地址 |
| `int64_t` | 8 字节 | 8 的倍数地址 |

### 2. Padding 在哪里

```
struct Bad { char a; int32_t b; char c; };

ASCII 内存布局:

偏移:     0      1    2    3      4         5         6      7    8    9   10   11
        ┌─────┐ ┌───────────────────────────┐ ┌─────┐ ┌──────────────────────┐
字段:    │  a  │ │        PADDING            │ │      b      │ │  c  │ │      PADDING      │
        └─────┘ └───────────────────────────┘ └─────────────┘ └─────┘ └──────────────────────┘
大小:      1B          3B (凑到 4)          4B          1B          3B (凑到 4 的倍数)

总计: 12 bytes
```

推导：
- offset 0: `a` 放任意地址 ✅
- offset 1-3: **PADDING**！`b` 必须放 4 的倍数地址 → 跳到 offset 4
- offset 4-7: `b` ✅
- offset 8: `c` ✅
- offset 9-11: **PADDING**！struct 总大小必须是最大对齐（4）的倍数 → 凑到 12

### 3. 字段顺序优化

```
struct Good { int32_t b; char a; char c; };
偏移:  0   1   2   3   4   5  6  7
      ┌─────────────┐ ┌───┐ ┌───┐ ┌──┐
      │      b      │ │ a │ │ c │pad│
      └─────────────┘ └───┘ └───┘ └──┘
      4 bytes         1B    1B   2B

总计: 8 bytes (比 Bad 省 4 字节!)
```

**技巧**：按大小从大到小排列字段：`int64_t` → `int32_t` → `int16_t` → `int8_t`

### 4. 64 位类型的 padding

```
struct WithInt64 { char tag; int64_t value; };

  offset 0: tag (1B)
  offset 1-7: 7 bytes PADDING (value 要 8 对齐)
  offset 8-15: value (8B)
  Total: 16 bytes

[ tag:1B ][ padding:7B ][ value:8B ] = 16B
```

小 `char` 在大 `int64_t` 前面会浪费 7 字节。

### 5. `__attribute__((packed))`

```c
struct __attribute__((packed)) Packed {
    char a;
    int32_t b;
    char c;
};
/* sizeof = 6 — 完全没有 padding！ */
```

代价：CPU 需要额外的内存读取周期来读取不对齐的 `b`。ARM 等架构上可能直接硬件异常。

### 6. 嵌套结构体的 layout

```
struct Inner { int32_t a; char b; };      /* 8 bytes */
struct Outer { char x; struct Inner i; }; /* ? */

  Outer.x 放 offset 0
  Inner 的对齐 = 4 (最大成员 int32_t)
  所以 Inner 从 offset 4 开始 (3 bytes padding)
  Inner 占 8 bytes (offset 4-11)
  Total: 12 bytes
```

嵌套结构体把内层的对齐要求传递给外层。

## 常见错误（Error-First）

### ❌ 错误 1: 假设 sizeof = 字段和

```c
struct S { char c; int32_t i; };  /* 1+4 = 5 字节？NO → 8 字节 */
```

✅ **修复**: 永远用 `sizeof` 和 `offsetof` 确认

```c
#include <stddef.h>
printf("sizeof: %" PRIdMAX "\n", (intmax_t)sizeof(struct S));
printf("offsetof(c): %" PRIdMAX "\n", (intmax_t)offsetof(struct S, c));
printf("offsetof(i): %" PRIdMAX "\n", (intmax_t)offsetof(struct S, i));
```

### ❌ 错误 2: 网络传输直接 `send(&struct, sizeof)`

```c
struct NetPkt { uint8_t ver; uint32_t len; };
send(sock, &pkt, sizeof(pkt));  /* ❌ padding 也被发出去了！ */
```

不同的编译器/平台 padding 可能不同，对端解析错位。

✅ **修复**: 手动序列化为连续字节

```c
uint8_t buf[5];
buf[0] = pkt.ver;
buf[1] = (pkt.len >> 24) & 0xFF;
buf[2] = (pkt.len >> 16) & 0xFF;
buf[3] = (pkt.len >> 8) & 0xFF;
buf[4] = pkt.len & 0xFF;
```

### ❌ 错误 3: 依赖 packed 做跨平台二进制

```c
struct __attribute__((packed)) Pkt;
/* ❌ packed 在不同编译器上的行为不保证一致 */
```

✅ **修复**: 跨平台二进制协议做手动序列化

## 动手练习

<details>
<summary>🟢 入门: 预测 sizeof</summary>

不运行代码，预测：
```c
struct A { char a; char b; int32_t c; };  /* ? */
struct B { int32_t c; char a; char b; };  /* ? */
struct C { char a; int32_t c; char b; };  /* ? */
```

答案: A=8, B=8, C=12

</details>

<details>
<summary>🟡 中级: 手动计算 padding</summary>

用 `offsetof` 打印每个字段的偏移，手动推算 padding 位置。

</details>

## 故障排查（FAQ）

<details>
<summary>Q: `struct { char a; int64_t b; }` 为什么是 16 字节？</summary>

`int64_t` 要 8 字节对齐。offset 0 放 `char a`（1B），跳到 offset 8 放 `b`（8B），总大小凑到 8 的倍数 = 16。

```
[ a:1B ][ padding:7B ][ b:8B ] = 16B
```

</details>

<details>
<summary>Q: packed 真的会让程序变慢吗？</summary>

x86 上影响不大（不崩溃，但可能需要多次内存访问）。ARM 上可能硬件异常。嵌入式开发中谨慎使用。

</details>

## 知识扩展

### C11 `<stdalign.h>`

```c
#include <stdalign.h>
printf("%zu\n", alignof(int32_t));  /* 4 */
printf("%zu\n", alignof(int64_t));  /* 8 */

struct Aligned {
    _Alignas(16) int32_t data[4];  /* 强制 16 字节对齐 */
};
```

### C 标准对齐保证

C 标准要求：struct 的起始地址必须对齐到其最大成员的对齐要求，总大小必须是最大对齐的倍数。

## 小结

- `sizeof(struct)` ≠ 字段和——编译器插入 padding 满足对齐
- 字段排列顺序影响大小——大字段在前更紧凑
- `__attribute__((packed))` 消除 padding 但有性能代价
- 嵌套结构体的对齐 = 内层最大对齐
- 永远用 `sizeof` 和 `offsetof` 确认布局，不要靠猜

## 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| 对齐 | Alignment | 数据地址必须是其大小的倍数 |
| 填充 | Padding | 编译器插入的无用字节 |
| sizeof | sizeof | 编译时求值，返回类型字节数 |
| offsetof | offsetof | 返回成员在 struct 中的偏移 |
| aligned | `_Alignas` (C11) | 强制指定对齐 |
| packed | `__attribute__((packed))` | GCC/Clang 消除 padding |

## 延伸阅读

- [cppreference: Alignment](https://en.cppreference.com/w/c/language/alignment)
- C11 标准 §6.2.8: Alignment
- [Beej's Guide: Structures and Alignment](https://beej.us/guide/bgc/)

## 继续学习

| 方向 | 链接 |
|------|------|
| 上一章 → | [结构体与函数](./struct_functions.md) |
| 下一章 → | [typedef](./typedef.md) — 类型别名和函数指针 typedef |
