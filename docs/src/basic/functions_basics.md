# 函数基础 (Function Basics)

> "我发现，函数就像厨房里的专用工具——每个工具只做一件事，但你把它们组合起来就能做出一顿大餐。" —— 我的理解

## 开篇故事

走进一个专业的厨房。你看到切菜板、打蛋器、榨汁机、烤箱——每种工具都有一个明确的职责。切菜板不负责加热，烤箱不负责搅拌。但当你组合这些工具时，就能做出一道完整的菜。

函数（Function）就是编程世界里的「专用工具」。一个函数只做一件事：计算面积、打印日志、验证输入。单独看，每个函数都很简单；但组合起来，就能构建复杂的程序。

C 语言没有「类」或「对象」的概念，函数就是它最重要的代码组织单元。整个 C 标准库——`printf`、`malloc`、`strlen`——全是函数。

> "把大任务拆成小函数，就像把大工程拆成工序——每一步都可控，每一步都可复用。"

## 本章适合谁

- 已经理解变量、数据类型、运算符的 C 初学者
- 第一次接触函数概念，想搞清楚「声明」和「定义」区别的人
- 被编译器「implicit declaration」错误搞晕过的人
- 想理解参数传递（值传递）和返回机制的人

## 你会学到什么

- 函数的声明（Declaration）与定义（Definition）——为什么必须先声明后使用
- 参数（Parameters）——单参数、多参数、值传递（Pass by Value）
- 返回类型（Return Type）——`int`、`float`、`char`、`void`
- `return` 关键字的作用——返回值并退出函数
- 调用约定——如何正确地调用一个函数
- 常见错误与修复方法

## 前置要求

- 了解 C 基本数据类型（`int`、`float`、`char`）
- 会使用 `printf` 输出
- 掌握变量声明和赋值
- 了解运算符（`+`、`-`、`*`、`/`、`%`）

> 如果还没学过「循环」，建议先看「循环」章节——函数和循环常常一起使用。

## 第一个例子

```c
#include <stdio.h>

/* 函数声明：告诉编译器 add 函数存在 */
int add(int a, int b);

int main(void) {
    int x = 3, y = 5;
    int result = add(x, y);  /* 调用函数 */
    printf("3 + 5 = %d\n", result);
    return 0;
}

/* 函数定义：实现 add 的具体逻辑 */
int add(int a, int b) {
    return a + b;
}
```

运行结果：

```
3 + 5 = 8
```

拆解一下这个例子中发生了什么：

1. **声明** `int add(int a, int b);` — 告诉编译器「有一个叫 `add` 的函数，它接收两个 `int`，返回一个 `int`」
2. **定义** `int add(int a, int b) { return a + b; }` — 告诉编译器「`add` 函数实际上做了什么」
3. **调用** `add(x, y)` — 在 `main` 函数中调用 `add`，传入 `x` 和 `y` 的值
4. **返回** `return a + b;` — 计算结果 `8` 返回给调用者

## 原理解析

### 1. 声明 vs 定义

**声明（Declaration）** 是「预告」——它告诉编译器函数的签名（返回值类型 + 函数名 + 参数列表），但不包含具体的实现。

**定义（Definition）** 是「正片」——它包含函数实际执行的代码。

```c
/* 声明：只有签名，没有实现 */
int multiply(int x, int y);

/* 定义：签名 + 实现体 */
int multiply(int x, int y) {
    return x * y;
}
```

**我的理解**：声明像菜单上的菜名（告诉你有什么），定义像厨师的配方（告诉你怎么做）。

在 C 语言中，**函数必须先声明后使用**。如果定义出现在调用之前，可以省略单独的声明（定义本身就充当了声明）。但如果定义在调用之后，就一定要先声明：

```c
/* ❌ 错误：在定义之前调用，编译器不知道 greeting 是什么 */
int main(void) {
    greeting("World");  /* 编译错误 */
    return 0;
}

void greeting(const char *name) {
    printf("Hello, %s!\n", name);
}

/* ✅ 正确：先声明，再调用，最后定义 */
void greeting(const char *name);  /* 声明在前 */

int main(void) {
    greeting("World");  /* ✅ 编译器知道了签名 */
    return 0;
}

void greeting(const char *name) {
    printf("Hello, %s!\n", name);  /* 定义在后 */
}
```

### 2. 参数（Parameters）：值传递（Pass by Value）

C 语言中，**函数参数是值传递的**。调用函数时，实参的值会被**复制**一份传给形参，原变量不会改变。

```c
void double_value(int x) {
    x = x * 2;
    printf("  函数内部: x = %d\n", x);  /* 修改的是副本 */
}

int main(void) {
    int num = 21;
    printf("  调用前: num = %d\n", num);   /* 21 */
    double_value(num);
    printf("  调用后: num = %d\n", num);   /* 21 — 没变！ */
    return 0;
}
```

```
  调用前: num = 21
  函数内部: x = 42      ← 函数内改了副本
  调用后: num = 21      ← 原变量不受影响
```

**ASCII 内存示意图——值传递时发生了什么**：

```
调用 double_value(num) 时：

栈空间:
┌─────────────────────┐
│ main() 的栈帧       │
│   num = 21 ← 原始值 │
│                     │
├─────────────────────┤
│ double_value() 栈帧 │
│   x = 21 ← 复制品！  │  ← 修改 x 不影响 num
│                     │
└─────────────────────┘
```

**我的理解**：想象你给朋友复印了一份文件。朋友在复印件上写字，原件上不会有任何变化。值传递就是这样——函数拿到的是「复印件」。

### 3. 返回类型（Return Type）

函数的返回类型决定了它「吐出」什么类型的数据：

```c
/* 返回 int */
int square(int n) {
    return n * n;
}

/* 返回 float */
float half(float n) {
    return n / 2.0f;
}

/* 返回 char */
char grade(int score) {
    if (score >= 90) return 'A';
    if (score >= 80) return 'B';
    if (score >= 60) return 'C';
    return 'F';
}

/* 返回 void（无返回值）——只做一件事，不返回数据 */
void print_separator(void) {
    printf("-------\n");
}
```

`void` 表示「这个函数什么都不返回」。它只执行操作（如打印、修改全局变量等）。

**我的理解**：`void` 函数就像工厂里的机器——它干活（打孔、喷漆），但不产出可以带走的东西。

### 4. return 关键字

`return` 做两件事：
1. **返回一个值**（如果是 `void` 函数则没有值）
2. **立即退出当前函数**，回到调用者

```c
int absolute(int n) {
    if (n >= 0) {
        return n;    /* 退出函数，返回 n */
    }
    return -n;       /* 退出函数，返回 -n */
}
```

`void` 函数也可以用 `return;` 提前退出：

```c
void print_if_positive(int n) {
    if (n <= 0) {
        return;  /* 提前退出，什么都不打印 */
    }
    printf("%d\n", n);
}
```

**我常犯的错**：忘记非 `void` 函数的所有路径都有 `return`。如果控制流走到了函数末尾却没有 `return`，编译器会报错（尤其是用了 `-Wall -Werror` 时）。

### 5. 单参数 vs 多参数

```c
/* 单参数 */
double square_root(double x) {
    /* ... */
}

/* 多参数（用逗号分隔） */
int max_of_three(int a, int b, int c) {
    int max = a;
    if (b > max) max = b;
    if (c > max) max = c;
    return max;
}

/* 无参数 */
int get_constant(void) {
    return 42;
}
```

**注意**：C 语言中 `void` 在参数列表中明确表示「没有参数」。写成 `int func()` （空括号）在 C 中表示「参数未知」（C 风格），而 `int func(void)` 才是「无参数」（现代推荐写法）。

### Python / JavaScript 对比

| 特性 | Python | JavaScript | C |
|------|--------|------------|---|
| 函数定义 | `def f(x):` | `function f(x) { }` | `int f(int x) { }` |
| 返回 | `return x` | `return x;` | `return x;` |
| 类型声明 | 可选（注解） | 无 | **必须声明** |
| 参数传递 | 引用传递 | 引用传递 | **值传递** |
| 无返回值 | 隐式返回 `None` | 隐式返回 `undefined` | 使用 `void` |

## 常见错误

### ❌ 错误 1：调用未声明的函数

```c
int main(void) {
    int result = add(3, 5);  /* ❌ 编译器不知道 add 是什么 */
    return 0;
}

int add(int a, int b) {
    return a + b;
}
```

编译器报错（C99 之后）：

```
error: implicit declaration of function 'add' [-Werror=implicit-function-declaration]
```

**我最初的困惑**：明明 `add` 函数在后面定义了，为什么不能用？

✅ **修正**：先声明后使用。

```c
int add(int a, int b);  /* ✅ 声明在前 */

int main(void) {
    int result = add(3, 5);  /* ✅ 编译器知道了签名 */
    return 0;
}

int add(int a, int b) {     /* 定义在后 */
    return a + b;
}
```

**原理**：C 编译器是「从上到下」编译的。读到 `add(3, 5)` 时，如果还没见过声明，就不知道 `add` 接受什么参数、返回什么类型。

### ❌ 错误 2：返回类型不匹配

```c
int get_name(void) {
    return "hello";  /* ❌ "hello" 是字符串（char*），不是 int */
}
```

✅ **修正**：让返回类型与实际返回值匹配。

```c
const char *get_name(void) {  /* ✅ 返回字符串指针 */
    return "hello";
}
```

### ❌ 错误 3：忘记使用返回值（编译器警告）

```c
int add(int a, int b) {
    return a + b;
}

int main(void) {
    add(3, 5);  /* ⚠️ 调用了但没有使用返回值 — 相当于白算 */
    return 0;
}
```

有些编译器会给出 `unused value` 警告。

✅ **修正**：接收返回值或明确丢弃它。

```c
int main(void) {
    int result = add(3, 5);  /* ✅ 使用返回值 */
    printf("%d\n", result);
    return 0;
}
```

### ❌ 错误 4：参数类型不匹配

```c
int add(int a, int b) {
    return a + b;
}

int main(void) {
    double x = 3.5, y = 5.7;
    int result = add(x, y);  /* ⚠️ double 隐式转 int，丢失小数部分 */
    return 0;
}
```

结果：`add(3.5, 5.7)` 实际计算的是 `3 + 5 = 8`（小数被截断）。

✅ **修正**：参数类型匹配。

```c
double add_double(double a, double b) {
    return a + b;
}

int main(void) {
    double x = 3.5, y = 5.7;
    double result = add_double(x, y);  /* ✅ 3.5 + 5.7 = 9.2 */
    return 0;
}
```

## 动手练习

### 🟢 练习 1：编写 multiply 函数

写一个函数 `int multiply(int a, int b)`，返回两个整数的乘积。在 `main` 中调用它。

```c
/* 提示：把 add 函数中的 return a + b 改成 return a * b */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

int multiply(int a, int b) {
    return a * b;
}

int main(void) {
    printf("6 × 7 = %d\n", multiply(6, 7));  /* 42 */
    return 0;
}
```
</details>

### 🟡 练习 2：编写 is_even 函数（返回布尔值）

写一个函数 `int is_even(int n)`，如果 `n` 是偶数返回 `1`（真），否则返回 `0`（假）。在 `main` 中用 `printf` 判断数字。

```c
/* 提示：偶数 % 2 == 0；C 中没有 bool 类型，用 int 代替 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

int is_even(int n) {
    return (n % 2) == 0;  /* 结果为 0 或 1 */
}

int main(void) {
    int nums[] = {2, 3, 4, 5, 6};
    for (int i = 0; i < 5; i++) {
        printf("%d 是%s偶数\n",
               nums[i], is_even(nums[i]) ? "" : "不");
    }
    return 0;
}
```
</details>

### 🔴 练习 3：同时返回和与差（使用指针）

写一个函数 `void sum_and_diff(int a, int b, int *sum, int *diff)`，通过指针参数同时返回 `a+b` 和 `a-b` 的结果。

```c
/* 高级提示：
   1. 参数 sum 和 diff 是指针类型
   2. 在函数内用 *sum = ... 和 *diff = ... 写入结果
   3. 调用时用 &result_sum 和 &result_diff 传入变量地址
*/
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>

void sum_and_diff(int a, int b, int *sum, int *diff) {
    *sum = a + b;   /* 通过指针写入结果 */
    *diff = a - b;  /* 通过指针写入结果 */
}

int main(void) {
    int x = 10, y = 3;
    int result_sum, result_diff;

    sum_and_diff(x, y, &result_sum, &result_diff);

    printf("%d + %d = %d\n", x, y, result_sum);   /* 13 */
    printf("%d - %d = %d\n", x, y, result_diff);   /* 7 */
    return 0;
}
```

**原理**：值传递不能修改调用者的变量，但**指针传递**可以把「地址」交给函数，让函数直接修改原变量。这是 C 中「函数返回多个值」的标准模式。下一章「指针」会详细讲解。
</details>

## 故障排查（FAQ）

### Q: 为什么编译器说 "implicit declaration of function"？

**A:** 这意味着你在声明或定义之前调用了这个函数。C 编译器从上到下扫描，读到函数调用时必须知道它的签名。

修复方法：在函数调用之前添加声明（如 `int add(int a, int b);`），或者把函数定义移到调用处之前。

### Q: 函数可以没有 return 语句吗？

**A:** 只有 `void` 函数可以没有 `return`。非 `void` 函数如果所有执行路径都有 `return`，可以省略最后的 `return`（但编译器可能会警告）。建议所有路径都写上 `return`。

### Q: 函数可以返回数组吗？

**A:** **不能直接返回数组。** C 语言规定 `return` 只能返回一个值（标量）。但可以：

1. 返回指向数组的指针（需要确保指针指向的内存仍然有效）
2. 把数组封装在 `struct` 中返回
3. 让调用者传入一个数组指针，由函数填充

```c
/* 方式 3: 调用者分配——推荐 */
void fill_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = i * i;
    }
}

int main(void) {
    int data[5];
    fill_array(data, 5);
    return 0;
}
```

### Q: `int func()` 和 `int func(void)` 一样吗？

**A:** 不一样，但在现代 C 中应该用 `void`：
- `int func()` — 旧式声明，表示「参数未知」，编译器不检查参数
- `int func(void)` — 明确表示「没有参数」，编译器会检查

总是用 `func(void)`，这是 C99 之后的推荐写法。

### Q: 一个函数可以有多个 return 语句吗？

**A:** 可以！每个 `return` 都会立即退出函数。常用于提前返回：

```c
int divide(int a, int b) {
    if (b == 0) {
        return 0;  /* 除数为 0，提前返回 */
    }
    return a / b;   /* 正常情况 */
}
```

## 知识扩展（选学）

### 内联函数（Inline Functions）— C99 引入

`inline` 关键字提示编译器将函数体直接展开到调用处，消除函数调用开销。适用于短小且频繁调用的函数：

```c
/* 内联函数：编译器可能直接展开代码 */
static inline int max(int a, int b) {
    return (a > b) ? a : b;
}
```

**我的理解**：内联就像把小工具直接搬到工作台上，而不是每次都要去工具箱里拿。省去了「走过去拿—用完放回」的开销。

注意：`inline` 只是**提示**，编译器不一定要照做。而且 C 的内联函数必须搭配 `static` 使用（在每个包含它的 `.c` 文件中都有定义）。

### 文件内的 static 函数

在 C 中，如果你希望一个函数**只在当前源文件内可见**，其他 `.c` 文件不能调用，可以用 `static`：

```c
/* utils.c */
static void helper_internal(void) {
    /* 只有 utils.c 能调用它 */
}

/* 这是对外接口 */
void public_api(void) {
    helper_internal();  /* 同一文件，可以调用 */
}
```

在其他文件中：
```c
/* main.c */
extern void helper_internal(void);  /* ❌ 链接错误：helper_internal 不可见 */
```

**模式**：公开函数放头文件声明，内部 `static` 函数不声明。这就是**模块化的基础**——隐藏实现细节。

## 小结

恭喜！你已经了解了 C 语言函数的核心概念。让我帮你回顾一下——

- **声明**告诉编译器函数存在（签名），**定义**是函数的具体实现
- C 语言函数**必须先声明后调用**（除非定义在调用之前）
- **参数是值传递**（Pass by Value）——函数收到的是实参的副本，修改不影响原变量
- **返回类型**可以是 `int`、`float`、`char` 等任何类型，也可以是 `void`（无返回值）
- `return` 关键字**返回值并退出函数**
- 常见错误：调用未声明函数 → 加声明；返回类型不匹配 → 改类型；忘记使用返回值 → 接收它

> **我的理解**：函数的本质是「封装」——把一段有特定功能的代码打包成一个工具，给它取个名字。下次需要时，只需调用名字就行。好的函数应该「单一职责」——只做一件事，但做得好。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 函数（Function） | 一段可重复调用的代码块 |
| 声明（Declaration） | 告诉编译器函数的签名，不含实现 |
| 定义（Definition） | 函数的完整实现（签名 + 函数体） |
| 参数（Parameter） | 函数定义中的变量名（形参） |
| 实参（Argument） | 调用函数时传入的具体值（实参） |
| 返回类型（Return Type） | 函数执行后返回的数据类型 |
| 返回值（Return Value） | 函数通过 `return` 传回的具体数据 |
| 值传递（Pass by Value） | 参数以副本方式传递，不影响原变量 |
| 调用（Call / Invoke） | 执行一个函数的行为 |
| void | 表示「无返回值」或「无参数」 |
| 隐式声明（Implicit Declaration） | 未声明就调用函数导致的编译错误 |

## 延伸阅读

- [cppreference: Functions (C)](https://en.cppreference.com/w/c/language/functions)
- [Beej's Guide to C: Functions](https://beej.us/guide/bgc/)
- K&R《C 程序设计语言》第 1.7 节：函数
- 《C Primer Plus》第 5 章：C 语句

## 继续学习

函数是 C 语言中最重要的代码组织工具。现在你已经理解了函数的基本概念，下一章我们将深入探讨**函数的作用域与链接**——`static`、`extern` 关键字如何控制函数在文件间的可见性。

> 💡 **提示**：试着把你之前写的重复代码（比如循环中的重复计算）提取成函数。如果一个代码片段出现了两次以上，它可能就应该是一个函数！

[← 上一章：循环](./loops.md) | [下一章：函数作用域 →](./function_scope.md)
