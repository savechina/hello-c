# 位运算与内存操作 (Bitwise Operations & Memory Ops)

## 开篇故事

想象一堵墙上的开关面板：空调、电灯、风扇……每个开关独立控制一路电路，拧开空调不会影响到电灯。位运算就是编程世界的「开关面板」——每一个 bit 是一个独立的开关，你操作其中一位，其他位完全不受影响。

在操作系统权限、网络协议、嵌入式寄存器、数据库索引这些领域，位运算无处不在。它不是冷门的数学游戏，是底层编程的基本功。

和硬件对话的方式，就从控制一个 bit 开始。

## 本章适合谁

- 学过算术运算符, 想了解 C 语言底层操作能力的学习者
- 准备接触嵌入式/操作系统/网络编程的人
- 用过 Python 的 `&`/`|`/`^` 运算符, 想了解 C 语言细节的人
- 想要理解「权限位」,「标志位」等底层概念的人

## 你会学到什么

1. 位运算 AND/OR/XOR/NOT 的含义和用法
2. 左移 `<<` 和右移 `>>` 的语义
3. Bitmask 模式：设置/清除/翻转/检查位
4. Struct bit fields（位字段）
5. `memcpy` / `memmove` / `memset` 的区别与安全用法
6. Endianness（字节序）概念与检测
7. 实用模式：权限系统、字节打包/解包

## 前置要求

- 已掌握 [运算符与表达式](./operators.md)
- 理解二进制数的表示
- 了解 [指针基础](./pointers.md)（内存操作部分）

## 第一个例子

最简单的位运算 —— 用 AND 提取特定 bit：

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t flags = 0b10110101;

    /* 检查 bit 0 是否为 1 */
    if (flags & (1u << 0)) {
        printf("Bit 0 is set!\n");
    }

    /* 清除 bit 0, 其他位不变 */
    flags &= ~(1u << 0);
    printf("After clearing bit 0: 0x%02" PRIx8 "\n", flags);

    return 0;
}
```

输出：

```
Bit 0 is set!
After clearing bit 0: 0xb4
```

这段代码做了两件事：
- `flags & 1` 检查 bit 0 是否为 1
- `flags & ~1` 清除 bit 0, 其他位保持不变

## 原理解析

### 1. 基本位运算：AND / OR / XOR / NOT

C 语言提供 4 种按位逻辑运算：

| 运算符 | 名称 | 规则 | 示例 |
|--------|------|------|------|
| `&` | AND | 对应位都为 1 则结果 1 | `1100 & 1010 = 1000` |
| `\|` | OR | 有一方为 1 则结果 1 | `1100 \| 1010 = 1110` |
| `^` | XOR | 不同则 1, 相同则 0 | `1100 ^ 1010 = 0110` |
| `~` | NOT | 逐位取反 | `~1100 = 0011` |

```c
uint8_t a = 0b11001010;
uint8_t b = 0b10100101;

uint8_t c_and = a & b;  /* 0b10000000 */
uint8_t c_or  = a | b;  /* 0b11101111 */
uint8_t c_xor = a ^ b;  /* 0b01101111 */
uint8_t c_not = ~a;     /* 0b00110101 */
```

**ASCII 位图**：

```
┌──────────────────────────────────────────────────────────┐
│                    基本位运算                              │
│                                                          │
│  a =  1 1 0 0 1 0 1 0   (0xCA)                          │
│  b =  1 0 1 0 0 1 0 1   (0xA5)                          │
│  ─────────────────────                                   │
│                                                          │
│  a&b  1 0 0 0 0 0 0 0   (0x80) ← 都为 1 才得 1           │
│  a|b  1 1 1 0 1 1 1 1   (0xEF) ← 有 1 就得 1             │
│  a^b  0 1 1 0 1 1 1 1   (0x6F) ← 不同才得 1              │
│  ~a   0 0 1 1 0 1 0 1   (0x35) ← 逐位取反                │
│                                                          │
│  记忆口诀:                                                 │
│  AND → 掩码取位  (清零不需要的位)                           │
│  OR  → 设置标志  (把需要的位设为 1)                         │
│  XOR → 翻转位    (与 1 异或翻转, 与 0 异或保留)           │
│  NOT → 求补码    (所有位取反)                              │
└──────────────────────────────────────────────────────────┘
```
```

```
逐位操作前后对比:

  原始 a:    1 1 0 0 1 0 1 0   (0xCA)
  应用 AND: & 0 0 0 0 1 1 1 1   (掩码 0x0F, 取低 4 位)
  ─────────────────────────
  结果:       0 0 0 0 1 0 1 0   ← 高 4 位被清零, 低 4 位保留

  原始 a:    1 1 0 0 1 0 1 0   (0xCA)
  应用 OR:  | 0 0 0 0 1 1 1 1   (掩码 0x0F, 设置低 4 位)
  ─────────────────────────
  结果:       1 1 0 0 1 1 1 1   ← 低 4 位全变 1, 高 4 位不变

  原始 a:    1 1 0 0 1 0 1 0   (0xCA)
  应用 XOR: ^ 0 0 0 0 1 1 1 1   (掩码 0x0F, 翻转低 4 位)
  ─────────────────────────
  结果:       1 1 0 0 0 1 0 1   ← 低 4 位取反, 高 4 位不变

  原始 a:    1 1 0 0 1 0 1 0   (0xCA)
  左移 2:  <<          2
  ─────────────────────────
  结果:       0 0 1 0 1 0 0 0   ← 高 2 位丢失, 低 2 位补 0
              └─ 溢出丢失 ─┘
```

### 2. 移位运算：`<<` 和 `>>`

```c
uint32_t val = 0x00000001;

val << 8  → 0x00000100  (左移 8 位, 右侧补 0)
val << 16 → 0x00010000  (左移 16 位)
```

**右移有逻辑右移和算术右移之分**：
- 无符号数 (`uint32_t`)：右移补 0（逻辑右移）
- 有符号数 (`int32_t`)：通常是算术右移（补符号位，保持正负性），但具体定义依赖于实现

**❌ 常见错误：移位溢出（未定义行为）**

```c
uint32_t x = 1;
x << 32;  /* ❌ UB! 移位位数 >= 位宽 */
x << 33;  /* ❌ UB! */
```

```c
/* ✅ 修复: 移位前检查边界 */
int32_t shift = 32;
if (shift >= 0 && shift < 32) {
    result = x << shift;
} else {
    /* 跳过或报错 */
}
```

### 3. Bitmask 模式 —— 权限系统

Bitmask 是位运算最常见的用途 —— 用每一位表示一个开关：

```c
#define FLAG_READ    (1u << 0)  /* 0b00000001 */
#define FLAG_WRITE   (1u << 1)  /* 0b00000010 */
#define FLAG_EXECUTE (1u << 2)  /* 0b00000100 */
#define FLAG_DELETE  (1u << 3)  /* 0b00001000 */

uint32_t flags = 0;

/* 设置位 (添加权限) */
flags |= FLAG_READ;   /* 添加 READ */
flags |= FLAG_WRITE;  /* 添加 WRITE */

/* 清除位 (移除权限) */
flags &= ~FLAG_READ;  /* 移除 READ */

/* 翻转位 (切换权限) */
flags ^= FLAG_EXECUTE;  /* 切换 EXECUTE */

/* 检查位 (检查权限) */
if (flags & FLAG_WRITE) { ... }
```

四种基本操作的通用公式：

| 操作 | 公式 | 说明 |
|------|------|------|
| 设置位 | `flags \|= (1 << n)` | 将第 n 位设为 1 |
| 清除位 | `flags &= ~(1 << n)` | 将第 n 位清为 0 |
| 翻转位 | `flags ^= (1 << n)` | 第 n 位取反 |
| 检查位 | `if (flags & (1 << n))` | 判断第 n 位是否为 1 |

### 4. Python `int.bit_length()` vs C 位操作

```python
# Python
n = 1023
n.bit_length()   # → 10 (需要 10 位)
bin(1023)        # → '0b1111111111'
```

```c
// C
uint32_t val = 1023;
int32_t bits = 0;
while (val > 0) {
    val >>= 1;
    bits++;
}
// bits = 10
```

C 语言没有内置 `bit_length()` —— 需要手动循环或用编译器内置函数（如 `__builtin_clz`）。

### 5. Endianness（字节序）

多字节数据在内存中的存储顺序有两种约定：

```
  uint32_t = 0x01020304

  Little Endian (Intel/ARM 常见):
  地址  +0   +1   +2   +3
        [04] [03] [02] [01]    ← 低位字节在前

  Big Endian (网络字节序):
  地址  +0   +1   +2   +3
        [01] [02] [03] [04]    ← 高位字节在前
```

检测当前平台字节序：

```c
uint32_t val = 0x01020304;
uint8_t *bytes = (uint8_t *)&val;

if (bytes[0] == 0x04) {
    printf("Little Endian\n");
} else {
    printf("Big Endian\n");
}
```

### 6. memcpy / memmove / memset

```c
#include <string.h>

uint8_t src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
uint8_t dst[8];

/* memcpy: 源和目标不重叠 */
memcpy(dst, src, 8);

/* memmove: 源和目标可能重叠 (安全) */
uint8_t buf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
memmove(buf + 2, buf, 6);  /* 安全地前移 */

/* memset: 逐字节填充 */
memset(dst, 0, 8);  /* dst 全清零 */
```

**memcpy vs memmove**：

```
  memcpy:  src: [A B C D E]
           dst: [1 2 3 4 5]   ← 不重叠区域, 直接复制 ✅

  memcpy ❌ 当重叠时:
           buf: [ 1 2 3 4 5 ]
           memmove(buf+2, buf, 3)
           → 用 memmove 而非 memcpy, 避免数据被覆盖前还没复制完
```

## 常见错误

### ❌ 错误 1：移位溢出（未定义行为）

```c
uint32_t x = 1;
x << 32;   /* ❌ UB! 右操作数 >= 位宽 */
x >> 32;   /* ❌ UB! */
```

✅ **修正**：检查移位范围。

```c
if (shift >= 0 && shift < 32) {
    result = x << shift;
}
```

### ❌ 错误 2：用 memcpy 处理重叠内存

```c
uint8_t buf[8] = {1, 2, 3, 4, 5, 6, 7, 8};
memcpy(buf + 2, buf, 6);  /* ❌ 源和目标重叠, 行为未定义 */
```

✅ **修正**：使用 `memmove`。

```c
memmove(buf + 2, buf, 6);  /* ✅ 安全处理重叠区域 */
```

### ❌ 错误 3：混淆 `&` 和 `&&`

```c
int flags = 5;  /* 0b101 */
if (flags & 1) { ... }   /* ✅ 位运算: 检查 bit 0 */
if (flags && 1) { ... }  /* ✅ 逻辑运算: 5 和 1 都为非零 → true */
```

两者在这个例子中结果相同, 但语义完全不同：
- `&` 是按位与, 逐 bit 操作
- `&&` 是逻辑与, 判断真/假

## 动手练习

### 🟢 入门：设置和清除单个 bit

声明 `uint8_t flags = 0`, 设置 bit 3, 然后清除 bit 3, 每次打印二进制表示。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint8_t flags = 0;
    printf("初始: %08" PRIu8 "\n", flags);

    flags |= (1u << 3);
    printf("设置 bit 3: %08" PRIu8 "\n", flags);  /* 00001000 */

    flags &= ~(1u << 3);
    printf("清除 bit 3: %08" PRIu8 "\n", flags);  /* 00000000 */

    return 0;
}
```

</details>

### 🟡 中级：权限系统

实现一个权限检查系统。定义 `READ=1, WRITE=2, EXEC=4`, 创建一个 `uint8_t` 权限变量, 演示添加权限、移除权限、检查权限。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>

#define P_READ    (1u << 0)
#define P_WRITE   (1u << 1)
#define P_EXEC    (1u << 2)

int main(void) {
    uint8_t perms = 0;

    perms |= P_READ | P_WRITE;
    printf("添加 READ + WRITE\n");

    if (perms & P_READ)  printf("  ✅ READ\n");
    if (perms & P_WRITE) printf("  ✅ WRITE\n");
    if (perms & P_EXEC)  printf("  ✅ EXEC\n");

    perms &= ~P_WRITE;
    printf("移除 WRITE\n");
    if (!(perms & P_WRITE)) printf("  ❌ WRITE 已移除\n");

    return 0;
}
```

</details>

### 🔴 挑战：字节打包/解包

实现 `pack_bytes(uint8_t b3,b2,b1,b0) → uint32_t` 和 `unpack_bytes(uint32_t, uint8_t* out)` 函数。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

static uint32_t pack_bytes(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)
{
    return ((uint32_t)b3 << 24) |
           ((uint32_t)b2 << 16) |
           ((uint32_t)b1 << 8) |
           (uint32_t)b0;
}

static void unpack_bytes(uint32_t val, uint8_t *out)
{
    out[0] = (uint8_t)val;
    out[1] = (uint8_t)(val >> 8);
    out[2] = (uint8_t)(val >> 16);
    out[3] = (uint8_t)(val >> 24);
}
```

</details>

## 故障排查 (FAQ)

**Q：`x & 1` 和 `x && 1` 有什么区别？**

A：`&` 是逐位 AND（返回新数值），`&&` 是逻辑 AND（返回真/假）。当 `x` 是非 0 整数时两者结果相同（都为真），但 `x & 1` 返回的是 0 或 1，而 `x && 1` 返回的是 1（true）。

**Q：为什么移位运算要用 `1u` 而不是 `1`？**

A：`1` 是有符号 `int`，左移可能导致符号位问题。`1u` 是 `unsigned int`，移位行为明确定义。

```c
1 << 31;   /* ❌ int 的符号位移位 = UB */
1u << 31;  /* ✅ unsigned int 移位 = 0x80000000 */
```

**Q：`memmove` 比 `memcpy` 慢吗？**

A：`memmove` 需要做额外的重叠检测，可能稍慢，但安全性远高于 `memcpy`。不确定是否重叠时，**始终用 `memmove`**。

## 知识扩展 (选学)

### Struct Bit Fields

C 允许在 struct 中直接指定字段的位数：

```c
struct Flag7 {
    uint32_t enabled    : 1;    // 1 bit
    uint32_t visibility : 2;    // 2 bits (0-3)
    uint32_t mode       : 3;    // 3 bits (0-7)
    uint32_t reserved   : 26;   // 剩余 26 bits
};
```

**注意**：bit field 的布局（bit 顺序、填充）由编译器决定，不可移植。用于硬件寄存器映射时需要查编译器文档。

### 位运算技巧

```c
/* 判断奇偶 */
bool is_odd = (n & 1);

/* 切换符号 */
int negate = ~n + 1;  /* 补码取负 */

/* 交换两个变量（无需临时变量） */
a ^= b; b ^= a; a ^= b;

/* 判断 x 是否是 2 的幂 */
bool is_pow2 = (n > 0) && ((n & (n - 1)) == 0);
```

### 网络字节序转换

```c
#include <arpa/inet.h>

uint32_t host_val = 0x01020304;
uint32_t net_val  = htonl(host_val);  /* 主机序 → 网络序 */
uint32_t back     = ntohl(net_val);   /* 网络序 → 主机序 */
```

## 小结

本章的核心要点：

- **AND (`&`)** / **OR (`|`)** / **XOR (`^`)** / **NOT (`~`)** 是逐 bit 逻辑运算
- **左移 (`<<`)** / **右移 (`>>`)** 必须确保移位位数 < 位宽
- **Bitmask** 是位运算最实用的模式：设置、清除、翻转、检查
- **bit field** 可以直接指定 struct 字段的位数，但注意不可移植性
- **`memcpy`** 不处理重叠，**`memmove`** 安全处理重叠
- **`memset`** 逐字节填充内存
- **Endianness**：Little Endian 低位在前，Big Endian 高位在前
- 移位溢出和 `memcpy` 重叠是两类最常见的安全错误

## 术语表

| 英文 | 中文 |
|------|------|
| Bitwise AND/OR/XOR/NOT | 位与/或/异或/非 |
| Left/Right shift | 左移/右移 |
| Bitmask | 位掩码 |
| Set/Clear/Toggle/Check bit | 设置/清除/翻转/检查位 |
| Bit field | 位字段 |
| Little Endian / Big Endian | 小端/大端字节序 |
| memcpy / memmove / memset | 内存拷贝/安全拷贝/内存填充 |
| Undefined behavior (UB) | 未定义行为 |
| Type erasure | 类型擦除 |

## 延伸阅读

- [C17 标准 §6.5.10-12 — 位运算](https://en.cppreference.com/w/c/language/operator_arithmetic) — 位运算定义
- [cppreference - Bitwise operations](https://en.cppreference.com/w/c/language/operator_arithmetic) — 完整参考
- [Byte order (endianness)](https://en.wikipedia.org/wiki/Endianness) — 字节序 Wikipedia 条目

**选择建议**：先理解位运算基本概念，再深入学习 bitmask 模式和网络字节序。

## 继续学习

位运算是底层编程的必备工具。它让你能够精确控制数据表示 —— 从硬件寄存器到网络协议，从权限系统到数据压缩。

- [上一章](./variadic_functions.md)：可变参数函数
- [下一章](./stdlib.md)：标准库精要
