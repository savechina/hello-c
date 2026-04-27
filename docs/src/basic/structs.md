# 结构体（Structures）

想象你有一张名片。姓名、电话、邮箱打包在一张卡片上。你不需要拿三张纸条——一张写名字，一张写电话，一张写邮箱——递出去时还得担心会不会少了一张。

```c
// 单个变量只能装一个值
int age = 20;
// 数组只能装同一种类型
int scores[5];
// 但一个"学生"既有字符串、又有整数、还有浮点数！
```

结构体就是 C 语言给出的答案——它像一个文件袋，可以把不同类型的数据装在一起，贴上标签，统一管理。

## 开篇故事

想象你有一张名片。上面印着姓名、电话、邮箱，三个信息打包在一张卡片上。你不需要带着三张纸条——一张写名字，一张写电话，一张写邮箱——每次递出去的时候还得担心会不会少了一张。

结构体就是名片的作用。它把不同类型的变量捆成一个整体，贴好字段名，统一管理。

```c
struct Student {
    char name[32];
    int32_t age;
    float score;
};
// 一个学生 = 一份完整档案，不是三个散落在各处的抽屉
```

没有结构体之前，管理 100 个学生要同时维护三个数组——删一个学生，三个数组都要改。稍不留神就错位。有了结构体之后，学生是一个「实体」。删除就是删一份，传递就是传一份，代码的意图变得清晰。

> "结构体让代码从'一堆变量'变成'有意义的东西'。"

## 本章适合谁

- 已经了解 C 语言基本变量和数据类型
- 想学怎么把不同类型的数据（int、char[]、float）打包在一起
- 好奇 Python 的 `class` / `dict` 在 C 语言中等价是什么的人

## 你会学到什么

- 结构体的定义与声明语法（`struct` 关键字）
- 结构体初始化（顺序初始化、指定初始化 designated initializers）
- 成员访问运算符（`.` 和 `->`）
- 嵌套结构体（struct 里面套 struct）
- 把结构体作为函数参数传递
- 函数返回结构体

## 前置要求

- 完成 [变量](./variables.md) 和 [数据类型](./datatype.md) 章节
- 理解 [函数](./functions.md) 的基本概念

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

// 定义一个结构体类型
struct Student {
    char name[32];
    int32_t age;
    float score;
};

int main(void) {
    // 创建一个 Student 变量并初始化
    struct Student stu = {"张三", 20, 95.5f};
    
    // 访问成员
    printf("姓名: %s\n", stu.name);
    printf("年龄: %d\n", stu.age);
    printf("成绩: %.1f\n", stu.score);
    
    return 0;
}
```

输出：
```
姓名: 张三
年龄: 20
成绩: 95.5
```

### 分步解析

1. `struct Student { ... };` — 定义了一个结构体类型，包含三个成员
2. `struct Student stu = {...}` — 创建一个结构体变量并初始化
3. `stu.name` — 通过 `.` 运算符访问成员

## 原理解析

### C struct vs Python class / dict

作为从 Python 过来的程序员，我第一次看到 C 的 struct 时，忍不住把它和 Python 做比较：

| 特性 | C `struct` | Python `class` | Python `dict` |
|------|-----------|----------------|---------------|
| 类型安全 | ✅ 编译时检查 | ⚠️ 运行时检查 | ❌ 无类型 |
| 内存布局 | 连续内存，紧凑 | 对象头 + 动态字典 | 哈希表，开销大 |
| 方法 | ❌ 只能有数据 | ✅ 可以有方法 | ❌ 只能有数据 |
| 继承 | ❌ 不支持 | ✅ 支持 | ❌ 不支持 |
| sizeof | ✅ 编译时可知 | ❌ 不暴露 | ❌ 不暴露 |
| 可变字段 | ❌ 编译时固定 | ✅ 运行时可加 | ✅ 运行时可加 |
| 内存占用 | 紧凑（有 padding） | 较大（对象头部 ~48 字节+） | 更大（哈希表开销 ~240 字节起始） |

```
C struct 的内存布局（紧凑、连续）:
[ name: 32 bytes | age: 4 bytes | padding: 4 bytes | score: 4 bytes ] = 44 bytes (aligned to 48)

Python dict 的内存布局（分散、有哈希开销）:
dict object → hash table → {key: value, key: value, ...}
每个键值对还有独立的 str object + int object
```

### 1. 结构体定义语法

```c
// 方式 1: 先定义类型，再声明变量
struct Student {
    char name[32];
    int32_t age;
    float score;
};

struct Student stu1;  // 声明变量

// 方式 2: 定义类型的同时声明变量
struct Student {
    char name[32];
    int32_t age;
    float score;
} stu2;  // 紧跟定义

// 方式 3: 匿名结构体（较少用）
struct {
    char name[32];
    int32_t age;
} stu3;  // 无类型名，只能声明这一次
```

### 2. 结构体初始化

```c
// 方式 1: 顺序初始化
struct Student stu1 = {"李四", 21, 88.0f};

// 方式 2: 指定初始化 (C99, 推荐!)
struct Student stu2 = {
    .name = "王五",
    .age = 22,
    .score = 92.5f
};
// ✅ 指定初始化的优势: 可以只初始化部分成员
//    未指定的成员自动初始化为 0 / NULL
struct Student stu3 = { .age = 20 };  // 其余 = { '\0', 0, 0.0f }

// 方式 3: 逐个成员赋值
struct Student stu4;
// ❌ stu4 = {"赵六", 19, 76.0f}; // 错误！只能在声明时这样做
stu4.age = 19;
stu4.score = 76.0f;
// 对于数组成员，需要用 strcpy 或 strncpy
```

### 3. 成员访问（`.` 运算符）

```c
struct Student stu = {.name = "小明", .age = 20, .score = 90.0f};

// 使用 '.' 访问成员
printf("姓名: %s\n", stu.name);   // 直接访问
stu.age = 21;                      // 修改成员
stu.score = stu.score + 5.0f;      // 参与运算
```

> **关键点**：`.` 左侧必须是结构体变量本身（不是指针）。

### 4. 嵌套结构体

```c
struct Date {
    int32_t year;
    int32_t month;
    int32_t day;
};

struct Student {
    char name[32];
    struct Date birthday;  // 嵌套结构体
    float score;
};

struct Student stu = {
    .name = "小红",
    .birthday = {.year = 2003, .month = 5, .day = 18},
    .score = 96.0f
};

// 访问嵌套成员用多个 '.'
printf("生日: %d-%02d-%02d\n",
       stu.birthday.year,
       stu.birthday.month,
       stu.birthday.day);
```

### 内存布局：struct 在内存中

```
struct Student stu = {.name="张三", .age=20, .score=95.5};

内存地址（低 → 高）:
┌──────────────┐
│  name[0..31] │  32 bytes  (姓名: "张三\0..." )
├──────────────┤
│      age     │   4 bytes  (20)
├──────────────┤
│   (padding)  │   4 bytes  (对齐填充，编译器自动插入)
├──────────────┤
│    score     │   4 bytes  (95.5f)
└──────────────┘
  Total: 44 bytes (通常对齐到 48)
```

> 💡 **关键**：结构体中可能存在 **padding**（填充字节），这是 CPU 内存对齐要求导致的。具体见下一章《结构体字段与内存布局》。

## 常见错误

### ❌ 错误 1: 未初始化的成员

```c
struct Student stu;  // 未初始化！

// ❌ 成员的值的未定义的（garbage value）
printf("分数: %f\n", stu.score);  // 输出随机数！
```

✅ **修正**: 初始化全部或归零

```c
// 方式 A: 全部归零
struct Student stu = {0};

// 方式 B: 指定初始化（更安全）
struct Student stu = {
    .name = "",
    .age = 0,
    .score = 0.0f
};
```

### ❌ 错误 2: 用 `==` 比较两个结构体

```c
struct Student a = {.name = "小明", .age = 20};
struct Student b = {.name = "小明", .age = 20};

// ❌ 不能用 == 比较结构体！C 标准不支持
// if (a == b) { ... }  // 编译错误！
```

✅ **修正**: 逐成员比较或使用 `memcmp`（注意 padding 可能影响结果）

```c
if (a.age == b.age && strcmp(a.name, b.name) == 0) {
    // 手动逐个比较
}
```

### ❌ 错误 3: struct 关键字遗忘

```c
Student stu;  // ❌ C 语言不行！必须写 struct Student
```

✅ **修正**: C 语言中必须写 `struct` 前缀（或者用 `typedef`）

```c
struct Student stu;  // ✅ 正确
// 或者
typedef struct Student Student;
Student stu2;        // ✅ 用 typedef 后不需要 struct
```

## 动手练习

<details>
<summary>🟢 入门: 创建并打印 Book 结构体</summary>

定义一个 `Book` 结构体，包含书名（title）、作者（author）、价格（price），创建实例并打印。

```c
#include <stdio.h>
#include <stdint.h>

struct Book {
    char title[64];
    char author[32];
    float price;
};

int main(void) {
    struct Book b = {.title = "C Primer Plus", .author = "Stephen Prata", .price = 89.0f};
    printf("书名: %s\n", b.title);
    printf("作者: %s\n", b.author);
    printf("价格: ¥%.2f\n", b.price);
    return 0;
}
```

**输出**:
```
书名: C Primer Plus
作者: Stephen Prata
价格: ¥89.00
```

</details>

<details>
<summary>🟡 中级: 结构体数组与计算平均分</summary>

创建 `Student` 结构体数组，存入 3 名学生信息，计算并输出平均分。

```c
#include <stdio.h>
#include <stdint.h>

struct Student {
    char name[32];
    float score;
};

int main(void) {
    struct Student class[3] = {
        {.name = "张三", .score = 85.0f},
        {.name = "李四", .score = 92.0f},
        {.name = "王五", .score = 78.0f},
    };
    
    float sum = 0.0f;
    int32_t n = (int32_t)(sizeof(class) / sizeof(class[0]));
    for (int32_t i = 0; i < n; i++) {
        sum += class[i].score;
        printf("%s: %.1f\n", class[i].name, class[i].score);
    }
    printf("平均分: %.1f\n", sum / n);
    return 0;
}
```

**输出**:
```
张三: 85.0
李四: 92.0
王五: 78.0
平均分: 85.0
```

</details>

<details>
<summary>🔴 挑战: 嵌套结构体 + 函数返回</summary>

定义 `Point` 和 `Circle` 结构体（嵌套 Point），编写一个函数计算两个圆的面积差。

```c
#include <stdio.h>
#include <stdint.h>

struct Point {
    float x;
    float y;
};

struct Circle {
    struct Point center;
    float radius;
};

float circle_area(struct Circle c) {
    return 3.14159f * c.radius * c.radius;
}

struct Circle create_circle(float x, float y, float r) {
    struct Circle c;
    c.center.x = x;
    c.center.y = y;
    c.radius = r;
    return c;  // 函数返回 struct
}

int main(void) {
    struct Circle c1 = create_circle(0.0f, 0.0f, 5.0f);
    struct Circle c2 = create_circle(3.0f, 4.0f, 3.0f);
    
    printf("圆1 面积: %.2f\n", circle_area(c1));
    printf("圆2 面积: %.2f\n", circle_area(c2));
    printf("面积差:  %.2f\n", circle_area(c1) - circle_area(c2));
    return 0;
}
```

**输出**:
```
圆1 面积: 78.54
圆2 面积: 28.27
面积差:  50.27
```

</details>

## 故障排查

<details>
<summary>Q: 为什么我的结构体初始化报错 "excess elements in struct initializer"？</summary>

A: 初始化列表中的元素数量超过了结构体的成员数量。

```c
struct Point { int32_t x; int32_t y; };
struct Point p = {1, 2, 3};  // ❌ 3 个值但只有 2 个成员
```

✅ 修正——检查成员数量，或使用指定初始化：

```c
struct Point p = {.x = 1, .y = 2};  // ✅ 清晰且安全
```

</details>

<details>
<summary>Q: 结构体可以包含自己吗？（递归定义）</summary>

A: **不能直接包含**，但可以包含指向自己的指针。

```c
// ❌ 不行——无限递归大小
struct Node {
    struct Node next;  // 编译错误！sizeof 无法计算
};

// ✅ 可以——指针大小固定
struct Node {
    int32_t data;
    struct Node *next;  // 指针大小固定（8 bytes on 64-bit）
};
```

</details>

<details>
<summary>Q: 为什么 struct 前面要加 struct 关键字？太繁琐了！</summary>

A: C 语言的设计哲学是"显式优于隐式"。`struct Student` 明确表示"这是一个结构体类型"，与函数、变量名不冲突。

如果你嫌繁琐，可以用 `typedef`：

```c
typedef struct Student {
    char name[32];
    int32_t age;
} Student;

Student s = {"小明", 20};  // ✅ 不需要写 struct
```

</details>

## 知识扩展

### typedef 简化结构体

```c
// 传统写法（每次都要写 struct）
struct Point p1;

// typedef 写法（省略 struct）
typedef struct Point {
    float x;
    float y;
} Point;

Point p2;  // 简洁！
```

### union —— 共享内存的 "结构体"

与 struct 类似，但所有成员**共享同一块内存**，大小等于最大成员：

```c
union Data {
    int32_t i;
    float f;
    char str[8];
};
// sizeof(union Data) = 8（最长的成员）
```

## 小结

这一章我发现：

- `struct` 把不同类型的数据打包成一个有意义的整体
- 初始化有顺序初始化、指定初始化（`.member = value`）两种
- `.` 运算符访问结构体成员
- 嵌套结构体可以组合更复杂的数据模型
- 函数可以接收结构体参数、也可以返回结构体
- C 的 struct 比 Python class 更轻量——没有方法、没有动态属性、没有额外开销

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 结构体 | Structure (struct) | 将不同类型数据组合在一起的复合类型 |
| 成员 | Member | 结构体中的字段/变量 |
| 成员访问运算符 | Member access operator | `.` 运算符，用于访问结构体成员 |
| 指定初始化 | Designated initializer (C99) | 用 `.member = value` 语法显式指定成员值 |
| 嵌套结构体 | Nested struct | 结构体的成员本身也是一个结构体 |
| typedef | Type definition | 为类型创建别名，简化声明 |
| 结构体数组 | Array of structs | 由结构体变量组成的数组 |
| 匿名结构体 | Anonymous struct | 没有类型名的结构体 |

## 延伸阅读

- [cppreference: Structures and unions](https://en.cppreference.com/w/c/language/struct)
- [Beej's Guide: Structures](https://beej.us/guide/bgc/output/html/multipage/structures.html)
- K&R《C 程序设计语言》第 6 章：Structures

## 继续学习

| 下一步 | 方向 |
|--------|------|
| 下一章 → | [结构体字段与内存布局](./struct_fields.md) — Padding、对齐、位域 |
| 复习 ← | [函数](./functions.md) — 参数传递、返回值 |
| 衔接 → | [指针](../pointers/pointers.md) — 结构体指针与 `->` 运算符 |
