# 错误处理（Error Handling）

> 「调试是系统地消除错误，而不是系统地证明自己没犯错。」 —— 我学完本章后的感悟

## 开篇故事

想象一家医院的急诊分诊系统（Triage System）。病人送来，护士先量血压、测体温（`errno` 检查），如果生命体征异常就启动应急预案（`perror` 快速报告），必要时转诊给专科医生（回调链依次处理），极端情况下直接叫救护车送 ICU（`setjmp/longjmp` 紧急跳转）。

C 语言的错误处理就是这套逻辑。C 没有 try/catch 这样的「异常魔法」——每一次函数调用都可能失败，你必须亲手检查每一个返回值、处理每一个错误码。这听起来很繁琐，但正是这种「繁琐」让你完全掌控每个错误场景：你知道哪一步出了问题、为什么出问题、该怎么处理。

本章带你从零建立 C 语言的错误处理体系。

## 本章适合谁

- 写 C 代码从不检查返回值的「乐观派」
- 被 `Segmentation fault` 折磨但不知道哪里错的人
- 听到 `errno`、`perror`、`setjmp` 这些词会觉得陌生的初学者
- 想建立可扩展错误处理系统的中级开发者

## 你会学到什么

- `errno` + `<errno.h>` 错误码系统
- `perror` / `strerror`——让错误信息可读
- `setjmp/longjmp` 非本地跳转——C 的"异常"机制
- 错误回调链（Callback Chain）——可扩展的错误处理管道

## 前置要求

- 能编译运行基本 C 程序
- 了解函数返回值的基本概念

## 第一个例子

让我们从一个最常见的 C 代码错误开始：没有检查返回值的代码。

```c
#include <stdio.h>

int main(void) {
    /* ❌ 危险的写法：假设 fopen 100% 成功 */
    FILE *fp = fopen("config.txt", "r");
    char buf[256];
    fgets(buf, sizeof(buf), fp);  /* ← 如果 fopen 失败，fp = NULL → fgets 崩 */
    printf("%s\n", buf);
    fclose(fp);
    return 0;
}
```

如果 `config.txt` 不存在，`fopen` 返回 `NULL`。然后你把 `NULL` 传给 `fgets`——**未定义行为**。程序可能立刻崩溃，也可能假装什么都没发生，最可怕的是：它**偶尔**工作。

### 修复：加上错误检查

```c
#include <stdio.h>
#include <errno.h>   /* errno 定义 */
#include <string.h>  /* strerror */

int main(void) {
    FILE *fp = fopen("config.txt", "r");

    if (fp == NULL) {
        /* 方式 1: perror 自动打印 errno */
        perror("fopen failed");
        /* 输出: fopen failed: No such file or directory */

        /* 方式 2: strerror 获取错误字符串 */
        printf("错误码 %d: %s\n", errno, strerror(errno));

        return 1;  /* 错误返回码 */
    }

    char buf[256];
    fgets(buf, sizeof(buf), fp);
    printf("%s\n", buf);
    fclose(fp);
    return 0;  /* 成功返回码 */
}
```

**核心思想**：每次可能失败的函数调用，都必须检查返回值。C 没有异常机制，错误信息就藏在返回值和 `errno` 里。

## 原理解析

### 1. `errno` — 线程局部错误码

`errno` 是 C 标准库定义的**线程局部变量**（thread-local variable），由库函数在出错时自动设置。

**工作原理**：

```c
#include <errno.h>
#include <math.h>

int main(void) {
    errno = 0;  /* ← 重要：使用前清零！ */

    double result = sqrt(-1.0);

    if (errno != 0) {
        printf("出错了! errno = %d\n", errno);
    }

    return 0;
}
```

**关键规则**：

| 规则 | 说明 |
|------|------|
| 使用前清零 | 成功时不会清零 errno，所以调用前必须设 `errno = 0` |
| 只在出错时设置 | 库函数成功时不修改 errno |
| 不保留历史 | 连续出错时，后面的错误会覆盖前面的 errno |
| 线程局部 | 多线程中每个线程有独立的 errno 副本 |

**常见 errno 值**（POSIX 标准）：

```
 1  EPERM      Operation not permitted        — 没有操作权限
 2  ENOENT     No such file or directory      — 文件/目录不存在
13  EACCES     Permission denied               — 权限拒绝
22  EINVAL     Invalid argument                — 无效参数
```

### 2. `perror` — 快速打印错误

```c
#include <stdio.h>

FILE *fp = fopen("missing.txt", "r");
if (fp == NULL) {
    perror("Error opening file");
    /* 输出: Error opening file: No such file or directory */
}
```

`perror` 是你手动拼接前缀字符串和 `errno` 对应的文本——调试时获取可读错误信息最快的方法。

**输出格式**：`前缀字符串: errno 对应的错误文本\n`

### 3. `strerror` — 获取错误字符串

```c
#include <string.h>
#include <errno.h>

printf("%s\n", strerror(errno));   // 当前错误
printf("%s\n", strerror(2));       // "No such file or directory"
printf("%s\n", strerror(13));      // "Permission denied"
```

`strerror` 返回一个 `char*` 指向静态字符串，你可以自由使用它（比如写入自定义日志、格式化输出）。

**perror vs strerror 选择**：

| 场景 | 推荐 | 原因 |
|------|------|------|
| 快速调试打印 | `perror` | 一行搞定，自带换行 |
| 自定义格式输出 | `strerror` | 返回字符串，可嵌入 printf |
| 日志文件写入 | `strerror` | 可以自己控制格式 |

### 4. `setjmp/longjmp` — 非本地跳转

C 没有 try/catch，但可以用 `setjmp`/`longjmp` 实现类似效果。

**类比**：`setjmp` 是「游戏存档点」，`longjmp` 是「读取存档」。`longjmp` 把程序状态恢复到 `setjmp` 保存的位置。

```c
#include <setjmp.h>

jmp_buf env;  /* 保存跳转环境的缓冲区 */

void deep_function(void) {
    /* 模拟：深层函数中检测到不可恢复错误 */
    printf("deep_function: 遇到严重错误!\n");
    longjmp(env, 1);  /* 跳回 setjmp 处，让 setjmp 返回 1 */
    /* 这行永远不会执行 */
}

int main(void) {
    int ret = setjmp(env);  /* 保存当前环境 */

    if (ret == 0) {
        /* 正常执行路径：setjmp 首次调用返回 0 */
        printf("正常路径：调用 deep_function\n");
        deep_function();
    } else {
        /* 错误恢复路径：longjmp 跳回，setjmp 返回 longjmp 的第二个参数 */
        printf("错误恢复：从 deep_function 跳回，ret = %d\n", ret);
    }

    printf("继续执行\n");
    return 0;
}
```

**输出**：
```
正常路径：调用 deep_function
deep_function: 遇到严重错误!
错误恢复：从 deep_function 跳回，ret = 1
继续执行
```

**关键理解**：

- `setjmp(env)`：保存当前调用环境到 `env`，**首次调用返回 0**
- `longjmp(env, val)`：恢复到 `env` 保存的环境，`setjmp` **重新返回 val**（非 0）
- `jmp_buf`：一个缓冲区，保存寄存器状态和栈指针

**多层调用示例**：

```c
jmp_buf env;

void layer_c(void) { longjmp(env, 2); }  /* 错误码 2 = layer_c 错误 */
void layer_b(void) { layer_c(); }
void layer_a(void) { layer_b(); }

int main(void) {
    int ret = setjmp(env);
    if (ret == 0) {
        layer_a();  /* 正常路径 */
    } else {
        printf("从深层函数跳回! 错误码 = %d\n", ret);
    }
    return 0;
}
```

这里 `layer_c` 直接跳回 `main`，跳过了 `layer_b` 和 `layer_a` 的返回。

**⚠️ 重要警告**：

- `longjmp` **跳过**中间栈帧的析构/清理代码——局部变量不会自动释放，内存可能泄漏
- 不要用它做**正常控制流**，只做**错误恢复**
- 跳回后，`setjmp` 和 `longjmp` 之间的局部变量值是**未定义的**（除非声明为 `volatile`）

### 5. 错误回调链（Error Callback Chains）

当错误发生时，你可能需要同时做几件事：记日志、通知用户、释放资源。回调链让你把这些操作注册成**管道**，错误发生时依次执行。

**类比**：就像医院的多级转诊——基层医生处理不了，转给专科医生，专科也处理不了，转给上级医院。每一级都有机会处理或继续传递。

```c
typedef void (*error_callback_fn)(int code, const char *msg, void *data);

/* 回调 1: 日志记录 */
void log_callback(int code, const char *msg, void *data) {
    fprintf(stderr, "[LOG] 错误 #%d: %s\n", code, msg);
}

/* 回调 2: 用户通知 */
void notify_callback(int code, const char *msg, void *data) {
    char *user = (char *)data;
    printf("[NOTIFY] 用户 %s: 错误 #%d — %s\n", user, code, msg);
}

/* 回调 3: 资源清理 */
void cleanup_callback(int code, const char *msg, void *data) {
    printf("[CLEANUP] 正在清理资源...\n");
}
```

注册和触发：

```c
register_error_callback(log_callback, NULL,   "log");
register_error_callback(notify_callback, (void*)"Alice", "notify");
register_error_callback(cleanup_callback, NULL,  "cleanup");

trigger_error_chain(42, "配置文件解析失败");
```

**输出**：
```
=== 触发错误回调链 (code=42, msg="配置文件解析失败") ===
→ 调用: log
[LOG] 错误 #42: 配置文件解析失败
→ 调用: notify
[NOTIFY] 用户 Alice: 错误 #42 — 配置文件解析失败
→ 调用: cleanup
[CLEANUP] 正在清理资源...
```

**优势**：
- **解耦**：业务代码只管 `trigger_error_chain`，不关心谁在监听
- **可扩展**：新模块注册新回调即可，不需要修改已有代码
- **灵活**：每个回调可以带自己的 `user_data`

## 常见错误

### ❌ 错误 1：不检查函数返回值

```c
FILE *fp = fopen("important.txt", "r");  // ❌ 假设立刻成功
fgets(buf, 100, fp);                      // ❌ fp = NULL → fgets 崩溃!
```

✅ **修复**：永远检查可能失败的函数。

```c
FILE *fp = fopen("important.txt", "r");
if (fp == NULL) {
    perror("fopen failed");
    return -1;
}
```

### ❌ 错误 2：忘了清零 errno

```c
// ❌ 没清零 errno
double r = sqrt(4.0);           // 成功
if (errno != 0) {               // 如果之前有错误残留 → 误判!
    printf("Error!\n");
}
```

✅ **修复**：调用前清零。

```c
errno = 0;
double r = sqrt(4.0);
if (errno != 0) {
    perror("sqrt failed");
}
```

### ❌ 错误 3：用 assert 处理运行时错误

```c
int read_input(int *value) {
    if (*value < 0) {
        assert(*value >= 0);    // ❌ 发布版 assert 被关闭，检查就消失了!
    }
}
```

✅ **修复**：assert 只查编程错误，运行时错误用 `if + return`。

```c
if (*value < 0) {
    errno = EINVAL;
    return -1;  // 发布版也有效
}
```

### ❌ 错误 4：setjmp 后使用非 volatile 局部变量

```c
int main(void) {
    int x = 10;           // ❌ 非 volatile
    if (setjmp(env) == 0) {
        x = 20;
        longjmp(env, 1);
    }
    printf("x = %d\n", x);  // ❌ x 的值是未定义的!
}
```

✅ **修复**：需要跨 longjmp 保留值的变量，声明为 `volatile`。

```c
volatile int x = 10;  // ✅ volatile
```

## 动手练习

### 🟢 练习 1：检查 malloc 失败

```c
/* 分配 1GB 内存（大概率失败），用 perror 打印错误
   然后用 NULL 检查安全处理 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    size_t huge = (size_t)1024 * 1024 * 1024 * 1024;
    void *ptr = malloc(huge);

    if (ptr == NULL) {
        perror("malloc huge memory");
        return 1;
    }

    free(ptr);
    return 0;
}
```
</details>

### 🟡 练习 2：实现带错误码的除法函数

```c
/* 实现 int safe_div(int a, int b, int *result)
   - b == 0 → errno = EINVAL, 返回 -1
   - result == NULL → errno = EINVAL, 返回 -1
   - 成功 → 返回 0
   测试所有分支 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <errno.h>

int safe_div(int a, int b, int *result) {
    if (result == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (b == 0) {
        errno = EINVAL;
        return -1;
    }
    *result = a / b;
    return 0;
}

int main(void) {
    int r;
    if (safe_div(10, 3, &r) == 0) {
        printf("10/3 = %d\n", r);
    }
    if (safe_div(10, 0, &r) != 0) {
        fprintf(stderr, "除以零: %s\n", strerror(errno));
    }
    return 0;
}
```
</details>

### 🔴 练习 3：用 longjmp 实现错误恢复

```c
/* 写一个三层函数调用：layer_a → layer_b → layer_c
   layer_c 中用 longjmp 跳回 main
   用不同的错误码（1, 2, 3）区分错误来源 */
```

<details>
<summary>点击查看答案</summary>

```c
#include <stdio.h>
#include <setjmp.h>

jmp_buf env;

void layer_c(void) {
    printf("layer_c: 错误! longjmp(3)\n");
    longjmp(env, 3);  /* 错误码 3 表示来自 layer_c */
}

void layer_b(void) {
    printf("layer_b: 调用 layer_c\n");
    layer_c();
}

void layer_a(void) {
    printf("layer_a: 调用 layer_b\n");
    layer_b();
}

int main(void) {
    int ret = setjmp(env);
    if (ret == 0) {
        printf("正常路径: 调用 layer_a\n");
        layer_a();
    } else {
        printf("错误回跳! 错误码 = %d\n", ret);
    }
    printf("继续执行\n");
    return 0;
}
```
</details>

## 故障排查

### Q: 「Segmentation fault (core dumped)」是什么？

访问了不属于自己的内存。常见原因：
- 解引用 NULL 指针
- 使用已 `free()` 的内存
- 数组越界
- 栈溢出（无限递归）

### Q: errno 和返回值同时检查会冲突吗？

不会。典型模式：

```c
errno = 0;
long result = strtol("abc", NULL, 10);
if (result == 0 && errno != 0) {
    // 出错了，errno 告诉你为什么
    perror("strtol failed");
}
```

### Q: setjmp/longjmp 和线程安全吗？

**是的**，`jmp_buf` 保存的是当前线程的栈状态。但注意：**不能跨线程 longjmp**——只能在自己线程内 setjmp 然后在自己线程内 longjmp。

### Q: Callback chain 中某个回调崩溃了怎么办？

实际项目中，可以在 `trigger_error_chain` 外面加一层 protection：

```c
// 记录错误但不让单个回调崩溃
for (int i = 0; i < handler.count; i++) {
    // 可以考虑: 每个回调在独立错误处理中执行
    handler.callbacks[i].fn(code, msg, handler.callbacks[i].user_data);
}
```

## 知识扩展

### AddressSanitizer（ASan）

GCC/Clang 内置的内存错误检测工具：

```bash
gcc -fsanitize=address -g main.c -o main
./main
# 自动检测: 越界、use-after-free、栈溢出等
```

### Valgrind

运行时内存错误检测工具（更强大）：

```bash
gcc -g main.c -o main
valgrind ./main
# 报告: 内存泄漏、未初始化变量、越界等
```

### 错误处理模式的演进

| 模式 | 何时用 | 示例 |
|------|--------|------|
| 返回值 + errno | 库函数、系统调用 | `fopen`, `sqrt` |
| 返回状态码 | 自定义函数 | `safe_div` 返回 0/-1 |
| setjmp/longjmp | 深层错误恢复 | 多层解析器报错 |
| 回调链 | 多模块错误通知 | 应用级错误管道 |

## 小结

祝贺！你已经掌握了 C 语言的错误处理体系。让我总结一下——

- **`errno`**：库函数的线程局部错误码，使用前需清零
- **`perror`**：快速打印错误信息（`前缀: 错误文本`）
- **`strerror`**：获取错误码对应的字符串，可嵌入任何格式输出
- **`setjmp/longjmp`**：C 的"异常"机制——非本地跳转到 `setjmp` 存档点，适合深层错误恢复
- **错误回调链**：注册→触发管道，解耦 + 可扩展

> **我的理解**：C 的错误处理哲学是「检查每一个返回值」——没有异常机制，没有 try/catch。每次函数调用都可能失败，你的代码必须检查。这很繁琐，但它让你完全掌控每个错误场景。学会这套体系后，你写的 C 代码会比 90% 的 C 初学者更健壮。

## 术语表

| 术语（中 → 英） | 说明 |
|-----------------|------|
| errno | C 库函数的线程局部错误码 |
| perror | 打印 errno 对应的错误信息 |
| strerror | 返回错误码对应的字符串 |
| setjmp | 保存当前调用环境（「存档」） |
| longjmp | 恢复到 setjmp 保存的环境（「读档」） |
| jmp_buf | 保存跳转环境的缓冲区类型 |
| 非本地跳转 | Non-local jump — 跨函数跳转 |
| 回调链 | Callback chain — 依次调用的回调管道 |
| Segmentation fault | 非法内存访问导致的崩溃 |
| 线程局部变量 | Thread-local variable — 每个线程独立副本 |

## 延伸阅读

- [cppreference: Error Handling (C)](https://en.cppreference.com/w/c/error)
- [cppreference: setjmp / longjmp](https://en.cppreference.com/w/c/program/setjmp)
- [POSIX errno constants](https://www.gnu.org/software/libc/manual/html_node/Error-Codes.html)
- K&R《C 程序设计语言》第 7.4 章：错误处理

## 继续学习

你已经掌握了 C 语言的错误处理核心工具。下一章我们将学习**并发编程**（多线程、原子操作、互斥锁），错误处理在并发场景下变得更加重要——多线程中的 `errno` 是线程局部的、`setjmp/longjmp` 不能跨线程使用。

> 💡 **提示**：在你现有代码中搜索所有没有检查返回值的 `malloc`/`fopen`/`strtol` 调用，加上 `NULL` 检查。你会立刻消灭一批潜在的崩溃点。

[← 上一章：(待添加)]() | [下一章：并发编程 (待添加) →]()
