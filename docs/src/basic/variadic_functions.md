# 可变参数函数 (Variadic Functions)

> "printf 就像一个万能插座——你可以往里插任何数量的电器。但如果你不告诉它有多少个，它可能会把电引到不该去的地方，然后'烧坏'。" —— 我发现

## 开篇故事

想象你买了一台万能榨汁机。它有个大口子，可以往里放一个苹果、两个香蕉、五个橘子——数量不定。但榨汁机需要知道两件事：第一，你往里放了什么水果（类型）；第二，你放了多少个（数量）。

`printf` 就是 C 语言里的万能榨汁机——它接受任意数量的参数，但靠格式字符串 `%d %s %f` 来告诉它参数的类型和数量。如果你传参和格式不匹配，就像把苹果皮扔进了榨汁机——结果不可预测。

可变参数函数（Variadic Functions）就是让你自己造"榨汁机"的能力。C 语言用 `<stdarg.h>` 提供了一套工具：`va_list` 装参数，`va_start` 开始提取，`va_arg` 一个接一个取出来，`va_end` 清洗收尾。

```c
int sum = my_sum(3, 10, 20, 30);  // "3" 告诉函数要处理 3 个参数
```

## 本章适合谁

- 已经会写普通函数，参数数量和类型固定的 C 初学者
- 对 `printf("%s", name, age)` 背后如何工作感到好奇的人
- 想自己写一个类似 `printf` 的日志函数的人
- 听说过 `va_list` 但不知道为什么需要它的人

## 你会学到什么

- `<stdarg.h>` 头文件的作用
- `va_list` —— 可变参数列表的"容器"
- `va_start` —— 初始化，标记从哪里开始取参数
- `va_arg` —— 逐个提取参数，每次推进一个
- `va_end` —— 清理资源（必不可少）
- `printf` 是如何用这套机制工作的
- 两种控制参数边界的模式：count 参数 vs sentinel value
- `va_copy` —— 如何遍历参数列表两次

## 前置要求

- 已经理解函数声明、定义、参数传递（见[函数](./functions.md)章节）
- 理解指针概念（见[指针](./pointers.md)章节）
- 知道 `printf` 的基本用法和格式字符串

> 如果你还没学函数或指针，建议先补上这两章。可变参数函数的核心就是"把参数列表当成一段内存来遍历"。

## 第一个例子

下面是一个最简单的可变参数函数——计算 N 个整数的和：

```c
#include <stdio.h>
#include <stdarg.h>

int sum(int count, ...) {
    va_list args;
    va_start(args, count);

    int total = 0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);  // 取下一个参数，当作 int
    }

    va_end(args);
    return total;
}

int main(void) {
    int result = sum(3, 10, 20, 30);
    printf("sum = %d\n", result);  // 输出: sum = 60
    return 0;
}
```

运行结果：

```
sum = 60
```

这段代码做了四件事：
1. `va_list args;` — 声明一个"参数列表容器"
2. `va_start(args, count);` — 告诉容器从 `count` 之后开始取
3. `va_arg(args, int);` — 每次取一个，总共取 `count` 次
4. `va_end(args);` — 清理，收尾

## 原理解析

### 1. 四个核心宏

`<stdarg.h>` 提供了四个核心宏（本质上是一段底层代码）：

| 宏 | 作用 | 示例 |
|----|------|------|
| `va_list` | 声明参数列表变量 | `va_list args;` |
| `va_start(args, last_named)` | 初始化，指向第一个可变参数 | `va_start(args, count);` |
| `va_arg(args, type)` | 取当前参数并推进到下一个 | `va_arg(args, int)` |
| `va_end(args)` | 清理（使 `args` 失效） | `va_end(args);` |

**三步曲记忆法**：
```
va_start(args, last_named)  → 告诉从哪里开始
va_arg(args, type)           → 一个一个取出来
va_end(args)                 → 清理收尾
```

```
可变参数在栈上的布局 (x86-64 调用约定):

示例调用: sum(3, 10, 20, 30);

    高地址
    ┌────────────────────────────┐
    │  第 3 个可变参数: 30       │  ← va_arg 第三次调用
    ├────────────────────────────┤
    │  第 2 个可变参数: 20       │  ← va_arg 第二次调用
    ├────────────────────────────┤
    │  第 1 个可变参数: 10       │  ← va_arg 第一次调用
    ├────────────────────────────┤
    │  last_named: count = 3     │  ← 固定参数, va_start 从这里开始
    ├────────────────────────────┤
    │  返回地址 / 其他寄存器     │
    └────────────────────────────┘
    低地址

va_list 内部指针移动:
  ① va_start(args, count)  →  args 指向 count 之后 (第 1 个可变参数)
  ② va_arg(args, int)      →  读取 10, 指针前进到 20
  ③ va_arg(args, int)      →  读取 20, 指针前进到 30
  ④ va_arg(args, int)      →  读取 30, 指针到达末尾
```

### 2. 为什么需要 `last_named` 参数？

C 语言的可变参数函数**至少需要一个固定参数**。`va_start` 需要这个参数来确定可变参数的起始位置。

```c
int sum(int count, ...) {
    //         ↑ last_named 参数
    //         va_start 从这里后面的内存开始读
}
```

### 3. 两种控制边界的模式

可变参数函数没有内置的"参数数量"信息——你必须自己告诉它什么时候停止。两种主流模式：

**模式 A: Count 参数（推荐）**

```c
int sum(int count, ...) {
    va_list args;
    va_start(args, count);
    int total = 0;
    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);
    }
    va_end(args);
    return total;
}

// 使用：第一个参数告诉函数有多少个
sum(3, 10, 20, 30);
```

**模式 B: Sentinel Value（哨兵值）**

```c
void print_ints(int first, ...) {
    printf("%d", first);

    va_list args;
    va_start(args, first);
    int val;
    while ((val = va_arg(args, int)) != -1) {  // -1 是哨兵
        printf(", %d", val);
    }
    va_end(args);
}

// 使用：末尾加 -1 标记结束
print_ints(1, 2, 3, -1);
```

| 模式 | 优点 | 缺点 |
|------|------|------|
| Count 参数 | 类型安全，不会误判 | 调用者必须准确传 count |
| Sentinel Value | 直观（类似 `NULL` 结尾的 `argv`） | 哨兵值不能出现在真实数据中 |

### 4. printf 的内部实现

`printf` 本质上是这样的：

```c
int printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int count = vprintf(fmt, args);  // "v" 版本接受 va_list
    va_end(args);
    return count;
}
```

格式字符串 `fmt` 中的 `%d`、`%s`、`%f` 告诉 `vprintf`：
- `%d` → 取一个 `int`（`va_arg(args, int)`）
- `%s` → 取一个 `char*`（`va_arg(args, char*)`）
- `%f` → 取一个 `double`（`va_arg(args, double)`）

你传的格式和实际类型不匹配，就是未定义行为（undefined behavior）。

### 5. va_copy — 遍历两次

`va_list` 被 `va_arg` 消费后就无法重置。如果需要多次遍历同一组参数，用 `va_copy` 复制一份：

```c
void demo(int count, ...) {
    va_list args1, args2;
    va_start(args1, count);
    va_copy(args2, args1);  // 复制 args1 到 args2

    int sum1 = 0, sum2 = 0;
    for (int i = 0; i < count; i++) sum1 += va_arg(args1, int);
    for (int i = 0; i < count; i++) sum2 += va_arg(args2, int);

    va_end(args2);
    va_end(args1);
}
```

## 常见错误

### ❌ 错误 1：va_arg 类型不匹配

```c
sum(2, 3.14, 2.72);  // ❌ 传入 double，但 va_arg(args, int) 当 int 读
```

`double` 是 8 字节，`int` 是 4 字节——`va_arg` 读了 4 字节，得到的是一个垃圾值。更严重的是，指针位置偏移错误，后续参数全部错位。

✅ **修复**：`va_arg` 的类型必须与传入的实际类型完全一致。

```c
sum(2, 3, 2);  // ✅ 传入 int
```

### ❌ 错误 2：count > 实际参数数量

```c
sum(5, 1, 2);  // ❌ 说 5 个，只传 2 个 → 后面 3 次读的是栈上垃圾
```

`va_arg` 会继续读栈上的随机内存，返回值完全不可控。

✅ **修复**：永远确保 count 不超过实际传入的参数数。

### ❌ 错误 3：忘记 va_end

```c
int broken(int count, ...) {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; i++) sum += va_arg(args, int);
    // 忘记 va_end(args); → 未定义行为
    return sum;
}
```

`va_end` 在某些平台上是空操作（宏展开为空），但在另一些平台（如使用寄存器传递参数的架构上）它是必需的清理步骤。忘记它，程序可能在某些平台上"巧合正常工作"，在其他平台上崩溃——这是最危险的 bug 类型。

✅ **修复**：每次 `va_start` 必须配对 `va_end`。

### ❌ 错误 4：va_list 被重复使用

```c
va_list args;
va_start(args, count);
// 第一次遍历
for (int i = 0; i < count; i++) sum += va_arg(args, int);
// 第二次遍历 —— args 已经指到末尾了！
for (int i = 0; i < count; i++) avg += va_arg(args, int);  // ❌ 全读到越界
va_end(args);
```

✅ **修复**：用 `va_copy` 复制一份：

```c
va_list args1, args2;
va_start(args1, count);
va_copy(args2, args1);
// 用 args1 遍历...
// 用 args2 遍历...
va_end(args2);
va_end(args1);
```

## 动手练习

### 🟢 练习 1：实现 variadic average 函数

写一个可变参数函数，计算 N 个整数的平均值。

```c
double average(int count, ...);
// average(3, 10, 20, 30) → 20.0
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdarg.h>

double average(int count, ...) {
    va_list args;
    va_start(args, count);

    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += va_arg(args, int);
    }

    va_end(args);
    return (double)sum / count;
}
```
</details>

### 🟡 练习 2：实现 variadic max 函数

写一个可变参数函数 $，用 sentinel value 模式（INT_MIN 作为哨兵），返回最大值。

```c
#include <limits.h>

int max_sentinel(int first, ...) {
    int max = first;

    va_list args;
    va_start(args, first);
    int val;
    while ((val = va_arg(args, int)) != INT_MIN) {
        if (val > max) max = val;
    }
    va_end(args);
    return max;
}

// 使用: max_sentinel(3, 8, 1, 9, 2, INT_MIN) → 9
```

### 🔴 练习 3：实现 variadic print 函数（带格式字符串）

写一个类似 `printf` 的日志函数，自动添加 `[MYLOG]` 前缀和换行符。

```c
void mylog(const char *fmt, ...);

// mylog("User %s logged in from %s", "Alice", "192.168.1.1");
// 输出: [MYLOG] User Alice logged in from 192.168.1.1
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdarg.h>

void mylog(const char *fmt, ...) {
    printf("[MYLOG] ");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);  // vprintf 消费 va_list
    va_end(args);

    printf("\n");
}
```
</details>

## 故障排查 (FAQ)

### Q: 为什么 `va_arg(args, int)` 读 double 会段错误？

因为 `va_arg` 的第二个参数不仅决定**如何解释**读到的数据，还决定**读多少字节**：

- `va_arg(args, int)` → 从栈上读 4 字节，解释为 int
- `double` → 在栈上占 8 字节

当你用 `int` 去读一个 `double` 时：
1. 只读了 double 的前 4 个字节 → 得到垃圾值
2. 指针只前进 4 字节（而不是 8）→ 下一个 `va_arg` 从 double 的中间开始读 → 两个参数都错位了

最终可能读到不属于当前栈帧的内存，触发段错误。

### Q: `va_list` 可以重复使用吗？

**不可以**。每次调用 `va_arg` 都会推进内部指针。遍历一次后就指向末尾了。如果需要再次遍历，必须用 `va_copy` 复制一份。

### Q: 可变参数函数可以有返回值吗？

**可以**。可变参数只影响函数的输入（参数列表），不影响输出（返回值）。`sum()`, `printf()` 都有返回值。

### Q: 为什么可变参数函数至少要有一个固定参数？

因为 `va_start` 需要知道可变参数从哪儿开始——它通过最后一个固定参数的地址来推算。没有固定参数，编译器就不知道从哪里开始读可变参数。

### Q: `__VA_ARGS__` 和 `va_list` 有什么区别？

- `__VA_ARGS__` 是**预处理器宏**（`#define`），在编译前展开，用于宏的可变参数
- `va_list` 是**运行时机制**（`<stdarg.h>`），用于函数的可变参数

```c
// 宏级别的可变参数（编译时展开）
#define LOG(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

// 函数的可变参数（运行时）
void my_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}
```

## 知识扩展 (选学)

### vprintf 系列函数

当你写了一个可变参数函数，想转发给 `printf` 处理时，不能直接用 `printf`——你需要 `printf` 家族的 `v` 前缀版本：

| 函数 | 对应 | 接受参数 |
|------|------|---------|
| `vprintf(fmt, args)` | `printf(...)` | `va_list args` |
| `vfprintf(stream, fmt, args)` | `fprintf(...)` | `va_list args` |
| `vsnprintf(buf, size, fmt, args)` | `snprintf(...)` | `va_list args` |

**典型用法**：包装 `printf` 添加前缀

```c
void warn(const char *fmt, ...) {
    fprintf(stderr, "[WARN] ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}
```

`printf` 本身就是这样实现的——它收到 `...`，用 `va_start` 包装成 `va_list`，然后交给 `vprintf` 处理。

### 编译器格式字符串检查

GCC 和 Clang 支持 `__attribute__((format))`，让你的函数也有 `printf` 那样的编译期格式检查：

```c
void mylog(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)));
//                          ↑ fmt 在第 1 个参数
//                               可变参数从第 2 个开始
```

加上这个属性后，`mylog("%d", "hello")` 会在编译时报类型不匹配警告。

## 小结

本章的核心要点：

- **可变参数函数** = 至少一个固定参数 + `...` + `<stdarg.h>`
- **四步曲**：`va_list` 声明 → `va_start` 初始化 → `va_arg` 逐个提取 → `va_end` 清理
- **类型必须匹配**：`va_arg(args, type)` 的 `type` 必须与实际传入的完全一致
- **边界必须明确**：用 count 参数或 sentinel value 告诉函数何时停止
- **`vprintf` 系列**：可变参数函数中转发参数的标准方式
- **每次 `va_start` 必须配对 `va_end`**，忘记它是最危险的错误之一

## 术语表

| 英文 | 中文 |
|------|------|
| Variadic Function | 可变参数函数 |
| `va_list` | 可变参数列表类型 |
| `va_start` | 初始化可变参数列表 |
| `va_arg` | 提取下一个可变参数 |
| `va_end` | 清理可变参数列表 |
| `va_copy` | 复制可变参数列表 |
| Count Parameter | Count 参数（用数字控制参数数量） |
| Sentinel Value | 哨兵值（用特殊值标记结束） |
| Format String | 格式字符串（如 `"%d %s"`） |
| `vprintf` 家族 | 接受 `va_list` 的 printf 版本 |
| Format String Checking | 格式字符串编译期检查 |
| Undefined Behavior | 未定义行为（UB） |

## 延伸阅读

- [cppreference: Variadic Functions (C)](https://en.cppreference.com/w/c/variadic) — `<stdarg.h>` 的完整参考
- [cppreference: vprintf family](https://en.cppreference.com/w/c/io/vfprintf) — `vprintf` / `vfprintf` / `vsnprintf` 文档
- K&R《C 程序设计语言》第 7.3 章：变参函数的定义与实现
- [Beej's Guide to C: Variadic Functions](https://beej.us/guide/bgc/) — 简明的入门教程
- [GCC: Function Attributes — format](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html) — 格式字符串检查属性

## 继续学习

可变参数函数让你打破了"函数参数数量和类型必须固定"的限制——这是实现 `printf`、日志函数、格式化输出等通用工具的基础。下一章我们将学习**位运算与内存操作**，掌握位级操作和内存级函数。

- [上一章](./void_generic.md)：void* 泛型编程
- [下一章](./bit_ops.md)：位运算与内存操作
