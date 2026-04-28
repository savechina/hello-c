# 嵌套结构体（Nested Structs）

就像俄罗斯套娃——一个大盒子里面套着小盒子，小盒子里可能还有更小的盒子。

## 开篇故事

你去餐厅点餐，菜单上有"套餐"概念。一个套餐包含：主菜、配菜、饮料。而这些菜品本身可能也有属性——主菜的口味（辣/不辣）、配菜的份量（大/中/小）。

这就是嵌套——一个集合里面包含另一个集合。

在 C 语言中，结构体也可以嵌套结构体。一个 `Person` 包含 `Date`（生日）和 `Address`（地址），而 `Address` 可能又包含 `City`（城市信息）。层层嵌套，模型越来越精确。

我第一次写嵌套结构体时，初始化列表写成了这样：

```c
struct Person p = {"Alice", {2000, 6, 15}, {"Beijing", "Chaoyang"}};
```

编译器没报错，但我看得眼花。后来发现了指定初始化的嵌套写法——` .birthday = {.year = 2000} `——像找到了宝藏。

> "嵌套结构体让数据模型像真正的世界：事物包含事物，世界包含世界。"

## 本章适合谁

- 已经学了 [结构体基础](./struct_basics.md)，知道 `.` 访问成员
- 想把复杂数据分层组织，而不是全部塞到一个结构体里
- 好奇 C 语言如何实现"多层数据模型"

## 你会学到什么

- 嵌套结构体的定义与初始化
- 多级成员访问（`.` 链式）
- 部分初始化与零初始化规则
- 嵌套结构体的赋值与拷贝
- 深层嵌套（3 层以上）访问模式

## 前置要求

- 完成 [结构体基础](./struct_basics.md) 章节

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

struct Date {
    int32_t year;
    int32_t month;
    int32_t day;
};

struct Person {
    char name[32];
    struct Date birthday;
};

int main(void) {
    struct Person p = {
        .name = "Alice",
        .birthday = {.year = 2000, .month = 6, .day = 15},
    };
    printf("%s born: %04d-%02d-%02d\n",
           p.name, p.birthday.year, p.birthday.month, p.birthday.day);
    return 0;
}
```

输出：
```
Alice born: 2000-06-15
```

关键：访问嵌套成员用链式 `.` — `p.birthday.year`。每个点进入一层。

## 原理解析

### 1. 嵌套结构体的定义

```c
/* 内层结构体先定义 */
struct Date {
    int32_t year, month, day;
};

/* 外层结构体使用内层类型 */
struct Person {
    char name[32];
    struct Date birthday;  /* 嵌套 */
};
```

内层结构体必须在外部可见（先定义或前置声明）。

### 2. 初始化嵌套结构体

```c
/* 位置初始化 */
struct Person p1 = { "Bob", {1999, 1, 1} };

/* 指定初始化（推荐）— 清晰 */
struct Person p2 = {
    .name     = "Charlie",
    .birthday = {.year = 2001, .month = 12, .day = 25},
};

/* 只初始化一个嵌套字段，其余归零 */
struct Person p3 = { .birthday.year = 2000 };
/* p3.name = "", p3.birthday.month = 0, p3.birthday.day = 0 */
```

### 3. 链式访问

```c
printf("%d", p.birthday.year);    /* 两层访问 */
```

每多一个 `.` 就进入一层嵌套。超过 3 层就考虑重构了——太深的数据访问链是代码坏味道。

### 4. 嵌套结构体赋值

```c
struct Person a = { .name = "Dave", .birthday = {.year = 1998} };
struct Person b = a;  /* 深拷贝 — 嵌套部分也复制 */

b.birthday.year = 2020;
printf("a.birthday.year = %" PRId32 "\n", a.birthday.year);  /* 仍然是 1998 */
```

**我发现**：很多人担心嵌套结构体赋值是"浅引用"。C 不是这样——`b = a` 会把所有嵌套字段**逐字节**复制过来。

### 5. 内联定义（匿名嵌套）

```c
struct Person {
    char name[32];
    struct { int32_t year, month, day; } birthday;
} p = { .name = "Eve" };
```

匿名嵌套结构体需要外层有个名字来访问内层成员，否则无法初始化。

## 常见错误（Error-First）

### ❌ 错误 1: 内层结构体未定义就使用

```c
struct Person {
    struct Date birthday;  /* ❌ Date 还没定义！ */
};

struct Date { int32_t year; };  /* 定义太晚了 */
```

✅ **修复**: 先定义内层，或用前置声明

```c
struct Date;  /* 前置声明 — 允许指针，不允许值 */
```

但如果是嵌套值（不是指针），必须完整定义在前。

### ❌ 错误 2: 多层指定初始化写成两级

```c
struct Person p;
p.birthday = { .year = 2000 };  /* ❌ 不能在赋值时用 {} */
```

✅ **修复**: 只能在声明时聚合初始化

```c
p.birthday.year = 2000;  /* ✅ 逐字段赋值 */
```

## 动手练习

<details>
<summary>🟢 入门: 嵌套 Book 结构体</summary>

定义 `Author`（name, country）和 `Book`（title, Author author, int pages）。创建实例打印。

</details>

<details>
<summary>🟡 中级: 3 层嵌套</summary>

定义 `Inner`（value）、`Middle`（Inner inner, id）、`Outer`（Middle middle, name）。用指定初始化访问最深层的值。

</details>

## 故障排查（FAQ）

<details>
<summary>Q: 嵌套结构体的 sizeof 怎么算？</summary>

把内层结构体当作一个整体，按照外层字段的对齐规则计算。内层的对齐要求会传递给外层：

```
struct Inner { int32_t a; char b; };   /* sizeof = 8 (4+1+3 pad) */
struct Outer { char x; struct Inner i; };  /* sizeof = 12 (1+3 pad+8)*/
```

</details>

<details>
<summary>Q: 嵌套结构体能用 == 比较吗？</summary>

不能。和顶层结构体一样，C 不支持 `==` 比较结构体（包括嵌套结构体），需要逐字段比较。

</details>

## 知识扩展

### C11 匿名结构体

```c
struct Vec3 {
    union {
        struct { float x, y, z; };
        float data[3];
    };
};
/* v.x 和 v.data[0] 访问同一内存 */
```

## 小结

- 嵌套结构体 = 结构体成员本身也是一个结构体
- 访问用链式 `.` — `outer.inner.field`
- 指定初始化支持嵌套：`.inner = {.field = value}`
- 拷贝是深复制，不是引用
- 内层结构体类型必须先完整定义

## 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| 嵌套结构体 | Nested Struct | 结构体的成员本身也是结构体 |
| 链式访问 | Chained Access | 用多个 `.` 逐层访问 |
| 深拷贝 | Deep Copy | 嵌套字段也会被完整复制 |
| 内联定义 | Inline Definition | 在外层结构体内定义内层类型 |
| 前置声明 | Forward Declaration | 提前声明类型，后续再定义 |

## 延伸阅读

- [cppreference: Structures](https://en.cppreference.com/w/c/language/struct)
- K&R《C 程序设计语言》§6.2

## 继续学习

| 方向 | 链接 |
|------|------|
| 上一章 → | [结构体基础](./struct_basics.md) |
| 下一章 → | [结构体与函数](./struct_functions.md) — 传值、传指针、返回结构体 |
