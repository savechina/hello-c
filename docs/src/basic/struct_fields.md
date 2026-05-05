# 结构体字段与内存布局（Struct Fields & Memory Layout）

上一章学会了 struct 的基本用法，但你可能有过这样的困惑——

```c
struct A { char c; int i; };   // 1 + 4 = 5 字节？
struct B { int i; char c; };   // 4 + 1 = 5 字节？
// 但 sizeof 告诉我们: sizeof(A) = 8, sizeof(B) = 8！
// 多出来的 3 个字节去哪了？
```

答案就是 **padding** —— 编译器偷偷在字段之间插入了填充字节，让每个字段都"对齐"到 CPU 最舒服的地址上。本章我们就把这个幕后黑手揪出来。

## 开篇故事

想象你搬家打包，把一个杯子、一本厚书、一支笔依次塞进纸箱。杯子和书之间有空隙，书和笔之间也有空隙。这些空隙不是浪费——它们让物品不会因为紧贴而互相挤压变形。

编译器在 struct 里做的事情差不多。它给每个字段之间塞 padding（填充字节），让每个字段都"对齐"到 CPU 最舒服的位置上。`int32_t` 喜欢待在 4 的倍数地址上，`int64_t` 要 8 的倍数。编译器不会问你喜欢不喜欢，它直接帮你把间距排好。

```c
struct A { char c; int32_t i; };   // 1 + 3(padding) + 4 = 8 字节
struct B { int i; char c; };       // 4 + 1 + 3(padding) = 8 字节
// 字段一样，顺序不同，sizeof 都是 8
```

第一次遇到这个问题的人会以为编译器出了 bug。它没有——它只是在遵循 CPU 的对齐规则。理解了这一点，你就不会再惊讶于 `sizeof` 永远比你想的大。

> "padding 不是浪费，而是 CPU 的效率税。"

## 本章适合谁

- 已经学过 [结构体基础](./structs.md)，知道 struct 怎么用
- 好奇 `sizeof` 为什么算出来比预料大的 C 学习者
- 想了解底层内存布局的嵌入式/系统程序员

## 你会学到什么

- 结构体字段逐个访问的细节
- struct padding（填充字节）与 alignment（内存对齐）
- `sizeof` 在含 padding 的结构体上的表现
- `__attribute__((packed))` 的作用与代价
- 位域（Bit Fields）的用法
- 嵌套结构体的字段访问
- 内存布局的 ASCII 可视化

## 前置要求

- 完成 [结构体基础](./structs.md) 章节
- 了解 C 语言基本数据类型的大小（`int32_t` = 4 字节等）

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

struct Compact {
    int32_t a;    // 4 bytes
    int32_t b;    // 4 bytes
};

struct Gappy {
    char   a;     // 1 byte
    int32_t b;    // 4 bytes (需要 4 字节对齐)
    char   c;     // 1 byte
};

int main(void) {
    printf("Compact: sizeof = %zu\n", sizeof(struct Compact));  // 8
    printf("Gappy:   sizeof = %zu\n", sizeof(struct Gappy));    // 12 (!)
    return 0;
}
```

明明 `Gappy` 只有 1+4+1 = 6 字节的真实数据，`sizeof` 却是 12！中间有 6 字节是 padding。

## 原理解析

### 1. 字段访问细节

```c
struct Student {
    char name[32];
    int32_t age;
    float score;
};

struct Student stu = {.name = "小明", .age = 20, .score = 95.0f};

// ✅ 正常访问
printf("%s\n", stu.name);   // 直接访问
stu.age = 21;               // 修改

// ✅ 通过指针访问
struct Student *ptr = &stu;
printf("%d\n", ptr->age);   // -> 等价于 (*ptr).age
```

### 2. 内存对齐（Alignment）与 Padding

CPU 读取内存时，不是按字节一个个读的，而是按"字"（word）读取的。在 64 位机器上，一个字通常是 8 字节。

**对齐规则**：
- `int8_t`（1 字节）—— 任意地址
- `int16_t`（2 字节）—— 地址必须是 2 的倍数
- `int32_t`（4 字节）—— 地址必须是 4 的倍数
- `int64_t`（8 字节）—— 地址必须是 8 的倍数

```
struct Gappy { char a; int32_t b; char c; };

ASCII 内存布局:

偏移地址:  0    1    2    3    4    5    6    7    8    9   10   11
         ┌────┐ ┌──────────────────┐ ┌────┐ ┌──────────┐ ┌──────────┐
字段:    │  a │ │    PADDING       │ │     b      │ │  c   │ │ PADDING  │
         └────┘ └──────────────────┘ └────────────┘ └────┘ └──────────┘
大小:     1B       3 bytes (补到   4 bytes         1B      3 bytes
                    4 字节边界)                          (补齐到 8 的倍数)

总计: 12 bytes

为什么这样？
- offset 0: a 放在 0 (任意地址都可以)
- offset 1-3: PADDING! b 必须放在 4 的倍数地址
- offset 4-7: b 放在 [4] (✅ 4 的倍数)
- offset 8: c 放在 [8]
- offset 9-11: PADDING! struct 总大小必须是最大对齐数（4）的倍数
```

### C struct vs Python dict 内存对比

```
C struct (紧凑、连续、编译器控制布局):
  ┌──────────────────────────────────────┐
  │ struct Header {                      │
  │   uint8_t  version;  // 1 byte       │  ← offset 0
  │   uint32_t length;   // 4 bytes      │  ← offset 4 (padding +3)
  │   uint8_t  flag;     // 1 byte       │  ← offset 8
  │   /* padding x3 */                  │  ← offset 9,10,11
  │ }                                    │         Total: 12 bytes
  └──────────────────────────────────────┘

Python dict (分散、有哈希表 + 对象开销):
  ┌──────────────────────────┐         ┌──────────────────┐
  │ dict object (240 bytes)  │────┐   │ int object (28 b)│
  │ hash table               │────┤   └──────────────────┘
  │  "version" → pointer─────┘    │
  │  "length"  → pointer─────┐   │
  │  "flag"    → pointer─────┐│   │
  └──────────────────────────┘│   │
                              └───┤  ┌──────────────────┐
                                  │ │ int object (28 b)│
                              ┌───┘  └──────────────────┘
                              │
                              └───┐  ┌──────────────────┐
                                  │ │ int object (28 b)│
                                  │  └──────────────────┘
                                  └──
  Total: ~324 bytes (比 C struct 大 27 倍!)
```

### 3. 优化字段顺序减少 padding

```c
// ❌ 浪费的排列: 12 bytes
struct Bad {
    char   a;     // 1 + 3 padding
    int32_t b;    // 4
    char   c;     // 1 + 3 padding
};

// ✅ 紧凑的排列: 8 bytes
struct Good {
    int32_t b;    // 4
    char   a;     // 1
    char   c;     // 1 + 2 padding
};

// 排序技巧: 按大小从大到小排列字段
//  int64_t → int32_t → int16_t → int8_t
```

### 4. `__attribute__((packed))` 去除 padding

```c
struct __attribute__((packed)) Packed {
    char   a;     // 1 byte
    int32_t b;    // 4 bytes (不再有 padding!)
    char   c;     // 1 byte
};
// sizeof = 6 字节! 紧凑了。

// 代价: CPU 可能需要多次内存访问来读取 b（性能下降）
// 某些架构（如 ARM）直接不允许不对齐访问 → 硬件异常
```

### 5. 位域（Bit Fields）

当字段只需要很少的 bit 时，可以用位域节省空间：

```c
struct Flags {
    unsigned int enabled : 1;   // 只用 1 bit
    unsigned int level   : 3;   // 3 bits (0-7)
    unsigned int reserved: 4;   // 4 bits
};
// sizeof = 4 bytes (一个 unsigned int)

// ⚠️ 位域的限制:
// - 不能取地址 (&flags.enabled 是非法的)
// - 不同编译器可能有不同的布局
// - 不适合跨平台二进制协议
```

## 常见错误

### ❌ 错误 1: 假设 struct 大小等于字段大小之和

```c
struct S { char c; int32_t i; };
// ❌ 错误假设: sizeof(struct S) == 1 + 4 == 5
// ✅ 实际: sizeof(struct S) == 8（有 3 字节 padding）
```

✅ **修正**: 用 `sizeof` 和 `offsetof` 检查实际布局

```c
#include <stddef.h>
printf("sizeof: %zu\n", sizeof(struct S));
printf("offset of c: %zu\n", offsetof(struct S, c));
printf("offset of i: %zu\n", offsetof(struct S, i));
```

### ❌ 错误 2: 网络传输时直接 `send(&struct, sizeof(struct))`

```c
struct NetPkt {
    uint8_t  version;
    uint32_t length;
};
// ❌ 直接发送 struct 会把 padding 也发过去！
// 对端用不同编译器或对齐规则时，字段就错位了
```

✅ **修正**: 序列化/打包为连续字节再发送

```c
uint8_t buf[5];
buf[0] = pkt.version;
// 手动编码 length
buf[1] = (pkt.length >> 24) & 0xFF;
buf[2] = (pkt.length >> 16) & 0xFF;
buf[3] = (pkt.length >> 8) & 0xFF;
buf[4] = pkt.length & 0xFF;
send(sock, buf, 5);
```

### ❌ 错误 3: 位域跨平台传输

```c
struct Flags f = {.enabled = 1, .level = 3};
send(sock, &f, sizeof(f));  // ❌ 位域的 bit 布局因编译器而异
```

✅ **修正**: 位域不要用于网络/文件等二进制协议。

## 动手练习

<details>
<summary>🟢 入门: 预测 sizeof 输出</summary>

不运行代码，预测下列 sizeof 结果，然后运行验证：

```c
struct A { char a; char b; int32_t c; };
struct B { int32_t c; char a; char b; };
struct C { char a; int32_t c; char b; };
```

**答案**:
- `sizeof(A)` = 8  (2+4, 1+1+2 padding + 4)
- `sizeof(B)` = 8  (4+1+1+2 padding)
- `sizeof(C)` = 12 (1+3 padding + 4 + 1+3 padding)

</details>

<details>
<summary>🟡 中级: 用 packed 消除 padding</summary>

给 `struct Gappy` 加上 packed 属性，打印去掉 padding 后的大小。

```c
#include <stdio.h>
#include <stdint.h>

struct __attribute__((packed)) PackedGappy {
    char a;
    int32_t b;
    char c;
};

int main(void) {
    printf("PackedGappy sizeof = %zu (期望 6)\n", sizeof(struct PackedGappy));
    return 0;
}
```

**输出**: `PackedGappy sizeof = 6 (期望 6)`

</details>

<details>
<summary>🔴 挑战: 手动实现内存对齐检查</summary>

编写代码验证 struct 字段的对齐：用 `offsetof` + `sizeof` 推算 padding 位置。

```c
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

struct Example {
    char   a;
    int32_t b;
    char   c;
};

int main(void) {
    printf("字段    offset  sizeof\n");
    printf("a       %-7zu  %zu\n",  offsetof(struct Example, a), sizeof(((struct Example*)0)->a));
    printf("b       %-7zu  %zu\n",  offsetof(struct Example, b), sizeof(((struct Example*)0)->b));
    printf("c       %-7zu  %zu\n",  offsetof(struct Example, c), sizeof(((struct Example*)0)->c));
    printf("struct total: %zu\n", sizeof(struct Example));
    return 0;
}
```

</details>

## 故障排查

<details>
<summary>Q: 为什么 `struct { char a; int64_t b; }` 的大小是 16 而不是 9？</summary>

A: `int64_t` 要求 8 字节对齐。偏移 0 放了 `char a`（1 字节），接下来 `int64_t b` 必须放在 8 的倍数地址，所以中间有 7 字节 padding。总大小还要补齐到最大对齐（8）的倍数：1 + 7 + 8 = 16。

```
[ a: 1B ] [ padding: 7B ] [ b: 8B ] = 16B
```

</details>

<details>
<summary>Q: packed 会让程序变慢吗？</summary>

A:**会**。CPU 处理不对齐数据时，可能需要两次内存读取+合并。在 x86 上影响较小（不会崩溃），但在 ARM 等架构上可能触发硬件异常。

</details>

<details>
<summary>Q: offsetof 是什么？</summary>

A: `offsetof(type, member)` 是 `<stddef.h>` 中的宏，计算成员在结构体中的偏移（字节）：

```c
offsetof(struct S, field)  // 返回 field 距离 struct 起点的字节偏移
```

</details>

## 知识扩展

### C 标准规定的对齐保证

C 标准要求编译器保证：**struct 的起始地址对齐到其最大成员的对齐要求**。

```
struct { char c; int32_t i; };
// 最大对齐 = 4 (int32_t)
// 所以整个 struct 必须放在 4 的倍数地址上
// 总大小必须是最⼤对齐的倍数 → 8
```

### `_Alignof` 与 `_Alignas`（C11）

```c
#include <stdalign.h>

printf("alignof(int32_t) = %zu\n", alignof(int32_t));  // 4

// C11 _Alignas 指定对齐
struct Aligned {
    _Alignas(16) int32_t data[4];  // 强制 16 字节对齐
};
```

## 小结

这一章我发现：

- `sizeof(struct)` ≠ 各字段大小之和——编译器会插入 padding 满足对齐
- 字段排列顺序影响 struct 大小——大字段在前更紧凑
- `__attribute__((packed))` 可以消除 padding，但有性能代价
- 位域可以进一步压缩空间，但不可移植
- `offsetof` 和 `sizeof` 是排查 struct 内存布局的好帮手
- C struct 比 Python dict 紧凑得多——系统编程的效率优势

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 对齐 | Alignment | 数据在内存中的地址必须是其大小的倍数 |
| 填充 | Padding | 编译器插入的无用字节，用于满足对齐要求 |
| sizeof | sizeof operator | 编译时求值，返回类型所占字节数 |
| offsetof | offsetof macro | 返回成员在 struct 中的字节偏移 |
| 位域 | Bit Field | 用冒号指定字段占用的 bit 数 |
| packed | `__attribute__((packed))` | GCC/Clang 扩展，消除 struct padding |
| 字边界 | Word Boundary | CPU 一次读取的数据块大小（通常 = 机器字长） |
| 序列化 | Serialization | 将 struct 转为连续字节流的过程 |

## 延伸阅读

- [cppreference: Alignment](https://en.cppreference.com/w/c/language/alignment)
- [cppreference: Bit fields](https://en.cppreference.com/w/c/language/bit_field)
- [Beej's Guide: Structures and Alignment](https://beej.us/guide/bgc/output/html/multipage/structures.html)
- C11 标准 §6.2.8: Alignment

## 继续学习

| 下一步 | 方向 |
|--------|------|
| 下一章 → | [指针](./pointers.md) — struct 指针与 `->` 运算符 |
| 复习 ← | [结构体基础](./structs.md) — struct 定义、初始化、` .` 访问 |
| 深入 → | [网络编程](../advance/network.md) — struct 的序列化与网络字节序 |
