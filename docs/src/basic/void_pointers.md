# void* 指针 (Void Pointers)

## 开篇故事

想象一个万能充电适配器——能插美标、欧标、英标的插座，因为插头是通用的。但连上设备后，你必须知道那台设备需要多少伏特。适配器接错了电压会烧设备——`void*` 转错了类型，数据就乱套。

`void*` 就是 C 的万能指针，可以指向任何类型。但读之前，必须**转回正确类型**。

## 本章适合谁

- 已掌握具体类型指针（`int*`、`char*`）
- 正在读标准库函数（`qsort`、`bsearch`、`memset`）源码
- 想了解 C 语言如何实现"通用指针"

## 你会学到什么

- `void*` 的本质——无类型指针
- 安全转换回具体类型（`*(int32_t *)vp`）
- `const void*` 的用法
- 为什么 `void*` 不能做指针算术
- 与 Python/C++ 泛型的认知对照

> 🔥 **下一步**: [void* 泛型编程](./void_generic.md) — 学习如何用 `void*` 设计泛型函数、Type Tag 模式、qsort 回调机制、C11 `_Generic` 等进阶内容。

## 前置要求

- 已完成 [指针运算](./pointer_arith.md)

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int32_t i = 42;
    double  d = 3.14;
    char    c = 'Z';

    void *vp1 = &i;  /* void* 接收 int32_t*，无需强转 */
    void *vp2 = &d;
    void *vp3 = &c;

    /* ⚠️ 读取前必须先转回正确类型 */
    printf("i = %" PRId32 "\n", *(int32_t *)vp1);
    printf("d = %.2f\n",   *(double *)vp2);
    printf("c = '%c'\n",   *(char *)vp3);

    return 0;
}
```

两条核心规则：
- `void*` **可以接收**任何类型地址（编译器内置隐式转换）
- `void*` **不能直接解引用**——必须先 `(type *)` 转回具体类型

## 原理解析

### 1. `void*` 的本质

`void` 表示「无类型」，`void*` 因此被称为**通用指针** (universal pointer)。

内存中 `void*` 和 `int32_t*`、`char*` **完全一样**——都是 8 字节地址。区别在于**编译器如何看待它**：

| 类型        | 解引用大小 | `p+1` 步长  | 能直接 `*p`？ |
| ----------- | ---------- | ----------- | ----------- |
| `int32_t *` | 4 字节     | +4 字节     | ✅          |
| `double *`  | 8 字节     | +8 字节     | ✅          |
| `char *`    | 1 字节     | +1 字节     | ✅          |
| `void *`    | **不确定** | **无法确定** | ❌ 编译错误  |

### 2. 转回具体类型

```c
int32_t value = 42;
void *vp = &value;

/* ✅ 正确: 先转为 int32_t*，再解引用 */
int32_t recovered = *(int32_t *)vp;

/* ❌ 错误: 直接解引用 void* */
/* int32_t bad = *vp; */
/* error: invalid use of undefined type 'void' */
```

**类比**：`void*` 像拆了标签的快递盒。必须先贴标签（类型转换），才能打开。

### 3. `const void*`

`const void*` 指向只读数据——保证不会被修改：

```c
void print_data(const void *data, size_t size) {
    /* 只能读，不能修改 data 指向的内容 */
}
```

`qsort` 的比较函数用 `const void*`——保证排序不会修改元素：

```c
int cmp_int(const void *a, const void *b);
```

## 常见错误

### ❌ 错误 1：直接解引用 `void*`

```c
void *vp = &x;
int n = *vp;   /* ❌ 编译错误 */

/* ✅ 修正 */
int n = *(int32_t *)vp;
```

### ❌ 错误 2：转回错误类型（运行时 UB）

```c
double d = 3.14;
void *vp = &d;
int n = *(int32_t *)vp;  /* ❌ 编译通过，数据全错！*/
```

**最危险的 void* 错误**——编译器不警告，但读出的数据完全错误。

### ❌ 错误 3：`void*` 指针算术

```c
void *vp = buf;
vp++;  /* ❌ 标准 C 无定义！编译器不知道 void 的大小 */

/* ✅ 修正：转成 unsigned char* 再运算 */
unsigned char *cp = (unsigned char *)vp;
cp++;   /* 前进 1 字节 */
```

## 动手练习

### 🟢 入门：void* 赋值与转换

声明 `int32_t`、`double` 和 `char` 变量，用 `void*` 分别指向它们，正确转换回原类型并打印。

<details><summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

int main(void) {
    int32_t i = 100;
    double  d = 2.718;
    char    c = 'X';
    void *vp;

    vp = &i;
    printf("int:    %" PRId32 "\n", *(int32_t *)vp);

    vp = &d;
    printf("double: %.3f\n", *(double *)vp);

    vp = &c;
    printf("char:   '%c'\n", *(char *)vp);

    return 0;
}
```

</details>

## 故障排查 (FAQ)

**Q：为什么不用 `uintptr_t` 代替 `void*`？**

A：`uintptr_t` 是整数类型，不能直接解引用。`void*` 的价值是「可间接寻址 + 类型擦除」——你总需要在某个时刻把它转回具体类型来读写数据。

**Q：`void*` 能和 `NULL` 比较吗？**

A：可以。`void*` 支持所有指针比较运算：

```c
void *vp = NULL;
if (vp == NULL) { /* ... */ }  /* ✅ */
```

## 小结

- **`void*` 是无类型指针**, 可接收任何对象地址
- **不能直接解引用**——必须先转回具体类型 `(type *)`
- **不能做指针算术**——先转成 `unsigned char*`
- **隐式转换**——任何指针赋给 `void*` 不需要强转

## 术语表

| 英文 | 中文 | 解释 |
|------|------|------|
| void pointer | void 指针 / 无类型指针 | 可指向任何类型的通用指针 |
| Type erasure | 类型擦除 | 抹掉类型信息换取灵活性 |
| Type casting | 类型转换 | 将 `void*` 转回具体类型 |

## 延伸阅读

- [cppreference - Void pointer](https://en.cppreference.com/w/c/language/pointer) — void* 操作参考

## 继续学习

你已经掌握了 `void*` 指针的核心概念。下一步，我们将学习如何用 `void*` 设计泛型函数——这是 `qsort`、`bsearch` 等标准库 API 的设计原理。

- [上一章](./pointer_arith.md)：指针运算——数组等价性、指针算术
- [下一章](./void_generic.md)：void* 泛型编程——Type Tag、qsort 模式、C11 `_Generic`

---

> 本章代码位于 [`src/basic/void_pointers_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/void_pointers_sample.c)。
