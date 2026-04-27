# 循环：for / while / do-while

> "我发现，循环就像生活中的重复劳动——掌握了它，你就能让计算机替你搬砖。" —— 我常犯的错

## 开篇故事

大一写 C 语言时，我有个"坏习惯"：想打印数字 1 到 100，我就老老实实写了 100 行 `printf`。室友看我代码，问我是不是在练打字速度。

直到我学了循环（Loops），我才恍然大悟——**原来 3 行代码就能搞定的事，我居然写了 100 行**。从此我爱上了循环，但也掉进了另一个坑：死循环（Infinite Loop）。有一次我的程序卡住了，风扇狂转，我差点以为电脑坏了——后来发现是 `while(1)` 里忘了加 `break`。

本章我就带你掌握 C 语言的三种循环，让你写出优雅又安全的重复代码。

## 本章适合谁

- 已经学过变量、运算符、控制流（if/else/switch）的 C 初学者
- 经常复制粘贴相似代码，想学会自动化重复的人
- 被 `break`、`continue` 搞晕过的人

## 你会学到什么

- `for` 循环：初始化、条件判断、递增三步曲
- `while` 循环：条件驱动的循环
- `do-while` 循环：至少执行一次的循环
- `break` 跳出循环与 `continue` 跳过本次迭代
- 嵌套循环（Nested Loops）与乘法表
- 循环不变量（Loop Invariant）概念
- 无限循环的安全使用
- 如何选择合适的循环类型

## 前置要求

- 了解 C 基本数据类型（`int`、`double`）
- 会使用 `printf` 输出
- 了解 `if/else` 条件判断

> 如果你还没学过控制流，建议先看「控制流」章节。

## 第一个例子：打印 1 到 5

```c
#include <stdio.h>

int main(void) {
    for (int i = 1; i <= 5; i++) {
        printf("%d ", i);
    }
    putchar('\n');
    return 0;
}
```

运行结果：

```
1 2 3 4 5
```

就这么简单！但你有没有想过 `for` 括号里三个部分各自在什么时候执行？让我帮你拆开看。

## 原理解析

### 1. for 循环：三步曲

`for` 循环是最常用的循环，语法如下：

```c
for (初始化; 条件; 递增) {
    循环体;
}
```

**执行顺序**：

1. **初始化**：只在循环开始前执行一次（声明循环变量 `int i = 1`）
2. **条件**：每次循环开始前判断（`i <= 5`），为真则进入循环体，为假则退出
3. **循环体**：执行 `{}` 中的代码
4. **递增**：每次循环体结束后执行（`i++`），然后回到步骤 2

**我常犯的错**：把递增忘了写，结果变成死循环——`for (int i = 1; i <= 5; )`，`i` 永远是 1，条件永远为真。

```c
/* 正确写法 */
for (int i = 1; i <= 5; i++) {
    printf("%d ", i);
}

/* 多变量写法——同时控制两个变量 */
for (int i = 0, j = 10; i < j; i++, j--) {
    printf("i=%d, j=%d\n", i, j);
}
```

### 2. while 循环：条件驱动

当循环次数未知时，`while` 是更好的选择：

```c
while (条件) {
    循环体;
}
```

**我的理解**：`while` 就像在门口放个保安，每次进去前都要检查条件。如果一开始条件就是假的，循环体一次都不会执行。

```c
/* 计算 1+2+...+100 */
int sum = 0;
int i = 1;          /* 初始化在循环外 */

while (i <= 100) {  /* 条件判断 */
    sum += i;
    i++;            /* 递增在循环体内——容易遗漏！ */
}
```

### 3. do-while 循环：至少执行一次

```c
do {
    循环体;
} while (条件);
```

**关键区别**：`do-while` 先执行、后判断，所以循环体**至少执行一次**。常用于"先询问用户输入，再验证"的场景。

```c
int input;

do {
    printf("请输入正数（输入 0 退出）：");
    scanf("%d", &input);
    if (input > 0) {
        printf("你输入了：%d\n", input);
    }
} while (input != 0);
```

### 4. break 与 continue

| 关键字 | 行为 | 记忆口诀 |
|--------|------|----------|
| `break` | 立刻跳出**整个循环** | "打破，不干了" |
| `continue` | 跳过本次迭代，直接进入下一次 | "这次不算，再来" |

```c
/* break: 找到第一个 3 的倍数就停止 */
for (int i = 1; i <= 10; i++) {
    if (i % 3 == 0) {
        printf("找到 %d，break！\n", i);
        break;
    }
}

/* continue: 跳过偶数，只打印奇数 */
for (int i = 1; i <= 10; i++) {
    if (i % 2 == 0) {
        continue;
    }
    printf("%d ", i);
}
/* 输出: 1 3 5 7 9 */
```

### 5. 嵌套循环

循环里再套循环，像俄罗斯套娃：

```c
/* 九九乘法表 (1-5) */
for (int i = 1; i <= 5; i++) {        /* 外层：行 */
    for (int j = 1; j <= i; j++) {    /* 内层：列 */
        printf("%d×%d=%-4d", j, i, i * j);
    }
    putchar('\n');
}
```

**我的理解**：外层循环每走一步，内层循环要跑完一整圈。外层 5 次 × 内层平均 3 次 = 大约 15 次迭代。

### 6. 循环不变量（Loop Invariant）

循环不变量是指：**在每次循环迭代前后都保持为真的条件**。它帮助我们证明循环的正确性。

```c
/* 找数组最大值 */
int arr[] = {3, 7, 2, 9, 1};
int max = arr[0];

for (int i = 1; i < 5; i++) {
    /* 不变量：max 始终是 arr[0..i-1] 中的最大值 */
    if (arr[i] > max) {
        max = arr[i];
    }
}
/* 循环结束后：max 是 arr[0..4] 中的最大值 → 9 */
```

### 7. 无限循环与安全退出

```c
/* 安全写法：无限循环 + break */
for (;;) {
    /* 做某件事 */
    if (某个条件) {
        break;  /* 必须有退出机制！ */
    }
}

/* 同样安全的 while(1) 写法 */
while (1) {
    /* 做某件事 */
    if (某个条件) {
        break;
    }
}
```

**我常犯的错**：写 `while(1)` 时忘了写 `break`，程序卡死。所以写无限循环时，我会在循环体第一行就写下 `if (...) break;`，防止忘记。

### 8. 如何选择合适的循环类型

| 循环类型 | 适用场景 | 典型例子 |
|----------|---------|---------|
| `for` | 已知循环次数，或有明确的初始化-条件-递增模式 | 遍历数组、计数 |
| `while` | 未知循环次数，依赖某个条件 | 读取文件直到 EOF |
| `do-while` | 至少需要执行一次 | 菜单选择、输入验证 |

## 常见错误

### ❌ 错误 1：for 循环漏写递增，导致死循环

```c
for (int i = 1; i <= 5; ) {
    printf("%d ", i);
    /* 忘了 i++，i 永远是 1，无限循环！ */
}
```

**编译器不会报错**——语法完全正确，但程序会卡死。

✅ **修正**：补上递增语句。

```c
for (int i = 1; i <= 5; i++) {  /* ✅ 加上 i++ */
    printf("%d ", i);
}
```

### ❌ 错误 2：do-while 漏写分号

```c
do {
    printf("hello\n");
} while (x < 5)  /* ❌ 编译错误：expected ';' before '}' token */
```

✅ **修正**：`do-while` 的 `while(...)` 后必须有分号。

```c
do {
    printf("hello\n");
} while (x < 5);  /* ✅ 加分号 */
```

### ❌ 错误 3：continue 在 for 循环中跳过了递增

```c
for (int i = 0; i < 10; i++) {
    if (i == 5) {
        continue;  /* continue 会跳到 i++，所以这种写法是安全的 */
    }
    printf("%d ", i);
}
/* 注意：上面的代码是正确的。但如果递增放在循环体内： */

int j = 0;
while (j < 10) {
    if (j == 5) {
        continue;  /* ❌ continue 跳过 j++，j 永远 = 5，死循环！ */
    }
    printf("%d ", j);
    j++;
}
```

✅ **修正**：确保 `continue` 不会跳过后置的递增操作。

```c
int j = 0;
while (j < 10) {
    if (j == 5) {
        j++;        /* ✅ continue 前先递增 */
        continue;
    }
    printf("%d ", j);
    j++;
}
```

## 动手练习

### 🟢 练习 1：计算 1 到 100 中所有偶数的和

```c
/* 用 for 循环实现，结果应为 2550 */
```

<details>
<summary>点击查看答案</summary>

```c
int sum = 0;
for (int i = 2; i <= 100; i += 2) {
    sum += i;
}
printf("1..100 偶数之和 = %d\n", sum);  /* 2550 */
```

或者用 `continue`：

```c
int sum = 0;
for (int i = 1; i <= 100; i++) {
    if (i % 2 != 0) {
        continue;
    }
    sum += i;
}
```
</details>

### 🟡 练习 2：用嵌套循环打印金字塔

```c
/* 打印如下图案（5 行）：
    *
   ***
  *****
 *******
*********
*/
```

<details>
<summary>点击查看答案</summary>

```c
int rows = 5;
for (int i = 1; i <= rows; i++) {
    /* 打印空格 */
    for (int s = 0; s < rows - i; s++) {
        putchar(' ');
    }
    /* 打印星号 */
    for (int j = 0; j < 2 * i - 1; j++) {
        putchar('*');
    }
    putchar('\n');
}
```

**思路**：第 `i` 行有 `(rows - i)` 个前导空格和 `(2*i - 1)` 个星号。
</details>

### 🔴 练习 3：用循环实现二分查找（Binary Search）

```c
/* 在有序数组中查找目标值，返回索引（找不到返回 -1）
   int arr[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
   查找 23 → 返回 5
   查找 7  → 返回 -1
*/
```

<details>
<summary>点击查看答案</summary>

```c
int binary_search(int arr[], int n, int target) {
    int left = 0;
    int right = n - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1;  /* 未找到 */
}

/* 测试 */
int arr[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
int n = (int)(sizeof(arr) / sizeof(arr[0]));

printf("查找 23: 索引 %d\n", binary_search(arr, n, 23));  /* 5 */
printf("查找 7:  索引 %d\n", binary_search(arr, n, 7));   /* -1 */
```

**循环不变量**：如果 `target` 存在于数组中，它一定在 `arr[left..right]` 范围内。每次迭代将搜索范围减半，时间复杂度 O(log n)。
</details>

## 故障排查（FAQ）

### Q: `for`、`while`、`do-while` 可以互相替代吗？

**可以**，任何循环都能用另外两种改写。选择的标准是**可读性**——哪个最清晰地表达你的意图就用哪个。

### Q: `break` 能跳出多层嵌套循环吗？

**不能**。`break` 只跳出**最内层**的循环。如果需要跳出多层，可以用 `goto`（争议但有实用场景）或设置标志变量：

```c
int found = 0;
for (int i = 0; i < 10 && !found; i++) {
    for (int j = 0; j < 10; j++) {
        if (arr[i][j] == target) {
            found = 1;
            break;
        }
    }
}
```

### Q: 循环变量在循环结束后还能用吗？

如果在 `for` 的初始化中声明变量（C99 起支持），它的**作用域仅限于循环体**：

```c
for (int i = 0; i < 5; i++) {
    printf("%d ", i);
}
/* i 在这里已不存在，继续使用会编译错误 */
```

### Q: `while(1)` 和 `for(;;)` 有什么区别？

**没有区别**。两者都编译为相同的无限循环。选择哪个取决于个人风格，我更喜欢 `for (;;)`，因为它更明确地表达了"无初始化、无条件、无递增"。

## 知识扩展（选学）

### 循环优化：循环展开（Loop Unrolling）

编译器会自动做循环展开，手动展开有时能提升性能：

```c
/* 原始：每次迭代处理 1 个元素 */
for (int i = 0; i < 100; i++) {
    process(arr[i]);
}

/* 展开：每次迭代处理 4 个元素，减少循环开销 */
for (int i = 0; i < 100; i += 4) {
    process(arr[i]);
    process(arr[i+1]);
    process(arr[i+2]);
    process(arr[i+3]);
}
```

### 范围-based 循环

C 语言没有像 C++ 的 `for (int x : arr)` 语法，但你可以用宏模拟：

```c
#define FOR_EACH(elem, arr, len) \
    for (size_t _i = 0; _i < (len) && ((elem) = (arr)[_i]); _i++)

int nums[] = {1, 2, 3};
int val;
FOR_EACH(val, nums, 3) {
    printf("%d\n", val);
}
```

### GCC 的 `__attribute__((hot))`

给频繁执行的循环加上此属性，可以提示编译器优化：

```c
for (int i = 0; i < 1000000; i++)
    __attribute__((hot)) process(arr[i]);
```

## 小结

恭喜！你已经掌握了 C 语言的循环结构。让我帮你回顾一下——

- **`for`**：适合已知次数的循环，三步曲：初始化 → 条件 → 递增
- **`while`**：条件驱动，可能一次都不执行
- **`do-while`**：至少执行一次，适合输入验证
- **`break`** 终止整个循环，**`continue`** 跳过本次迭代
- 嵌套循环 = 外层走一步，内层跑一圈
- 循环不变量帮助你理解并证明循环的正确性
- 无限循环必须有安全的退出机制（`break`）
- 选择循环类型的标准：**哪个最清晰地表达意图**

> **我的理解**：循环不是"会写就行"，关键在于**选择正确的类型**和**保证循环终止**。每次写循环前，我都会问自己：初始条件是什么？终止条件是什么？每次迭代在靠近终止条件吗？

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| 循环（Loop） | 重复执行某段代码的结构 |
| for 循环 | 三步曲循环：初始化、条件、递增 |
| while 循环 | 条件驱动的循环 |
| do-while 循环 | 先执行后判断的循环 |
| 迭代（Iteration） | 循环体的一次执行 |
| 递增（Increment） | 循环变量每次迭代后的更新操作 |
| break | 跳出当前循环 |
| continue | 跳过本次迭代，进入下一次 |
| 嵌套循环（Nested Loop） | 循环体内包含另一个循环 |
| 循环不变量（Loop Invariant） | 在每次迭代前后始终为真的条件 |
| 无限循环（Infinite Loop） | 永远不会自行终止的循环 |
| 死循环 | 非预期的无限循环（通常是 bug） |
| 循环展开（Loop Unrolling） | 减少循环次数以提升性能的技术 |

## 延伸阅读

- [cppreference: Iteration Statements (C)](https://en.cppreference.com/w/c/language/for)
- [Beej's Guide to C: Loops](https://beej.us/guide/bgc/)
- K&R《C 程序设计语言》第 1.10 节：循环
- 《C Primer Plus》第 6 章：C 控制语句：循环

## 继续学习

循环是 C 语言"重复执行"的基础。下一章我们将学习**预处理器与宏**，了解编译之前发生的事情——`#define`、`#include`、条件编译等如何在代码运行前完成魔法般的替换。

> 💡 **提示**：试着把你之前写的重复代码改成循环形式。如果你的代码里有连续 3 行相似的结构，几乎一定可以用循环简化！

[← 上一章：控制流](./control-flow.md) | [下一章：预处理器与宏 →](./preprocessor.md)
