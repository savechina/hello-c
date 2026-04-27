#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>
#include "advance/async_sample.h"

/* ============================================================
 * Async & POSIX Threads Demo
 *
 * Demonstrates:
 *   1. Basic thread creation (pthread_create / pthread_join)
 *   2. Race condition on shared counter (error-first)
 *   3. Mutex (pthread_mutex_t) — fixing the race condition
 *   4. Condition variables — producer/consumer pattern
 *   5. Thread-local storage (TLS) — per-thread data
 * ============================================================ */

/* ── 1. Basic thread creation ── */

typedef struct {
    int32_t id;
    const char *message;
} ThreadArg;

static void *thread_worker(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    printf("  [线程 %" PRId32 "] %s\n", a->id, a->message);
    return NULL;
}

static void demo_basic_threads(void) {
    printf("=== 1. 基本线程创建 (pthread_create / pthread_join) ===\n");

    ThreadArg args[3];
    pthread_t threads[3];
    const char *messages[3] = {
        "我在运行！(我是厨师 A)",
        "我在运行！(我是厨师 B)",
        "我在运行！(我是厨师 C)",
    };

    for (int32_t i = 0; i < 3; i++) {
        args[i].id = i;
        args[i].message = messages[i];
        pthread_create(&threads[i], NULL, thread_worker, &args[i]);
    }

    for (int32_t i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("  所有线程执行完毕。\n\n");
}

/* ── 2. Race condition (error-first) ── */

static int64_t g_counter = 0;

static void *race_increment(void *arg) {
    long n = *(const long *)arg;
    for (long i = 0; i < n; i++) {
        g_counter++;  /* ❌ 不安全：counter++ 不是原子的 */
    }
    return NULL;
}

static void demo_race_condition(void) {
    printf("=== 2. 竞态条件 (Race Condition) — 错误演示 ===\n");

    g_counter = 0;
    pthread_t t[2];
    long n = 50000;

    pthread_create(&t[0], NULL, race_increment, &n);
    pthread_create(&t[1], NULL, race_increment, &n);
    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);

    printf("  结果: counter = %" PRId64 " (期望: %ld)\n", g_counter, n * 2);
    if (g_counter != n * 2) {
        printf("  ⚠️  结果不对！这就是竞态条件——多个线程同时修改共享数据\n");
    }
    printf("  ❌ counter++ 被拆成: LOAD → ADD → STORE，线程交错导致覆盖\n\n");
}

/* ── 3. Mutex: fixing the race condition ── */

static int64_t g_counter_safe = 0;
static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static void *safe_increment(void *arg) {
    long n = *(const long *)arg;
    for (long i = 0; i < n; i++) {
        pthread_mutex_lock(&g_mutex);
        g_counter_safe++;
        pthread_mutex_unlock(&g_mutex);
    }
    return NULL;
}

static void demo_mutex(void) {
    printf("=== 3. 互斥锁 (Mutex) — 修复竞态条件 ===\n");

    g_counter_safe = 0;
    pthread_t t[2];
    long n = 50000;

    pthread_create(&t[0], NULL, safe_increment, &n);
    pthread_create(&t[1], NULL, safe_increment, &n);
    pthread_join(t[0], NULL);
    pthread_join(t[1], NULL);

    printf("  结果: counter = %" PRId64 " (期望: %ld)\n", g_counter_safe, n * 2);
    if (g_counter_safe == n * 2) {
        printf("  ✅ 结果正确！互斥锁保证每次只有一个线程修改 counter\n");
    }
    printf("\n");
}

/* ── 4. Condition variable: producer/consumer ── */

static int32_t g_buffer = -1;
static int g_ready = 0;
static int g_done = 0;
static pthread_mutex_t g_cv_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_cv_cond  = PTHREAD_COND_INITIALIZER;

static void *cv_producer(void *arg) {
    (void)arg;
    for (int32_t i = 1; i <= 3; i++) {
        pthread_mutex_lock(&g_cv_mutex);
        g_buffer = i;
        g_ready = 1;
        pthread_cond_signal(&g_cv_cond);
        pthread_mutex_unlock(&g_cv_mutex);
        printf("  [生产者] 产生数据: %d\n", (int)i);
    }

    pthread_mutex_lock(&g_cv_mutex);
    g_done = 1;
    pthread_cond_signal(&g_cv_cond);
    pthread_mutex_unlock(&g_cv_mutex);
    return NULL;
}

static void *cv_consumer(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&g_cv_mutex);
        while (!g_ready && !g_done) {
            pthread_cond_wait(&g_cv_cond, &g_cv_mutex);
        }
        if (g_done && !g_ready) {
            pthread_mutex_unlock(&g_cv_mutex);
            break;
        }
        printf("  [消费者] 收到数据: %d\n", (int)g_buffer);
        g_ready = 0;
        pthread_mutex_unlock(&g_cv_mutex);
    }
    return NULL;
}

static void demo_condition_variable(void) {
    printf("=== 4. 条件变量 (Condition Variable) — 生产者/消费者 ===\n");

    g_buffer = -1;
    g_ready = 0;
    g_done = 0;

    pthread_t producer, consumer;
    pthread_create(&producer, NULL, cv_producer, NULL);
    pthread_create(&consumer, NULL, cv_consumer, NULL);
    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    printf("  ✅ 生产者/消费者模式完成。\n");
    printf("  关键: cond_wait 原子地释放锁 + 休眠，被 signal 后重新加锁。\n\n");
}

/* ── 5. Thread-local storage (TLS) ── */

static __thread int64_t tls_counter = 0;

static void *tls_worker(void *arg) {
    long id = (long)arg;
    for (int i = 0; i < 5; i++) {
        tls_counter++;
    }
    printf("  [线程 %" PRId32 "] tls_counter = %" PRId64 " (独立副本, 无需 mutex)\n",
           (int32_t)id, tls_counter);
    return NULL;
}

static void demo_tls(void) {
    printf("=== 5. 线程局部存储 (Thread-Local Storage) ===\n");

    pthread_t t[3];
    for (long i = 0; i < 3; i++) {
        pthread_create(&t[i], NULL, tls_worker, (void *)i);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(t[i], NULL);
    }

    printf("  ✅ 每个线程的 tls_counter 互不干扰。\n\n");
}

/* ── Coordinator entry ── */

int main_async_sample(void) {
    printf("========================================\n");
    printf("  异步与线程 (POSIX Threads)\n");
    printf("========================================\n\n");

    demo_basic_threads();
    demo_race_condition();
    demo_mutex();
    demo_condition_variable();
    demo_tls();

    printf("异步与线程演示完毕。\n");
    return 0;
}
