# 结构体与函数（Struct Functions）

结构体传给函数，是整份拷贝过去，还是只传个地址？

## 开篇故事

想象你要给朋友看你的简历。你有两种选择：

1. **复印一份**整份简历给他（传值）——他改不了你手里的原件，但如果简历有 100 页，复印很费时
2. **把地址告诉他**，让他去你家里看（传指针）——零拷贝，他改了你家的那份也会变

C 语言函数传结构体也有这两种方式。默认是方式 1（传值 = 整份拷贝），但通常我们用方式 2（传指针 = 只传地址）。

我第一次写结构体函数时，传了一个 200 字节的大结构体进去，函数调用像蜗牛一样慢。后来改成传指针，性能立刻提升。那之后我再也不用值传递传递大结构体了。

> "小结构体传值没问题，大结构体传指针是铁律。"

## 本章适合谁

- 已经会定义和使用结构体
- 知道函数参数传递的基本概念
- 想知道 struct 作为参数和返回值时的行为

## 你会学到什么

- 按值传递结构体（整份拷贝）
- 按指针传递结构体（零拷贝）
- `const` 指针：承诺不修改
- 函数返回结构体
- 结构体数组 + 指针运算

## 前置要求

- 完成 [嵌套结构体](./nested_structs.md) 章节
- 理解 [指针](./pointers.md) 的基本概念

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

struct Point { float x; float y; };

/* 传指针（推荐） */
float distance(const struct Point *a, const struct Point *b) {
    float dx = a->x - b->x;
    float dy = a->y - b->y;
    return dx * dx + dy * dy;  /* 不展开 sqrt, 比较距离用平方够 */
}

int main(void) {
    struct Point p1 = { 0.0f, 0.0f };
    struct Point p2 = { 3.0f, 4.0f };
    printf("dist^2 = %.1f\n", distance(&p1, &p2));
    return 0;
}
```

输出：
```
dist^2 = 25.0
```

关键：用 `const struct Point *` —— 指针（不拷贝），const（承诺不修改）。双赢。

## 原理解析

### 1. 传值传递

```c
struct Big { int32_t data[50]; };  /* 200 字节 */

void by_value(struct Big b) {
    b.data[0] = 999;  /* 修改的是副本 */
}
```

传值时，整个结构体通过栈拷贝到函数内部。对于大型结构体，这是性能杀手。

```
调用栈:
┌──────────────────────┐
│  caller 的 Big       │  ← 原始 200 字节
│                      │
└──────────────────────┘
        ↓ 拷贝
┌──────────────────────┐
│  by_value 参数 b     │  ← 新的 200 字节在对方栈帧
│                      │
└──────────────────────┘
```

### 2. 传指针传递（推荐）

```c
void by_ptr(const struct Big *b) {
    printf("%d\n", b->data[0]);  /* 直接访问原始数据 */
}
```

指针只传 8 字节（64 位），无论结构体多大。`const` 承诺函数不会修改它指向的数据。

| 方式 | 拷贝量 | 能否修改 | 适用 |
|------|--------|---------|------|
| 传值 | 整个结构体 | ✅ 改的是副本 | 小结构体（≤16 字节） |
| 传指针 | 8 字节 | ✅ 改原始数据 | 大结构体、需要修改 |
| `const` 指针 | 8 字节 | ❌ 只读 | 大结构体、只读 |

### 3. `->` 指针访问运算符

```c
struct Point p = { 1.0f, 2.0f };
struct Point *ptr = &p;

/* 以下等价 */
ptr->x     ==  (*ptr).x     ==  p.x
```

`->` 是 `(*ptr).` 的语法糖。每次你写 `ptr->member`，编译器翻译成 `(*ptr).member`。

### 4. 函数返回结构体

```c
struct Point make_point(float x, float y) {
    struct Point p;
    p.x = x;
    p.y = y;
    return p;  /* 返回结构体 — 调用者收到副本 */
}
```

返回结构体时，编译器通常通过**返回值优化**（RVO）避免拷贝——直接在调用者的栈空间构造结果。所以返回结构体并不一定慢。

### 5. 结构体数组 + 函数

```c
float total_area(const struct Rect arr[], int32_t count) {
    float sum = 0.0f;
    for (int32_t i = 0; i < count; i++) {
        sum += arr[i].width * arr[i].height;
    }
    return sum;
}
```

数组传参退化为指针——不拷贝任何元素。

## 常见错误（Error-First）

### ❌ 错误 1: 大结构体传值导致性能问题

```c
struct BigTable { int32_t data[1000]; };

void process(struct BigTable t) {  /* ❌ 每次调用拷贝 4000 字节！ */
    /* ... */
}
```

✅ **修复**: 传指针

```c
void process(const struct BigTable *t) {  /* ✅ 只传 8 字节 */
    /* ... */
}
```

### ❌ 错误 2: 用 `.` 访问指针

```c
struct Point *p = &origin;
printf("%f\n", p.x);  /* ❌ p 是指针，不是结构体 */
```

✅ **修复**: 用 `->`

```c
printf("%f\n", p->x);  /* ✅ */
```

### ❌ 错误 3: 返回局部结构体指针

```c
struct Point *bad_func(void) {
    struct Point p = { 1.0f, 2.0f };
    return &p;  /* ❌ p 在栈上，函数返回后销毁 */
}
```

✅ **修复**: 返回结构体本身（值），不是指针

```c
struct Point good_func(void) {
    struct Point p = { 1.0f, 2.0f };
    return p;  /* ✅ 返回副本，安全 */
}
```

## 动手练习

<details>
<summary>🟢 入门: 写一个移动函数</summary>

定义 `struct Vec2 { float x, y; }`，写 `void move(Vec2 *v, float dx, float dy)` 修改坐标。

</details>

<details>
<summary>🟡 中级: 计算所有矩形总面积</summary>

写 `float total_rect_area(const struct Rect rects[], int count)`。

</details>

## 故障排查（FAQ）

<details>
<summary>Q: 什么时候该传值、什么时候传指针？</summary>

| 规则 | 做法 |
|------|------|
| 结构体 ≤ 指针大小（≤ 8 字节），只读 | 传值或 const 指针都行 |
| 结构体 > 指针大小，只读 | `const struct T *` |
| 需要修改结构体 | `struct T *` |
| 返回新结构体 | 直接 return struct（编译器 RVO 优化） |

</details>

## 知识扩展

### RVO（Return Value Optimization）

现代编译器对返回结构体做了优化：不通过返回值拷贝，而是让调用者分配空间，被调函数直接写入那个空间。用 `-O2` 编译时，返回结构体和传出参数性能接近。

## 小结

- 传值 = 整份拷贝（大结构体性能差）
- 传指针 = 只传 8 字节（`const` 承诺不修改）
- `->` 用于指针访问成员，等价于 `(*ptr).`
- 返回结构体是安全的，编译器 RVO 优化
- 结构体数组传参退化为指针

## 术语表

| 术语 | 英文 | 说明 |
|------|------|------|
| 传值 | Pass by Value | 整个结构体拷贝到函数栈帧 |
| 传指针 | Pass by Pointer | 只传结构体地址 |
| 箭头运算符 | Arrow Operator (`->`) | 指针访问成员 |
| RVO | Return Value Optimization | 返回值优化避免拷贝 |
| const 指针 | Pointer to Const | `const T *` 承诺不修改 |

## 延伸阅读

- [cppreference: Function calls](https://en.cppreference.com/w/c/language/function)
- K&R §6.3: Pointers and Arrays

## 继续学习

| 方向 | 链接 |
|------|------|
| 上一章 → | [嵌套结构体](./nested_structs.md) |
| 下一章 → | [结构体内存布局](./struct_memory_layout.md) — padding、对齐、sizeof |
