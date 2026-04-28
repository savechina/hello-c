/**
 * @file async_pool_sample.c
 * @brief 线程池模式 (Thread Pool Pattern)
 *
 * Demonstrates:
 *   1. ThreadPool creation — fixed worker count + task queue
 *   2. Task submission — enqueue work items for workers
 *   3. Graceful shutdown — signal workers to exit, join them
 *   4. Practical: pool processing an array (sum, max, square)
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "advance/async_pool_sample.h"

/* ============================================================
 * ThreadPool implementation
 * ============================================================ */

#define POOL_MAX_TASKS 64

typedef void (*TaskFunc)(void *arg);

typedef struct {
    TaskFunc func;
    void *arg;
} Task;

typedef struct {
    pthread_t *workers;
    Task *queue;
    int queue_head;
    int queue_tail;
    int queue_count;
    int max_queue;
    int shutdown;

    pthread_mutex_t mutex;
    pthread_cond_t  not_empty;
    pthread_cond_t  not_full;
} ThreadPool;

static void *worker_loop(void *arg);
static void pool_init(ThreadPool *pool, int num_workers, int max_queue);

static void pool_init(ThreadPool *pool, int num_workers, int max_queue)
{
    pool->workers = calloc((size_t)num_workers, sizeof(pthread_t));
    pool->queue = calloc((size_t)max_queue, sizeof(Task));
    pool->queue_head = 0;
    pool->queue_tail = 0;
    pool->queue_count = 0;
    pool->max_queue = max_queue;
    pool->shutdown = 0;
    pthread_mutex_init(&pool->mutex, NULL);
    pthread_cond_init(&pool->not_empty, NULL);
    pthread_cond_init(&pool->not_full, NULL);

    for (int i = 0; i < num_workers; i++) {
        pthread_create(&pool->workers[i], NULL, worker_loop, pool);
    }
}

static void *worker_loop(void *arg)
{
    ThreadPool *pool = (ThreadPool *)arg;

    while (1) {
        pthread_mutex_lock(&pool->mutex);

        while (pool->queue_count == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->not_empty, &pool->mutex);
        }

        if (pool->shutdown && pool->queue_count == 0) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        Task task = pool->queue[pool->queue_head];
        pool->queue_head = (pool->queue_head + 1) % pool->max_queue;
        pool->queue_count--;

        pthread_cond_signal(&pool->not_full);
        pthread_mutex_unlock(&pool->mutex);

        task.func(task.arg);
    }

    return NULL;
}

static int pool_submit(ThreadPool *pool, TaskFunc func, void *arg)
{
    pthread_mutex_lock(&pool->mutex);

    while (pool->queue_count == pool->max_queue && !pool->shutdown) {
        pthread_cond_wait(&pool->not_full, &pool->mutex);
    }

    if (pool->shutdown) {
        pthread_mutex_unlock(&pool->mutex);
        return -1;
    }

    pool->queue[pool->queue_tail].func = func;
    pool->queue[pool->queue_tail].arg = arg;
    pool->queue_tail = (pool->queue_tail + 1) % pool->max_queue;
    pool->queue_count++;

    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->mutex);
    return 0;
}

static void pool_shutdown(ThreadPool *pool, int num_workers)
{
    pthread_mutex_lock(&pool->mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->not_empty);
    pthread_mutex_unlock(&pool->mutex);

    for (int i = 0; i < num_workers; i++) {
        pthread_join(pool->workers[i], NULL);
    }

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->not_empty);
    pthread_cond_destroy(&pool->not_full);
    free(pool->workers);
    free(pool->queue);
}

/* ============================================================
 * Demo 1: 创建线程池 (Pool Creation)
 * ============================================================ */

static void async_pool_create_sample(void)
{
    printf("--- 1. 创建线程池 (ThreadPool Creation) ---\n");

    ThreadPool pool;
    const int num_workers = 2;
    const int max_queue = POOL_MAX_TASKS;

    /* 只是创建 — 不提交任务，演示后立即关闭 */
    pool_init(&pool, num_workers, max_queue);
    printf("  线程池已创建: %d 个 worker, 队列容量 %d\n", num_workers, max_queue);

    pool_shutdown(&pool, num_workers);
    printf("  线程池已关闭。\n\n");
}

/* ============================================================
 * Demo 2: 提交任务 (Task Submission)
 * ============================================================ */

typedef struct {
    int32_t id;
    int32_t value;
    int32_t result;
} IntTask;

static void task_print_square(void *arg)
{
    IntTask *t = (IntTask *)arg;
    t->result = t->value * t->value;
    printf("  [task %" PRId32 "] %d^2 = %d\n", t->id, (int)t->value, (int)t->result);
}

static void task_add_one(void *arg)
{
    IntTask *t = (IntTask *)arg;
    t->result = t->value + 1;
    printf("  [task %" PRId32 "] %d + 1 = %d\n", t->id, (int)t->value, (int)t->result);
}

static void async_pool_submit_sample(void)
{
    printf("--- 2. 提交任务到线程池 (Task Submission) ---\n");

    ThreadPool pool;
    const int num_workers = 2;
    pool_init(&pool, num_workers, POOL_MAX_TASKS);

    IntTask tasks[6];
    for (int32_t i = 0; i < 6; i++) {
        tasks[i].id = i;
        tasks[i].value = 10 + (int)i;
        tasks[i].result = 0;
    }

    /* 提交 6 个任务 — 2 个做平方，4 个做加一 */
    pool_submit(&pool, task_print_square, &tasks[0]);
    pool_submit(&pool, task_print_square, &tasks[1]);
    pool_submit(&pool, task_add_one, &tasks[2]);
    pool_submit(&pool, task_add_one, &tasks[3]);
    pool_submit(&pool, task_add_one, &tasks[4]);
    pool_submit(&pool, task_add_one, &tasks[5]);

    pool_shutdown(&pool, num_workers);

    printf("  所有任务完成，结果:\n");
    for (int i = 0; i < 2; i++) {
        printf("    square(%d) = %d\n", (int)tasks[i].value, (int)tasks[i].result);
    }
    for (int i = 2; i < 6; i++) {
        printf("    add_one(%d) = %d\n", (int)tasks[i].value, (int)tasks[i].result);
    }
    printf("\n");
}

/* ============================================================
 * Demo 3: 优雅关闭 (Graceful Shutdown)
 * ============================================================ */

typedef struct {
    int32_t id;
    const char *label;
} LabelTask;

static void task_labels(void *arg)
{
    LabelTask *t = (LabelTask *)arg;
    printf("  [task %" PRId32 "] 执行: %s\n", t->id, t->label);
}

static void async_pool_shutdown_sample(void)
{
    printf("--- 3. 优雅关闭 (Graceful Shutdown) ---\n");

    ThreadPool pool;
    const int num_workers = 2;
    pool_init(&pool, num_workers, POOL_MAX_TASKS);

    LabelTask labels[] = {
        {0, "准备数据"},
        {1, "处理数据"},
        {2, "写入文件"},
    };

    pool_submit(&pool, task_labels, &labels[0]);
    pool_submit(&pool, task_labels, &labels[1]);
    pool_submit(&pool, task_labels, &labels[2]);

    /* shutdown 会:
     *   1. 设置 shutdown 标志
     *   2. broadcast 唤醒所有 worker
     *   3. worker 处理完队列后退出
     *   4. join 回收所有线程
     */
    pool_shutdown(&pool, num_workers);

    printf("  线程池已优雅关闭: 所有待处理任务都执行完了。\n\n");
}

/* ============================================================
 * Coordinator
 * ============================================================ */

int main_async_pool_sample(void)
{
    printf("========================================\n");
    printf("  线程池模式 (Thread Pool Pattern)\n");
    printf("========================================\n\n");

    async_pool_create_sample();
    async_pool_submit_sample();
    async_pool_shutdown_sample();

    printf("线程池模式演示完毕。\n");
    return 0;
}
