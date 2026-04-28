# 结构体基础（Struct Basics）

结构体就像一张名片——姓名、电话、邮箱打包在一张卡片上。

## 开篇故事

想象你去面试，HR 要求你填写一张登记表——姓名、年龄、部门、工号，全部填在同一张纸上。如果你用四个独立的变量来管理这些信息，每次传递就要传四个参数；如果有人离职了，你要同时修改四个地方。

结构体就是 C 语言给出的答案。它把所有相关的数据打包成一个"实体"，一次传递、一次修改、一次思考。

```c
struct Employee {
    char name[32];
    int32_t age;
    char department[16];
    int32_t badge;
};
```

第一次见到结构体时，我以为它只是"把变量放到一起"。后来我才发现，它是 C 语言中**唯一**能把不同类型数据组合成有意义整体的方式。没有结构体，C 代码就是一堆散落在各处的变量；有了结构体，代码开始表达"东西"。

> "结构体让代码从'一堆变量'变成'有意义的实体'。"

## 本章适合谁

- 已经掌握了 C 语言的基本变量和数据类型
- 想把不同类型的数据（`int`、`char[]`、`float`）打包在一起
- 好奇 Python 的 `dict`、Java 的 `class` 在 C 中等价是什么的人

## 你会学到什么

- 结构体的定义与声明语法 (`struct`)
- 顺序初始化与指定初始化 (designated initializers)
- 成员访问运算符 (`.`) 
- 结构体数组
- 结构体赋值与浅拷贝
- 指定初始化的边界情况

## 前置要求

- 完成 [变量](./variables.md) 和 [数据类型](./datatype.md) 章节
- 理解 [数组](./arrays.md) 的基本概念

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

struct Student {
    char name[32];
    int32_t age;
    float score;
};

int main(void) {
    struct Student stu = {"Alice", 20, 95.5f};
    printf("Name: %s, Age: %d, Score: %.1f\n",
           stu.name, stu.age, stu.score);
    return 0;
}
```

输出：
```
Name: Alice, Age: 20, Score: 95.5
```

### 分步解析

1. `struct Student { ... };` — 定义结构体类型，包含三个成员
2. `struct Student stu = {...}` — 创建变量并初始化
3. `stu.name` — 通过 `.` 运算符访问成员

## 原理解析

### 1. 结构体定义：三种写法

```c
/* 方式 A: 先定义类型，再声明变量（推荐） */
struct Student {
    char name[32];
    int32_t age;
    float score;
};
struct Student s1;

/* 方式 B: 定义和声明一起做 */
struct Student {
    char name[32];
    int32_t age;
} s2;

/* 方式 C: 匿名结构体（一次性的，少用） */
struct {
    char name[32];
    int32_t age;
} s3;
```

**我的建议**：用方式 A。类型和变量分离，后续可以声明多个变量，可读性最好。

### 2. 初始化方式

```c
/* 顺序初始化 — 必须按声明顺序 */
struct Student s1 = {"Bob", 21, 88.0f};

/* 指定初始化 (C99, 推荐) — 顺序随意, 可省略 */
struct Student s2 = {
    .name  = "Charlie",
    .score = 92.5f,
    /* .age 没写 → 自动初始化为 0 */
};

/* 全部归零 */
struct Student s3 = { 0 };
```

**我发现**：指定初始化是 C 最被低估的特性之一。它允许你只填写关心的字段，其余自动归零——这在写 API 时非常有用。

### 3. 成员访问：`.` 运算符

```c
struct Student s = { .name = "Dave", .age = 22 };

/* 读取 */
printf("%s\n", s.name);

/* 修改 */
s.age = 23;

/* 参与运算 */
s.score = s.score + 5.0f;
```

> **关键**：`.` 左侧必须是结构体变量本身（不是指针）。指针用 `->`，见下一章《结构体与函数》。

### 4. 结构体数组

```c
struct Student class[3] = {
    { .name = "A", .age = 20 },
    { .name = "B", .age = 21 },
    { .name = "C", .age = 22 },
};

for (int32_t i = 0; i < 3; i++) {
    printf("%s (age %d)\n", class[i].name, class[i].age);
}
```

### 5. 结构体赋值

```c
struct Student a = { .name = "Old", .age = 18 };
struct Student b = a;  /* 浅拷贝 — 逐字段复制 */

b.age = 99;  /* 修改 b 不影响 a */
printf("a.age = %d  (still 18)\n", a.age);
```

**我犯过的错**：以为结构体赋值是"引用"。不是——它复制所有字段的数据。修改拷贝不会影响原始结构体。

## 常见错误（Error-First）

### ❌ 错误 1: 未初始化的成员

```c
struct Student stu;  /* 栈上未初始化 → 垃圾值 */
printf("score: %f\n", stu.score);  /* ❌ 随机数字 */
```

✅ **修复**: 初始化或归零

```c
struct Student stu = { 0 };     /* 全部归零 */
struct Student stu2 = { .age = 20 };  /* 其余自动归零 */
```

### ❌ 错误 2: 用 `==` 比较两个结构体

```c
struct Student a = { .age = 20 };
struct Student b = { .age = 20 };
/* if (a == b) */  /* ❌ C 不支持结构体 == 比较 */
```

✅ **修复**: 逐成员比较

```c
if (a.age == b.age && strcmp(a.name, b.name) == 0) {
    /* 相等 */
}
```

### ❌ 错误 3: 忘记 `struct` 前缀

```c
Student stu;  /* ❌ 编译错误: 不知道 Student 是什么 */
```

✅ **修复**:

```c
struct Student stu;  /* ✅ 正确 */
```

## 动手练习

<details>
<summary>🟢 入门: 创建 Book 结构体</summary>

定义 `Book` 结构体（title、author、price），创建实例并打印。

```c
struct Book {
    char title[64];
    char author[32];
    float price;
};

struct Book b = { .title = "C Primer Plus", .author = "Stephen Prata", .price = 89.0f };
printf("%s by %s: ¥%.2f\n", b.title, b.author, b.price);
```

</details>

<details>
<summary>🟡 中级: 指定初始化求平均分</summary>

创建 `Student` 数组，用指定初始化存入 3 人成绩，计算平均分。

</details>

## Python dict vs C struct 对比

| 特性 | C `struct` | Python `dict` | Python `class` |
|------|-----------|---------------|----------------|
| 类型安全 | ✅ 编译时 | ❌ 运行时 | ⚠️ 运行时报错 |
| 内存布局 | 连续、紧凑 | 哈希表分散存储 | 对象头 + `__dict__` |
| 方法 | ❌ 只有数据 | ❌ 只有数据 | ✅ 可以包含方法 |
| sizeof | ✅ 编译期已知 | ❌ 不暴露 | ❌ 不暴露 |
| 动态字段 | ❌ 编译期固定 | ✅ 运行时添加 | ✅ 运行时添加 |
| 内存开销 | 紧凑（~字段总大小+padding） | ~240 字节起 + 每个键值对对象 | ~48 字节对象头 + `__dict__` |

```
C struct 内存布局（紧凑连续）:
┌──────────────┬────────┬──────────────┐
│ name: 32 字节 │ age: 4 │ score: 4 字节 │ = 40-44 字节
└──────────────┴────────┴──────────────┘

Python dict 内存布局（分散有开销）:
dict object → hash table → {"name": str_obj, "age": int_obj, ...}
  ~240 bytes base + 每个对象 ~28 bytes + 哈希冲突开销
```

## 故障排查（FAQ）

<details>
<summary>Q: 为什么初始化列表报 "excess elements"？</summary>

初始化元素数量超过了结构体成员数量。用指定初始化可以避免：

```c
struct Point { int32_t x; int32_t y; };
struct Point p = {.x = 1, .y = 2};  /* ✅ 清晰且安全 */
```

</details>

<details>
<summary>Q: 结构体可以包含自己吗？</summary>

不能直接包含（无限递归大小），但可以包含指向自己的指针：

```c
/* ❌ 编译错误 */
struct Node { struct Node next; };

/* ✅ 正确 — 指针大小固定 */
struct Node { int32_t data; struct Node *next; };
```

</details>

## 知识扩展

### 匿名嵌套结构体（C11）

C11 允许匿名嵌套结构体，可以直接访问内层成员：

```c
struct Point3D {
    struct { float x; float y; float z; };  /* 匿名 */
};
struct Point3D p;
p.x = 1.0f;  /* 直接访问，不需要 p.inner.x */
```

## 小结

这一章我发现：

- `struct` 把不同类型数据打包成一个有意义的实体
- 初始化有顺序和指定两种，推荐指定初始化
- `.` 运算符访问结构体成员（. 左侧是结构体变量）
- 结构体赋值是浅拷贝，不是引用
- 数组中的结构体可以通过 `[i].member` 访问

## 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| 结构体 | Structure (struct) | 将不同类型数据组合在一起的复合类型 |
| 成员 | Member / Field | 结构体中的字段 |
| 成员访问 | Member Access | 用 `.` 运算符访问结构体成员 |
| 指定初始化 | Designated Initializer (C99) | 用 `.member = value` 语法 |
| 浅拷贝 | Shallow Copy | 结构体赋值逐字段复制 |
| 匿名结构体 | Anonymous Struct | 没有类型名的结构体 |

## 延伸阅读

- [cppreference: Structures and unions](https://en.cppreference.com/w/c/language/struct)
- [Beej's Guide: Structures](https://beej.us/guide/bgc/output/html/multipage/structures.html)
- K&R《C 程序设计语言》第 6 章

## 继续学习

| 方向 | 链接 |
|------|------|
| 上一章 → | [const 正确性](./const_correctness.md) |
| 下一章 → | [嵌套结构体](./nested_structs.md) — struct 里面套 struct |
