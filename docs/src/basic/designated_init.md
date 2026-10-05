# 指定初始化与复合字面量 (Designated Initializers)

```admonish note
**本章讲什么** — C99/C17 的指定初始化(`.member = value`)与复合字面量 `(T){ ... }`:让初始化自带名字、部分初始化自动清零、一次性对象随手可造。
**对应源码** — [`src/basic/designated_init_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/designated_init_sample.c)
```

## 开篇故事

「位置初始化」把**顺序**当成了没写进文档的契约:

```c
DiVideo v = {"intro", 1920, 1080, 30};
```

哪位同事把结构体字段重排了一下?编译器不会提醒你——`1920` 依然乖乖写进**第一个字段**,只是第一个字段已经不是 `width` 了。数据静静错掉,测试若没覆盖到,能活很久。

C99 给出的解法简单粗暴:**初始化时把字段名写出来**。

```c
DiVideo v = { .width = 1920, .height = 1080 };
```

顺序不再承载语义,重排字段不会破坏任何初始化;再配上「未指定字段自动清零」的保证,一半的初始化样板代码直接消失。

## 本章适合谁

- 结构体初始化写了一长串数字、自己都认不出哪个是哪个的初学者
- 被「改了字段顺序,初始化全错」坑过的维护者
- 想写稀疏数组/查找表、配置结构体的程序员

## 你会学到什么

1. 错例:位置初始化的脆弱性(错例 → 为什么错 → 正确写法)
2. 指定初始化:顺序无关、自文档
3. 部分初始化 = 其余清零(C17 6.7.9p21)
4. 数组下标初始化器 `[i] = v` 与 union 激活成员
5. 复合字面量当实参;静态 vs 自动生命周期的生死线

## 错例:位置初始化的脆弱性

```c
DiVideo v = {"intro", 1920, 1080, 30};

printf("name=%s width=%d height=%d fps=%d\n", v.name, v.width, v.height, v.fps);
/* 当前: name=intro width=1920 height=1080 fps=30 — 看起来没错? */
```

### 为什么错

错的不是这一次初始化,是它**依赖的契约**:

```
  typedef struct {
      const char *name;   ← 第 1 个: "intro"
      int width;          ← 第 2 个: 1920
      int height;         ← 第 3 个: 1080
      int fps;            ← 第 4 个: 30
  } DiVideo;
```

现在有人出于「语义分组」把字段改成 `{name, fps, width, height}`:

- `{"intro", 1920, 1080, 30}` **依然编译通过**——初始值是按位置塞的
- 结果:`fps = 1920`、`width = 1080`、`height = 30`
- 没有警告,没有崩溃,只有悄悄错掉的配置

三个字段还算好的;十幾個字段的结构体,手工数位置本身就是 bug 制造机。**位置初始化把「字段顺序」变成了隐式 API,而隐式 API 没人守护。**

### 正确写法:指定初始化

```c
DiVideo v = {
    .height = 1080,
    .name = "intro",
    .fps = 60,
    .width = 1920,
};
```

- 写的时候乱序,读的时候按定义顺序打印——**顺序彻底无关**
- 每个值旁边就是字段名,代码即文档
- 字段重排?初始化器写的是名字,不受影响

## 原理解析

### 1. 部分初始化 = 其余字段清零

```c
DiVideo v = { .name = "clip" };
/* width=0 height=0 fps=0 ← 标准保证为 0, 不是栈上垃圾 */
```

规则(C17 6.7.9p21):**初始化器没提到的成员,按「静态存储期对象的初始化规则」处理——标量就是 0**。这条规则同时适用于:

- 静态/全局对象
- 局部自动对象的部分初始化
- 数组未指定的元素

所以「只写我关心的字段」是安全且 idiomatic 的写法。可测的版本:

```c
/* Unity 直测入口: 只指定 .fps, 其余字段必须被标准清零 (C17 6.7.9p21) */
int di_sum_of_defaults(int fps)
{
    DiVideo v = { .fps = fps };
    return v.width + v.height + v.fps;
}
```

`di_sum_of_defaults(30)` 恰好返回 `30`——`width`、`height` 若不是 0,和就不等于入参。测试就是这条标准规则的守门人。

### 2. 数组的指定初始化:稀疏表利器

```c
int slots[8] = { [2] = 42, [5] = 7 };
```

```
  下标:   0    1    2    3    4    5    6    7
  值:     0    0   42    0    0    7    0    0
                └未指定的下标全部清零┘
```

- 不用为凑位置写一串 `0, 0, 0, 42`
- 下标写在初始化器里,和值的对应关系一目了然
- 查找表、稀疏矩阵、状态机转移表的标准姿势

### 3. union 指定初始化:激活成员由你定

```c
DiValue v = { .d = 3.14 };
```

union 所有成员共享同一块内存,初始化器**决定哪个成员是激活的**:

- `v.d = 3.14` 合法读取
- 此刻读 `v.i` 是「用另一种解释读同一块字节」——实现定义行为,可能是陷阱表示
- 换激活成员 = 重新初始化整个 union(再写一个指定初始化),而不是直接给另一个成员赋值后心存侥幸

### 4. 复合字面量:一次性对象,直接当实参

```c
static int di_area(DiSize s)
{
    return s.w * s.h;
}

di_area((DiSize){ .w = 3, .h = 4 });    /* = 12 */
```

`(DiSize){ ... }` 是 C99 **复合字面量**(compound literal):类型 + 初始化器,当场造出一个匿名对象。对照传统写法:

```c
DiSize tmp;               /* 需要临时变量... */
tmp.w = 3;
tmp.h = 4;
di_area(tmp);             /* 用完 tmp 还赖在作用域里 */
```

复合字面量三行变一行,且**没有多余的命名对象**。它同样是指定初始化的载体:`(T){ .member = v }` 可以只初始化关心的字段。

### 5. 生命周期:静态 vs 自动的生死线

| 位置 | 存储期 | 地址能持有多久 |
|------|--------|----------------|
| 文件作用域的复合字面量 | 静态 | 整个程序运行期 |
| 块作用域,无 `static` | 自动 | 所在作用域结束即失效 |
| 块作用域,带 `static` | 静态 | 整个程序运行期 |

**危险写法(仅示意,不编译)**——源码里以注释保留,因为正确地「错误示范」比口头描述更有记忆点:

```c
/* ❌ 危险 (仅示意, 不编译): 块作用域的复合字面量是自动存储期,
 *   返回它的地址会悬空 (dangling), 解引用即 use-after-free:
 *       const DiSize *bad(void) { return &(DiSize){ .w = 3, .h = 4 }; }
 *   对比: 文件作用域的复合字面量天然静态存储期, 地址始终有效。
 */
```

规则一句话:**块内复合字面量和局部变量同生命周期**——别把它塞进返回值或堆到堆结构里。

## 常见错误

### ❌ 错误 1:同一字段写两次

```c
DiVideo v = { .width = 1920, .width = 42 };   /* ❌ 约束违规, 编译错误 */
```

指定初始化器每个成员最多出现一次(位置初始化混用设计器时也一样)。重复即报错——这是好事,说明有歧义就该在编译期解决。

### ❌ 错误 2:设计器用错成员名

```c
DiVideo v = { .widht = 1920 };   /* ❌ 'widht' is not a member */
```

拼错成员名编译不过——对比位置初始化的「拼错只能运行期发现(或永远发现不了)」。**自文档的另一半是自校验。**

### ❌ 错误 3:拿复合字面量的地址跨作用域用

```c
const DiSize *p = &(DiSize){ .w = 3, .h = 4 };   /* 块作用域内没问题 */
/* p 指向的对象随作用域结束失效 — 函数返回后 p 悬空 */
```

只在当前作用域内使用复合字面量地址;要长期存在,把复合字面量放到文件作用域。

### ❌ 错误 4:以为部分初始化「可能」是垃圾

```c
DiVideo v = { .name = "x" };
if (v.fps == 0) { /* 担心这里是碰运气 */ }
```

标准明确保证清零(6.7.9p21),不是「多数编译器碰巧如此」。放心依赖它——但反过来,**没写初始化器的非聚合类型变量**(`int x;`)才是未初始化的垃圾,两者别搞混。

## 动手练习

### 🟢 入门:把位置初始化改写成指定初始化

```c
struct Point { int x, y, z; };
struct Point p = {100, 0, 200};    /* 只想设置 x 和 z */
```

<details><summary>点击查看答案</summary>

```c
struct Point p = { .x = 100, .z = 200 };   /* y 自动清零为 0 */
```

</details>

### 🟡 中级:稀疏 ASCII 表

用数组指定初始化造一张表:下标 `'A'`(65)处为 1,下标 `'a'`(97)处为 1,其余为 0,长度 128。

<details><summary>点击查看答案</summary>

```c
unsigned char upper_lower[128] = {
    ['A'] = 1,
    ['a'] = 1,
};
/* upper_lower[65] == 1, upper_lower[97] == 1, 其余全 0 */
```

</details>

### 🔴 挑战:配置解析 + 复合字面量

写 `void apply(const struct Config *cfg)`,调用方用复合字面量只覆盖两个字段(`timeout` 默认 30、`retries` 默认 3,其余走默认值语义)。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>

struct Config {
    int timeout;
    int retries;
    int verbose;
};

void apply(const struct Config *cfg)
{
    printf("timeout=%d retries=%d verbose=%d\n",
           cfg->timeout, cfg->retries, cfg->verbose);
}

int main(void)
{
    struct Config defaults = { .timeout = 30, .retries = 3, .verbose = 0 };

    apply(&defaults);                                   /* 全量配置 */
    apply(&(struct Config){ .timeout = 5 });            /* 只改 timeout, 其余清零 */
    apply(&(struct Config){ .timeout = 5,
                            .retries = defaults.retries,
                            .verbose = defaults.verbose });  /* 「继承其余默认」的显式写法 */
    return 0;
}
```

注意第二、三个调用的差异:部分初始化是**清零**而不是「继承别的对象」——想要继承,就显式引用它。

</details>

## 故障排查 (FAQ)

**Q:`designated initializer` 是 C23 才有的吧?**

A:不是。**C99 就进了标准**,C11/C17 原样保留,是本项目 `-std=c17` 的合法语法。C23 的相关扩展是允许在更宽泛的位置(如复合字面量、通用初始化器语法统一)使用设计器。

**Q:指定初始化和宏配合有什么坑?**

A:初始化器列表必须是真正的初始化器——不能用宏展开出「半个列表」后接逗号的花招(某些预处理器组合会碎)。保持初始化器整体由一个字面量列表构成最稳。

**Q:数组 `[n] = v` 的 `n` 可以是变量吗?**

A:不可以。下标必须是**编译期常量表达式**——指定初始化的全部魔力都在编译期完成。

**Q:union 我直接给第二个成员赋值不行吗?**

A:给未激活成员赋值在 C99 起**合法**(6.5.16.3:写入使该成员成为激活成员),但读取仍然只能读激活成员。最清晰的模式仍是用指定初始化**声明**你从一开始就想要哪个成员。

## 小结

- 位置初始化把字段顺序变成隐式 API——重排字段 = 静默错数据
- `.member = value`:顺序无关、自文档、拼错成员名直接编译错误
- 部分初始化 = 其余字段**清零**(C17 6.7.9p21),不是垃圾值
- `[i] = v` 造稀疏表;union 的 `.member` 决定激活成员
- 复合字面量 `(T){ ... }`:一次性对象直接当实参
- **块内复合字面量是自动存储期**——地址别带出作用域

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| Designated initializer | 指定初始化 | `.member = value` 形式的初始化器 |
| Positional initialization | 位置初始化 | 按字段顺序逐个给值的传统写法 |
| Compound literal | 复合字面量 | `(T){ ... }` 构造的匿名对象 |
| Zero-fill / zero-init | 清零初始化 | 未指定成员按静态初始化规则置 0 |
| Active member | 激活成员 | union 当前可合法访问的成员 |
| Storage duration | 存储期 | 静态 / 自动 / 动态 / 线程,决定对象寿命 |

## 延伸阅读

- [cppreference - aggregate initialization](https://en.cppreference.com/w/c/language/aggregate_initialization) — 指定初始化与清零规则
- [cppreference - compound literal](https://en.cppreference.com/w/c/language/compound_literal) — 语法与生命周期
- [C17 standard §6.7.9 / §6.5.2.5](https://port70.net/~nsz/c/c17/n1570.html) — 规范原文

## 继续学习

- [上一章](./static_assert.md):编译期断言 `_Static_assert`
- [下一章](./integer_safety.md):整数安全 (CERT INT30/31/33-C)

---

> 本章代码位于 [`src/basic/designated_init_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/designated_init_sample.c),测试位于 [`test/basic/test_designated_init_sample.c`](https://github.com/savechina/hello-c/blob/main/test/basic/test_designated_init_sample.c)。
