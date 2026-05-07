# 结构体内存布局（Struct Memory Layout）

`sizeof(struct)` 为什么总比我计算的大？

## 开篇故事

想象你搬家打包，把一个杯子、一本厚书、一支笔依次塞进纸箱。杯子和书之间有空隙，书和笔之间也有空隙。这些空隙不是浪费——它们让物品不会因为紧贴而互相挤压变形。

编译器在 struct 里做的事情差不多。它给每个字段之间塞 padding（填充字节），让每个字段都"对齐"到 CPU 最舒服的位置上。`int32_t` 喜欢待在 4 的倍数地址上，`int64_t` 要 8 的倍数。编译器不会问你喜欢不喜欢，它直接帮你把间距排好。

```c
struct A { char c; int32_t i; };   // 1 + 3(padding) + 4 = 8 字节
struct B { int32_t i; char c; };   // 4 + 1 + 3(padding) = 8 字节
// 字段一样，顺序不同，sizeof 都是 8
```

第一次遇到这个问题的人会以为编译器出了 bug。它没有——它只是在遵循 CPU 的对齐规则。理解了这一点，你就不会再惊讶于 `sizeof` 永远比你想的大。

> "padding 不是浪费，而是 CPU 的效率税。"

## 本章适合谁

- 已经学过[结构体基础](./structs.md)，知道 struct 怎么用
- 好奇 `sizeof` 为什么算出来比预料大的 C 学习者
- 想了解底层内存布局的嵌入式 / 系统程序员

## 你会学到什么

- 内存对齐（Alignment）规则
- 编译器 padding 的位置和原因
- 字段排列顺序对 `sizeof` 的影响
- `__attribute__((packed))` 消除 padding 及其代价
- 位域（Bit Fields）的用法与限制
- 嵌套结构体的内存布局推导
- `offsetof` 宏检查布局
- C struct 与 Python dict 的内存对比
- 网络传输中的序列化问题

## 前置要求

- 完成[结构体基础](./structs.md)章节
- 知道 `int8_t` = 1 字节、`int32_t` = 4 字节、`int64_t` = 8 字节

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

struct Compact {
    int32_t a;    // 4 bytes
    int32_t b;    // 4 bytes
};

struct Gappy {
    char    a;    // 1 byte
    int32_t b;    // 4 bytes (需要 4 字节对齐)
    char    c;    // 1 byte
};

int main(void) {
    printf("Compact: sizeof = %zu\n", sizeof(struct Compact));  // 8
    printf("Gappy:   sizeof = %zu\n", sizeof(struct Gappy));    // 12 (!)
    return 0;
}
```

明明 `Gappy` 只有 1+4+1 = 6 字节的真实数据，`sizeof` 却是 12。中间有 6 字节是 padding。

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

### 2. 内存对齐（Alignment）规则

CPU 读取内存时，不是按字节一个个读的，而是按"字"（word）读取的。在 64 位机器上，一个字通常是 8 字节。

**黄金规则**：类型大小为 N 字节 → 地址必须是 N 的倍数。

| 类型 | 大小 | 对齐要求 |
|------|------|---------|
| `int8_t` / `char` | 1 字节 | 任意地址 |
| `int16_t` | 2 字节 | 2 的倍数地址 |
| `int32_t` | 4 字节 | 4 的倍数地址 |
| `int64_t` | 8 字节 | 8 的倍数地址 |

### 3. Padding 位置图解

```
struct Gappy { char a; int32_t b; char c; };

内存布局:

偏移地址:  0    1    2    3    4    5    6    7    8    9   10   11
          ┌────┐ ┌──────────────────┐ ┌────────────┐ ┌────┐ ┌──────────┐
字段:     │  a │ │    PADDING       │ │      b     │ │  c │ │  PADDING │
          └────┘ └──────────────────┘ └────────────┘ └────┘ └──────────┘
大小:      1B       3 bytes (补到      4 bytes         1B      3 bytes
                      4 字节边界)                           (补齐到 8 的倍数)

总计: 12 bytes
```

推导：
- offset 0: `a` 放在任意地址 ✅
- offset 1-3: **PADDING**！`b` 必须放在 4 的倍数地址 → 跳到 offset 4
- offset 4-7: `b` 放在 [4] ✅（4 的倍数）
- offset 8: `c` 放在 [8] ✅
- offset 9-11: **PADDING**！struct 总大小必须是最大对齐数（4）的倍数 → 凑到 12

**64 位类型的 padding 更夸张**：

```
struct WithInt64 { char tag; int64_t value; };

[ tag:1B ][ padding:7B ][ value:8B ] = 16B
```

小 `char` 在大 `int64_t` 前面会浪费 7 字节。`sizeof` 不是 1+8=9，而是 16。

### 4. 字段顺序优化减少 padding

```c
// ❌ 浪费的排列: 12 bytes
struct Bad {
    char    a;     // 1 + 3 padding
    int32_t b;     // 4
    char    c;     // 1 + 3 padding
};

// ✅ 紧凑的排列: 8 bytes
struct Good {
    int32_t b;     // 4
    char    a;     // 1
    char    c;     // 1 + 2 padding
};

// 排序技巧: 按大小从大到小排列字段
//  int64_t → int32_t → int16_t → int8_t
```

```
struct Good 的布局:

偏移:  0   1   2   3   4   5  6  7
      ┌─────────────┐ ┌───┐ ┌───┐ ┌──┐
      │     b       │ │ a │ │ c │pad│
      └─────────────┘ └───┘ └───┘ └──┘
      4 bytes         1B    1B   2B

总计: 8 bytes（比 Bad 省 4 字节！）
```

### 5. `__attribute__((packed))` 去除 padding

```c
struct __attribute__((packed)) Packed {
    char    a;     // 1 byte
    int32_t b;     // 4 bytes (不再有 padding!)
    char    c;     // 1 byte
};
// sizeof = 6 字节! 紧凑了。

// 代价: CPU 可能需要多次内存访问来读取 b（性能下降）
// 某些架构（如 ARM）直接不允许不对齐访问 → 硬件异常
```

**什么时候用 packed**：
- 网络协议头部定义（与线缆上的字节流精确匹配）
- 文件格式解析（读二进制文件头）
- 内存极度受限的嵌入式场景

**什么时候不用**：
- 大多数应用层代码（性能 > 节省的几字节）
- 跨平台共享的数据结构（不同编译器的 packed 行为不完全一致）

### 6. 位域（Bit Fields）

当字段只需要很少的 bit 时，可以用位域节省空间：

```c
struct Flags {
    unsigned int enabled : 1;   // 只用 1 bit
    unsigned int level   : 3;   // 3 bits (0-7)
    unsigned int reserved: 4;   // 4 bits
};
// sizeof = 4 bytes（一个 unsigned int）
```

**位域的限制**：
- 不能取地址（`&flags.enabled` 是非法的）
- 不同编译器可能有不同的 bit 布局
- 不适合跨平台二进制协议

### 7. 嵌套结构体内存布局

嵌套结构体把内层的对齐要求传递给外层。

```
struct Inner { int32_t a; char b; };    /* 8 bytes */
struct Outer { char x; struct Inner i; };

  Outer.x 放 offset 0
  Inner 的对齐 = 4（最大成员 int32_t）
  所以 Inner 从 offset 4 开始（3 bytes padding）
  Inner 占 8 bytes（offset 4-11）
  Total: 12 bytes
```

规则：**嵌套结构体的起始偏移，对齐到其内部最大成员的对齐要求**。

### 8. `offsetof` 检查布局

`offsetof(type, member)` 是 `<stddef.h>` 中的宏，计算成员在结构体中的偏移：

```c
#include <stddef.h>

struct Example {
    char   a;
    int32_t b;
    char   c;
};

printf("offset of a: %zu\n", offsetof(struct Example, a));  // 0
printf("offset of b: %zu\n", offsetof(struct Example, b));  // 4
printf("offset of c: %zu\n", offsetof(struct Example, c));  // 8
printf("struct total: %zu\n", sizeof(struct Example));       // 12
```

通过 offset 差值可以算出 padding 位置：
- `offsetof(b) - (offsetof(a) + sizeof(a))` = 4 - (0 + 1) = **3 字节 padding** 在 a 和 b 之间

### 9. C struct vs Python dict 对比

```
C struct（紧凑、连续、编译器控制布局）:
  ┌──────────────────────────────────────┐
  │ struct Header {                      │
  │   uint8_t  version;  // 1 byte       │  ← offset 0
  │   uint32_t length;   // 4 bytes      │  ← offset 4 (padding +3)
  │   uint8_t  flag;     // 1 byte       │  ← offset 8
  │   /* padding x3 */                  │  ← offset 9,10,11
  │ }                                    │         Total: 12 bytes
  └──────────────────────────────────────┘

Python dict（分散、有哈希表 + 对象开销）:
  Total: ~324 bytes（比 C struct 大 27 倍！）
```

## 常见错误

### ❌ 错误 1: 假设 struct 大小等于字段大小之和

```c
struct S { char c; int32_t i; };
// ❌ 错误假设: sizeof(struct S) == 1 + 4 == 5
// ✅ 实际: sizeof(struct S) == 8
```

✅ **修正**: 永远用 `sizeof` 和 `offsetof` 确认布局，不要靠猜。

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

✅ **修正**: 手动序列化为连续字节

```c
uint8_t buf[5];
buf[0] = pkt.version;
buf[1] = (pkt.length >> 24) & 0xFF;
buf[2] = (pkt.length >> 16) & 0xFF;
buf[3] = (pkt.length >> 8) & 0xFF;
buf[4] = pkt.length & 0xFF;
send(sock, buf, 5);
```

### ❌ 错误 3: 依赖 packed 做跨平台二进制

```c
struct __attribute__((packed)) Pkt;
// ❌ packed 在不同编译器上行为不保证一致
```

✅ **修正**: 跨平台二进制协议做手动序列化（逐字节编码 + 网络字节序 htonl/ntohl）。

### ❌ 错误 4: 位域跨平台传输

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
- `sizeof(A)` = 8  (1+1+2 padding + 4)
- `sizeof(B)` = 8  (4+1+1+2 padding)
- `sizeof(C)` = 12 (1+3 padding + 4 + 1+3 padding)

</details>

<details>
<summary>🟡 中级: 用 packed 消除 padding</summary>

给 struct 加上 packed 属性，打印去掉 padding 后的大小。

```c
#include <stdio.h>
#include <stdint.h>

struct __attribute__((packed)) PackedGappy {
    char a;
    int32_t b;
    char c;
};

int main(void) {
    printf("PackedGappy sizeof = %zu（期待 6）\n", sizeof(struct PackedGappy));
    return 0;
}
```

**输出**: `PackedGappy sizeof = 6（期待 6）`

</details>

<details>
<summary>🔴 挑战: 手动验证对齐 + 推算 padding</summary>

用 `offsetof` 打印每个字段的偏移，手动推算 padding 的位置。

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

推算：a(1B, offset 0) → padding 3B → b(4B, offset 4) → c(1B, offset 8) → padding 3B = 12 总大小。

</details>

## 故障排查

<details>
<summary>Q: 为什么 `struct { char a; int64_t b; }` 的大小是 16 而不是 9？</summary>

`int64_t` 要求 8 字节对齐。offset 0 放了 `char a`（1 字节），接下来 `int64_t b` 必须放在 8 的倍数地址，所以中间有 7 字节 padding。总大小还要补齐到最大对齐（8）的倍数：1 + 7 + 8 = 16。

```
[ a:1B ][ padding:7B ][ b:8B ] = 16B
```

</details>

<details>
<summary>Q: packed 会让程序变慢吗？</summary>

**会**。CPU 处理不对齐数据时，可能需要两次内存读取+合并。在 x86 上影响较小（不会崩溃），但在 ARM 等架构上可能触发硬件异常。嵌入式开发中谨慎使用。

</details>

<details>
<summary>Q: `offsetof` 是什么？怎么用？</summary>

`offsetof(type, member)` 是 `<stddef.h>` 中的宏，返回成员在结构体中的偏移（字节）。它是编译期求值的，不产生运行时开销。

```c
#include <stddef.h>
offsetof(struct S, field)  // 返回 field 距离 struct 起点的字节偏移
```

常用于：序列化、调试布局、实现容器数据结构（如 Linux 内核的 `container_of` 宏）。

</details>

<details>
<summary>Q: 结构体可以包含自己吗？（递归定义）</summary>

不能直接包含，但可以包含指向自己的指针。

```c
// ❌ 不行——无限递归大小
struct Node { struct Node next; };  // 编译错误！

// ✅ 可以——指针大小固定（8 bytes on 64-bit）
struct Node { int32_t data; struct Node *next; };
```

</details>

## 知识扩展

### C11 `_Alignof` 与 `_Alignas`

```c
#include <stdalign.h>

printf("alignof(int32_t) = %zu\n", alignof(int32_t));  // 4
printf("alignof(int64_t) = %zu\n", alignof(int64_t));  // 8

// _Alignas 强制指定对齐（通常用于 SIMD 或 DMA 缓冲区）
struct Aligned {
    _Alignas(16) int32_t data[4];  // 强制 16 字节对齐
};
// sizeof(struct Aligned) = 16（而不是 4*4=16，但多了一个对齐保证）
```

### C 标准规定的对齐保证

C 标准要求编译器保证：**struct 的起始地址对齐到其最大成员的对齐要求**。

```c
struct { char c; int32_t i; };
// 最大对齐 = 4 (int32_t)
// 所以整个 struct 必须放在 4 的倍数地址上
// 总大小必须是最大对齐的倍数 → 8
```

### 网络编程中的序列化建议

跨平台传输结构体时，padding 是最大的隐患。不同架构、不同编译器的对齐规则不同。

```
发送端 (x86, GCC):   [ ver:1B | pad:3B | len:4B ] = 8 bytes
接收端 (ARM, Clang): [ ver:1B | pad:3B | len:4B ] = 8 bytes  // 碰巧一样
但如果编译器或 packed 属性不同，布局就全乱了
```

**推荐做法**：用固定宽度的整数类型（`uint8_t` / `uint32_t`）+ 手动序列化 + 网络字节序。

## 小结

这一章我发现：

- `sizeof(struct)` ≠ 各字段大小之和——编译器插入 padding 满足对齐
- 字段排列顺序影响 struct 大小——大字段在前更紧凑
- `__attribute__((packed))` 可以消除 padding，但有性能代价
- 位域可以进一步压缩空间，但不可移植
- 嵌套结构体把内层对齐要求传递给外层
- 永远用 `sizeof` 和 `offsetof` 确认布局，不要靠猜
- 跨平台传输结构体要做手动序列化
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
| alignas | `_Alignas` (C11) | 强制指定对齐方式 |
| alignof | `_Alignof` (C11) | 查询类型的对齐要求 |

## 延伸阅读

- [cppreference: Alignment](https://en.cppreference.com/w/c/language/alignment)
- [cppreference: Bit fields](https://en.cppreference.com/w/c/language/bit_field)
- [Beej's Guide: Structures and Alignment](https://beej.us/guide/bgc/output/html/multipage/structures.html)
- C11 标准 §6.2.8: Alignment
- [The Lost Art of Structure Packing](http://www.catb.org/esr/structure-packing/) — 经典长文

## 继续学习

| 下一步 | 方向 |
|--------|------|
| 复习 ← | [结构体基础](./structs.md) — struct 定义、初始化、成员访问 |
| 下一章 → | [typedef](./typedef.md) — 类型别名 |
| 深入 → | [联合体](./unions.md) — 共享内存的 struct |
| 应用 → | [网络编程（后续章节）] — struct 序列化与网络字节序 |
