# typedef（类型别名）

给类型起个外号，让你少打几个字。

## 开篇故事

每次写 C 代码，我都要在 struct 前面加 `struct`——`struct Student s; struct Rectangle r;`。这就像每次叫人名字前都要说"先生"——"张先生你好，李先生再见"——礼貌过头了。

`typedef` 就是给类型起外号。一次定义，以后直接用。`Student s;` 比 `struct Student s;` 简洁多了。

我第一次用 `typedef` 是因为看 Linux 内核代码——内核里几乎看不到裸的 `struct`，全是 `struct task_struct *current` 这种。后来发现 `typedef` 在函数指针里才是真正的救星：`typedef int (*CompareFn)(const void *, const void *);`，没有它写出来的声明像天书。

> "typedef 不是创建新类型，而是给已有类型起个好记的外号。"

## 本章适合谁

- 已经会定义和使用结构体
- 觉得 `struct StructName` 太繁琐的 C 学习者
- 想理解函数指针声明的读者

## 你会学到什么

- `typedef` 的基本语法和语义
- `typedef struct` 简化结构体声明
- `typedef` 用于数组和指针
- 函数指针 typedef（简化复杂声明）
- Callback 模式的 typedef 实践

## 前置要求

- 完成 [结构体内存布局](./struct_memory_layout.md) 章节
- 理解 [函数](./functions.md) 的基本概念

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

typedef struct Student {
    char name[32];
    int32_t age;
} Student;

/* 现在可以这样声明： */
Student s = { .name = "Alice", .age = 20 };

int main(void) {
    printf("%s, age %" PRId32 "\n", s.name, s.age);
    return 0;
}
```

不需要写 `struct Student s`——直接 `Student s`。少打了 7 个字符，但更关键的是代码意图更清晰。

## 原理解析

### 1. typedef 的本质

```c
typedef int32_t Age;

Age a = 20;    /* 等价于 int32_t a = 20; */
```

`typedef` 不创建新类型，只是给已有类型加了一个"别名"。编译器眼里 `Age` 和 `int32_t` 完全一样。

### 2. typedef + struct

```c
/* 传统写法 */
struct Student { char name[32]; int32_t age; };
struct Student s;  /* 必须写 struct */

/* typedef 写法 */
typedef struct Student {
    char name[32];
    int32_t age;
} Student;
Student s;  /* 不需要 struct 前缀 */
```

这是 C 项目中**最常见**的 typedef 用法。C++ 不需要——C++ 的 struct 名自动成为类型名。

```
Python 的 class/实例          C struct + typedef
─────────────────           ─────────────────
class Student:                typedef struct Student { ... } Student;
    pass                      Student s;  ← 简洁声明
s = Student()                 s = (Student){ .name = "A" };
```

### 3. Python dict vs C typedef 对比

| 特性 | C `typedef` | Python `dict` |
|------|------------|---------------|
| 类型安全 | ✅ 编译时检查（别名仍是原类型） | ❌ 运行时才能发现键不存在 |
| IDE 提示 | ✅ 完整的成员自动补全 | ⚠️ 字典键只能靠字符串推断 |
| sizeof | ✅ 编译期可知 | ❌ 不暴露 |
| 内存开销 | 0（只是编译期别名） | 每次创建字典 ~240 bytes |
| 运行时修改 | ❌ 编译期固定 | ✅ 可动态加字段 |
| 性能 | 直接访问字段（CPU 友好） | 哈希表查找（有开销） |

```
C typedef + struct:
  Student s;                    ← 编译期解析为 struct Student
  s.name = "Alice";             ← 直接内存偏移，零运行时开销

Python dict:
  s = {"name": "Alice"}          ← 创建 dict 对象 + hash table
  s["name"]                      ← 哈希查找 "name" → 返回值
```

### 4. typedef 用于数组和指针

```c
typedef int32_t IntArray10[10];   /* IntArray10 是 "含10个int32_t的数组" */
typedef char *StringPtr;          /* StringPtr 是 char* 的别名 */

IntArray10 arr;    /* 等价于 int32_t arr[10]; */
StringPtr greeting = "hello";     /* 等价于 char *greeting = "hello"; */
```

### 5. 函数指针 typedef

```c
/* 没有 typedef: 天书 */
int32_t (*fp)(int32_t, int32_t) = &add;

/* 用 typedef: 清晰 */
typedef int32_t (*BinaryOp)(int32_t, int32_t);
BinaryOp fp = &add;

/* 作为函数参数 */
int32_t apply(BinaryOp op, int32_t x, int32_t y) {
    return op(x, y);
}
```

**我发现**：函数指针不加 typedef 几乎不可读。加 typedef 后，意图一目了然。

### 6. Callback 模式

```c
typedef void (*Visitor)(int32_t index, int32_t value, void *context);

void traverse(int32_t data[], int32_t len, Visitor visit, void *ctx) {
    for (int32_t i = 0; i < len; i++) {
        visit(i, data[i], ctx);
    }
}
```

这是 C 语言实现"遍历框架 + 自定义回调"的标准模式——类似于 Python 的 `for item in iterable: callback(item)`。

## 常见错误（Error-First）

### ❌ 错误 1: typedef 不是新类型

```c
typedef int32_t Age;
Age a = 20;
int32_t b = a;  /* ✅ 编译完全一致 — Age 就是 int32_t */
```

如果你想要"类型安全"的 Age（不允许和 int 混用），typedef 做不到——你要么用 struct 包装，要么用 Rust。

### ❌ 错误 2: typedef 位置导致误解

```c
typedef char *StringPtr;
StringPtr p1, p2;  /* ✅ 两个都是 char* */

char *p3, p4;      /* ⚠️ p3 是 char*, p4 是 char！ */
```

用 typedef 可以避免这种常见的 C 陷阱。

## 动手练习

<details>
<summary>🟢 入门: 给 Point 结构体加 typedef</summary>

把 `struct Point { float x, y; };` 改为 typedef 版本，验证可以省略 `struct` 关键字。

</details>

<details>
<summary>🟡 中级: 写一个 filter 函数</summary>

```c
typedef bool (*Predicate)(int32_t value);
int32_t filter(int32_t data[], int32_t len, Predicate pred, int32_t out[]);
```

</details>

## 故障排查（FAQ）

<details>
<summary>Q: typedef 和 #define 有什么区别？</summary>

`typedef` 是编译器层面的类型别名，受类型系统约束。`#define` 是文本替换，可能出错：

```c
#define IntPtr int*
IntPtr a, b;  /* 展开为: int* a, b; → b 是 int，不是 int*! */

typedef int *IntPtr;
IntPtr a, b;  /* ✅ a 和 b 都是 int* */
```

</details>

## 知识扩展

### 复杂声明的阅读方法：螺旋法则

```
void (*signal(int, void (*)(int)))(int);
   └─── 最内层 ─┘
   signal 是函数，参数是 int 和 void(*)(int)，返回 void(*)(int)

用 typedef 简化:
typedef void (*SigHandler)(int);
SigHandler signal(int, SigHandler);  /* 清晰多了 */
```

## 小结

- typedef 给类型起别名，不创建新类型
- `typedef struct` 是最常见用法：省去 `struct` 前缀
- 函数指针 typedef 大幅简化声明
- Callback 模式依赖 typedef 建立清晰接口
- `typedef` 比 `#define` 类型安全得多

## 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| 类型别名 | Type Alias | 给已有类型一个简短新名字 |
| typedef | Type Definition | 关键字，声明类型别名 |
| 函数指针 | Function Pointer | 指向函数入口地址的指针 |
| Callback | 回调 | 通过函数指针把行为作为参数传入 |
| 螺旋法则 | Clockwise/Spiral Rule | 从内到外阅读 C 声明的方法 |

## 延伸阅读

- [cppreference: Typedef](https://en.cppreference.com/w/c/language/typedef)
- K&R《C 程序设计语言》§5.11: Typedef
- [The "Clockwise Rule"](https://en.wikipedia.org/wiki/C_operator_precedence)

## 继续学习

| 方向 | 链接 |
|------|------|
| 上一章 → | [结构体内存布局](./struct_memory_layout.md) |
| 下一章 → | [联合体](./unions.md) — 共享内存的数据类型 |
