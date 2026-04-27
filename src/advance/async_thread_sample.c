/**
 * @file async_thread_sample.c
 * @brief 线程创建与生命周期 (Thread Creation & Lifecycle)
 *
 * Demonstrates:
 *   1. Basic thread creation (pthread_create + pthread_join)
 *   2. Passing data to threads via void* (struct pointer)
 *   3. Returning data from threads (pthread_exit + pthread_join retval)
 *   4. Detached threads (pthread_detach — fire-and-forget)
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "advance/async_thread_sample.h"

/* ============================================================
 * Demo 1: 基本线程创建 (Basic Thread Creation)
 * ============================================================ */

static void *demo_thread_basic_worker(void *arg)
{
    int32_t id = *(const int32_t *)arg;
    printf("  [线程 %" PRId32 "] 我开始工作了！(我是厨师 %c)\n",
           id, (int)('A' + id));
    /* ... 做点实际工作 ... */
    printf("  [线程 %" PRId32 "] 我做完了！\n", id);
    return NULL;
}

static void demo_thread_basic(void)
{
    printf("--- 1. 基本线程创建 (pthread_create + pthread_join) ---\n");

    const int32_t n = 3;
    pthread_t threads[3];
    int32_t ids[3] = {0, 1, 2};

    /* 创建线程 — 每个线程拿到不同的 id 指针 */
    for (int32_t i = 0; i < n; i++) {
        pthread_create(&threads[i], NULL, demo_thread_basic_worker, &ids[i]);
    }

    /* 等待所有线程完成 */
    for (int32_t i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("  ✅ 所有线程执行完毕。\n\n");
}

/* ============================================================
 * Demo 2: 向线程传递数据 (Passing Data via void*)
 * ============================================================ */

typedef struct {
    int32_t worker_id;
    const char *task_name;
    int32_t repeat_count;
} TaskArgs;

static void *demo_thread_with_data_worker(void *arg)
{
    TaskArgs *task = (TaskArgs *)arg;
    printf("  [员工 %" PRId32 "] 开始任务: \"%s\", 重复 %d 次\n",
           task->worker_id, task->task_name, (int)task->repeat_count);
    for (int32_t i = 0; i < task->repeat_count; i++) {
        printf("    → 第 %d/%d 次 ...\n", (int)(i + 1), (int)task->repeat_count);
    }
    printf("  [员工 %" PRId32 "] \"%s\" 完成！\n", task->worker_id, task->task_name);
    return NULL;
}

static void demo_thread_with_data(void)
{
    printf("--- 2. 向线程传递数据 (struct via void*) ---\n");

    TaskArgs tasks[3];
    pthread_t threads[3];

    /* 构造任务参数 — 用栈上的数组，main 等 join 后才返回，安全 */
    tasks[0].worker_id = 0;
    tasks[0].task_name = "切菜";
    tasks[0].repeat_count = 3;

    tasks[1].worker_id = 1;
    tasks[1].task_name = "炒菜";
    tasks[1].repeat_count = 2;

    tasks[2].worker_id = 2;
    tasks[2].task_name = "端盘子";
    tasks[2].repeat_count = 5;

    for (int32_t i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, demo_thread_with_data_worker, &tasks[i]);
    }
    for (int32_t i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("  ✅ 所有任务完成。\n\n");
}

/* ============================================================
 * Demo 3: 线程返回值 (Returning Data from Threads)
 * ============================================================ */

typedef struct {
    int64_t sum;
    int32_t status; /* 0 = success */
} TaskResult;

static void *demo_thread_return_worker(void *arg)
{
    int32_t n = *(const int32_t *)arg;
    TaskResult *result = malloc(sizeof(TaskResult));
    if (result == NULL) {
        fprintf(stderr, "  [错误] malloc 失败\n");
        pthread_exit(NULL);
    }

    int64_t s = 0;
    for (int32_t i = 1; i <= n; i++) {
        s += i;
    }
    result->sum = s;
    result->status = 0;

    printf("  [worker] 计算 sum(1..%" PRId32 ") = %" PRId64 "\n", n, s);
    pthread_exit((void *)result);
}

static void demo_thread_return(void)
{
    printf("--- 3. 线程返回值 (pthread_exit + pthread_join retval) ---\n");

    pthread_t t;
    int32_t n = 100;
    TaskResult *result = NULL;

    pthread_create(&t, NULL, demo_thread_return_worker, &n);
    pthread_join(t, (void **)&result);

    if (result != NULL) {
        printf("  [主线程] 收到结果: sum = %" PRId64 ", status = %d\n",
               result->sum, (int)result->status);
        free(result);
    }

    printf("  ✅ 线程返回值演示完毕。\n\n");
}

/* ============================================================
 * Demo 4: 分离线程 (Detached Thread — Fire-and-forget)
 * ============================================================ */

static void *demo_thread_detached_worker(void *arg)
{
    int32_t id = *(const int32_t *)arg;
    printf("  [分离线程 %" PRId32 "] 我在后台安静地工作...\n", id);
    printf("  [分离线程 %" PRId32 "] 做完了，自动回收。\n", id);
    return NULL;
}

static void demo_thread_detached(void)
{
    printf("--- 4. 分离线程 (pthread_detach — Fire-and-forget) ---\n");

    int32_t id = 42;
    pthread_t t;

    pthread_create(&t, NULL, demo_thread_detached_worker, &id);
    /* 告诉系统：这个线程结束后自动回收，不需要 join */
    pthread_detach(t);

    /* 稍微等一下让分离线程执行完，生产代码不依赖这个 sleep */
    /* 这里只是为了演示能看到输出 */

    /* pthread_join(t, NULL);  ← 不能调用！已经 detach 了 */
    printf("  [主线程] 我没有 join，但分离线程会自己回收。\n");
    printf("  ✅ 分离线程演示完毕。\n\n");
}

/* ============================================================
 * Coordinator
 * ============================================================ */

int main_async_thread_sample(void)
{
    printf("========================================\n");
    printf("  线程创建与生命周期 (Thread & Lifecycle)\n");
    printf("========================================\n\n");

    demo_thread_basic();
    demo_thread_with_data();
    demo_thread_return();
    demo_thread_detached();

    printf("线程创建与生命周期演示完毕。\n");
    return 0;
}
