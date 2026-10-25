/**
 * @file asm_sample.c
 * @brief 内联汇编 (Inline Assembly) —— GCC/Clang 扩展 `__asm__` 语法
 *
 * ⚠️ 命名说明: 严格 `-std=c17` 会定义 __STRICT_ANSI__, 关键字 `asm`
 *    (GNU 扩展) 被隐藏, 必须写 `__asm__` —— 它在任何模式下都可用。
 *
 * ⚠️ Windows/MSVC: x64 MSVC 没有内联汇编 (只在 32 位 x86 支持 __asm),
 *    请改用编译器 intrinsics (如 <intrin.h> 的 __cpuid)。本文件未编写
 *    MSVC 路径, 不保证在 MSVC 下编译。
 *
 * ============================================================
 * 约束速查表 (Constraint Cheat-Sheet)
 * ============================================================
 *
 * | 写法        | 含义                          | 示例                        |
 * |-------------|-------------------------------|-----------------------------|
 * | "=r"(v)     | 输出: v 只写入 (write-only)   | 结果放入某个通用寄存器      |
 * | "+r"(v)     | 读写: v 进入也离开 asm        | 原地修改: v += x            |
 * | "r"(v)      | 输入: v 只读                  | 常量/变量值塞进寄存器        |
 * | "m"(v)      | 输入: v 是内存操作数          | 直接对内存做运算            |
 * | "=m"(v)     | 输出: 写入内存                | asm 结果写回内存            |
 * | "i"(n)      | 输入: 立即数 (整型常量表达式) | 机器指令的立即数字段        |
 * | "0"(v)      | 匹配约束: 绑定到第 0 个操作数 | 输入复用输出所在的寄存器    |
 * | "=&r"(v)    | earlyclobber: 输出在所有输入  | 输出寄存器与输入寄存器不共用 |
 * |             | 被读取前就可能被写入          |                             |
 * | ~"memory"   | clobber: 声明改写了所有内存   | 编译器重排的屏障            |
 * | ~"cc"       | clobber: 改写了条件码寄存器   | x86 EFLAGS / ARM CPSR      |
 *
 * clobber (破坏清单) 是和编译器的「契约」: 告诉它这段汇编除了声明的
 * 输出之外还动了什么。漏报 clobber = 编译器基于错误假设重排代码 = 难查的
 * 诡异 bug。这是内联汇编第一大错误来源。
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "advance/asm_sample.h"

/* ============================================================
 * Demo 1: 基础 —— 用汇编做加法 (约束的最小闭环)
 * ============================================================
 *
 * 同一个 C 函数, 两套汇编模板按 CPU 架构切换:
 *   - x86_64:  addl %1, %0   (AT&T 语法: 源在前, 目的在后)
 *   - AArch64: add %0, %1, %2 (三操作数: 目的 = 源1 + 源2)
 */

/* 用汇编把 a + b 加起来。约束告诉编译器:
 *   x86_64  : "+r"(sum) 输入输出同一个寄存器, "r"(b) 只读寄存器
 *   AArch64 : "=r"(sum) 只写输出, "r"(a)/"r"(b) 只读输入 */
static int asm_add(int a, int b) {
  int sum;
#if defined(__x86_64__) || defined(_M_X64)
  /* AT&T 语法: addl src, dst → dst = dst + src。
   * "+r" 让 sum 以初始值 a 进入寄存器, 加完再出来。 */
  sum = a;
  __asm__("addl %1, %0" : "+r"(sum) : "r"(b));
#elif defined(__aarch64__) || defined(__ARM_ARCH)
  /* AArch64 的 add 是三操作数: dst, src1, src2。
   * "=r"(sum) 是纯输出; 两个输入各占一个寄存器。
   * %w0/%w1/%w2 的 "w" 后缀强制打印 32 位寄存器名 (w0/w1/w2):
   * int 是 32 位, 裸 %0 会被 clang 以 64 位 (x0) 展开 → 宽度不匹配报错。 */
  __asm__("add %w0, %w1, %w2" : "=r"(sum) : "r"(a), "r"(b));
#else
  /* 兜底: 此平台暂无汇编示例, 使用普通 C 实现 —— 保证任何平台都能编译。 */
  (void)a;
  (void)b;
  sum = a + b;
#endif
  return sum;
}

static void asm_basic_sample(void) {
  printf("--- 1. 基础: 汇编加法 + 约束 ---\n");
  printf("  __asm__ 完整形态: __asm__ (volatile) (\"模板\") : 输出 : 输入 : clobber)\n");
  printf("  模板里 %%0/%%1/%%n 按序对应输出+输入操作数, 编译器自动填寄存器名\n\n");

  int s1 = asm_add(2, 40);
  int s2 = asm_add(-7, 7);
  printf("  asm_add(2, 40)  = %d (期望 42)\n", s1);
  printf("  asm_add(-7, 7)  = %d (期望 0)\n", s2);

  printf("\n  关键理解: 你没写任何寄存器名! 约束 \"%s\" 让编译器自己挑一个\n",
         "r");
  printf("  通用寄存器并生成正确的移动指令 —— 这就是 extended asm 和裸\n");
  printf("  __asm__ 块的本质区别: 操作数位置的 %%0/%%1 由编译器管理,\n");
  printf("  函数调用约定 (哪些寄存器可随便用、哪些必须保存) 也由它兜底。\n\n");
}

/* ============================================================
 * Demo 2: volatile 限定符 —— 防优化的正确姿势与经典陷阱
 * ============================================================
 *
 * 编译器眼里, 普通 __asm__ 是「一段可分析、可移动、可删除的代码」。
 * volatile __asm__ 是「外部事件, 一次都不能少、顺序也不能变」。
 *
 * 何时必须 volatile:
 *   1. 没有输出操作数 (如内存屏障) —— 否则编译器认为「无事发生」直接删除
 *   2. 执行次数有外部意义 (轮询、时序敏感指令、MMIO) —— 否则可能被合并
 *   3. 位置有外部意义 —— 否则可能被移出循环或重排
 *
 * 经典陷阱 (见下方 ❌ 注释): 把 volatile 当成「修 clobber 漏报」的膏药。
 * volatile 只约束「这条 asm 自身」的保留与顺序, 不会替你声明 clobber!
 * 漏报 clobber 时编译器照样可以把【其它代码】重排到你的 asm 旁边出错。
 * 修 clobber 靠 ~"memory"/~"cc", 不靠 volatile。
 */

static void asm_volatile_sample(void) {
  printf("--- 2. volatile: 防优化与经典陷阱 ---\n");

  /* ✅ 正确用法 A: 空模板 + memory clobber = 纯编译屏障。
   * 这条 asm 不生成任何机器指令, 但 volatile 保证它不被删除,
   * ~"memory" 保证屏障两侧的内存读写不跨过它重排。
   * 单线程下也能观测到效果: 见下面的演示变量。 */
  int before = 1, after = 2;
  int snapshot;
  __asm__ volatile("" ::: "memory");
  snapshot = before + after;
  printf("  [屏障] memory clobber 后读取 snapshot = %d\n", snapshot);

  /* ✅ 正确用法 B: 执行次数有外部意义 → volatile。
   * 循环体里的 nop 若不加 volatile, 编译器有权整个删除或把 3 次
   * 合并成 1 次 (它认为 nop 没有任何可观察效果)。加了 volatile,
   * 「每次迭代都执行」成为契约, -O2 下也必须保留 3 份。 */
  int runs = 0;
  for (int i = 0; i < 3; i++) {
    __asm__ volatile("nop");
    runs++;
  }
  printf("  [循环] volatile nop 保留 3 份, C 侧计数 runs = %d\n", runs);

  /* ❌ 经典陷阱 (仅注释, 不要这样写):
   *
   *   int x = 0;
   *   __asm__("" : "+r"(x));        // 漏报: 若模板真的改了内存/条件码,
   *                                  // 却没有 ~"memory"/~"cc" clobber
   *   __asm__ volatile("" : "+r"(x)); // 加 volatile 也救不了 —— 它不补 clobber!
   *
   * 正确姿势: 缺什么 clobber 就报什么 (~"memory" / ~"cc"),
   * volatile 只回答「这条 asm 本身要不要保留在原地」。两个正交, 别混用。 */

  printf("\n  ⚠️  volatile vs clobber 是两个正交的契约:\n");
  printf("     volatile → 这条 asm 自身: 不删、不并、不移\n");
  printf("     clobber  → 这条 asm 对周边环境 (内存/条件码) 的破坏面\n\n");
}

/* ============================================================
 * Demo 3: 架构专属指令 —— x86 cpuid / AArch64 mrs (条件编译)
 * ============================================================
 *
 * 内联汇编最常见的正当用途: 访问 C 标准库没有的【架构指令】。
 * 这些指令通常有现成 intrinsic (__get_cpuid / sysctl), 生产代码优先
 * 用 intrinsic; 这里直接演示 asm 版本, 为了看清约束怎么配合多输出。
 */

#if defined(__x86_64__) || defined(_M_X64)

/* cpuid: 输入 EAX=leaf (这里 leaf 1), 输出 EAX/EBX/ECX/EDX 四个寄存器。
 * "=a/=b/=c/=d" 把四个输出分别绑到 EAX/EBX/ECX/EDX 四个寄存器 ——
 * 这是「约束直接指定物理寄存器」的例子 (cpuid 硬编码要求这四个)。
 * "1"(1) / "3"(0): 输入复用第 1/第 3 个操作数位置 (0-based)。 */
static uint32_t asm_cpuid_leaf1_edx(void) {
  uint32_t eax, ebx, ecx, edx;
  __asm__("cpuid"
          : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
          : "1"(1u), "3"(0u));
  (void)ebx; /* 本 demo 只用 EDX, EBX/EAX/ECX 仅占位输出 */
  (void)ecx;
  return edx;
}

static void asm_arch_sample(void) {
  printf("--- 3. 架构专属: cpuid leaf 1 (x86_64) ---\n");

  uint32_t edx = asm_cpuid_leaf1_edx();
  /* EDX bit 26 = SSE2。x86_64 架构强制要求 SSE2 (AMD64 ISA 的一部分),
   * 所以此位在 x86_64 上恒为 1 —— 在 32 位 x86 上才需要真正探测。 */
  const int sse2 = (edx >> 26) & 1u;
  printf("  cpuid(leaf=1) EDX = 0x%08" PRIX32 "\n", edx);
  printf("  SSE2 (EDX bit 26) = %d (%s)\n", sse2, sse2 ? "支持" : "不支持");
  printf("  提示: x86_64 ABI 强制 SSE2, 这一位恒为 1;\n");
  printf("        在 32 位 x86 上它才真正用来探测 CPU 能力。\n\n");
}

#elif defined(__aarch64__) || defined(__ARM_ARCH)

/* mrs: 读系统寄存器。cntvct_el0 是 ARM 通用计时器的用户态可见计数器
 * (CNTPCT_EL0)。输出是 64 位, 用 "=r" 绑一个通用寄存器 (Xn)。
 * macOS/iOS/多数 Linux 发行版允许 EL0 直读它 (vDSO 的时间函数底层也用它)。 */
static uint64_t asm_read_cntvct(void) {
  uint64_t v;
  __asm__ volatile("mrs %0, cntvct_el0" : "=r"(v));
  return v;
}

static void asm_arch_sample(void) {
  printf("--- 3. 架构专属: cntvct_el0 通用计时器 (AArch64) ---\n");

  uint64_t t1 = asm_read_cntvct();
  /* 干一点活让计时器往前走 (纯 C 循环, 供对照)。
   * 用 uint64_t 累加: int 累加到 10 万量级会 signed overflow (UB)! */
  volatile uint64_t sink = 0;
  for (int i = 0; i < 100000; i++) {
    sink += (uint64_t)i;
  }
  uint64_t t2 = asm_read_cntvct();
  (void)sink;

  printf("  mrs cntvct_el0: t1 = %" PRIu64 "\n", t1);
  printf("  mrs cntvct_el0: t2 = %" PRIu64 "\n", t2);
  printf("  Δ = %" PRIu64 " ticks (%s)\n", t2 - t1,
         t2 > t1 ? "计时器单调递增 ✓" : "相等 (间隔太短, 计时器未走一格)");
  printf("  注意: 这里用了 volatile —— mrs 无输出依赖, 不加 volatile\n");
  printf("        编译器有权把两次读合并成一次 (见 Demo 2)。\n\n");
}

#else

static void asm_arch_sample(void) {
  printf("--- 3. 架构专属指令 ---\n");
  printf("  此平台暂无汇编示例, 使用普通 C 实现。\n");
  printf("  (本章只覆盖 x86_64 与 AArch64; 其它架构的 CPU 特性探测\n");
  printf("   请查对应 ISA 手册, 或用操作系统提供的 intrinsic/sysctl。)\n\n");
}

#endif

/* ============================================================
 * Coordinator
 * ============================================================ */

int main_asm_sample(void) {
  printf("========================================\n");
  printf("  内联汇编 (Inline Assembly)\n");
  printf("========================================\n\n");

  printf("  GCC/Clang 扩展 __asm__ (extended asm):\n");
  printf("    __asm__ (volatile) (\"指令模板\") : 输出操作数 : 输入操作数 : clobber)\n");
  printf("    与编译器对话的三张嘴: 约束 (constraints) 管数据流,\n");
  printf("    操作数占位符 (%%0,%%1,...) 管位置, clobber 清单管副作用。\n\n");

  asm_basic_sample();
  asm_volatile_sample();
  asm_arch_sample();

  printf("========================================\n");
  printf("  内联汇编演示完毕。\n");
  printf("========================================\n\n");
  return 0;
}
