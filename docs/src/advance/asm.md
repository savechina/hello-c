# 内联汇编 (Inline Assembly)

> "内联汇编是你和 CPU 之间最后的直通电话——但电话线另一头没有编译器帮你兜底。"——我发现

---

## 开篇故事

写 C 的默认假设是：**编译器比你更懂怎么生成机器码**。99% 的情况下这个假设成立。但总有那么些时刻，C 语言够不着硬件：

- CPU 有一条指令，C 标准库没暴露（x86 的 `cpuid`、AArch64 的系统寄存器 `mrs`）
- 内核/驱动代码需要精确控制寄存器或内存屏障
- 你在写加密/压缩内核，需要字节序颠倒或位操作指令

这时的两条路：

- **Intrinsic（内建函数）**——编译器提供的「C 函数皮、单条指令芯」，如 `__cpuid()`。**优先选它**。
- **内联汇编（inline asm）**——在 C 代码里直接嵌汇编指令。GCC/Clang 扩展语法，功能最强、坑也最深。

本章教你看懂并写对 extended asm 的最小集合：**约束（constraints）、clobber 清单、volatile、架构条件编译**。

## 本章适合谁

- 读内核/驱动/加密库源码时被 `__asm__` 语法劝退过
- 想在 C 里访问某条架构指令，但不知道从哪下手
- 面试被问「inline asm 和 intrinsic 的区别」
- 已读 [volatile 章节](volatile.md)，想知道 asm 场景下 volatile 还有什么用

## 你会学到什么

1. **extended asm 完整语法**——`输出 : 输入 : clobber` 三段式
2. **约束速查表**——`=r` / `+r` / `r` / `m` / `i` / `memory` clobber
3. **两个架构分支**——x86_64 `addl`/`cpuid`，AArch64 `add`/`mrs`
4. **volatile 的真正职责**——防删除/防合并/防移动，而不是补 clobber
5. **「此平台暂无示例」兜底模式**——`#else` 用普通 C 保证到处能编译

## 前置要求

- 已掌握：条件编译 `#if`（见 [条件编译章节](../basic/conditional_comp.md)）
- 已读：[volatile 章节](volatile.md)（asm 的 volatile 是同一概念的另一面）
- 了解：汇编只是「寄存器 + 内存 + 指令」，不需要会写汇编

## ⚠️ 命名与可移植性警告

本章代码用 **`__asm__`** 而不是 `asm`：

```c
__asm__("nop");   /* ✅ 任何 -std 模式都可用 (双下划线是 GNU 保留名) */
asm("nop");       /* ❌ -std=c17 严格模式下, asm 关键字被 __STRICT_ANSI__ 隐藏 */
```

**MSVC 没有 x64 内联汇编**——Windows 上请用 `<intrin.h>` 的 intrinsic（如 `__cpuid`）。本章不覆盖 MSVC 路径。

## 语法解剖

```c
__asm__ (volatile) ("指令模板 %0 %1 ...")
                   : 输出操作数      /* "=r"(result) */
                   : 输入操作数      /* "r"(a), "r"(b) */
                   : clobber 清单    /* ~"memory", ~"cc" */
```

四块拼图：

| 部分 | 作用 | 编译器做什么 |
|------|------|-------------|
| 指令模板 | 真正要执行的汇编 | 把 `%0`/`%1` 替换成寄存器/内存名 |
| 输出操作数 | asm 写出的结果 | 挑寄存器/内存放结果，之后按 C 语义回收 |
| 输入操作数 | asm 读入的值 | 把 C 变量/常量预装进寄存器 |
| clobber 清单 | 输出之外的副作用 | 据此安排代码重排和寄存器分配 |

**核心思想**：你写指令，编译器管「数据怎么进出、寄存器够不够用」。这就是 extended asm 和老式裸 `__asm__` 块（所有寄存器、内存全手动）的本质区别。

## 约束速查表

| 写法 | 含义 | 典型用法 |
|------|------|----------|
| `"=r"(v)` | 输出，只写 | 计算结果放进通用寄存器 |
| `"+r"(v)` | 输出，读写 | 原地修改：`v` 以旧值进入、新值出来 |
| `"r"(v)` | 输入，只读 | 把变量值装进寄存器 |
| `"m"(v)` | 输入，内存操作数 | 指令直接对内存操作 |
| `"i"(n)` | 输入，立即数 | 整型常量表达式填进指令立即数字段 |
| `"0"(v)` | 匹配约束 | 输入复用第 0 号操作数所在的寄存器 |
| `~"memory"` | clobber | 声明改写了全部内存（编译器屏障） |
| `~"cc"` | clobber | 声明改写了条件码（x86 EFLAGS / ARM CPSR） |

完整示例见 `src/advance/asm_sample.c` 顶部的同款表格。

## 代码讲解

### Demo 1：汇编加法——约束的最小闭环

```c
static int asm_add(int a, int b) {
  int sum;
#if defined(__x86_64__) || defined(_M_X64)
  sum = a;
  __asm__("addl %1, %0" : "+r"(sum) : "r"(b));   /* AT&T: dst += src */
#elif defined(__aarch64__) || defined(__ARM_ARCH)
  __asm__("add %0, %1, %2" : "=r"(sum) : "r"(a), "r"(b));  /* 三操作数 */
#else
  sum = a + b;   /* 兜底: 普通 C 实现, 保证任何平台能编译 */
#endif
  return sum;
}
```

两个架构的指令形状不同：

- **x86_64**（AT&T 语法）：`addl %1, %0` 是二操作数，`%0 += %1`。`"+r"` 让 `sum` 带着初值 `a` 进寄存器、加完再出来。
- **AArch64**：`add %0, %1, %2` 是三操作数，`%0 = %1 + %2`。`"=r"(sum)` 是纯输出，两个输入各占一个寄存器。

你没写任何一个寄存器名——**约束让编译器自己挑**。这就是 extended asm 的价值：换个优化级别、换个编译器版本，寄存器分配变了，你的模板照样对。

### Demo 2：volatile——防优化的正确姿势

编译器眼里：

- 普通 `__asm__` = 可分析、可移动、可删除的代码
- `volatile __asm__` = 外部事件，一次都不能少、顺序也不能变

```c
/* ✅ 空模板 + memory clobber = 纯编译屏障 (不生成机器指令) */
__asm__ volatile("" ::: "memory");

/* ✅ 执行次数有外部意义 → volatile, -O2 下也保留 3 份 nop */
for (int i = 0; i < 3; i++)
  __asm__ volatile("nop");
```

何时必须 volatile：

1. **没有输出操作数**（如上面的屏障）——否则编译器认为「无事发生」直接删除
2. **执行次数有外部意义**（轮询、时序指令、MMIO）——否则可能被合并
3. **位置有外部意义**——否则可能被移出循环或重排

### Demo 3：架构专属指令——cpuid / mrs

内联汇编最常见的正当用途：**访问 C 标准库没有的架构指令**。

x86_64 分支用 `cpuid`（leaf 1）读 EDX，检查 SSE2 位（bit 26）：

```c
uint32_t eax, ebx, ecx, edx;
__asm__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "1"(1u), "3"(0u));
```

`"=a"`/`"=b"`/`"=c"`/`"=d"` 是**约束直接指定物理寄存器**的例子——`cpuid` 硬编码要求结果在 EAX/EBX/ECX/EDX，没得挑。注意：x86_64 ABI 强制要求 SSE2，这一位恒为 1；它在 32 位 x86 上才真正用来探测。

AArch64 分支用 `mrs` 读通用计时器：

```c
uint64_t v;
__asm__ volatile("mrs %0, cntvct_el0" : "=r"(v));
```

`cntvct_el0` 是 ARM 通用计时器的用户态可见计数器，macOS/iOS/多数 Linux 允许 EL0 直读（vDSO 的时间函数底层也用它）。这里必须 `volatile`——两次读之间没有数据依赖，不加 volatile 编译器有权把两次读合并成一次。

其它架构走 `#else` 兜底：

```c
#else
  /* 此平台暂无汇编示例, 使用普通 C 实现 */
  sum = a + b;
#endif
```

**「兜底分支永远存在」是本章的可移植性铁律**——任何平台 clone 下来都能编译，只是拿到普通 C 实现。

## 常见错误

### ❌ 错误 1：漏报 clobber

```c
__asm__("" : "+r"(x));              /* ❌ 模板若真的改了内存, 编译器不知道 */
```

**✅ 正确**：缺什么报什么：

```c
__asm__("" : "+r"(x) :: "memory");  /* 声明改写了全部内存 */
```

漏报 clobber = 编译器基于错误假设重排代码 = 难查的诡异 bug。这是内联汇编第一大错误来源。

### ❌ 错误 2：拿 volatile 当补丁修 clobber

```c
__asm__ volatile("" : "+r"(x));     /* ❌ volatile 不补 clobber! */
```

**✅ 正确**：volatile 和 clobber 是**两个正交的契约**：

| 契约 | 回答的问题 |
|------|-----------|
| `volatile` | 这条 asm 自身要不要保留在原地（不删/不并/不移） |
| `~"memory"` / `~"cc"` | 这条 asm 对周边环境（内存/条件码）的破坏面 |

volatile 只约束 asm 自己，漏报 clobber 时编译器照样可以把**其它代码**重排到你的 asm 旁边出错。

### ❌ 错误 3：用 `asm` 而不是 `__asm__`

```c
asm("nop");       /* ❌ -std=c17 严格模式编译失败 */
```

**✅ 正确**：`__asm__` 双下划线版在任何 `-std` 模式下都可用。

## 动手练习

### 🟢 入门：说出约束含义

下面每行约束各是什么意思？

```c
__asm__("..." : "=r"(out) : "r"(in1), "i"(42) : "memory");
```

<details><summary>点击查看答案</summary>

- `"=r"(out)`：结果写进某个通用寄存器，之后作为 C 变量 `out` 的值
- `"r"(in1)`：`in1` 的值装进某个通用寄存器
- `"i"(42)`：立即数 42 直接嵌进指令（不需要寄存器）
- `~"memory"`：这条 asm 改写了全部内存，编译器不许把内存读写跨过它重排

</details>

### 🟡 中级：给 asm_add 补一个减法

仿照 `asm_add` 写 `asm_sub(a, b)`，x86_64 用 `subl`，AArch64 用三操作数 `sub`。

<details><summary>点击查看思路</summary>

```c
#if defined(__x86_64__) || defined(_M_X64)
  sum = a;
  __asm__("subl %1, %0" : "+r"(sum) : "r"(b));
#elif defined(__aarch64__) || defined(__ARM_ARCH)
  __asm__("sub %0, %1, %2" : "=r"(sum) : "r"(a), "r"(b));
#else
  sum = a - b;
#endif
```

</details>

### 🔴 挑战：读懂 cpuid 的匹配约束

`cpuid` 例子里 `"1"(1u), "3"(0u)` 的数字 1 和 3 指的是什么？

<details><summary>点击查看答案</summary>

操作数编号是 0-based，跨输出和输入统一编号。输出占了 0/1/2/3（EAX/EBX/ECX/EDX），所以输入 `"1"` 匹配到第 1 号操作数（EBX 位置）、`"3"` 匹配到第 3 号（EDX 位置）——即：把 leaf 号 1 写进 EAX 所在位置（通过匹配复用），把 sub-leaf 0 写进 ECX。这是「输入绑定到指定输出寄存器」的高级用法，让 cpuid 的寄存器约定全部由约束表达，模板里一个寄存器名都不用写。

</details>

## 故障排查

**Q：模板里的 `%0` 编译报错「invalid operand」？**

A：检查占位符编号是否超出输出+输入操作数总数。`%n` 是 0-based，跨输出和输入统一编号。

**Q：AArch64 上 `add %0, %1, %2` 汇编报错宽度不匹配？**

A：int 是 32 位，GCC/Clang 会自动打印 `w0/w1/w2`（32 位寄存器名）。如果手写宽后缀 `%w0`，确保所有操作数都是 32 位。混用 64/32 位寄存器名会报错。

**Q：`cpuid` 在 x86_64 上编译报 EBX 被占用？**

A：x86_64 上 `"=b"` 约束由编译器自动处理 EBX 保存/恢复（32 位 PIC 才有 GOT 冲突问题）。本章只覆盖 x86_64，32 位 x86 需额外小心。

## 小结

- **extended asm 三段式**：`: 输出 : 输入 : clobber`，模板里 `%n` 引用操作数
- **约束速查**：`=r` 只写 / `+r` 读写 / `r` 只读 / `m` 内存 / `i` 立即数
- **clobber 是契约**：漏报 = 编译器错误重排 = 诡异 bug；修 clobber 靠 `~"memory"`/`~"cc"`
- **volatile 与 clobber 正交**：volatile 管 asm 自身（不删/不并/不移），clobber 管副作用面
- **`__asm__` 不是 `asm`**：严格 `-std` 模式下前者永远可用
- **优先用 intrinsic**：`__cpuid`/`sysctl` 等已覆盖大多数场景；asm 留给 intrinsic 够不着的指令
- **兜底分支永远存在**：`#else` 用普通 C，任何平台都能编译

## 术语表

| 英文 | 中文 |
|------|------|
| Inline Assembly | 内联汇编 |
| Extended asm | 扩展 asm（带操作数的 GCC/Clang 语法） |
| Constraint | 约束（操作数的寄存器/内存分配要求） |
| Clobber | 破坏清单（asm 的副作用声明） |
| Operand | 操作数（模板里 `%n` 引用的值） |
| Template | 指令模板（asm 的汇编指令文本） |
| Intrinsic | 内建函数（编译器提供的单指令 C 函数） |
| Earlyclobber | 提前破坏（输出在输入被读前就可能被写） |
| Memory barrier | 内存屏障（约束内存读写顺序） |
| Condition codes | 条件码（x86 EFLAGS / ARM CPSR） |

## 平台验证状态（诚实声明）

| 平台 | 状态 | 说明 |
|------|------|------|
| **macOS arm64** (Apple clang) | ✅ **已编译 + 已运行 + ASan 通过** | AArch64 分支实测 |
| Linux x86_64 (GCC) | ⚠️ **已编写，未在本机验证** | x86_64 分支按 GCC 文档人工编写 |
| Linux arm64 (GCC) | ⚠️ 已编写，未验证 | AArch64 分支与 macOS 同源，但 GCC 寄存器分配行为未实测 |
| Windows x64 (MSVC) | ❌ **未编写** | MSVC 无 x64 内联汇编，请用 `<intrin.h>` |
| 其它平台 | ✅ `#else` 兜底 | 普通 C 实现，保证能编译 |

验证命令（macOS arm64）：

```bash
make build    # 编译
make run      # 运行 advance 全流程（含本章）
make asan     # AddressSanitizer + UBSan
```

## 延伸阅读

- [GCC: Extended Asm](https://gcc.gnu.org/onlinedocs/gcc/Extended-Asm.html) — 官方权威文档
- [GCC: Constraints](https://gcc.gnu.org/onlinedocs/gcc/Constraints.html) — 约束完整列表
- [Clang Language Extensions: Inline Assembly](https://clang.llvm.org/docs/LanguageExtensions.html#inline-assembly) — Clang 侧说明
- [Intel SDM Volume 2: cpuid](https://www.intel.com/sdm) — x86 指令手册
- [ARM ARM: System Register Descriptions](https://developer.arm.com/documentation) — AArch64 系统寄存器

## 继续学习

内联汇编是「编译器管不到的最后一公里」。理解了约束和 clobber 的契约精神，再读 Linux 内核的 `arch/*/include/asm/` 头文件就不会被 `__asm__` 迷宫吓住——它们和本章 demo 是同一套骨架。

- [上一章](volatile.md)：volatile (易变限定符)
- [下一章](review.md)：阶段复习
