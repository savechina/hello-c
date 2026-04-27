#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>
#include "advance/atomic_types_sample.h"

/*
 * atomic-types 演示 (Atomic Types with <stdatomic.h>)
 *
 * 🟡 中等难度 — C11 原子类型、内存顺序、volatile vs atomic
 */

/* ---------------------------------------------------------
   Demo 1: 非原子操作导致竞态条件 (Error-First: 先展示错误)
   --------------------------------------------------------- */

static int g_nonatomic_counter = 0;

static void *nonatomic_inc_thread(void *arg)
{
    (void)arg;
    for (int i = 0; i < 100000; i++) {
        g_nonatomic_counter++;
    }
    return NULL;
}

static void demo_race_condition(void)
{
    printf("=== Demo 1: 竞态条件 (Race Condition) ===\n");

    g_nonatomic_counter = 0;
    pthread_t t1, t2;
    pthread_create(&t1, NULL, nonatomic_inc_thread, NULL);
    pthread_create(&t2, NULL, nonatomic_inc_thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("  期望: 200000, 实际: %d\n", g_nonatomic_counter);
    printf("  如果实际值 < 200000, 说明发生了竞态条件 (丢失更新)\n");

    printf("\n  竞态原理 (两个线程同时读同一个值):\n");
    printf("    线程 A: 读 counter=5\n");
    printf("    线程 B: 读 counter=5  ← 在 A 写回之前!\n");
    printf("    线程 A: 写 counter=6\n");
    printf("    线程 B: 写 counter=6  ← 覆盖了 A 的结果! 应该得到 7\n");
    printf("  这就是 lost-update 问题。\n\n");
}

/* ---------------------------------------------------------
   Demo 2: atomic_int 基础用法
   --------------------------------------------------------- */

static void demo_atomic_int_basic(void)
{
    printf("=== Demo 2: atomic_int 基础 ===\n");

    atomic_int atomic_counter;
    atomic_init(&atomic_counter, 0);

    int32_t val = atomic_load(&atomic_counter);
    printf("  atomic_load = %" PRId32 "\n", val);

    atomic_store(&atomic_counter, 42);
    printf("  atomic_store(42) → reload = %" PRId32 "\n",
           (int32_t)atomic_load(&atomic_counter));

    int32_t old = atomic_fetch_add(&atomic_counter, 8);
    printf("  atomic_fetch_add(8): old=%" PRId32 ", new=%" PRId32 "\n",
           old, (int32_t)atomic_load(&atomic_counter));

    old = atomic_exchange(&atomic_counter, 99);
    printf("  atomic_exchange(99): old=%" PRId32 ", new=%" PRId32 "\n",
           old, (int32_t)atomic_load(&atomic_counter));

    atomic_fetch_add(&atomic_counter, 1);
    printf("  atomic_fetch_add(1) → %" PRId32 "\n",
           (int32_t)atomic_load(&atomic_counter));

    printf("\n");
}

/* ---------------------------------------------------------
   Demo 3: 用 atomic_int 修复竞态条件
   --------------------------------------------------------- */

static atomic_int g_atomic_counter;

static void *atomic_inc_thread(void *arg)
{
    (void)arg;
    for (int i = 0; i < 100000; i++) {
        atomic_fetch_add(&g_atomic_counter, 1);
    }
    return NULL;
}

static void demo_atomic_fix(void)
{
    printf("=== Demo 3: atomic_int 修复竞态条件 ===\n");

    atomic_init(&g_atomic_counter, 0);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, atomic_inc_thread, NULL);
    pthread_create(&t2, NULL, atomic_inc_thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    int32_t final_val = (int32_t)atomic_load(&g_atomic_counter);
    printf("  期望: 200000, 实际: %" PRId32 "\n", final_val);

    if (final_val == 200000) {
        printf("  ✅ 原子操作保证了正确的结果!\n");
    } else {
        printf("  ❌ 仍有问题 (理论上不应发生)\n");
    }
    printf("\n");
}

/* ---------------------------------------------------------
   Demo 4: atomic_flag — 最简单的原子类型
   --------------------------------------------------------- */

static void demo_atomic_flag(void)
{
    printf("=== Demo 4: atomic_flag (自旋锁) ===\n");

    atomic_flag lock = ATOMIC_FLAG_INIT;

    bool was_set = atomic_flag_test_and_set(&lock);
    printf("  第一次 test_and_set: was_set=%s (应为 false)\n",
           was_set ? "true" : "false");

    was_set = atomic_flag_test_and_set(&lock);
    printf("  第二次 test_and_set: was_set=%s (应为 true)\n",
           was_set ? "true" : "false");

    atomic_flag_clear(&lock);
    was_set = atomic_flag_test_and_set(&lock);
    printf("  clear 后 test_and_set: was_set=%s (应为 false)\n",
           was_set ? "true" : "false");

    printf("\n  自旋锁模拟 (test_and_set 循环):\n");

    atomic_flag lock2 = ATOMIC_FLAG_INIT;

    printf("    获取锁...");
    while (atomic_flag_test_and_set(&lock2)) {
        /* 自旋等待 */
    }
    printf(" 完成!\n");

    printf("    释放锁...\n");
    atomic_flag_clear(&lock2);

    printf("\n");
}

/* ---------------------------------------------------------
   Demo 5: memory_order — 内存顺序模型
   --------------------------------------------------------- */

static void demo_memory_order(void)
{
    printf("=== Demo 5: memory_order (内存顺序) ===\n");

    printf("  memory_order 层级 (从弱到强):\n");
    printf("    1. memory_order_relaxed     — 仅原子性, 不保证顺序\n");
    printf("    2. memory_order_consume     — 依赖操作之间的顺序\n");
    printf("    3. memory_order_acquire     — 后续读不会被重排到此操作之前\n");
    printf("    4. memory_order_release     — 前面操作不会被重排到此操作之后\n");
    printf("    5. memory_order_acq_rel     — acquire + release (读-改-写)\n");
    printf("    6. memory_order_seq_cst     — 全序一致性 (默认最强)\n\n");

    atomic_int relaxed_var;
    atomic_int strong_var;
    atomic_init(&relaxed_var, 0);
    atomic_init(&strong_var, 0);

    atomic_store_explicit(&relaxed_var, 10, memory_order_relaxed);
    atomic_store_explicit(&strong_var,  10, memory_order_seq_cst);

    int32_t rv = atomic_load_explicit(&relaxed_var, memory_order_relaxed);
    int32_t sv = atomic_load_explicit(&strong_var,  memory_order_seq_cst);

    printf("  relaxed  读写结果: %" PRId32 "\n", rv);
    printf("  seq_cst  读写结果: %" PRId32 "\n", sv);

    printf("\n  选用指南:\n");
    printf("    默认用 seq_cst (安全, 编译器不激进优化)\n");
    printf("    仅当性能分析确认瓶颈时, 才降级到 relaxed\n");
    printf("    acquire/release 用于生产者-消费者模式\n\n");
}

/* ---------------------------------------------------------
   Demo 6: volatile vs atomic — 关键区别
   --------------------------------------------------------- */

static void demo_volatile_vs_atomic(void)
{
    printf("=== Demo 6: volatile vs atomic ===\n");

    printf("  volatile: 告诉编译器「别优化我」—— 每次读写都走内存\n");
    printf("  atomic:   保证多线程下的原子操作 + 内存顺序\n\n");

    volatile int32_t vol = 0;
    atomic_int atm;
    atomic_init(&atm, 0);

    vol = 42;
    atomic_store(&atm, 42);

    printf("  volatile int  = %d\n", (int)vol);
    printf("  atomic int    = %" PRId32 "\n",
           (int32_t)atomic_load(&atm));

    printf("\n  对比表:\n");
    printf("  +------------------+------------------+------------------+\n");
    printf("  | 特性              | volatile          | atomic           |\n");
    printf("  +------------------+------------------+------------------+\n");
    printf("  | 阻止编译器优化    | ✅               | ✅               |\n");
    printf("  | 硬件级原子操作    | ❌               | ✅               |\n");
    printf("  | 线程安全          | ❌               | ✅               |\n");
    printf("  | 内存顺序保证      | ❌               | ✅               |\n");
    printf("  | 适用场景          | 信号处理/MMIO    | 多线程共享变量   |\n");
    printf("  +------------------+------------------+------------------+\n");

    printf("\n  【重要结论】\n");
    printf("    volatile ≠ 线程安全! 多线程共享变量必须用 atomic。\n");
    printf("    atomic ≈ thread-safe volatile (但不是所有 atomic 都阻止编译器优化)。\n\n");
}

/* ---------------------------------------------------------
   Demo 7: CAS (Compare-And-Swap) — 无锁编程核心
   --------------------------------------------------------- */

static void demo_cas(void)
{
    printf("=== Demo 7: CAS (Compare-And-Swap) ===\n");

    atomic_int shared;
    atomic_init(&shared, 10);

    int32_t expected = 10;
    int32_t desired  = 20;
    bool ok = atomic_compare_exchange_strong(&shared, &expected, desired);

    printf("  初始值: 10\n");
    printf("  CAS(10→20): %s, 当前值=%" PRId32 "\n",
           ok ? "成功" : "失败", (int32_t)atomic_load(&shared));

    expected = 10;
    ok = atomic_compare_exchange_strong(&shared, &expected, 30);
    printf("  CAS(10→30): %s, expected 更新为=%" PRId32 "\n",
           ok ? "成功" : "失败", expected);
    printf("  当前值保持不变: %" PRId32 "\n",
           (int32_t)atomic_load(&shared));

    printf("\n  CAS 自旋计数器:\n");
    atomic_int spin_counter;
    atomic_init(&spin_counter, 0);
    for (int32_t i = 0; i < 5; i++) {
        int32_t cur = atomic_load(&spin_counter);
        while (!atomic_compare_exchange_weak(&spin_counter, &cur, cur + 1)) {
            /* cur 自动更新为最新值, 重试 */
        }
    }
    printf("  自旋 +5 次后: %" PRId32 "\n",
           (int32_t)atomic_load(&spin_counter));

    printf("\n");
}

/* ---------------------------------------------------------
   Demo 8: 原子操作的内存布局可视化
   --------------------------------------------------------- */

static void demo_memory_layout(void)
{
    printf("=== Demo 8: 原子操作的内存布局 ===\n");

    atomic_int a_val;
    atomic_init(&a_val, 100);
    atomic_flag a_flag = ATOMIC_FLAG_INIT;

    printf("  atomic_int 大小: %zu 字节 (与 int32_t 相同)\n",
           sizeof(a_val));
    printf("  atomic_flag 大小: %zu 字节\n", sizeof(a_flag));
    printf("  a_val 地址: %p\n", (void *)&a_val);

    printf("\n  原子操作的硬件实现 (简化):\n");
    printf("    x86:    LOCK prefix → XADD, CMPXCHG 指令\n");
    printf("    ARM:    LDREX / STREX 指令 (独占访问)\n");
    printf("    RISC-V: AMO (原子内存操作) 指令\n\n");
}

/* ---------------------------------------------------------
   入口函数
   --------------------------------------------------------- */

int main_atomic_types_sample(void)
{
    printf("========================================\n");
    printf("  atomic-types 原子类型 (C11 <stdatomic.h>)\n");
    printf("========================================\n\n");

    demo_race_condition();
    demo_atomic_int_basic();
    demo_atomic_fix();
    demo_atomic_flag();
    demo_memory_order();
    demo_volatile_vs_atomic();
    demo_cas();
    demo_memory_layout();

    printf("atomic-types 演示完毕.\n");
    return 0;
}
