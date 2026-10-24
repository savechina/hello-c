# C99 inline 函数

```admonish note
**本章讲什么** — `inline` 只是给编译器的**优化建议**,不改变语义。错误先行:C99 外部 `inline` 定义不带外部声明会导致"缺外部定义"链接错误;正确姿势是头文件里的 `static inline` 小函数。实测:直接调用与取地址调用结果必须一致。
**对应源码** — [`src/basic/inline_functions_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/inline_functions_sample.c)
```

## 开篇故事

"把小函数标上 `inline`,程序就快了" —— 这是流传最广的半真半假说法。`inline` 在 C99 里的真实身份是:**请求**编译器把函数体在调用点展开(省一次调用开销),同时**承诺**语义与普通函数完全一致。它可以影响性能(好或坏),但绝不能影响正确性 —— 本章用"直接调用 vs 函数指针调用必须同值"来实测这条铁律。

## 本章适合谁

- 用过 `inline` 但说不清 C99 三种写法区别的人
- 被"缺外部定义"(`undefined symbol`)链接错误坑过的读者
- 写头文件库、关心调用开销的程序员

## 你会学到什么

1. `static inline`(头文件标准姿势)的语义:每个 TU 一份静态副本,永远安全
2. C99 外部 `inline` 的三种声明组合,以及"只有定义没有外部声明"为什么链接失败
3. `inline` 只是优化建议:直接调用与取地址调用必须产生相同结果

## 错例:外部 inline 的链接陷阱

```c
/* ❌ 错例 (仅示意; 链接阶段报 undefined symbol) */
/* header.h */
inline int bad_clamp(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }
/* 只有 inline 定义, 没有 extern 声明 → 该 TU 不生成外部符号 */

/* main.c */
int r = bad_clamp(5, 0, 10);   /* 调用了外部符号 → 链接器找不到 */
```

### 为什么错

C99 规定:纯 `inline` 定义**不**提供外部定义。如果编译器选择不内联、按普通调用处理,链接器就找不到符号。标准给了三条出路:

| 写法 | 结果 |
|------|------|
| `inline int f(){...}` 只有定义 | ❌ 无外部定义, 可能链接失败 |
| `extern inline int f(){...}` | ✅ 提供外部定义 |
| `static inline int f(){...}` | ✅ 每 TU 一份内部副本, 最安全 |

## 正确写法:头文件里的 static inline

```c
/* inline_functions_sample.h */
static inline int inl_clamp(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}
```

```c
/* 调用点 */
int r1 = inl_clamp_via_call(5, 0, 10);   /* 直接调用: 编译器可内联 */
int r2 = inl_same_result_via_address();  /* 取地址调用: 必须同值 */
```

## 原理解析

### 1. 直接调用 vs 取地址调用:语义必须一致

```c
int inl_same_result_via_address(void)
{
    int direct  = inl_clamp(5, 0, 10);              /* 可能被内联 */
    int via_ptr = (*(int (*)(int, int, int))inl_clamp)(5, 0, 10);  /* 必须走调用 */
    return direct == via_ptr;   /* 永远 1 —— inline 不改语义 */
}
```

这是本章的测试核心:`TEST_ASSERT_EQUAL_INT(1, inl_same_result_via_address())`。如果某人用宏伪装 `inline`(`#define inl_clamp(...)`)且实现有副作用,这个断言就会抓到它。

### 2. inline 不保证内联

`inline` 关键字 = "请考虑在调用点展开"。编译器完全可以拒绝:

- 函数太大 → 拒绝
- 取了函数地址 → 必须生成实体,不能纯展开
- `-O0` 调试构建 → 常常全部不内联

所以:**不要用 `inline` 优化热点,用它表达"这是小工具函数"的意图**;真正的性能验证靠 profile 数据。

### 3. 为什么不用宏

宏文本替换没有类型检查、没有作用域、`#define MAX(a,b) ((a)>(b)?(a):(b))` 还会重复求值副作用。`static inline` 有类型、有作用域、参数只求值一次 —— 现代 C 里小函数宏的正统替代品。

## 动手练习

### 🟢 入门:static inline 最小工具

在头文件写 `static inline int inl_max(int a, int b)`,在两个 `.c` 里分别调用,验证链接无重复符号。

<details><summary>点击查看答案</summary>

```c
/* util.h */
static inline int inl_max(int a, int b) { return a > b ? a : b; }
/* 两个 TU 各有一份内部副本, 互不冲突 — 这就是 static 的意义 */
```

</details>

### 🟡 中级:验证不可内联场景

写一个取 `inl_clamp` 函数指针放进数组,遍历数组调用,断言与直接调用结果一致。

<details><summary>点击查看答案</summary>

```c
int (*ops[1])(int, int, int) = { inl_clamp };
int r = ops[0](7, 0, 10);        /* 函数指针调用: 编译器必须保留实体 */
TEST_ASSERT_EQUAL_INT(inl_clamp(7, 0, 10), r);
```

</details>

### 🔴 挑战:修复错例

把错例里的纯 `inline` 定义改成两种正确写法(`extern inline` 与 `static inline`),分别编译链接,对比符号表(`nm`)。

<details><summary>点击查看答案</summary>

```c
/* 方案 A: extern inline — 提供外部定义, 单 TU 库可用 */
extern inline int ext_clamp(int v, int lo, int hi);

/* 方案 B: static inline — 每 TU 内部副本, 头文件首选 */
static inline int st_clamp(int v, int lo, int hi);
```

</details>

## 故障排查 (FAQ)

**Q:为什么加了 `inline` 反而变慢?**

A:函数体膨胀导致指令缓存 miss 增加,或阻止了编译器其他优化。`inline` 是建议不是命令,该关就关,以 profile 为准。

**Q:`inline` 和 C++ 的 `inline` 一样吗?**

A:不一样。C++ 的 `inline` 自带外部定义语义(还允许重复定义);C99 的纯 `inline` 定义**不**提供外部定义 —— 这是最常见的跨语言踩坑点。

**Q:调试器里 `inline` 函数断不上?**

A:`-O0` 下通常可断;开了优化后调用点已展开,用 `__attribute__((noinline))`(Clang/GCC 扩展,非标准)临时阻止。

## 小结

- `static inline`:头文件小函数的标准姿势,永远链接安全
- 纯 `inline` 定义 ≠ 外部定义,可能 `undefined symbol`;需要外部定义用 `extern inline`
- `inline` 只影响是否展开,**不影响语义** —— 取地址调用必须与直接调用同值
- 不要用 `inline` 当性能开关;用它表达"小工具函数"意图

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 内联展开 | Inlining | 把函数体复制到调用点,省去调用开销 |
| 优化建议 | Optimization hint | 编译器可拒绝的请求,不影响语义 |
| 外部定义 | External definition | 提供跨 TU 链接符号的定义 |
| 重复求值 | Double evaluation | 宏参数被求值两次的副作用陷阱 |

## 延伸阅读

- [cppreference - inline specifier](https://en.cppreference.com/w/c/language/inline)
- [C17 standard §6.7.4](https://port70.net/~nsz/c/c17/n1570.html#6.7.4)

## 继续学习

- [上一章](./functions_basics.md):函数基础
- [下一章](./pointers.md):指针

---

> 本章代码位于 [`src/basic/inline_functions_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/inline_functions_sample.c),测试位于 [`test/basic/test_inline_functions_sample.c`](https://github.com/savechina/hello-c/blob/main/test/basic/test_inline_functions_sample.c)。
