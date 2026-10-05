/**
 * @file volatile_sample.c
 * @brief volatile (易变限定符) — Advance tutorial chapter
 *
 * 内容:
 *   1. 误解先行: 「volatile 共享计数器」仍是 data race / UB (C11 §5.1.2.4)
 *   2. 正确用法: volatile sig_atomic_t 信号标志 (signal/raise, 标准 C)
 *   3. 内存映射 I/O (MMIO) 占位讲解 — 本平台无真实设备, 只讲不跑
 *   4. volatile 计数循环 vs 普通循环 — 不依赖优化器行为
 *
 * CERT: EXP32-C (volatile 误用于并发). 测试: test/advance/test_volatile_sample.c
 */

#include <signal.h>
#include <stdio.h>

#include "advance/volatile_sample.h"

/* ---------------------------------------------------------
   Demo 1: 误解先行 (Error-First) — volatile 不是线程安全的
   --------------------------------------------------------- */

/*
 * ❌ 反例 (Counter-example — 不要执行, 仅作讲解):
 *
 *     volatile int shared_counter = 0;
 *     // 线程 A 与线程 B 同时执行:
 *     shared_counter++;              // 仍是 read → modify → write 三步
 *
 * 为什么是 UB (未定义行为):
 *   C11 §5.1.2.4 (多线程执行与数据竞争): 两个执行线程无同步地访问同一
 *   标量对象、且至少一个是写操作 → data race → 行为未定义。
 *   volatile 不改变这个判定, 因为它只承诺一件事 —— 编译器必须每次
 *   真正访问内存 (re-read), 不得把值缓存进寄存器。它不提供:
 *     - 原子性 (atomicity): ++ 拆成的读/改/写仍可被其他线程打断;
 *     - 内存顺序 (ordering): 编译器与 CPU 都可对访问重排序;
 *     - 免受其他执行流干扰 (interference): 无任何保证。
 *   后两条尤其致命: 即便每次读都走内存, 硬件缓存与乱序执行照样让
 *   另一个核看到陈旧/交错的值。
 *
 * 正确做法: _Atomic / atomic_int + atomic_fetch_add —— 见同目录
 * atomic_types_sample.c 与文档 docs/src/advance/atomic-types.md。
 */

static void volatile_misconception_sample(void)
{
    printf("=== Demo 1: 误解先行 (Error-First) ===\n");
    printf("  误解: 「共享计数器加上 volatile 就线程安全了」\n");
    printf("  事实: data race 仍然是 UB — C11 §5.1.2.4 不看 volatile;\n");
    printf("        volatile 只保证编译器重新读内存, 不保证原子性/内存序/免干扰。\n");
    printf("  正确: atomic_int + atomic_fetch_add (见 atomic-types 章)。\n");
    printf("  反例与逐条论证见本文件顶部注释。\n\n");
}

/* ---------------------------------------------------------
   Demo 2: 正确用法 — volatile sig_atomic_t 信号标志 (标准 C)
   --------------------------------------------------------- */

/*
 * C11 §7.14.1.1: 信号处理器中唯一可安全访问的对象是
 * volatile sig_atomic_t —— 「sig_atomic_t」保证处理器与主程序间的
 * 单次访问是原子的;「volatile」保证主程序的轮询循环每次重新读内存,
 * 不会把标志缓存进寄存器 (否则循环可能永远看不到它)。
 */
static volatile sig_atomic_t g_signal_flag = 0;

static void volatile_signal_handler(int signo)
{
    (void)signo;
    g_signal_flag = 1; /* 处理器只做最小动作: 置标志 */
}

int volatile_demo_poll(void)
{
    g_signal_flag = 0;

    if (signal(SIGINT, volatile_signal_handler) == SIG_ERR) {
        return 0; /* 安装失败按「未收到信号」报告, 保持确定性 */
    }

    (void)raise(SIGINT); /* 标准 C: raise 返回前处理器已执行完毕 */

    /* 有界轮询: 上限 1000 次、不 sleep —— 绝不无限等待 */
    int polls = 0;
    while (g_signal_flag == 0 && polls < 1000) {
        polls++;
    }

    (void)signal(SIGINT, SIG_DFL); /* 恢复默认动作, 不污染调用者环境 */
    return (g_signal_flag != 0) ? 1 : 0;
}

static void volatile_signal_sample(void)
{
    printf("=== Demo 2: 正确用法 — 信号标志 (signal/raise) ===\n");
    printf("  模式: handler 置 volatile sig_atomic_t 标志, 主循环轮询它。\n");

    int got = volatile_demo_poll();
    printf("  轮询结果: flag = %d (%s)\n", got, got ? "已置位" : "未置位");
    printf("  这是标准 C 保证可移植的用法: 单一标志 + 轮询, 无锁无 UB。\n");
    printf("  注意: 处理器里只能做「赋值 sig_atomic_t」这类最小动作,\n");
    printf("        printf/malloc 等异步信号不安全函数一概禁止。\n\n");
}

/* ---------------------------------------------------------
   Demo 3: 内存映射 I/O (MMIO) — 占位讲解 (prose only, 不执行)
   --------------------------------------------------------- */

static void volatile_mmio_sample(void)
{
    printf("=== Demo 3: 内存映射 I/O (MMIO) — 占位讲解 ===\n");
    printf("  本平台没有可映射的真实外设寄存器, 此处只讲原理、不写代码。\n");
    printf("  原理: 外设寄存器被映射到固定地址后, 读写该地址有硬件副作用:\n");
    printf("    - 编译器不知道地址背后有硬件; 若缓存/合并/删除访问就错了;\n");
    printf("    - volatile 强制每次访问真正发出总线读写, 次数与顺序按源码。\n");
    printf("  嵌入式典型写法 (示意, 本仓库不执行):\n");
    printf("    *(volatile uint32_t *)0x40021000U = 0x01U;  /* 写状态寄存器 */\n");
    printf("  限制: volatile 只约束编译器; 多核之间的顺序仍需 memory barrier。\n\n");
}

/* ---------------------------------------------------------
   Demo 4: volatile 计数循环 vs 普通循环
   --------------------------------------------------------- */

static void volatile_counter_loop_sample(void)
{
    printf("=== Demo 4: volatile 计数循环 vs 普通循环 ===\n");

    volatile int vol_counter = 0;
    for (int i = 0; i < 5; i++) {
        vol_counter++; /* 语义约束: 每次迭代回内存读-改-写, 不可被省略 */
    }

    int plain_counter = 0;
    for (int i = 0; i < 5; i++) {
        plain_counter++; /* 普通变量: 优化器可整体折叠为一次 plain += 5 */
    }

    printf("  volatile 计数: %d\n", (int)vol_counter);
    printf("  普通计数:      %d\n", plain_counter);
    printf("  两者结果都正确 —— 差别只在生成的访存次数与指令数。\n");
    printf("  ⚠️ 不要依赖优化器行为: 普通循环是否被折叠是实现自由,\n");
    printf("     本例不断言两者指令数差异, 只演示 volatile 的语义约束。\n\n");
}

/* ---------------------------------------------------------
   入口函数
   --------------------------------------------------------- */

int main_volatile_sample(void)
{
    printf("========================================\n");
    printf("  volatile 易变限定符 (正确用法与危险误解)\n");
    printf("========================================\n\n");

    volatile_misconception_sample();
    volatile_signal_sample();
    volatile_mmio_sample();
    volatile_counter_loop_sample();

    printf("volatile 演示完毕.\n");
    return 0;
}
