# 基础知识回顾与测验 (Basic Review)

## 开篇语

恭喜你走到这里！如果你已经读完了「基础篇」的所有章节，现在是我带你做一次全面复盘的时候了。

我发现大多数人学 C 语言有一个通病：**每个章节单独看都懂了，但把题目混在一起就懵了**。指针和数组到底什么关系？`const` 和 `#define` 到底什么时候用？`malloc` 之后忘记 `free` 到底会怎样？

这个回顾测验就是帮你把零散的知识点编织成一张完整的知识网络。20 道题目，从 🟢 入门到 🔴 挑战，覆盖变量、指针、内存、回调等 27+ 个核心话题。

**我的建议是**：先独立完成每一题，再点开答案对照。做错了不要紧——**错题才是你最有价值的收获**。

---

## US1: 变量、数据类型、函数、控制流、循环、预处理器

### 题目 1 🟢 [变量初始化] 代码预测

以下代码的输出是什么？

```c
#include <stdio.h>

int main(void) {
    int a;
    printf("%d\n", a);
    return 0;
}
```

<details><summary>查看答案</summary>

**答案**：**未定义行为 (Undefined Behavior)** — 输出一个随机垃圾值。

`a` 声明了但未初始化，它的值是栈上随机残留的数据。我的经验是：用 `-Wall -Wextra` 编译，GCC 会警告 `'a' is used uninitialized`。永远在声明时初始化你的变量：`int a = 0;`。

</details>

---

### 题目 2 🟢 [数据类型] 填空

```c
#include <stdint.h>
#include <stdio.h>
#include <____①____>   /* ① 填写头文件名 */

int main(void) {
    int32_t max = INT_MAX;
    int64_t larger = (int64_t)max + 1;  /* 防止溢出 */
    printf("%ld\n", (long)larger);
    return 0;
}
```

① 应该填入什么头文件？

<details><summary>查看答案</summary>

**答案**：`<limits.h>`

`INT_MAX`、`INT_MIN`、`UINT_MAX` 等常量定义在 `<limits.h>` 中。`<stdint.h>` 提供 `int32_t`、`int64_t` 等精确宽度类型，但极限常量在 `<limits.h>`。这是我在 datatype 章节反复强调的——写数值代码前查极限常量。

</details>

---

### 题目 3 🟡 [函数] 找 Bug

以下代码能编译通过吗？如果能，输出是什么？如果不能，为什么？

```c
#include <stdio.h>

int add(int a, int b);  /* 声明 */

int main(void) {
    printf("%d\n", add(3, 5));
    return 0;
}

/* 定义 */
int add(int a, int b) {
    a + b;  /* ← 注意这一行 */
}
```

<details><summary>查看答案</summary>

**答案**：编译器会**警告**（`-Wall` 下），但仍然编译通过。运行时输出**垃圾值**。

`a + b;` 这一行计算了结果但没有 `return`！控制到达非 `void` 函数末尾时，返回值由寄存器中的随机值决定。修复很简单：

```c
int add(int a, int b) {
    return a + b;  /* ✅ 加上 return */
}
```

我发现很多初学者会犯这个错误——写了表达式但忘了返回。编译时开 `-Wreturn-type` 可以捕获这类问题。

</details>

---

### 题目 4 🟡 [控制流] 代码预测

```c
#include <stdio.h>

int main(void) {
    int score = 85;

    if (score >= 90)
        printf("A\n");
        printf("优秀\n");
    else if (score >= 60)
        printf("C\n");

    return 0;
}
```

上面的代码**能否编译通过**？输出是什么？

<details><summary>查看答案</summary>

**答案**：**编译错误** — `else if` 没有匹配的 `if`。

问题在于 `if (score >= 90)` 后面没有花括号，所以 `printf("优秀\n");` 不属于 `if`，而 `else if` 就近匹配到了谁？实际上，C 的规则是 `else` 匹配最近的 `if`——但 `printf("优秀\n");` 是一个独立语句，隔开了 `if` 和 `else if`，导致编译器报错：`'else' without a previous 'if'`。

修复——永远加上花括号：

```c
if (score >= 90) {
    printf("A\n");
    printf("优秀\n");
} else if (score >= 60) {
    printf("C\n");
}
```

这是我第一篇「控制流」章节的「开篇故事」——我当初因为这个 bug 考了 55 分却打印了"恭喜！"。

</details>

---

### 题目 5 🟡 [循环] 找 Bug

```c
#include <stdio.h>

int main(void) {
    for (int i = 0; i < 5; ) {
        printf("%d ", i);
        if (i == 3) continue;
        i++;
    }
    printf("\n");
    return 0;
}
```

以上代码的运行结果是什么？有什么风险？

<details><summary>查看答案</summary>

**答案**：**死循环**。输出 `0 1 2 3 3 3 3 ...` 永远不停止。

当 `i == 3` 时，`continue` 跳过了 `i++`，所以 `i` 永远停留在 3，条件 `i < 5` 永远为真。这正是我在「循环」章节「常见错误 3」中警告的：**`continue` 在 `while` 循环中会跳过递增部分**。

修复——确保 `continue` 之前递增：

```c
if (i == 3) {
    i++;       /* ✅ continue 前先递增 */
    continue;
}
```

</details>

---

### 题目 6 🔴 [预处理器] 代码预测

```c
#include <stdio.h>

#define SQUARE(x) x * x

int main(void) {
    int result = SQUARE(3 + 2);
    printf("%d\n", result);
    return 0;
}
```

输出是 `25` 吗？为什么？

<details><summary>查看答案</summary>

**答案**：输出 **11**，不是 25！

宏展开是纯文本替换。`SQUARE(3 + 2)` 展开为：

```c
3 + 2 * 3 + 2
```

按运算符优先级：`3 + 6 + 2 = 11`。

这是我在「预处理器」章节被坑过无数次的经典陷阱。修复——给参数和整体都加上括号：

```c
#define SQUARE(x) ((x) * (x))  /* ✅ 现在 SQUARE(3+2) = ((3+2)*(3+2)) = 25 */
```

</details>

---

## US2: 指针、指针运算、字符串、结构体、枚举、作用域

### 题目 7 🟢 [指针] 代码预测

```c
#include <stdio.h>

int main(void) {
    int x = 10;
    int *p = &x;
    *p = 20;
    printf("x = %d\n", x);
    return 0;
}
```

`x` 的值会变成 20 吗？为什么？

<details><summary>查看答案</summary>

**答案**：**是的**，`x = 20`。

`p` 存储了 `x` 的地址，`*p = 20` 等价于 `x = 20`。这是指针最基本的用法——通过地址间接修改变量。记住：`*p` 就是 `x` 的别名。

</details>

---

### 题目 8 🟢 [指针运算] 填空

```c
#include <stdio.h>

int main(void) {
    int arr[] = {10, 20, 30, 40, 50};
    int *p = arr;
    printf("%d\n", *(p + 2));  /* 输出: __?__ */
    return 0;
}
```

`*(p + 2)` 的值是多少？为什么指针 +2 会前进 8 个字节？

<details><summary>查看答案</summary>

**答案**：输出 **30**。

`p` 指向 `arr[0]`（值 10）。`p + 2` 前进 2 个 `int` 元素（每个 4 字节，共 8 字节），指向 `arr[2]`（值 30）。

这就是我在「指针运算」章节讲的核心规则：**指针 +N 前进 N 个「元素」，不是 N 个字节**。编译器自动根据指针类型（`int*`）计算偏移量：`2 × sizeof(int) = 8 字节`。

同时 `*(p + 2)` 完全等价于 `p[2]` 等价于 `arr[2]`——它们在编译器层面是同一件事。

</details>

---

### 题目 9 🟡 [sizeof 陷阱] 找 Bug

```c
#include <stdio.h>

void print_len(int *arr) {
    size_t len = sizeof(arr) / sizeof(arr[0]);
    printf("len = %zu\n", len);
}

int main(void) {
    int data[] = {1, 2, 3, 4, 5};
    print_len(data);  /* 期望输出 5 */
    return 0;
}
```

实际输出是多少？为什么不是 5？

<details><summary>查看答案</summary>

**答案**：实际输出 **1**（或 **2** 取决于平台），不是 5。

这是 C 语言**最经典的 sizeof 陷阱**。当数组作为函数参数传递时，它**退化为指针**。所以在 `print_len` 中：
- `sizeof(arr)` = `sizeof(int*)` = 8 字节（64 位平台）
- `sizeof(arr[0])` = `sizeof(int)` = 4 字节
- `8 / 4 = 2`（不是 5！）

修复——在调用处计算好长度再传入：

```c
void print_len(int *arr, size_t len) {
    printf("len = %zu\n", len);
}

int main(void) {
    int data[] = {1, 2, 3, 4, 5};
    size_t len = sizeof(data) / sizeof(data[0]);  /* ✅ 在数组定义处计算 */
    print_len(data, len);
}
```

我发现 90% 的初学者在这道题上栽过跟头。核心教训：**一旦数组变成指针，sizeof 就再也无法知道原始长度**。

</details>

---

### 题目 10 🟡 [字符串] 找 Bug

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    char name[5];
    strcpy(name, "Hello");
    printf("%s\n", name);
    return 0;
}
```

这段代码有什么问题？如何修复？

<details><summary>查看答案</summary>

**答案**：**缓冲区溢出**！`"Hello"` 需要 6 字节（5 个字符 + 1 个 `\0`），但 `name` 只有 5 字节。

`\0` 被写入了 `name` 数组之外的内存——这是未定义行为，可能导致崩溃或安全漏洞。

修复——使用 `strncpy` 并确保 null 终止：

```c
char name[6];  /* ✅ 至少 6 字节 */
strncpy(name, "Hello", sizeof(name) - 1);
name[sizeof(name) - 1] = '\0';  /* ✅ 手动确保 null termination */
```

我常说：`strcpy` 应该在所有严肃项目中被禁用。永远使用 `strncpy` + 手动 `\0`，或者更好的 `snprintf`。

</details>

---

### 题目 11 🟡 [字符串] 代码预测

```c
#include <stdio.h>

int main(void) {
    char a[] = "hello";
    char b[] = "hello";
    if (a == b) {
        printf("相等\n");
    } else {
        printf("不相等\n");
    }
    return 0;
}
```

输出"相等"还是"不相等"？为什么？

<details><summary>查看答案</summary>

**答案**：输出 **"不相等"**。

`a` 和 `b` 是两个独立的数组，它们在内存中有不同的地址。`a == b` 比较的是**指针地址**（数组名退化为指向首元素的指针），不是字符串内容！两个不同的数组地址当然不相等。

正确比较字符串内容的方式：

```c
if (strcmp(a, b) == 0) {  /* ✅ 比较内容 */
    printf("内容相等\n");
}
```

这是 C 字符串的"第 1 号禁忌"——永远不要用 `==` 比较字符串。我在「字符串深度」章节反复强调过这一点。

</details>

---

### 题目 12 🟡 [结构体] 代码预测

```c
#include <stdio.h>
#include <stdint.h>

struct Point {
    int32_t x;
    int32_t y;
};

int main(void) {
    struct Point p1 = {3, 4};
    struct Point p2 = {3, 4};

    if (p1 == p2) {
        printf("相等\n");
    }
    return 0;
}
```

能编译通过吗？为什么？

<details><summary>查看答案</summary>

**答案**：**编译错误**。C 语言不支持用 `==` 比较结构体。

虽然 `p1` 和 `p2` 的成员值完全相同，但 C 标准没有定义结构体的 `==` 运算符。修复——逐成员比较：

```c
if (p1.x == p2.x && p1.y == p2.y) {  /* ✅ 逐成员比较 */
    printf("相等\n");
}
```

注意：不推荐使用 `memcmp(&p1, &p2, sizeof(struct Point))`——结构体中可能存在 padding 字节，它们的值是不确定的，可能导致 `memcmp` 误报不相等。

</details>

---

### 题目 13 🟡 [枚举] 找 Bug

```c
#include <stdio.h>

typedef enum { RED, GREEN, BLUE } Color;

void print_color(Color c) {
    switch (c) {
        case RED:   printf("红色\n"); break;
        case GREEN: printf("绿色\n"); break;
        /* 缺少 BLUE */
    }
}

int main(void) {
    print_color(BLUE);
    return 0;
}
```

调用 `print_color(BLUE)` 会怎样？如何防止这种 bug？

<details><summary>查看答案</summary>

**答案**：**什么都不会输出**——`BLUE` 没有被任何 `case` 匹配，且没有 `default` 分支，所以函数静默地什么都不做。

这是最危险的 bug 类型——静默失败。防止方法：永远在枚举的 `switch` 中加 `default`：

```c
void print_color(Color c) {
    switch (c) {
        case RED:   printf("红色\n"); break;
        case GREEN: printf("绿色\n"); break;
        case BLUE:  printf("蓝色\n"); break;
        default:    printf("未知颜色(%d)\n", c); break;  /* ✅ 安全兜底 */
    }
}
```

我在「枚举」章节强调过：C 的枚举底层是 `int`，可以赋任何整数值。没有 `default` 就无法捕获非法值。

</details>

---

### 题目 14 🔴 [作用域/生命周期] 找 Bug

```c
#include <stdio.h>

int *get_number(void) {
    int x = 42;
    return &x;  /* ← 注意这行 */
}

int main(void) {
    int *p = get_number();
    printf("%d\n", *p);
    return 0;
}
```

这段代码的行为是什么？为什么？

<details><summary>查看答案</summary>

**答案**：**未定义行为 (Undefined Behavior)** — 可能输出 42，也可能输出随机垃圾值，也可能崩溃。

`x` 是 `get_number` 的**局部变量**，存储在栈上。当 `get_number` 返回时，`x` 的栈帧被销毁，但 `p` 仍然指向那片已经释放的内存——这就是**悬垂指针 (Dangling Pointer)**。

修复方案有三种：

```c
/* 方案 1: static 变量 — 存储在 .data 段 */
int *get_number(void) {
    static int x = 42;
    return &x;
}

/* 方案 2: 堆分配 — 调用者负责 free */
int *get_number(void) {
    int *x = malloc(sizeof(int));
    *x = 42;
    return x;
}

/* 方案 3: 调用者分配 */
void get_number(int *result) {
    *result = 42;
}
```

这是我在「作用域」章节开篇的故事——我调了一整个下午才找到这个 bug。记住：**绝不要返回局部变量的地址**。

</details>

---

## US3: 内存管理、函数指针、回调、文件 I/O、void* 泛型、位运算

### 题目 15 🟢 [内存管理] 代码预测

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *p = malloc(sizeof(int) * 3);
    p[0] = 10; p[1] = 20; p[2] = 30;
    free(p);
    printf("%d\n", p[0]);  /* ← 注意这行 */
    return 0;
}
```

`printf` 会输出 10 吗？为什么？

<details><summary>查看答案</summary>

**答案**：**未定义行为** — 可能输出 10（运气好），可能输出垃圾值，可能崩溃。

`free(p)` 后，`p` 成为**悬垂指针**。那块内存已经被归还给操作系统（或堆管理器），你不再拥有它。继续使用它就是 **Use-After-Free**——这是最危险的内存错误之一。

修复——释放后立即置 `NULL`：

```c
free(p);
p = NULL;
/* printf("%d\n", p[0]); → 现在会立刻崩溃（段错误），这比静默损坏好调试得多 */
```

我的安全规则：**free + 置 NULL** 永远一起做，就像系安全带一样不能忘。

</details>

---

### 题目 16 🟡 [内存管理] 找 Bug

```c
#include <stdlib.h>

void *expand(void *buf) {
    buf = realloc(buf, 200);  /* ← 潜在问题 */
    return buf;
}
```

如果 `realloc` 分配失败，上述代码有什么问题？

<details><summary>查看答案</summary>

**答案**：**内存泄漏**。

当 `realloc` 失败时返回 `NULL`，但**原内存不会被释放**。如果直接 `buf = realloc(buf, 200)`，失败后 `buf` 变成 `NULL`，原内存块的地址丢失——无法再 `free(buf)`，造成泄漏。

修复——用临时变量保存返回值：

```c
void *expand(void *buf) {
    void *tmp = realloc(buf, 200);
    if (tmp == NULL) {
        free(buf);   /* ✅ 失败时释放原内存 */
        return NULL;
    }
    return tmp;  /* ✅ 成功时返回新地址 */
}
```

这是我在「内存管理」章节「常见错误 5」中的核心教训。我发现很多人不知道 `realloc` 的这个行为。

</details>

---

### 题目 17 🟡 [函数指针] 填空

```c
#include <stdio.h>

/* 声明一个函数指针 fptr，指向「接受 int 参数、返回 int」的函数 */
int (*fptr)(int) = NULL;

int triple(int x) { return x * 3; }

int main(void) {
    fptr = &triple;  /* 赋值 */
    printf("%d\n", fptr(7));  /* 调用 */
    return 0;
}
```

`fptr(7)` 的值是多少？`&triple` 可以简写为 `triple` 吗？

<details><summary>查看答案</summary>

**答案**：输出 **21**。`&triple` 可以简写为 `triple`——函数名会自动 decay 为函数指针，两者完全等价。

函数指针的声明语法很容易让人困惑。记住顺时针螺旋规则：从 `fptr` 开始，`*fptr` 是指针，`(*fptr)(int)` 是接受 `int` 的函数，返回 `int`。用 `typedef` 更清晰：

```c
typedef int (*int_transform_t)(int);
int_transform_t fptr = triple;  /* 一目了然 */
```

</details>

---

### 题目 18 🟡 [回调函数] 找 Bug

```c
#include <stdio.h>
#include <stdlib.h>

int cmp(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);  /* ← 问题在这行 */
}

int main(void) {
    int arr[] = {5, -2147483647, 3};
    qsort(arr, 3, sizeof(int), cmp);
    printf("%d\n", arr[0]);
    return 0;
}
```

`cmp` 函数有什么问题？对于包含 `INT_MIN` 的数组会发生什么？

<details><summary>查看答案</summary>

**答案**：**整数溢出导致错误的比较结果**。

当 `a = 5` 且 `b = -2147483647`（约等于 `INT_MIN`）时，`5 - (-2147483647)` 会溢出 `int` 的范围，导致符号翻转——原本 `5 > -2147483647`，但溢出后的结果可能是负数，`qsort` 会做出错误的排序决策。

修复——用安全的三态比较：

```c
int cmp(const void *a, const void *b) {
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);  /* ✅ 不会溢出 */
}
```

这是我在「回调函数」章节「常见错误 2」中强调的——永远不要假设 `a - b` 是安全的比较方式。`INT_MIN` 的存在会让减法溢出。

</details>

---

### 题目 19 🟡 [文件 I/O] 找 Bug

```c
#include <stdio.h>

int main(void) {
    FILE *fp = fopen("test.txt", "w");
    fprintf(fp, "Hello, world!\n");
    /* 忘记关闭文件 */
    return 0;
}
```

这段代码运行后，`test.txt` 的内容是什么？为什么？

<details><summary>查看答案</summary>

**答案**：文件**可能为空**或内容不完整。

`fprintf` 不会立即写入磁盘——数据先写到 `FILE*` 的内部缓冲区。如果**忘记 `fclose(fp)`**，缓冲区中的数据**可能不会被刷新**到磁盘，导致文件是空的或不完整的。

修复：

```c
FILE *fp = fopen("test.txt", "w");
if (fp == NULL) { /* 错误处理 */ }
fprintf(fp, "Hello, world!\n");
fclose(fp);  /* ✅ 必须调用！刷新缓冲区 + 释放资源 */
```

我在「文件 I/O」章节的第一篇故事就是这个 bug——第一次写 C 文件时忘记 `fclose`，盯着空文件困惑了很久。

</details>

---

### 题目 20 🔴 [void* + 位运算] 综合题

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    uint32_t val = 0x12345678;
    void *vp = &val;
    uint8_t *bytes = (uint8_t *)vp;

    printf("0x%02x\n", bytes[0]);

    /* 用位运算取出最高字节 */
    uint8_t high_byte = (val >> 24) & 0xFF;
    printf("0x%02x\n", high_byte);

    return 0;
}
```

假设是小端序 (Little Endian) 平台，两处 `printf` 分别输出什么？

<details><summary>查看答案</summary>

**答案**：
- 第一处：`0x78`（小端序中最低字节在地址 0）
- 第二处：`0x12`（`0x12345678 >> 24` = `0x00000012`，`& 0xFF` = `0x12`）

解释：
- `val = 0x12345678` 在小端序内存中存储为：`[78] [56] [34] [12]`
- `bytes[0]` 就是最低字节 `0x78`
- 位运算取最高字节：右移 24 位后，`0x12` 移到了最低位

这道题结合了我在「void* 泛型」和「位运算」两章的核心知识。`void*` 让你能指向任何类型，而位运算让你能精确操控每一个 bit。这两个工具合在一起，就是 C 语言"直接操作硬件"的能力。

</details>

---

## 总结回顾

恭喜完成全部 20 题！下面是各阶段的核心知识清单——对照检查你掌握得如何：

### US1 基础层 ✅

| 话题 | 核心要点 | 对应题目 |
|------|---------|---------|
| 变量初始化 | 始终初始化，未初始化 = 随机值 | Q1 |
| 数据类型极限 | `<limits.h>` 提供 `INT_MAX` 等 | Q2 |
| 函数返回 | 有返回值必须 `return` | Q3 |
| 控制流花括号 | 永远加 `{}` 避免悬挂 else | Q4 |
| 循环 continue | `continue` 会跳过 `while` 中的递增 | Q5 |
| 宏优先级 | 给参数和整体加双括号 | Q6 |

### US2 指针层 ✅

| 话题 | 核心要点 | 对应题目 |
|------|---------|---------|
| 指针解引用 | `*p` 等价于原变量 | Q7 |
| 指针算术步长 | `p+N` 前进 `N × sizeof(类型)` 字节 | Q8 |
| sizeof 陷阱 | 数组退化为指针后 sizeof 失效 | Q9 |
| 字符串安全 | `strcpy` → `strncpy` + 手动 `\0` | Q10 |
| 字符串比较 | 用 `strcmp`，不用 `==` | Q11 |
| 结构体比较 | C 不支持 `==` 比较 struct | Q12 |
| 枚举 default | switch 枚举永远加 default | Q13 |
| 悬垂指针 | 不返回局部变量地址 | Q14 |

### US3 进阶层 ✅

| 话题 | 核心要点 | 对应题目 |
|------|---------|---------|
| Use-After-Free | `free` 后立即 `p = NULL` | Q15 |
| realloc 安全 | 用临时变量保存返回值 | Q16 |
| 函数指针语法 | 顺时针螺旋规则阅读 | Q17 |
| 三态比较 | `(a > b) - (a < b)` 防溢出 | Q18 |
| 文件刷新 | 必须 `fclose` 才会刷缓冲区 | Q19 |
| 小端序 + 位运算 | `bytes[0]` = 最低字节，`>>` 取高位 | Q20 |

---

## 下一步

如果你全部答对了——恭喜，你的 C 语言基础非常扎实。建议进入「高级篇」继续探索更复杂的设计模式。

如果有错题——**不要跳过**。回到对应章节重新阅读，把代码改一改、编译一下、看看不同输入的输出。C 语言是一门需要动手的语言——光看不练是学不会的。

> ——我发现，每次回头复习这些基础，都会有新的理解。C 的核心概念并不多，但它们的组合能构建出极其强大的程序。掌握这些基础，你在任何编程语言中都会比别人理解得更深。
