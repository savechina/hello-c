/**
 * @file debugging_sample.c
 * @brief 调试与错误处理 (Debugging & Error Handling)
 *
 * Demonstrates:
 *   1. errno pattern (sqrt(-1) deliberate error)
 *   2. perror demonstration
 *   3. strerror usage
 *   4. assert() usage
 *   5. Error return pattern (functions returning status codes)
 *   6. Signal handling (SIGINT catch)
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include <signal.h>
#include "basic/debugging_sample.h"

/* ============================================================
   Section 1: errno 模式
   ============================================================ */

/**
 * 演示 errno 的设置和检查
 * 
 * errno 是一个线程局部变量，存储最近一次库函数的错误码。
 * 重要：errno 只在函数报错时设置，成功时不清零。
 */
static void demo_errno(void) {
    printf("  [errno] errno 初始值: %d\n", errno);

    /* 合法调用: sqrt(4.0) 成功，errno 不变 */
    errno = 0;  /* 先清零 */
    double result = sqrt(4.0);
    printf("  [errno] sqrt(4.0) = %.2f, errno = %d (成功时 errno 可能非 0)\n", result, errno);

    /* 错误调用: sqrt(-1) 会设置 errno */
    errno = 0;  /* 调用前清零 */
    result = sqrt(-1.0);
    printf("  [errno] sqrt(-1.0) = %f\n", result);
    printf("  [errno] errno = %d\n", errno);

    if (errno != 0) {
        printf("  [errno] 发生错误!\n");
    }
}

/* ============================================================
Section 2: perror 和 strerror
 ============================================================ */

/**
 * perror: 自动打印 errno 对应的错误信息
 * strerror: 返回错误码对应的字符串
 */
static void demo_perror_strerror(void) {
    printf("\n  [error messages] 演示 perror:\n");

    /* 触发一个文件打开错误 */
    errno = 0;
    FILE *fp = fopen("/nonexistent/path/to/file.txt", "r");
    if (fp == NULL) {
        perror("  [error messages] fopen failed");
        /* 输出类似:   [error messages] fopen failed: No such file or directory */
    }

    printf("\n  [error messages] 演示 strerror:\n");
    printf("  [error messages] errno %d: %s\n", errno, strerror(errno));
    printf("  [error messages] errno 2:  %s\n", strerror(2));
    printf("  [error messages] errno 13: %s\n", strerror(13));
}

/* ============================================================
Section 3: assert() 断言
 ============================================================ */

/**
 * assert 在调试阶段检查不变量 (invariant)。
 * 如果条件为假 (0)，程序中止并打印失败位置。
 * 
 * 编译时加 -DNDEBUG 可禁用所有 assert。
 */
static void demo_assert(void) {
    printf("\n  [assert] 使用 assert 检查不变量:\n");

    int x = 10;
    assert(x == 10);  /* 通过 */
    printf("  [assert] assert(x == 10): 通过\n");

    assert(x > 0);    /* 通过 */
    printf("  [assert] assert(x > 0): 通过\n");

    /* assert(x == 0);  ← 如果在调试中取消注释，程序会中止并报告错误 */
    printf("  [assert] assert(x == 0) 被注释掉了 (取消注释会触发 abort)\n");

    /* NDEBUG 的作用 */
#ifdef NDEBUG
    printf("  [assert] NDEBUG 已定义: assert 被禁用\n");
#else
    printf("  [assert] NDEBUG 未定义: assert 处于激活状态\n");
#endif
    printf("  [assert] 编译时加 -DNDEBUG 可关闭所有 assert (发布模式)\n");

    /* assert 的常见用法：检查指针和函数内部不变量 */
    int *ptr = malloc(sizeof(int));
    assert(ptr != NULL);  /* 检查 malloc 是否成功 */
    *ptr = 42;
    printf("  [assert] assert(ptr != NULL): 内存分配检查通过\n");
    free(ptr);
}

/* ============================================================
Section 4: 错误返回模式
 ============================================================ */

/**
 * C 函数错误返回惯例：
 *   成功 → 返回 0 (或有效值)
 *   失败 → 返回 -1 (或 NULL, 具体看 API 约定)
 *   错误详情 → errno 或输出参数
 */
static int safe_divide(int a, int b, int *result) {
    if (b == 0) {
        errno = EINVAL;
        return -1;  /* 失败 */
    }
    if (result == NULL) {
        errno = EINVAL;
        return -1;  /* 失败 */
    }
    *result = a / b;
    return 0;  /* 成功 */
}

static void demo_error_return(void) {
    printf("\n  [error return] 函数返回状态码模式:\n");

    int result = 0;

    /* 成功情况 */
    int status = safe_divide(10, 3, &result);
    if (status == 0) {
        printf("  [error return] 10 / 3 = %d (成功)\n", result);
    } else {
        printf("  [error return] 除法失败: %s\n", strerror(errno));
    }

    /* 失败情况: 除以零 */
    status = safe_divide(10, 0, &result);
    if (status == 0) {
        printf("  [error return] 10 / 0 = %d\n", result);
    } else {
        printf("  [error return] 10 / 0 失败: %s (errno = %d)\n", strerror(errno), errno);
    }

    /* 失败情况: NULL 指针 */
    status = safe_divide(10, 3, NULL);
    if (status == 0) {
        printf("  [error return] 成功\n");
    } else {
        printf("  [error return] NULL result 参数失败: %s\n", strerror(errno));
    }
}

/* ============================================================
Section 5: 信号处理 (Signal Handling)
 ============================================================ */

/**
 * C 中可以注册信号处理函数来响应系统信号。
 * 常用信号:
 *   SIGINT   - Ctrl+C (中断)
 *   SIGSEGV  - 段错误 (非法内存访问)
 *   SIGTERM  - 终止请求
 */
static void signal_handler(int sig) {
    printf("\n  [signal] 收到信号 %d，正在清理...\n", sig);
    printf("  [signal] 程序已安全退出。\n");
    exit(0);
}

static void demo_signal_handling(void) {
    printf("\n  [signal] 注册 SIGINT (Ctrl+C) 处理函数...\n");
    signal(SIGINT, signal_handler);
    printf("  [signal] 提示: 在交互式运行时按下 Ctrl+C 会触发信号处理。\n");
    printf("  [signal] 当前演示中跳过等待 (非交互式环境)。\n");
    printf("  [signal] \n");
    printf("  [signal] 常见信号:\n");
    printf("    SIGINT  (2)  → Ctrl+C  中断\n");
    printf("    SIGSEGV(11) → 段错误  非法访问\n");
    printf("    SIGTERM (15) → 终止请求 kill 默认信号\n");
}

/* ============================================================
Section 6: 完整调试流程演示
 ============================================================ */

/**
 * 综合演示: errno + perror + strerror + assert
 * 模拟一个可能失败的操作
 */
static int complex_operation(int input, int *output) {
    /* 1. 输入校验 */
    if (input < 0) {
        errno = EINVAL;
        perror("  [complex] Invalid input");
        return -1;
    }

    /* 2. 计算 */
    *output = input * 10;

    /* 3. 内部不变量检查 */
    assert(*output >= 0);

    return 0;  /* 成功 */
}

static void demo_debug_workflow(void) {
    printf("\n  [workflow] 综合调试流程 (errno + assert + perror):\n");

    int result = 0;

    /* 正常路径 */
    int status = complex_operation(5, &result);
    if (status == 0) {
        printf("  [workflow] complex_operation(5) = %d (成功)\n", result);
    }

    /* 错误路径 */
    status = complex_operation(-1, &result);
    if (status != 0) {
        printf("  [workflow] complex_operation(-1): 失败, errno = %d (%s)\n",
               errno, strerror(errno));
    }
}

/* ============================================================
   公共入口
   ============================================================ */

int main_debugging_sample(void) {
    printf("==== 调试与错误处理 (Debugging & Error Handling) ====\n\n");

    printf("--- errno 模式 ---\n");
    demo_errno();

    printf("\n--- perror 和 strerror ---\n");
    demo_perror_strerror();

    printf("\n--- assert() 断言 ---\n");
    demo_assert();

    printf("\n--- 错误返回模式 ---\n");
    demo_error_return();

    printf("\n--- 信号处理 ---\n");
    demo_signal_handling();

    printf("\n--- 调试流程综合 ---\n");
    demo_debug_workflow();

    printf("\ndebugging sample done.\n");
    return 0;
}
