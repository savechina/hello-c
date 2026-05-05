# 条件编译（Conditional Compilation）

> "一套代码，多套世界。" —— 在 C 语言中，我学会了用 `#ifdef` 写出适应不同平台的代码。

## 开篇故事

想象一位翻译。他面对讲中文的听众就用中文讲，面对讲英语的听众就用英语讲——同一个故事，不同语言。翻译不需要为每种语言写不同的演讲稿，他根据观众自动选择。

条件编译就是编译器的「翻译官」。同一份源代码，`#ifdef __APPLE__` 告诉编译器：如果你在为 macOS 编译，就保留这段代码；`#elif defined(__linux__)` 说：如果你在为 Linux 编译，换另一段。`#endif` 是结束标记。不是运行时切换，是**编译时**就已经选好了。

你的源代码是一本「多语言剧本」，编译器决定最终演出哪一版。

## 本章适合谁

- 想写出**跨平台** C 代码的开发者
- 对 `#ifndef`、`#define`、`#ifdef` 有基本了解，但不清楚实际应用场景

## 你会学到什么

1. `#ifdef`/`#ifndef`/`#elif`/`#else`/`#endif` 的完整用法
2. 平台检测宏（`__APPLE__`、`__linux__`、`__FreeBSD__`）
3. 功能测试宏（`_GNU_SOURCE`、`_POSIX_C_SOURCE`）
4. Debug vs Release 模式的条件编译
5. 常见的条件编译模式和陷阱

## 前置要求

完成"预处理器与宏"章节（了解 `#define` 机制）

## 第一个例子

```c
#ifdef __APPLE__
    printf("Hello from macOS!\n");
#elif defined(__linux__)
    printf("Hello from Linux!\n");
#else
    printf("Hello from an unknown platform!\n");
#endif
```

> [完整源码](https://github.com/[your-repo]/hello-c/blob/main/src/basic/conditional_comp_sample.c)

## 原理解析

条件编译是预处理器的核心功能之一。预处理器在**编译之前**扫描源码，根据条件决定保留哪些代码。

### #ifdef / #ifndef / #elif / #else / #endif

```c
#ifdef DEBUG
    printf("调试信息: x = %d\n", x);
#endif

#ifndef NDEBUG
    assert(x > 0);
#endif

#ifdef __APPLE__
    /* macOS 特有代码 */
#elif defined(__linux__)
    /* Linux 特有代码 */
#else
    /* 其他平台 */
#endif
```

```
条件编译决策树:

                    ┌──────────────┐
                    │  #ifdef X    │
                    └──────┬───────┘
                           │
          ┌────────────────┼────────────────┐
          │ X 已定义        │ X 未定义        │
          ▼                 ▼                 │
    ┌──────────┐     ┌──────────────┐        │
    │  代码段A  │     │  #elif Y     │        │
    └──────────┘     └──────┬───────┘        │
                   ┌────────┼────────┐       │
                   │ Y 已定义│ Y 未定义│       │
                   ▼        ▼        │       │
             ┌──────────┐ ┌──────┐   │       │
             │  代码段B  │ │ #else │   │       │
             └──────────┘ └──┬───┘   │       │
                             ▼       │       │
                       ┌──────────┐  │       │
                       │  代码段C  │  │       │
                       └──────────┘  │       │
                             │        │       │
                             └────────┴───────┘
                                        │
                                        ▼
                                  ┌──────────┐
                                  │  #endif  │
                                  └──────────┘

  未选中的代码在编译后完全不存在——不是在运行时跳过, 而是根本没被编译
```

### 平台检测宏

| 平台 | 宏 | 类型 |
|------|-----|------|
| macOS / Darwin | `__APPLE__` | 编译器定义 |
| Linux | `__linux__` | 编译器定义 |
| FreeBSD | `__FreeBSD__` | 编译器定义 |
| Solaris | `__sun` | 编译器定义 |
| Windows | `_WIN32` | 编译器定义 |

### 功能测试宏

```c
/* 在 #include 之前定义 */
#define _GNU_SOURCE
#include <string.h>  /* 现在可以用 strcasecmp() 了 */
```

### Debug vs Release

```c
#ifndef NDEBUG
    /* Debug 模式: assert 启用 */
    assert(ptr != NULL);
#else
    /* Release 模式: assert 被替换为空 */
#endif
```

编译 Release 时加上 `-DNDEBUG` 标志即可禁用所有 `assert()`。

## 常见错误

### ❌ 错误 1: `#ifdef` 没有匹配的 `#endif`

```c
#ifdef DEBUG
    printf("调试信息\n");
/* 忘记 #endif → 编译错误 */
```

**编译器报错:**
```
error: unterminated #ifdef
```

**✅ 修复:** 始终配对:
```c
#ifdef DEBUG
    printf("调试信息\n");
#endif
```

### ❌ 错误 2: 拼写错误的宏名导致静默失败

```c
#ifdef __APPLE__     /* 正确的 */
    mach_info();
#elif __LINUX__      /* 错误! 应该是 __linux__ (小写) */
    linux_info();    /* 永远不会执行 */
#endif
#endif

```

**✅ 修复:**
```c
#elif defined(__linux__)
    linux_info();
```

### ❌ 错误 3: 缺少 fallback（`#else` 中无 `#error`）

```c
#ifdef DEBUG
    verbose_log();
#elif defined(RELEASE)
    minimal_log();
#endif
/* 既不 DEBUG 也不 RELEASE 时，没有任何日志！*/
```

**✅ 修复:**
```c
#ifdef DEBUG
    verbose_log();
#elif defined(RELEASE)
    minimal_log();
#else
    #error "Unknown build type! Define DEBUG or RELEASE"
#endif
```

## 动手练习

### 🟢 入门: 平台检测

编写代码，在 macOS 上打印 "Darwin"，在 Linux 上打印 "Linux"，其他平台打印 "Other"。

<details><summary>点击查看答案</summary>

```c
#if defined(__APPLE__)
    printf("Darwin\n");
#elif defined(__linux__)
    printf("Linux\n");
#else
    printf("Other\n");
#endif
```

</details>

### 🟡 中级: Debug/Release 日志

实现一个日志宏，在 Debug 模式下打印文件名+行号+消息，在 Release 模式下只打印消息。

<details><summary>点击查看答案</summary>

```c
#ifndef NDEBUG
    #define LOG(msg) printf("[%s:%d] %s\n", __FILE__, __LINE__, msg)
#else
    #define LOG(msg) printf("%s\n", msg)
#endif
```

</details>

### 🔴 挑战: 多平台系统信息

编写一个函数，在 macOS 上显示 Mach kernel info，在 Linux 上显示 `/proc/version`，在未知平台显示 "unsupported"。使用条件编译处理不同的 `#include` 和不同的实现路径。

<details><summary>点击查看答案</summary>

```c
void show_system_info(void) {
#if defined(__APPLE__)
    /* macOS: 使用 sysctlbyname */
    char os_version[256];
    size_t len = sizeof(os_version);
    sysctlbyname("kern.osversion", os_version, &len, NULL, 0);
    printf("macOS version: %s\n", os_version);
#elif defined(__linux__)
    #include <sys/utsname.h>
    struct utsname info;
    uname(&info);
    printf("Linux kernel: %s\n", info.release);
#else
    printf("系统信息: 不支持的平台\n");
#endif
}
```

</details>

## 故障排查 (FAQ)

**Q: `#ifdef` 和 `#if defined()` 有什么区别？**

**A:** `#ifdef MACRO` 检查单个宏是否定义。`#if defined(MACRO)` 可以组合使用：`#if defined(A) && defined(B)`。对于复杂条件，推荐用 `#if defined()`.

**Q: `#pragma once` 和 include guards 哪个好？**

**A:** `#pragma once` 更简洁，但某些编译器可能不支持（虽然主流都支持）。推荐两者同时使用：

```c
#pragma once
#ifndef MY_HEADER_H
#define MY_HEADER_H
/* ... */
#endif
```

**Q: 条件编译会影响运行性能吗？**

**A:** 不会。条件编译在**编译前**处理，编译器只会看到被保留的代码。未匹配的分支在编译后的二进制中完全不存在。

## 知识扩展 (选学)

### Feature Test Macros: POSIX 标准

GNU/Linux 上的 `<features.h>` 头文件定义了整个 POSIX 标准的 API 可见性：

```c
/* C99 标准 */
#define _POSIX_C_SOURCE 200809L
#include <unistd.h>

/* GNU 扩展 */
#define _GNU_SOURCE
#include <string.h> /* strcasestr, asprintf 等 */
```

### 编译器预定义宏

C 编译器预定义了许多有用的宏:

- `__FILE__` — 当前文件名 (字符串)
- `__LINE__` — 当前行号 (整数)
- `__func__` — 当前函数名 (C99, 字符串)
- `__DATE__` — 编译日期 (字符串, "Mon DD YYYY")
- `__TIME__` — 编译时间 (字符串, "HH:MM:SS")

## 小结

**核心要点:**
1. `#ifdef`/`#ifndef`/`#elif`/`#else`/`#endif` 用于**编译时**条件选择
2. 平台检测宏（`__APPLE__`、`__linux__`）用于跨平台代码
3. `_GNU_SOURCE`、`_POSIX_C_SOURCE` 控制 API 可见性
4. **始终**在 `#else` 中使用 `#error` 覆盖未知情况

**关键术语:**
条件编译 → 预处理器根据条件决定保留哪些代码 → 不同于运行时的 if/else

## 术语表

| English | 中文 |
|---------|------|
| Conditional Compilation | 条件编译 |
| Preprocessor Directive | 预处理指令 |
| Platform Detection Macro | 平台检测宏 |
| Feature Test Macro | 功能测试宏 |
| Include Guard | 包含保护 |
| Fallback | 后备方案 |

## 延伸阅读

- [GNU CPP Manual: Conditionals](https://gcc.gnu.org/onlinedocs/cpp/Conditionals.html) — 官方条件编译文档
- [Predefined C/C++ Compiler Macros](https://sourceforge.net/p/predef/wiki/Compilers/) — 列出所有编译器预定义宏
- `sysinfo.c` in this project — 本项目中的跨平台系统信息完整示例

## 继续学习


← [位运算与内存操作](bit_ops.md) | [函数指针](function_pointers.md) →
