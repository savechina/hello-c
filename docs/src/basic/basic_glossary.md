# C 术语表 (C Terminology Glossary)

> 本术语表收录 C 语言核心概念的双语对照解释，便于快速查阅。每个术语均标注了对应章节的链接。

---

## 数据类型 (Data Types)

### 1. **int (Integer)**
整数类型 — C 语言中最基本的数值类型，通常为 4 字节（32 位），表示范围为 `−2,147,483,648` 到 `2,147,483,647`。[详见数据类型章节](datatype.md)

### 2. **float (Single-Precision Floating-Point)**
单精度浮点数 — 遵循 IEEE 754 标准，占 4 字节，约提供 6-7 位有效十进制数字。适合存储小数，但存在精度损失。[详见数据类型章节](datatype.md)

### 3. **double (Double-Precision Floating-Point)**
双精度浮点数 — 遵循 IEEE 754 标准，占 8 字节，约提供 15-16 位有效十进制数字，精度高于 `float`。[详见数据类型章节](datatype.md)

### 4. **char (Character)**
字符类型 — 占 1 字节，存储 ASCII 码或小型整数。在 C 中 `char` 本质就是整数，字符字面量 `'A'` 实际存储为 65。[详见数据类型章节](datatype.md) | [详见字符串深度章节](strings.md)

### 5. **void**
空类型 — 表示"无类型"，常用于函数返回值（`void func()`）、通用指针（`void *`）或显式声明无参数（`int main(void)`）。[详见 void* 泛型编程章节](void_generic.md) | [详见函数章节](functions.md)

### 6. **enum (Enumeration)**
枚举类型 — 将一组命名整型常量集合，默认从 0 开始递增。用于提高代码可读性，如 `enum Color { RED, GREEN, BLUE }`。[详见枚举章节](enums.md)

### 7. **union (联合体)**
联合体 — 多个成员共享同一块内存空间，任一时刻只有一个成员有效。常用于内存优化或类型双关（type punning）。[详见位运算与内存操作章节](bit_ops.md)

### 8. **struct (Structure)**
结构体 — 将多个不同类型的数据聚合为一个复合类型。每个成员拥有独立的内存地址，是 C 语言实现"面向对象"的基础。[详见结构体章节](structs.md)

### 9. **typedef**
类型定义 — 关键字，为已有类型创建别名，如 `typedef struct { ... } Point;`。不创建新类型，仅提升可读性和可维护性。[详见结构体章节](structs.md) | [详见数据类型章节](datatype.md)

---

## 指针与内存 (Pointers & Memory)

### 10. **Pointer**
指针 — 存储变量内存地址的特殊变量。通过 `*` 解引用访问目标值，通过 `&` 获取变量地址。指针是 C 语言的核心能力，也是入门最大难关。[详见指针基础章节](pointers.md)

### 11. **NULL Pointer**
空指针 — 值为 0 的指针，表示"不指向任何有效内存"。解引用空指针会导致段错误（Segmentation Fault）。应始终在使用前检查指针是否为 NULL。[详见指针基础章节](pointers.md) | [详见内存管理章节](memory_mgmt.md)

### 12. **Dangling Pointer**
悬垂指针 — 指向已释放内存的指针。常发生在 `free(p)` 后未将 `p` 置为 NULL，后续误用导致未定义行为。释放后应立即 `p = NULL`。[详见内存管理章节](memory_mgmt.md)

### 13. **malloc (Memory Allocation)**
动态内存分配 — 在堆（heap）上分配指定字节数的内存块。返回 `void *`，需强制转换为目标类型指针。分配后必须调用 `free()` 释放。[详见内存管理章节](memory_mgmt.md)

### 14. **free (Memory Deallocation)**
释放内存 — 将 `malloc` / `calloc` / `realloc` 分配的堆内存归还给系统。对同一指针重复 `free` 或对栈变量 `free` 均会导致未定义行为。[详见内存管理章节](memory_mgmt.md)

### 15. **realloc (Re-Allocation)**
重新分配 — 调整已分配内存块的大小。可能原地扩展或分配新块并拷贝数据，返回新指针。原指针在成功调用后不应再使用。[详见内存管理章节](memory_mgmt.md)

### 16. **calloc (Contiguous Allocation)**
连续分配 — 与 `malloc` 类似，但接收 `(num, size)` 两参数，且会将分配的内存全部初始化为零。适合分配数组。[详见内存管理章节](memory_mgmt.md)

### 17. **Heap vs Stack**
堆 vs 栈 — 栈内存由编译器自动管理（函数参数、局部变量），生命周期随作用域结束；堆内存由程序员手动分配和释放，生命周期自定。栈速度快但有大小限制，堆灵活但需防泄漏。[详见内存管理章节](memory_mgmt.md) | [详见变量与表达式章节](variables.md)

### 18. **Memory Leak**
内存泄漏 — 分配的堆内存未释放且失去引用，导致程序运行的内存持续消耗。长期运行的服务中尤为致命。可用 `valgrind` 检测。[详见内存管理章节](memory_mgmt.md) | [详见调试与错误处理章节](debugging.md)

---

## 字符串与 I/O (Strings & I/O)

### 19. **Null-Terminated String**
空终止字符串 — C 语言中字符串的本质：以 `\0`（ASCII 码 0）结尾的 `char` 数组。所有标准库字符串函数都依赖此约定。缺少 `\0` 会导致缓冲区越界读取。[详见字符串深度章节](strings.md)

### 20. **FILE Pointer**
文件指针 — `FILE *` 类型，指向标准库维护的文件流控制结构体。由 `fopen()` 返回，用于后续的读写操作。[详见文件 I/O 章节](file_io.md)

### 21. **fopen (File Open)**
打开文件 — 标准库函数，以指定模式（如 `"r"`, `"w"`, `"a"`, `"rb"`）打开文件并返回 `FILE *`。失败返回 NULL，需检查。成功打开后务必调用 `fclose()` 关闭。[详见文件 I/O 章节](file_io.md)

### 22. **fclose (File Close)**
关闭文件 — 刷新缓冲区中的未写数据、释放与文件关联的资源。未关闭文件可能导致数据丢失（缓冲区未刷盘）。[详见文件 I/O 章节](file_io.md)

### 23. **Buffer (缓冲区)**
缓冲区 — 内存中暂存输入输出数据的区域。I/O 操作通常先写入缓冲区再批量刷盘，减少系统调用次数。可通过 `fflush()` 强制刷出。[详见文件 I/O 章节](file_io.md) | [详见字符串深度章节](strings.md)

### 24. **EOF (End of File)**
文件结束标志 — 宏定义，值为 −1，表示文件读取已到末尾。`fgetc()`、`fgets()` 等函数读到文件尾时返回 EOF。注意 EOF 不是文件中实际存在的字符。[详见文件 I/O 章节](file_io.md)

---

## 预处理与编译 (Preprocessing & Compilation)

### 25. **Preprocessor**
预处理器 — 编译前的文本处理阶段，处理以 `#` 开头的指令（`#include`、`#define`、`#ifdef` 等）。不进行语法检查，只做文本替换。[详见预处理器与宏章节](preprocessor.md)

### 26. **Include Guard**
头文件守卫 — 防止头文件被多次包含的预处理模式：`#ifndef MYHEADER_H` / `#define MYHEADER_H` / `#endif`。也可用 `#pragma once`（非标准但广泛支持）。[详见头文件与模块系统章节](headers.md)

### 27. **Macro (宏)**
宏 — 预处理器定义的文本替换规则。分为对象宏（`#define PI 3.14159`）和函数宏（`#define MAX(a,b) ((a)>(b)?(a):(b))`）。函数宏无类型检查，注意括号优先级陷阱。[详见预处理器与宏章节](preprocessor.md)

### 28. **Conditional Compilation**
条件编译 — 通过 `#if`、`#ifdef`、`#ifndef`、`#elif`、`#else`、`#endif` 控制代码段的编译 inclusion，常用于跨平台适配和调试代码开关。[详见条件编译章节](conditional_comp.md)

### 29. **Translation Unit (翻译单元)**
翻译单元 — 一个 `.c` 源文件经预处理器展开所有 `#include` 后产生的独立编译输入。编译器每次编译一个翻译单元。[详见头文件与模块系统章节](headers.md)

### 30. **Linkage (Static / Extern)**
链接属性 — 决定符号在不同翻译单元间的可见性。`extern`（默认）允许跨文件访问；`static` 限制符号仅在当前翻译单元内可见，用于封装。[详见头文件与模块系统章节](headers.md) | [详见作用域章节](scope.md)

---

## 函数与控制流 (Functions & Control Flow)

### 31. **Function Pointer (函数指针)**
函数指针 — 存储函数入口地址的指针变量，声明如 `int (*fp)(int, int)`。用于实现运行时选择逻辑、回调机制和简易多态。[详见函数指针章节](function_pointers.md)

### 32. **Callback (回调函数)**
回调函数 — 作为参数传递给另一个函数的指针，由被调用方在适当时机"回调"。标准库中的 `qsort()` 和 `bsearch()` 都接受回调作为比较函数。[详见回调函数与多态章节](callbacks.md)

### 33. **Void Pointer (void*)**
无类型指针 — 可指向任意类型对象的通用指针。需要显式类型转换后才能解引用或进行指针算术。广泛用于泛型代码（如 `malloc` 返回值、回调参数）。[详见 void* 泛型编程章节](void_generic.md) | [详见函数指针章节](function_pointers.md)

### 34. **Variadic Function (可变参数函数)**
可变参数函数 — 接受不确定数量参数的函数，参数列表以 `...` 结尾，依赖 `<stdarg.h>` 中的 `va_list`、`va_start`、`va_arg`、`va_end` 宏实现解析。`printf` 即此类函数。[详见函数章节](functions.md) | [详见日志与格式化输出章节](logging.md)

### 35. **Return Value Parameter (返回值参数 / out-parameter)**
通过指针参数实现"多返回值" — C 函数仅直接返回一个值，如需返回多个结果，通过传入指针参数在函数体内修改外部变量。如 `int scanf(const char *, ...)` 的参数均为 out-parameter。[详见指针基础章节](pointers.md) | [详见函数章节](functions.md)

---

## 错误与调试 (Errors & Debugging)

### 36. **errno**
错误码全局变量 — `<errno.h>` 定义的外部整型变量，标准库函数失败时写入错误编号（如 `EACCES`、`ENOENT`）。配合 `perror()` 或 `strerror()` 获取可读错误信息。[详见调试与错误处理章节](debugging.md)

### 37. **assert**
断言 — `<assert.h>` 提供的宏，在表达式为假时立即终止程序并打印文件名、行号和失败表达式。仅在 `NDEBUG` 未定义时生效，适合捕获不应发生的编程错误。[详见调试与错误处理章节](debugging.md)

### 38. **Undefined Behavior (UB, 未定义行为)**
未定义行为 — C 标准未规定后果的情况，编译器可生成任意代码。常见诱因：解引用野指针、有符号整数溢出、缓冲区越界、未返回非 void 函数值。UB 是最危险的 bug，程序可能平时正常、上线崩溃。[详见调试与错误处理章节](debugging.md)

### 39. **Valgrind**
Valgrind — Linux 上的内存调试和分析工具套件。`memcheck` 子工具能检测内存泄漏、越界读写、使用未初始化值等。macOS 可用 `clang -fsanitize=address` 替代。[详见调试与错误处理章节](debugging.md) | [详见内存管理章节](memory_mgmt.md)

### 40. **GDB (GNU Debugger)**
GNU 调试器 — 强大的命令行调试工具，支持设置断点、单步执行、查看内存、分析核心转储（core dump）。是排查段错误和逻辑错误的终极武器。[详见调试与错误处理章节](debugging.md)

---

## 标准库 (Standard Library)

### 41. **stdlib.h (Standard Library)**
标准库头文件 — 提供内存管理（`malloc`/`free`/`realloc`/`calloc`）、数值转换（`atoi`/`strtod`）、伪随机数（`rand`/`srand`）、退出控制（`exit`/`atexit`）等通用工具。[详见标准库章节](stdlib.md) | [详见内存管理章节](memory_mgmt.md)

### 42. **stdio.h (Standard I/O)**
标准输入输出头文件 — 提供格式化 I/O（`printf`/`scanf`）、文件操作（`fopen`/`fclose`/`fread`/`fwrite`）、字符 I/O（`getchar`/`putchar`）等函数。是 C 程序最常被引入的头文件。[详见文件 I/O 章节](file_io.md) | [详见日志与格式化输出章节](logging.md)

### 43. **string.h (String Library)**
字符串库头文件 — 提供字符串操作（`strlen`/`strcpy`/`strncpy`/`strcat`/`strcmp`/`strstr`）、内存操作（`memcpy`/`memmove`/`memset`/`memcmp`）等函数。注意 `strcpy`/`strcat` 不安全，优先使用 `strncpy`。[详见字符串深度章节](strings.md)

### 44. **math.h (Math Library)**
数学库头文件 — 提供数学函数（`sin`/`cos`/`sqrt`/`pow`/`fabs`/`ceil`/`floor` 等）。链接时需添加 `-lm` 标志（Linux）。浮点运算注意 `==` 比较精度问题。[详见数据类型章节](datatype.md)

### 45. **ctype.h (Character Type Library)**
字符类型库头文件 — 提供字符分类（`isalpha`/`isdigit`/`isspace`/`isupper`）和转换（`toupper`/`tolower`）函数。接受 `int` 参数并处理 `EOF`，返回非零/零表示真假。[详见字符串深度章节](strings.md)

### 46. **time.h (Time Library)**
时间库头文件 — 提供时间操作（`time()`）、时间格式（`struct tm`）、格式化输出（`strftime()`）和高精度计时（`clock()`）。配合 `difftime()` 可计算时间差。[详见调试与错误处理章节](debugging.md)

---

> 💡 **使用提示**：点击术语链接可直接跳转到对应的详细讲解章节。按类别浏览或在 IDE 中搜索关键词快速定位。
