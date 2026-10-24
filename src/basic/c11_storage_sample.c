#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "basic/c11_storage_sample.h"

/*
 * C11 存储类说明符 (_Alignas / _Noreturn / _Thread_local)
 *
 * 错误先行: 先看注释里的约束违规错例, 再看可运行的正确用法。
 * _Alignof 的深挖在 结构体内存布局 章节 (struct_memory_layout_sample.c) — 本章只交叉引用。
 */

/* ── _Alignas: 请求比自然对齐更强的对齐 ── */

/* cache-line 对齐的静态缓冲 — DMA / 无锁环形队列的常见需求 */
_Alignas(64) static unsigned char c11s_cache_line[64];

/* ❌ 错例 (仅示意, 编译不过, C17 §6.7.5):
 *   _Alignas(1) int x;    // 小于 int 的自然对齐 → 约束违规
 *   _Alignas(3) char c;   // 不是 2 的幂      → 约束违规
 *   _Alignas(16) double d; // 大于自然对齐是【允许】的: 只多占空间, 不损正确性
 */

int c11s_cache_line_mod64(void)
{
    return (int)((uintptr_t)c11s_cache_line % 64u);
}

int c11s_alignof_int32(void)
{
    return (int)_Alignof(int32_t);
}

int c11s_alignof_int64(void)
{
    return (int)_Alignof(int64_t);
}

static void c11s_demo_alignas(void)
{
    _Alignas(16) int aligned_buf[4];

    printf("=== _Alignas: 请求更强对齐 ===\n");
    printf("  &c11s_cache_line = %p (mod 64 = %d, _Alignas(64) 生效)\n",
           (void *)c11s_cache_line, c11s_cache_line_mod64());
    printf("  &aligned_buf     = %p (mod 16 = %d)\n",
           (void *)aligned_buf,
           (int)((uintptr_t)aligned_buf % 16u));
    printf("  _Alignof(int32_t) = %d, _Alignof(int64_t) = %d ← 查询工具\n",
           c11s_alignof_int32(), c11s_alignof_int64());
    printf("  _Alignof 深挖见 结构体内存布局 章节 (struct_memory_layout_sample.c)\n");
    printf("  ❌ _Alignas(3) 是约束违规 (必须 2 的幂且 ≥ 自然对齐), 见注释错例\n\n");
}

/* ── _Noreturn: 声明"绝不返回" ── */

/* _Noreturn 是一份契约: 函数要么终止进程、要么终止线程、要么 longjmp,
 * 总之【绝不返回调用点】。编译器据此: 1) 调用点之后标记为不可达;
 * 2) 消除 "control reaches end of non-void function" 类警告。 */
static _Noreturn void c11s_exit_with_sum(int k)
{
    long sum = (long)k * (k + 1) / 2;

    printf("  [_Noreturn] 计算 sum(1..%d) = %ld, 即将 pthread_exit\n", k, sum);
    /* pthread_exit 终止当前线程 — _Noreturn 契约成立 */
    pthread_exit((void *)(intptr_t)sum);
    /* ❌ 若在这里写 return → 约束违规: '_Noreturn' function does return
     *   (clang 直接编译错误, 不是警告) */
}

/* _Noreturn 的真实用途: 错误路径收束。调用 die() 后的代码编译器视为不可达,
 * 这正是"fail fast + 收束控制流"的标准写法。 */
static _Noreturn void c11s_die(const char *reason)
{
    fprintf(stderr, "  [fatal] %s\n", reason);
    exit(1);
}

/* 演示 _Noreturn 收束后的控制流: 死代码路径调用 c11s_die,
 * 编译器知道它不返回, 所以后面直接跟 return 不会报警告。 */
static int c11s_checked_div(int a, int b, int *out)
{
    if (b == 0) {
        c11s_die("division by zero");
    }
    *out = a / b;
    return 1;
}

static void *c11s_noreturn_worker(void *arg)
{
    int k = (int)(intptr_t)arg;

    /* 这个 worker 的"函数体"本身就是 _Noreturn 的 — 无 return 语句,
     * 编译器不报 "control reaches end of non-void function" */
    c11s_exit_with_sum(k);
}

int c11s_noreturn_thread_sum(int k)
{
    pthread_t t;
    void *ret = NULL;

    if (k < 1) {
        k = 1;
    }
    if (pthread_create(&t, NULL, c11s_noreturn_worker, (void *)(intptr_t)k) != 0) {
        return -1;
    }
    if (pthread_join(t, &ret) != 0) {
        return -1;
    }
    return (int)(intptr_t)ret;
}

static void c11s_demo_noreturn(void)
{
    int dummy = 0;

    printf("=== _Noreturn: 声明绝不返回 ===\n");
    c11s_checked_div(10, 2, &dummy);
    printf("  c11s_checked_div(10, 2) = %d\n", dummy);
    printf("  (b==0 分支调用 _Noreturn die(), 之后的代码不可达 — 编译器据此免警告)\n");
    printf("  c11s_noreturn_thread_sum(5) = %d ← _Noreturn worker 经 pthread_exit 回传\n",
           c11s_noreturn_thread_sum(5));
    printf("  ❌ _Noreturn 函数里写 return 是约束违规, clang 直接拒绝编译\n\n");
}

/* ── _Thread_local: 每线程一份副本 ── */

/* 全局可见但每线程独立 — 这就是 _Thread_local 与普通 static 的唯一区别 */
static _Thread_local long c11s_tls_partial_sum;

/* 对照组: 普通 static 全局, 所有线程共享同一份 (多线程写它就是数据竞争) */
static long c11s_shared_partial_sum;

static void *c11s_tls_worker(void *arg)
{
    /* async_thread 章的传参铁律: 线程参数按值传, 用 (void *)(intptr_t)N,
     * 绝不传 &stack_local — 分离线程/长生命周期线程会拿到悬空指针。 */
    int k = (int)(intptr_t)arg;

    c11s_tls_partial_sum = 0;
    for (int i = 1; i <= k; i++) {
        c11s_tls_partial_sum += i; /* 只改自己的副本 — TSan 眼里无竞争 */
    }
    printf("  [worker k=%d] &c11s_tls_partial_sum = %p (每线程不同)\n",
           k, (void *)&c11s_tls_partial_sum);
    /* pthread_join 的返回值通道: 按值带回计算结果 */
    return (void *)(intptr_t)c11s_tls_partial_sum;
}

int c11s_tls_thread_sum(int n_threads)
{
    pthread_t threads[8];
    long total = 0;

    if (n_threads < 1) {
        n_threads = 1;
    }
    if (n_threads > 8) {
        n_threads = 8;
    }

    for (int i = 0; i < n_threads; i++) {
        if (pthread_create(&threads[i], NULL, c11s_tls_worker,
                           (void *)(intptr_t)(i + 1)) != 0) {
            return -1;
        }
    }
    for (int i = 0; i < n_threads; i++) {
        void *ret = NULL;
        if (pthread_join(threads[i], &ret) != 0) {
            return -1;
        }
        total += (int)(intptr_t)ret;
    }
    return (int)total;
}

static void c11s_demo_thread_local(void)
{
    c11s_tls_partial_sum = 777; /* 主线程自己的副本 */

    printf("=== _Thread_local: 每线程一份副本 ===\n");
    printf("  主线程 &c11s_tls_partial_sum = %p, 值 = %ld\n",
           (void *)&c11s_tls_partial_sum, c11s_tls_partial_sum);
    printf("  c11s_tls_thread_sum(3) = %d (worker k=1,2,3 各自算 sum, 再相加)\n",
           c11s_tls_thread_sum(3));
    printf("  主线程的副本仍然是 %ld ← 线程的写入没有污染它 (隔离)\n",
           c11s_tls_partial_sum);
    printf("  ❌ 错例: 把上面的 _Thread_local 换成普通 static — 3 个线程\n");
    printf("     同时读改写【同一块内存】, 结果取决于调度 (数据竞争 = UB);\n");
    printf("     对照: c11s_shared_partial_sum (普通 static, 所有线程同一地址 %p)\n\n",
           (void *)&c11s_shared_partial_sum);
}

/* ── public entry ── */

int main_c11_storage_sample(void)
{
    printf("--- C11 存储类说明符 (_Alignas / _Noreturn / _Thread_local) ---\n");
    printf("C11 §6.7: 三种存储类 — 对齐契约、不可达契约、线程私有存储。\n\n");

    c11s_demo_alignas();
    c11s_demo_noreturn();
    c11s_demo_thread_local();

    printf("C11 存储类说明符演示完毕.\n");
    return 0;
}
