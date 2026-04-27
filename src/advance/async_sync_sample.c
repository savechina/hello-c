/**
 * @file async_sync_sample.c
 * @brief 同步原语 (Synchronization Primitives — Mutex, Condvar, Atomic)
 *
 * Demonstrates:
 *   1. Race condition — shared counter without protection
 *   2. Mutex fix — pthread_mutex_t protects the counter
 *   3. Condition variable — producer/consumer pattern
 *   4. Atomic compare — C11 atomic_int vs mutex performance
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

#include "advance/async_sync_sample.h"

/* ============================================================
 * Demo 1: 竞态条件 (Race Condition)
 * ============================================================ */

static int64_t g_unsafe_counter = 0;

static void *race_increment(void *arg)
{
    long n = *(const long *)arg;
    for (long i = 0; i < n; i++) {
        g_unsafe_counter++;  /* ❌ 不是原子的 — LOAD/ADD/STORE 三步 */
    }
    return NULL;
}

static void demo_race_condition(void)
{
    printf("--- 1. 竞态条件 (Race Condition) — 错误演示 ---\n");

    g_unsafe_counter = 0;
    const long n = 50000;
    pthread_t t[2];

    pthread_create(&t[0], NULL, race_increment, (void *)&n);
    pthread_create(&t[1], NULL, race_increment, (void *)&n);
    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);

    printf("  结果: counter = %" PRId64 " (期望: %ld)\n",
           g_unsafe_counter, n * 2);
    if (g_unsafe_counter != n * 2) {
        printf("  ⚠️  结果不对！counter++ 被拆成 LOAD→ADD→STORE，线程交错导致覆盖\n");
    }
    printf("  ❌ 没有保护的 shared variable = 定时炸弹\n\n");
}

/* ============================================================
 * Demo 2: Mutex 修复 (Mutex Fix)
 * ============================================================ */

static int64_t g_safe_counter = 0;
static pthread_mutex_t g_sync_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *safe_increment(void *arg)
{
    long n = *(const long *)arg;
    for (long i = 0; i < n; i++) {
        pthread_mutex_lock(&g_sync_mutex);
        g_safe_counter++;
        pthread_mutex_unlock(&g_sync_mutex);
    }
    return NULL;
}

static void demo_mutex_fix(void)
{
    printf("--- 2. Mutex 修复 (Mutex Fix) — 保护临界区 ---\n");

    g_safe_counter = 0;
    const long n = 50000;
    pthread_t t[2];

    pthread_create(&t[0], NULL, safe_increment, (void *)&n);
    pthread_create(&t[1], NULL, safe_increment, (void *)&n);
    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);

    printf("  结果: counter = %" PRId64 " (期望: %ld)\n",
           g_safe_counter, n * 2);
    if (g_safe_counter == n * 2) {
        printf("  ✅ 每次都正确！mutex 保证一次只有一个线程修改 counter\n");
    }
    printf("\n");
}

/* ============================================================
 * Demo 3: 条件变量 — 生产者/消费者 (Condition Variable)
 * ============================================================ */

static int32_t g_cv_buffer = 0;
static int g_cv_ready = 0;
static int g_cv_done = 0;
static pthread_mutex_t g_cv_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_cv_cond  = PTHREAD_COND_INITIALIZER;

static void *cv_producer(void *arg)
{
    (void)arg;
    for (int32_t i = 1; i <= 3; i++) {
        pthread_mutex_lock(&g_cv_mutex);
        g_cv_buffer = i;
        g_cv_ready = 1;
        pthread_cond_signal(&g_cv_cond);
        pthread_mutex_unlock(&g_cv_mutex);
        printf("  [生产者] 产生数据: %d\n", (int)i);
    }

    /* 告诉消费者：不再有数据了 */
    pthread_mutex_lock(&g_cv_mutex);
    g_cv_done = 1;
    pthread_cond_signal(&g_cv_cond);
    pthread_mutex_unlock(&g_cv_mutex);
    return NULL;
}

static void *cv_consumer(void *arg)
{
    (void)arg;
    while (1) {
        pthread_mutex_lock(&g_cv_mutex);
        /* ✅ 必须用 while，不是 if — 防止虚假唤醒 */
        while (!g_cv_ready && !g_cv_done) {
            pthread_cond_wait(&g_cv_cond, &g_cv_mutex);
            /* cond_wait 原子地: 释放锁 + 休眠 → 被唤醒后重新获得锁 */
        }
        if (g_cv_done && !g_cv_ready) {
            pthread_mutex_unlock(&g_cv_mutex);
            break;
        }
        printf("  [消费者] 收到数据: %d\n", (int)g_cv_buffer);
        g_cv_ready = 0;
        pthread_mutex_unlock(&g_cv_mutex);
    }
    return NULL;
}

static void demo_condvar(void)
{
    printf("--- 3. 条件变量 (Condition Variable) — 生产者/消费者 ---\n");

    g_cv_buffer = 0;
    g_cv_ready = 0;
    g_cv_done = 0;

    pthread_t producer, consumer;
    pthread_create(&producer, NULL, cv_producer, NULL);
    pthread_create(&consumer, NULL, cv_consumer, NULL);
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    printf("  ✅ 生产者/消费者模式完成。\n");
    printf("  关键: cond_wait 原子地释放锁 + 休眠，signal 后重新加锁。\n\n");
}

/* ============================================================
 * Demo 4: C11 Atomic — 无锁计数 (Lock-Free Counter)
 * ============================================================ */

static _Atomic int64_t g_atomic_counter = 0;

static void *atomic_increment(void *arg)
{
    long n = *(const long *)arg;
    for (long i = 0; i < n; i++) {
        atomic_fetch_add(&g_atomic_counter, 1);
    }
    return NULL;
}

static void demo_atomic_compare(void)
{
    printf("--- 4. C11 Atomic vs Mutex 性能对比 ---\n");

    const long n = 50000;
    struct timespec ts_start, ts_end;

    /* --- 用 mutex 的版本 --- */
    g_safe_counter = 0;
    pthread_t mt[2];
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    pthread_create(&mt[0], NULL, safe_increment, (void *)&n);
    pthread_create(&mt[1], NULL, safe_increment, (void *)&n);
    pthread_join(mt[0], NULL);
    pthread_join(mt[1], NULL);
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    long mutex_ms = (ts_end.tv_sec - ts_start.tv_sec) * 1000 +
                    (ts_end.tv_nsec - ts_start.tv_nsec) / 1000000;

    /* --- 用 atomic 的版本 --- */
    g_atomic_counter = 0;
    pthread_t at[2];
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    pthread_create(&at[0], NULL, atomic_increment, (void *)&n);
    pthread_create(&at[1], NULL, atomic_increment, (void *)&n);
    pthread_join(at[0], NULL);
    pthread_join(at[1], NULL);
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    long atomic_ms = (ts_end.tv_sec - ts_start.tv_sec) * 1000 +
                     (ts_end.tv_nsec - ts_start.tv_nsec) / 1000000;

    printf("  Mutex  版本: counter = %" PRId64 ", 耗时 %ld ms\n",
           g_safe_counter, mutex_ms);
    printf("  Atomic 版本: counter = %" PRId64 ", 耗时 %ld ms\n",
           (int64_t)g_atomic_counter, atomic_ms);
    printf("  💡 原子操作不需要 mutex，计数场景更高效\n");
    printf("     但复杂操作（读取→修改→写入 超过 2 步）仍需要 mutex\n\n");
}

/* ============================================================
 * Coordinator
 * ============================================================ */

int main_async_sync_sample(void)
{
    printf("========================================\n");
    printf("  同步原语 (Mutex · Condvar · Atomic)\n");
    printf("========================================\n\n");

    demo_race_condition();
    demo_mutex_fix();
    demo_condvar();
    demo_atomic_compare();

    printf("同步原语演示完毕。\n");
    return 0;
}
