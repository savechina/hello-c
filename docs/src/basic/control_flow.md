# 控制流：if/else/switch

站在一个十字路口，红绿灯控制着车流方向。绿灯直行，红灯停下，左转箭头亮起时走左转道。信号灯不关心车里坐的是谁，只看当前状态决定放行。

```c
if (rain) {
    bring_umbrella();
} else {
    wear_sunglasses();
}
```

我意识到，原来我可以**让程序自己做决定**。控制流就是程序的"大脑"——它负责选择走哪条路、什么时候重复、什么时候停下来。

## 开篇故事

站在一个十字路口，红绿灯控制着车流方向。绿灯时直行，红灯时停下，左转箭头亮起时走左转道。交通信号灯不关心你车里坐的是谁，它只根据当前状态决定放行路线。每一辆车都在做同样的事情：看灯，选路。

程序的执行也是如此。默认情况下，代码从上到下依次执行，像直行通过一个十字路口。但现实世界充满了分支——下雨就带伞，天晴就戴太阳镜。C 语言的 `if`、`else`、`switch` 就是程序的红绿灯。它们根据条件的真假，让执行路径走向不同的分支。没有控制流，程序只是机械地念台词；有了控制流，程序才开始做选择。

**控制流就是程序的"大脑"——它负责选择走哪条路、什么时候重复、什么时候停下来**。

> 人生最重要的不是站在原地，而是知道在岔路口往哪走。——改编自 罗杰·贝肯

## 本章适合谁

- 已经掌握变量、数组和基本输入输出
- 想让程序根据条件做出不同响应
- 想理解 "missing break" 和 "dangling else" 为什么会导致难以追踪的 bug

## 你会学到什么

- `if / else if / else` 分支结构
- `switch / case / default` + `break` 的重要性
- 三元运算符 `?:` 的简洁写法
- 嵌套条件和悬挂 else 问题
- 常见陷阱：忘记 break、条件赋值与比较混淆

## 前置要求

完成 [变量](./variables.md) 和 [运算符](./operators.md) 章节。

## 第一个例子

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t score = 85;

    if (score >= 90) {
        printf("优秀 (A)\n");
    } else if (score >= 80) {
        printf("良好 (B)\n");
    } else if (score >= 60) {
        printf("及格 (C)\n");
    } else {
        printf("不及格 (F)\n");
    }

    return 0;
}
```

输出：
```
良好 (B)
```

### 分步解析

1. `if (score >= 90)` —— 先检查最高档
2. `else if (score >= 80)` —— 90 不满足，继续往下
3. `score >= 80` 为真，打印 "良好 (B)"，然后**跳过剩余分支**

## 原理解析

### if / else / else if 结构

```c
if (条件1) {
    // 条件1 为真 → 执行这里
} else if (条件2) {
    // 条件1 为假, 条件2 为真 → 执行这里
} else {
    // 所有条件都不为真 → 执行这里
}
```

执行流程像瀑布——从上往下逐层判断，**一旦命中就跳过其余**。

```
条件分支的执行流程:

                  ┌──────────────┐
                  │  程序进入     │
                  └──────┬───────┘
                         │
                   ┌─────▼──────┐
                   │ if (条件1)  │
                   └──┬──────┬──┘
                      │真    │假
                 ┌────▼──┐ ┌─▼─────────┐
                 │ 分支1  │ │ else if   │
                 │        │ │ (条件2)    │
                 └────┬──┘ └──┬───────┬─┘
                      │     真│       │假
                      │ ┌────▼──┐  ┌──▼─────┐
                      │ │ 分支2 │  │ 分支3  │
                      │ │       │  │(else)  │
                      │ └────┬──┘  └──┬─────┘
                      │      │       │
                      └──────┴───────┘
                             │
                    ┌────────▼────────┐
                    │  后续代码继续执行 │
                    └─────────────────┘

switch  vs  if-else 的选择:

  多分支定值比较 → switch   (如: day = 1~7)
  范围/复杂条件 → if-else  (如: score >= 60)
```

### switch / case

当需要根据一个整数的**多个可能值**选择分支时，`switch` 更清晰：

```c
int32_t day = 3;

switch (day) {
    case 1: printf("星期一\n"); break;
    case 2: printf("星期二\n"); break;
    case 3: printf("星期三\n"); break;
    default: printf("无效编号！\n"); break;
}
```

## ⚠️ 知识陷阱预警：switch fall-through

C 语言 `switch` 最著名的陷阱是 **fall-through**：如果 `case` 后面没有 `break`，程序会直接"掉落"执行下一个 `case` 的代码。

```c
int32_t level = 2;
switch (level) {
    case 1:
        printf("初级\n");
        break;
    case 2:
        printf("中级\n");
        // ← 忘了 break！
    case 3:
        printf("高级\n");
        break;
    default:
        printf("未知\n");
}
/* 输出：
   中级
   高级     ← 意外！这不是我们想要的
*/
```

**但 fall-through 有时是有意为之的设计**，不是 bug：
```c
/* ✅ 故意 fall-through：多个 case 共用同一行为 */
switch (ch) {
    case 'a': case 'e': case 'i':
    case 'o': case 'u':
        printf("元音\n");
        break;
    default:
        printf("辅音\n");
}
```

C23 引入了 `[[fallthrough]]` 属性来标记有意的 fall-through。在现代代码中，**每个 case 应该显式以 break、return、continue 或 [[fallthrough]] 结束**。

编译器警告：`-Wimplicit-fallthrough`（GCC/Clang）会在忘记 break 时提醒你。

### 三元运算符 `?:`

```c
int32_t age = 20;
const char *status = (age >= 18) ? "成年人" : "未成年人";
```

等价于：
```c
const char *status;
if (age >= 18) {
    status = "成年人";
} else {
    status = "未成年人";
}
```

三元运算符适合**简单的二选一**赋值，但别嵌套——一旦嵌套三层以上，代码就变成天书了。

## 常见错误

### ❌ 错误 1: switch 中忘记 break

```c
int32_t choice = 1;

// ❌ 没有 break → fall-through (穿透)
switch (choice) {
    case 1:
        printf("选项 A\n");
    case 2:
        printf("选项 B\n");
    default:
        printf("默认选项\n");
}
/*
输出:
  选项 A
  选项 B
  默认选项
*/
```

修复：

```c
// ✅ 每个 case 末尾加 break
switch (choice) {
    case 1:
        printf("选项 A\n");
        break;
    case 2:
        printf("选项 B\n");
        break;
    default:
        printf("默认选项\n");
        break;
}
```

> 例外：有时**故意穿透**是合理的（如多月共享同一天数的逻辑），但要加注释。

### ❌ 错误 2: 条件中使用 `=` 而非 `==`

```c
int32_t x = 5;

// ❌ 这是赋值！不是比较。x 被赋值为 0，条件为假
if (x = 0) {
    printf("x 是零。\n");
}
```

编译器（带 `-Wall`）会警告，但如果忽略警告就会发现 bug。我的习惯：

```c
// ✅ Yoda condition —— 如果写成 = 会编译报错
if (0 == x) {
    printf("x 是零。\n");
}
```

或者养成用 `==` 的习惯即可。

### ❌ 错误 3: 悬挂 else (Dangling Else)

```c
int32_t a = 5, b = 10;

// ❌ else 属于哪个 if？C 的规则是"就近匹配"
if (a > 3)
    if (b > 20)
        printf("b > 20\n");
else
    printf("这个 else 属于内层 if (b > 20)！\n");
```

修复——**永远用花括号**：

```c
// ✅ 明确匹配关系
if (a > 3) {
    if (b > 20) {
        printf("b > 20\n");
    }
} else {
    printf("这个 else 属于外层 if (a > 3)。\n");
}
```

## 动手练习

<details>
<summary>🟢 入门: 判断奇偶数</summary>

写一个程序，根据输入的整数判断是奇数还是偶数。

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t n = 7;

    if (n % 2 == 0) {
        printf("%d 是偶数。\n", n);
    } else {
        printf("%d 是奇数。\n", n);
    }

    return 0;
}
```

**输出**: `7 是奇数。`

</details>

<details>
<summary>🟡 中级: 简易计算器</summary>

用 switch 实现一个简易计算器，支持 `+ - * /` 四则运算。

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    double a = 10.0, b = 3.0;
    char op = '/';

    switch (op) {
        case '+':
            printf("%.2f + %.2f = %.2f\n", a, b, a + b);
            break;
        case '-':
            printf("%.2f - %.2f = %.2f\n", a, b, a - b);
            break;
        case '*':
            printf("%.2f * %.2f = %.2f\n", a, b, a * b);
            break;
        case '/':
            if (b != 0.0) {
                printf("%.2f / %.2f = %.2f\n", a, b, a / b);
            } else {
                printf("错误: 除以零！\n");
            }
            break;
        default:
            printf("未知运算符: %c\n", op);
            break;
    }

    return 0;
}
```

**输出**: `10.00 / 3.00 = 3.33`

</details>

<details>
<summary>🔴 挑战: 闰年判断</summary>

判断给定年份是否为闰年。规则：能被 4 整除但不能被 100 整除；或者能被 400 整除。

```c
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

bool is_leap_year(int32_t year) {
    if (year % 400 == 0) {
        return true;
    }
    if (year % 100 == 0) {
        return false;
    }
    if (year % 4 == 0) {
        return true;
    }
    return false;
}

int main(void) {
    int32_t years[] = {2024, 1900, 2000, 2023};
    int32_t n = (int32_t)(sizeof(years) / sizeof(years[0]));

    for (int32_t i = 0; i < n; i++) {
        const char *label = is_leap_year(years[i]) ? "闰年" : "平年";
        printf("%d: %s\n", years[i], label);
    }

    return 0;
}
```

**输出**:
```
2024: 闰年
1900: 平年
2000: 闰年
2023: 平年
```

</details>

## 故障排查 (FAQ)

<details>
<summary>Q: switch 的 case 后面能用变量吗？</summary>

A: **不能**。`case` 后面必须是**编译时常量** (compile-time constant)。

```c
int32_t x = 5;
switch (val) {
    case x:      // ❌ 编译错误
    case 5:      // ✅ 常量，可以
    case 2 + 3:  // ✅ 编译器能算出的常量，也可以
}
```

如果需要运行时比较，用 `if / else if`。

</details>

<details>
<summary>Q: switch 能用在字符串比较上吗？</summary>

A: **不能**。C 语言的 `switch` 只支持整数类型 (`int`、`char`、`enum`)。字符串比较需要用 `<string.h>` 的 `strcmp` 配合 `if / else if`：

```c
if (strcmp(name, "Alice") == 0) {
    // ...
} else if (strcmp(name, "Bob") == 0) {
    // ...
}
```

</details>

<details>
<summary>Q: if 条件里写 `if (flag = true)` 和 `if (flag == true)` 一样吗？</summary>

A: **不一样**！这恰恰是"错误 2"的变体。

- `flag == true`：比较，返回 bool
- `flag = true`：赋值，整个表达式的值就是 `true`——条件**永远成立**

所以这行代码等价于"无条件执行"，通常不是你想要的。

</details>

## 知识扩展 (选学)

### Duff's Device —— switch 的极限用法

```c
void send(int *to, int *from, int count) {
    int n = (count + 7) / 8;
    switch (count % 8) {
        case 0: do { *to++ = *from++;
        case 7:      *to++ = *from++;
        case 6:      *to++ = *from++;
        case 5:      *to++ = *from++;
        case 4:      *to++ = *from++;
        case 3:      *to++ = *from++;
        case 2:      *to++ = *from++;
        case 1:      *to++ = *from++;
                    } while (--n > 0);
    }
}
```

这是合法的 C 代码，利用了 switch 的 fall-through 特性做**循环展开** (loop unrolling) 优化。日常几乎不需要，但知道后你会重新认识 `switch` 的灵活性。

### 守卫子句 (Guard Clause)

在函数开头尽早 `return`，避免深层嵌套：

```c
// ❌ 深层嵌套
void process(int *data, int len) {
    if (data != NULL) {
        if (len > 0) {
            // 真正逻辑缩进很深
        }
    }
}

// ✅ 守卫子句
void process(int *data, int len) {
    if (data == NULL) return;
    if (len <= 0)     return;
    // 真正逻辑在这里，少了一层缩进
}
```
### 🎯 预测运行结果

先别急着运行——读下面的代码，预测它会输出什么？

```c
#include <stdio.h>
#include <stdint.h>

int main(void) {
    int32_t num = 2;
    switch (num) {
        case 1:
            printf("一 ");
        case 2:
            printf("二 ");
        case 3:
            printf("三 ");
            break;
        default:
            printf("其他 ");
    }
    printf("结束\n");
    return 0;
}
```

<details><summary>点击查看答案</summary>

**预测：** 程序会输出 `二 三 结束`

**实际输出：**
```
二 三 结束
```

**为什么？**

1. `num = 2` 匹配 `case 2`，开始执行
2. `case 2` 末尾**没有 `break`**，程序"穿透"(fall-through) 继续执行 `case 3`
3. `case 3` 打印"三 "，遇到 `break` 跳出 `switch`
4. 最后打印"结束"

**关键教训**：`switch` 的每个 `case` 末尾必须显式以 `break` 结束（除非故意需要 fall-through）。GCC/Clang 的 `-Wimplicit-fallthrough` 选项可以帮助你发现这类问题。

</details>

## 小结

通过这一章我发现：

- `if / else if / else` 是条件判断的基础结构——命中就执行，其余跳过
- `switch / case` 适合整数多分支场景，但**别忘了 `break`**
- 三元运算符 `?:` 适合简单的二选一赋值，别嵌套使用
- **悬挂 else**：C 的 else 就近匹配内层 if——用花括号消除歧义
- `=` 和 `==` 是常见的混淆源，编译器警告务必重视

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 控制流 | Control Flow | 程序执行的路径和顺序 |
| 分支 | Branch | 根据条件选择不同执行路径 |
| 穿透 | Fall-through | switch 中缺少 break 导致执行进入下一个 case |
| 悬挂 else | Dangling else | else 与哪个 if 匹配的歧义 |
| 三元运算符 | Ternary operator | `?:`，唯一的三元运算符 |
| 守卫子句 | Guard clause | 在函数开头提前 return 减少嵌套 |
| 编译时常量 | Compile-time constant | 编译时就能确定值的表达式 |
| 循环展开 | Loop unrolling | 将循环体展开多次以减少循环开销的优化技术 |

## 延伸阅读

- [C99 标准 §6.8.4 — 选择语句 (Selection statements)](https://en.cppreference.com/w/c/language/if)
- [Duff's Device — 经典 C 优化技巧](https://en.wikipedia.org/wiki/Duff%27s_device)
- [Guard clause — 维基百科](https://en.wikipedia.org/wiki/Guard_(computer_science))
- 上一章: [运算符](./operators.md)

## 继续学习

| 下一步 | 方向 |
|--------|------|
| 下一章 → | [循环](./loops.md) — for/while/do-while |
| 复习 ← | [运算符](./operators.md) |
| 深入 → | [预处理器](./preprocessor.md) — #define, #ifdef |
