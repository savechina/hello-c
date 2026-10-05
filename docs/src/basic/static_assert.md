# 编译期断言 (_Static_assert)

```admonish note
**本章讲什么** — C11 `_Static_assert`:把「我假设平台是这样的」写成编译器强制执行的契约,假设不成立就拒绝编译。
**对应源码** — [`src/basic/static_assert_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/static_assert_sample.c)
```

## 开篇故事

代码里到处是没写出来的假设:`int` 至少 4 字节、结构体没有填充、数组恰好 5 个元素。这些假设平时没人验证——直到某天换了个平台、有人改了结构体字段,程序开始输出诡异数据,而**编译器一声不吭**。

C11 给了你一个把假设变成契约的关键字:

```c
_Static_assert(sizeof(int) >= 4, "int 至少 4 字节 — 本教程整型演示的前提");
```

条件为假?**编译失败,附带你写的理由**。断言在编译期执行,零运行时开销,零机会漏到生产环境。

## 本章适合谁

- 写过「我假设 long 是 4 字节」这类代码却不自知的初学者
- 做序列化、协议、位运算,依赖内存布局的程序员
- 想在 CI 里让平台假设「一票否决」的维护者

## 你会学到什么

1. 文件作用域断言:`sizeof` 保证、结构体布局
2. `ARRAY_SIZE` 宏 + 长度断言的组合拳
3. `__STDC_VERSION__ >= 201112L` 特性门
4. 块作用域断言:把假设钉在使用点
5. 错例:一个错误的 `long` 假设如何当场拒绝编译(注释示意)

## 错例:错误的大小假设

```c
/* ❌ 错例 (取消注释即编译失败, 请勿启用):
 * _Static_assert(sizeof(long) == 4, "错误假设: long 恒为 4 字节");
 * macOS/Linux 64 位是 LP64, long = 8 字节 → static assertion failed。
 * 教训: 断言写下的是你的假设; 平台不满足时立刻编译报错,
 * 而不是带着错误假设编译通过、在运行期静默出错。
 * 正确姿势: 要么改用 stdint.h 定宽类型, 要么断言跨平台都成立的下界 (>= 4)。
 */
```

### 为什么错

64 位 macOS/Linux 是 **LP64** 数据模型:`int` 4 字节,`long` **8 字节**;Windows 64 位是 LLP64:`long` 4 字节。「`long` == 4 字节」只是一部分平台的局部真理。写下这个断言,编译器替你把假设摊在桌面上——取消注释,立刻看到:

```
error: static assertion failed: "错误假设: long 恒为 4 字节"
```

**这正是断言的价值**:错误在编译期、带着你的理由出现,而不是在运行期变成一个解释不了的 bug。

### 正确写法:要么定宽,要么断下界

```c
_Static_assert(sizeof(int) >= 4, "int 至少 4 字节 — 本教程整型演示的前提");
_Static_assert(sizeof(int32_t) == 4, "int32_t 必须恰好 4 字节 (stdint.h 的承诺)");
_Static_assert(sizeof(long) >= 4, "long 至少 4 字节 (ILP32 与 LP64 都满足)");
```

三条的差别是**断言强度**:

| 断言 | 含义 | 跨平台? |
|------|------|---------|
| `sizeof(int32_t) == 4` | 定宽类型必须精确等于 4 | ✅ `stdint.h` 保证,不满足是实现的 bug |
| `sizeof(int) >= 4` | 只断下界 | ✅ 所有现实平台都满足 |
| `sizeof(long) == 4` | 断精确值 | ❌ LP64 下必炸 |

**规则:断言跨平台都成立的真理,或你真正依赖的精确值;依赖精确值时优先选定宽类型。**

## 原理解析

### 1. 语法与位置

```c
_Static_assert( 常量表达式, 字符串字面量 );
```

- 条件必须是**整数常量表达式**(`sizeof`、`offsetof`、字面量都算)
- 字符串是**错误信息的一部分**——它会原样出现在编译错误里,所以要写清「为什么需要这个保证」
- 可出现在三个位置:**文件作用域**、**结构体/联合内部**、**块作用域**
- C11 起 `<assert.h>` 还提供同名宏 `static_assert`,语义相同

### 2. 特性门:`__STDC_VERSION__ >= 201112L`

```c
/* 特性门: _Static_assert 是 C11 (201112L) 关键字, 晚于 C11 的翻译单元才有 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
/* 本项目 -std=c17, 门常开 */
#else
#error "_Static_assert requires C11 or later (use -std=c11 / -std=c17)"
#endif
```

`__STDC_VERSION__` 是标准规定的宏,值形如 `201112L`(C11)、`201710L`(C17)。用特性门可以:

- 给老编译器(C90/C99)留一条**明确的报错路径**而不是神秘语法错误
- 给只有部分支持的编译器提供降级宏

本项目统一 `-std=c17`,门常开——但它示范了「用标准版本号守护新特性」的通用手法。

### 3. 结构体布局断言

```c
typedef struct {
    int32_t x;
    int32_t y;
} SaPoint;

_Static_assert(sizeof(SaPoint) == 2 * sizeof(int32_t), "两个 int32_t 成员之间不应有填充");
_Static_assert(offsetof(SaPoint, y) == sizeof(int32_t), "y 紧跟 x, 偏移量等于 x 的宽度");
```

```
  SaPoint (假设成立时):
  ┌───────────┬───────────┐
  │ x (4字节) │ y (4字节) │
  └───────────┴───────────┘
   offset=0    offset=4
```

- `sizeof` 断言总大小 → 有没有意外填充
- `offsetof(SaPoint, y)` 断言字段偏移 → 布局**逐字节**可依赖

谁需要这种保证?把结构体直接 `fwrite` 到文件、按偏移解析网络报文、与硬件寄存器映射的代码。注意断言用 `sizeof(int32_t)` 做基准而不是字面量 `4`/`8`——**让断言和字段类型说同一种语言**,改字段类型时断言自动跟着算。

### 4. `ARRAY_SIZE` 模式

```c
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const int sa_probe[5] = {10, 20, 30, 40, 50};

_Static_assert(ARRAY_SIZE(sa_probe) == 5, "增删数组元素时, 编译期提醒同步更新假设");
/* ARRAY_SIZE 只对真数组成立: 传入指针会衰变成 sizeof(T*)/sizeof(T),
 * 编译器不报错但结果与元素个数无关 — 用它之前先确认拿到的是数组 */
```

三件套一起用:

1. **`ARRAY_SIZE` 宏** — 长度不再手抄,增删元素自动同步
2. **长度断言** — 「我假设这里是 5 个」写进编译器;改数组时若别处依赖 5,当场报错
3. **衰变警告** — 传指针进去不报错但结果是 `8/4 = 2`(64 位)这种无意义数字,这是宏的固有盲区

遍历代码同样用它,保证「长度声明」和「长度使用」永远同源:

```c
int copy[ARRAY_SIZE(sa_probe)];
size_t i;

for (i = 0; i < ARRAY_SIZE(sa_probe); i++) {
    copy[i] = sa_probe[i];
}
```

### 5. 块作用域断言:假设钉在使用点

```c
int sa_clamp_u8(int v)
{
    /* 块作用域断言: 把本函数的隐含假设钉在使用点旁边 */
    _Static_assert(255 <= INT_MAX, "u8 上界必须放得进 int, 否则比较本身会溢出");

    if (v < 0) {
        return 0;
    }
    if (v > 255) {
        return 255;
    }
    return v;
}
```

这个断言保护的是**函数自己的逻辑**:`if (v > 255)` 要安全,前提是 255 能放进 `int`。断言放在函数体内,改逻辑的人一眼看到依赖——比翻到文件头找断言近得多。运行期对照:

```
sa_clamp_u8(999) = 255
sa_clamp_u8(-5)  = 0
sa_clamp_u8(42)  = 42
```

## 常见错误

### ❌ 错误 1:条件不是常量表达式

```c
int n = 5;
_Static_assert(n == 5, "nope");   /* ❌ 错误: n 不是编译期常量 */
```

断言跑在编译期,只认常量表达式。运行期才有的值请用 `assert()`(那是另一套机制,不进 Release)。

### ❌ 错误 2:断言写字符串比较式的「精确平台值」

```c
_Static_assert(sizeof(void *) == 4, "32 位指针");   /* ❌ 64 位平台全炸 */
```

除非你真的只支持 32 位——否则断下界或用条件编译分流。

### ❌ 错误 3:忘了断言只是「文档的强制执行」

```c
_Static_assert(sizeof(SaPoint) == 8, "看起来对");
/* 但代码里 fwrite(&p, 8, 1, f) 又手抄了一遍 8 */
```

断言值和代码里的魔法数各写各的,断言就失去意义。**让代码引用同一个表达式**(`sizeof(SaPoint)` / `ARRAY_SIZE`),断言守表达式,代码用表达式。

## 动手练习

### 🟢 入门:补一条断言

为 `double` 在你的平台上成立的、**跨平台安全**的断言写一行(提示:IEEE 754 双精度通常是 8 字节,但标准只保证下界)。

<details><summary>点击查看答案</summary>

```c
_Static_assert(sizeof(double) >= 8, "double 至少 8 字节 (IEEE 754 binary64 的下界)");
```

</details>

### 🟡 中级:给枚举和位标志断言

给下面的错误码枚举补断言,保证 `OK` 是 0、`MAX_CODE` 不超过 255:

```c
typedef enum { ERR_OK = 0, ERR_IO = 1, ERR_OOM = 2, ERR_MAX_CODE = 7 } ErrCode;
```

<details><summary>点击查看答案</summary>

```c
_Static_assert(ERR_OK == 0, "调用方约定 0 表示成功");
_Static_assert(ERR_MAX_CODE <= 255, "错误码要能塞进 uint8_t 传输");
```

</details>

### 🔴 挑战:协议头布局守卫

定义一个 8 字节的协议头(4 字节 magic + 2 字节 version + 2 字节 flags),用断言保证它恰好 8 字节、`version` 偏移为 4。

<details><summary>点击查看答案</summary>

```c
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t magic;
    uint16_t version;
    uint16_t flags;
} ProtoHeader;

_Static_assert(sizeof(ProtoHeader) == 8, "协议头必须恰好 8 字节");
_Static_assert(offsetof(ProtoHeader, version) == 4, "version 从偏移 4 开始");
_Static_assert(offsetof(ProtoHeader, flags) == 6, "flags 从偏移 6 开始");
```

</details>

## 故障排查 (FAQ)

**Q:报 `static assertion failed: ...`,怎么修?**

A:先读断言信息——它就是你(或同事)当初写的「为什么需要这个保证」。然后二选一:①代码确实依赖这个保证 → 改代码/改平台支持;②假设过时了 → 更新断言。**不要直接删断言**,那是把契约偷偷废掉。

**Q:`_Static_assert` 和运行期 `assert()` 怎么分工?**

A:`_Static_assert` 断**类型系统和平台事实**(sizeof、偏移、版本号),编译期、进 Release、零成本;`assert()` 断**运行期逻辑**(指针非空、值域),可被 `NDEBUG` 关掉。两者不互相替代。

**Q:为什么我的断言在 C90 项目里编译不过?**

A:`_Static_assert` 是 C11 关键字。要么切 `-std=c11`/`-std=c17`,要么用特性门 + 老式替代:

```c
typedef char sa_implies_c11[(cond) ? 1 : -1];   /* 条件为假 → 数组负长度 → 编译错误 */
```

**Q:断言失败的报错太短,能带更多信息吗?**

A:字符串就是给它准备的——把「依赖方」写进信息:`"协议头必须 8 字节 — 见 wire_format.md §2"`。比一句 `failed` 有用得多。

## 小结

- `_Static_assert(条件, 消息)`:条件为假 → **编译失败**,消息原样出现在错误里
- 断言跨平台都成立的真理,或选 `stdint.h` 定宽类型断精确值
- `ARRAY_SIZE` + 长度断言 = 数组假设的双重保险;注意指针衰变盲区
- `__STDC_VERSION__ >= 201112L` 特性门守护 C11 新特性,给老工具链明确报错
- 块作用域断言把「函数自己的假设」钉在使用点
- 断言是**可执行的文档**——删它之前先想想它守着谁

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| Static assertion | 静态断言 | 编译期求值,失败即拒绝编译 |
| Constant expression | 常量表达式 | 编译期可求值的表达式,断言条件的硬性要求 |
| Feature gate | 特性门 | 用 `__STDC_VERSION__` 按标准版本启用/禁用代码 |
| Struct padding | 结构体填充 | 对齐导致的成员间空隙,布局断言的主要侦测对象 |
| LP64 / LLP64 | — | 64 位数据模型:`long` 分别是 8 / 4 字节 |

## 延伸阅读

- [cppreference - static assertions](https://en.cppreference.com/w/c/language/static_assert) — 语法与位置
- [C11 standard §7.2 / 6.7.10](https://port70.net/~nsz/c/c11/n1570.html) — 断言与对象表示
- [CERT DCL00-C / MSC00-C](https://wiki.sei.cmu.edu/confluence/display/c/) — 关于编译期验证的合规建议

## 继续学习

- [上一章](./type_generic.md):类型泛型 `_Generic`
- [下一章](./designated_init.md):指定初始化与复合字面量

---

> 本章代码位于 [`src/basic/static_assert_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/static_assert_sample.c),测试位于 [`test/basic/test_static_assert_sample.c`](https://github.com/savechina/hello-c/blob/main/test/basic/test_static_assert_sample.c)。
