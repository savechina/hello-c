# 类型泛型 (_Generic)

```admonish note
**本章讲什么** — C11 的 `_Generic` 类型泛型选择:编译期按表达式类型选择分支,写出类型安全的"泛型宏"。错误先行:先看朴素 `max` 宏的双重求值错例,再看 `_Generic` 修复。
**对应源码** — [`src/basic/type_generic_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/type_generic_sample.c)
```

## 开篇故事

C 没有模板,也没有函数重载。想写一个 `max(a, b)`,只能用宏——而宏是纯文本替换,不检查类型、不控制求值次数。于是「宏展开两次,副作用执行两次」成了 C 程序员的集体阴影:

```c
int i = 3;
int got = NAIVE_MAX(i++, 0);   /* got 和 i 都不是你想要的 */
```

C11 给出了标准答案:`_Generic`——**一个在编译期根据表达式类型挑选表达式的选择器**。它不运行时判断,不产生分支代码,只是让「同一句源码在不同类型下走不同函数」成为语言特性。

## 本章适合谁

- 用过 `max`/`min`/`swap` 之类的宏,被副作用坑过的初学者
- 想给宏加上类型检查的程序员
- 好奇 C 如何在没有模板的情况下实现轻量泛型

## 你会学到什么

1. 朴素宏的双重求值错例(错例 → 为什么错 → 正确写法)
2. `_Generic` 的语法与三条铁律(控制表达式不求值、按类型匹配、无 default 即报错)
3. 用 `_Generic` 写类型安全的 `TG_MAX` / `TG_MIN` / `TG_SHOW`
4. C11 为什么必须枚举显式类型(没有 `typeof`),C23 `typeof` 如何改进

## 错例:朴素 max 宏的双重求值

```c
/* ❌ 错例宏: 纯文本替换, 实参 a 在条件与真分支里各出现一次 */
#define NAIVE_MAX(a, b) ((a) > (b) ? (a) : (b))

int i = 3;
int got = NAIVE_MAX(i++, 0);
```

运行结果:

```
期望: got = 3, i = 4   (i++ 只该执行一次)
实际: got = 4, i = 5   ← i++ 被执行了两次
```

### 为什么错

宏展开后,`i++` 在源码里出现了**两次**:

```c
((i++) > (0) ? (i++) : (0))
```

1. 先求值条件 `(i++) > (0)`:`3 > 0` 为真,`i` 变成 4
2. 条件为真,再求值真分支 `(i++)`:又自增一次,`i` 变成 5,表达式值为 4

注意:**这不是未定义行为**——`i++` 执行两次完全合法,结果是「良定义的错误」。正因为不触发任何 sanitizer,它比 UB 更隐蔽:编译器不报错,`-fsanitize=address,undefined` 也不报警,只有测试结果对不上时你才会发现。

### 正确写法:让每个实参只出现一次

思路:不再用文本替换拼表达式,而是**按类型分派到真函数**——函数调用保证实参各求值一次。

## 正确写法:`_Generic` 安全 max/min

```c
int tg_max_int(int a, int b)
{
    return a > b ? a : b;
}

/* 分支里的函数名必须先声明: 未选中的分支虽不求值, 但必须是合法表达式。
 * 故意不写 default — 未知类型宁可编译失败也不静默选错函数。
 * 字符串字面量经 array-to-pointer 衰变匹配 char* 分支 (§6.5.1.1p3)。 */
#define TG_MAX(a, b) _Generic((a),                      \
    int: tg_max_int,                                    \
    double: tg_max_double,                              \
    char *: tg_max_cstr,                                \
    const char *: tg_max_cstr                           \
    )((a), (b))

#define TG_MIN(a, b) _Generic((a),                      \
    int: tg_min_int,                                    \
    double: tg_min_double,                              \
    char *: tg_min_cstr,                                \
    const char *: tg_min_cstr                           \
    )((a), (b))
```

使用效果:

```c
int i = 3;
int r = TG_MAX(i++, 0);   /* r = 3, i = 4 — i++ 只执行一次 */
```

**为什么这次对了**:宏展开成

```c
_Generic((i++), int: tg_max_int, ...)((i++), 0)
```

看起来 `i++` 又出现了两次——但 `_Generic` 的**控制表达式 (即第一个 `(i++)) 不求值**,它只用来做类型匹配。真正求值的只有被选中分支随后的函数实参 `tg_max_int(i++, 0)`,每个实参恰好一次。

## 原理解析

### 1. 语法结构

```c
_Generic( 控制表达式, 分支1类型: 分支1表达式, 分支2类型: 分支2表达式, ... [, default: 表达式] )
```

三条铁律(C11 §6.5.1.1):

| 规则 | 含义 |
|------|------|
| 控制表达式不求值 | 只取其类型做匹配,`i++` 在这里是零副作用的 |
| 按类型选分支 | 控制表达式的类型(经 lvalue/array/function 转换)与哪个分支类型匹配,就求值哪个分支 |
| 无匹配即约束违规 | 找不到类型匹配且没有 `default` → **编译错误**,不是运行时错误 |

### 2. 每种类型一个真函数——类型安全的来源

```c
printf("  TG_MAX(%d, %d)            = %d\n", ia, ib, TG_MAX(ia, ib));
printf("  TG_MAX(%.2f, %.2f)      = %.2f\n", da, db, TG_MAX(da, db));
printf("  TG_MAX(\"%s\", \"%s\")   = %s\n", sa, sb, TG_MAX(sa, sb));
```

- `int` 实参 → 选中 `tg_max_int`,整数比较
- `double` 实参 → 选中 `tg_max_double`,浮点比较
- 字符串 → 选中 `tg_max_cstr`,内部用 `strcmp` 做字典序比较

最后一项值得停留:`sa > sb` 这样对**不相关对象的指针**做关系比较在 C 里是未定义行为,所以字符串分支必须走 `strcmp`:

```c
/* 字符串按 strcmp 语义比较: 用关系运算符直接比较不相关对象的指针是 UB */
static const char *tg_max_cstr(const char *a, const char *b)
{
    return strcmp(a, b) >= 0 ? a : b;
}
```

### 3. 类型匹配的细节

```c
const char *sa = "banana";
TG_MAX(sa, sb);        /* 控制表达式类型 const char * → 匹配 const char * 分支 */

TG_MAX("apple", "pie");
/* 字符串字面量是 char[6]/char[4], 经 array-to-pointer 衰变后类型为 char *
   → 匹配 char * 分支 */
```

所以宏里 `char *` 和 `const char *` **两个分支都要写**——它们是不同的类型,不互相匹配。这就是「C11 没有 `typeof`,分支必须枚举显式类型」的直接代价。

### 4. 故意不写 default

```c
int *p;
TG_MAX(p, p);          /* ❌ 编译错误: int* 不匹配任何分支 */
TG_MAX("apple", 1);    /* ❌ 编译错误: int 无法传给 const char * 参数 */
```

不写 `default` 是设计选择:**未知类型宁可编译失败,也不静默选错函数**。想支持新类型,就显式加一个分支——类型系统的白名单。

### 5. 分派打印:`TG_SHOW`

```c
#define TG_SHOW(x) _Generic((x),                        \
    int: tg_show_int,                                   \
    double: tg_show_double,                             \
    char *: tg_show_str,                                \
    const char *: tg_show_str                           \
    )((x))
```

```c
TG_SHOW(42);        /* int    → 42 */
TG_SHOW(3.14);      /* double → 3.14 */
TG_SHOW("hello");   /* str    → "hello" (len=5) */
```

同一句 `TG_SHOW(...)`,三种类型自动选对打印函数——这就是「泛型选择」的名字由来。

### 6. C11 的局限与 C23 `typeof` 的改进

| 版本 | 写法 | 问题/改进 |
|------|------|-----------|
| C11 | 每种类型枚举一个分支 | 新类型要手动加分支,漏了就编译错误(但这是保底安全) |
| C23 | `typeof(a)` 可在 `default` 分支按实参类型构造结果 | 新类型零成本接入;还能写「先存局部副本再比较」的宏,从根上消除双重求值 |

C11 没有 `typeof` 是历史事实——`_Generic` 是在没有类型运算符的约束下能做出的最优设计;C23 补上 `typeof` 后,两条路线可以组合使用。

## 常见错误

### ❌ 错误 1:以为 `_Generic` 会求值控制表达式

```c
int i = 3;
int a = _Generic(i++, int: 1, default: 0);   /* i 仍为 3! */
```

控制表达式**永不求值**。有副作用的表达式放进去等于没写——副作用必须出现在被选中的分支里。

### ❌ 错误 2:漏写 `const char *` 分支

```c
const char *s = "hi";
TG_MAX(s, "x");      /* ❌ 约束违规: const char* 不匹配 char* */
```

修复:两个分支都写(样例宏已包含两者)。

### ❌ 错误 3:分支函数未声明就使用宏

```c
#define TG_MAX(a, b) _Generic((a), int: tg_max_int, ...)((a), (b))
/* tg_max_int 此时尚未声明 → 即使传 double 也编译错误 */
```

未选中的分支虽不求值,但必须是**合法表达式**。所有分支函数要在宏首次使用前声明。

### ❌ 错误 4:用宏包 `_Generic` 时实参写了两次

```c
#define BAD(x) (TG_MAX(x, 0))    /* ✅ x 只出现一次 */
#define WORSE(x) (TG_MAX((x) + 1, (x) + 2))   /* ❌ (x)+1 之类的表达式又重复了 */
```

`_Generic` 只保证**它自己那份**实参单次求值;宏作者把实参复制几遍,它就求值几次。诚实转发实参是宏作者的责任。

## 动手练习

### 🟢 入门:给宏补分支

下面的宏对 `long` 传入时会编译失败,请补全分支(先写出 `tg_max_long` 再接进宏):

```c
long x = 100L, y = 200L;
TG_MAX(x, y);   /* ❌ 目前无匹配分支 */
```

<details><summary>点击查看答案</summary>

```c
static long tg_max_long(long a, long b)
{
    return a > b ? a : b;
}

#define TG_MAX(a, b) _Generic((a),                      \
    int: tg_max_int,                                    \
    long: tg_max_long,                                  \
    double: tg_max_double,                              \
    char *: tg_max_cstr,                                \
    const char *: tg_max_cstr                           \
    )((a), (b))
```

</details>

### 🟡 中级:类型名选择器

实现 `TG_TYPEOF(x)`,返回表示类型名的字符串字面量(`"int"` / `"double"` / `"char *"`...)。

<details><summary>点击查看答案</summary>

```c
#define TG_TYPEOF(x) _Generic((x),                \
    int: "int",                                   \
    double: "double",                             \
    char *: "char *",                             \
    const char *: "const char *",                 \
    default: "unknown"                            \
    )

const char *name = TG_TYPEOF(42);    /* "int" */
name = TG_TYPEOF(3.14);              /* "double" */
```

这次可以放心写 `default`——类型名打印是展示型场景,未知类型给个兜底即可。

</details>

### 🔴 挑战:类型安全的交换宏

用 `_Generic` 实现 `TG_SWAP(a, b)`,只支持 `int` 和 `double`,且每个实参只求值一次(提示:分派到接受指针的函数)。

<details><summary>点击查看答案</summary>

```c
static void tg_swap_int(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

static void tg_swap_double(double *a, double *b)
{
    double t = *a;
    *a = *b;
    *b = t;
}

#define TG_SWAP(a, b) _Generic((a),        \
    int: tg_swap_int,                      \
    double: tg_swap_double                 \
    )(&(a), &(b))
```

`&(a)` 让宏拿到地址,副作用表达式在地址实参处**恰好求值一次**。

</details>

## 故障排查 (FAQ)

**Q:编译报 `type 'X' is not compatible with any generic association type`,什么意思?**

A:控制表达式的类型没匹配上任何分支,且宏没写 `default`。要么补分支,要么检查实参类型是否符合预期(常见于漏写 `const char *`、传入数组没意识到已衰变)。

**Q:`_Generic` 有运行时开销吗?**

A:没有。匹配在编译期完成,生成的代码就是一次普通函数调用,零分支、零跳转表。

**Q:`_Generic` 和 `void*` 泛型(本仓库 [void_generic 章节](./void_generic.md))怎么选?**

A:`void*` 是**运行时无类型**——编译器不知道你传了什么,转错要自己负责;`_Generic` 是**编译期有类型**——转错直接编译失败。数据结构接口(如 `qsort`)用 `void*` 合理;表达式级的 `max`/`swap`/打印,优先 `_Generic`。

**Q:和 C++ 模板比呢?**

A:C++ 模板是实例化整段代码,能做任意复杂编译期计算;`_Generic` 只做「选一个表达式」,轻得多。C 的哲学:泛型选择够用就好,重活交给类型系统约束下的宏。

## 小结

- 朴素宏的双重求值是**良定义的错误结果**——不触发 UBSan,所以更隐蔽
- `_Generic` 控制表达式**永不求值**,只做类型匹配;选中的分支保证实参各求值一次
- 分支按类型白名单匹配;**不写 `default` 就是编译期类型检查**
- C11 无 `typeof`,分支必须枚举显式类型;C23 `typeof` 让新类型零成本接入
- 字符串比较走 `strcmp`,不要对不相关指针做关系运算(UB)
- 宏作者的责任:实参在你的展开里出现几次,就求值几次

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| Generic selection | 泛型选择 | `_Generic` 表达式,编译期按类型选分支 |
| Controlling expression | 控制表达式 | `_Generic` 第一个实参,只取类型不求值 |
| Generic association | 泛型分支 | `type: expression` 这样的配对 |
| Double evaluation | 双重求值 | 宏实参被展开多次导致副作用执行多次 |
| Array-to-pointer decay | 数组到指针衰变 | 数组表达式在多数语境下变成指针 |

## 延伸阅读

- [cppreference - _Generic](https://en.cppreference.com/w/c/language/_Generic) — 语法与匹配规则
- [C11 standard §6.5.1.1](https://port70.net/~nsz/c/c11/n1570.html#6.5.1.1) — 泛型选择的规范原文
- [C23 `typeof`](https://en.cppreference.com/w/c/keyword/typeof) — C23 对本章方案的改进

## 继续学习

- [上一章](./const_correctness.md):const 正确性
- [下一章](./static_assert.md):编译期断言 `_Static_assert`

---

> 本章代码位于 [`src/basic/type_generic_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/type_generic_sample.c),测试位于 [`test/basic/test_type_generic_sample.c`](https://github.com/savechina/hello-c/blob/main/test/basic/test_type_generic_sample.c)。
