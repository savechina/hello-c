/**
 * @file error_handling_sample.c
 * @brief 错误处理 (Error Handling) — Advance tutorial chapter
 *
 * Demonstrates:
 *   1. errno pattern + errno.h
 *   2. perror / strerror
 *   3. setjmp / longjmp for error recovery
 *   4. Error callback chains
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>
#include <math.h>

#include "advance/error_handling_sample.h"

/* ============================================================
   Section 1: errno 模式
   ============================================================ */

/**
 * errno 是线程局部变量，库函数出错时自动设置。
 * 使用前必须清零——否则无法区分旧错误和新错误。
 */
static void demo_errno(void)
{
    printf("  [errno] 演示 errno 模式:\n");

    /* 步骤 1: 清零 errno */
    errno = 0;

    /* 步骤 2: 触发错误 — sqrt(-1) 产生 domain error */
    double result = sqrt(-1.0);
    printf("  [errno] sqrt(-1.0) = %f\n", result);
    printf("  [errno] errno = %d\n", errno);

    if (errno != 0) {
        printf("  [errno] 检测到错误! errno = %d\n", errno);
    }

    /* 演示常见 errno 常量 */
    errno = 0;
    FILE *fp = fopen("/nonexistent/path/missing.txt", "r");
    if (fp == NULL) {
        printf("  [errno] fopen 失败: errno = %d (ENOENT = 文件不存在)\n", errno);
    }

    errno = 0;
    long val = strtol("not_a_number", NULL, 10);
    if (errno != 0) {
        printf("  [errno] strtol 失败: errno = %d (EINVAL = 无效参数)\n", errno);
    } else {
        (void)val;
    }
}

/* Error return codes for config parser demo */
static const int ERR_OK       =  0;
static const int ERR_OPEN     = -1;

/**
 * 模拟一个会返回错误码的函数
 *
 * 使用 -Werror 时，unused 静态函数会报错，
 * 这里用 main entry 里的调用来避免。
 */
static int parse_config(const char *path, int *value)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return ERR_OPEN;
    }

    /* 模拟: 假设文件内容是合法的 */
    if (value != NULL) {
        *value = 42;
    }
    fclose(fp);
    return ERR_OK;
}

/**
 * 未检查返回值的错误示范（用注释展示，不实际调用避免警告）
 *
 * 假设我们这样写：
 *
 *     int val;
 *     parse_config("/etc/app.conf", &val);  // ← 没检查返回值!
 *     printf("config value = %d\n", val);   // ← val 可能是未初始化的垃圾!
 *
 * 这就是「沉默失败」——parse_config 内部打开了文件失败，但我们不知情，
 * 继续用未初始化的 val →  undefined behavior。
 *
 * 正确的写法见 demo_error_first 函数。
 */

/**
 * 错误优先 (Error-first): 先检查返回值，再使用结果
 */
static void demo_error_first(void)
{
    printf("\n  [error-first] 错误优先模式:\n");

    int val = 0;
    int rc  = parse_config("/etc/app.conf", &val);

    if (rc != ERR_OK) {
        printf("  [error-first] parse_config 失败, rc = %d\n", rc);
        printf("  [error-first] 错误处理路径: 使用默认配置\n");
        val = 100;  /* 回退到安全默认值 */
    }

    printf("  [error-first] 最终 val = %d\n", val);
}

/* ============================================================
   Section 2: perror / strerror
   ============================================================ */

/**
 * perror: 自动拼接前缀字符串 + errno 对应的英文错误文本
 * strerror: 返回错误码对应的字符串（可用于自定义日志）
 */
static void demo_perror_strerror(void)
{
    printf("\n  [error-msg] perror vs strerror:\n");

    /* -- perror: 一键打印 + 自动换行 -- */
    errno = 0;
    FILE *fp2 = fopen("/tmp/nonexistent_file_xyz.txt", "r");
    if (fp2 == NULL) {
        perror("  [error-msg] fopen failed");
        /* 输出:   [error-msg] fopen failed: No such file or directory */
    }

    /* -- strerror: 返回 char* 供我们自由使用 -- */
    printf("  [error-msg] strerror(errno): %s\n", strerror(errno));

    /* 直接查常见错误码 */
    printf("  [error-msg] errno 2  → %s\n", strerror(2));
    printf("  [error-msg] errno 13 → %s\n", strerror(13));
    printf("  [error-msg] errno 22 → %s\n", strerror(22));

    if (fp2 != NULL) {
        fclose(fp2);
    }
}

/* ============================================================
   Section 3: setjmp / longjmp — 错误恢复机制
   ============================================================ */

/* --- Demo A: 基础 setjmp/longjmp --- */

/**
 * setjmp / longjmp 是 C 语言中唯一可以「跨函数跳转」的非本地跳转机制。
 *
 * 类比: setjmp 是「存档点」，longjmp 是「读档回到存档点」。
 *
 * 使用场景:
 *   - 深层嵌套函数中发生错误，需要一次性清理回到顶层
 *   - 避免在每个层级写 if (error) return ... 的错误传播代码
 *
 * 警告:
 *   - longjmp 跳过中间栈帧的析构/清理代码 — 内存可能泄漏
 *   - 不要用它做正常控制流，只做错误恢复
 */

static jmp_buf g_env;
static int g_step = 0;

static void deep_function_a(void)
{
    g_step = 1;
    printf("  [setjmp] deep_function_a: step=%d\n", g_step);
    /* 模拟: 遇到严重错误，longjmp 跳回 */
    printf("  [setjmp] deep_function_a: 发生错误! longjmp 跳回...\n");
    longjmp(g_env, 1);  /* 跳回 setjmp 处, setjmp 返回 1 */
    /* 这行代码不会被执行 */
}

static void demo_setjmp_longjmp(void)
{
    printf("\n  [setjmp] 基础 setjmp / longjmp:\n");

    int ret = setjmp(g_env);

    if (ret == 0) {
        /* 直接执行路径 (setjmp 返回 0) */
        printf("  [setjmp] setjmp 首次调用，进入正常路径\n");
        deep_function_a();
        printf("  [setjmp] deep_function_a 已返回 (不应看到这个!)\n");
    } else {
        /* 错误恢复路径 (longjmp 后, setjmp 返回 longjmp 的第二个参数) */
        printf("  [setjmp] longjmp 回跳! setjmp 返回 = %d\n", ret);
        printf("  [setjmp] 进入错误恢复路径\n");
    }

    printf("  [setjmp] 继续执行 setjmp/longjmp 之后的代码\n");
}

/* --- Demo B: 多层调用 + 错误码传递 --- */

static jmp_buf g_env2;

static void layer_c(void)
{
    printf("  [setjmp2] layer_c: 打开资源 A...\n");
    /* 模拟: layer_c 中发生致命错误 */
    printf("  [setjmp2] layer_c: 致命错误! longjmp(2)\n");
    longjmp(g_env2, 2);  /* 错误码 2 = layer_c 错误 */
}

static void layer_b(void)
{
    printf("  [setjmp2] layer_b: 调用 layer_c...\n");
    layer_c();
    printf("  [setjmp2] layer_b: 永远不会到这里\n");
}

static void layer_a(void)
{
    printf("  [setjmp2] layer_a: 调用 layer_b...\n");
    layer_b();
}

static void demo_setjmp_multilayer(void)
{
    printf("\n  [setjmp2] 多层 longjmp 传递:\n");

    int ret = setjmp(g_env2);

    if (ret == 0) {
        printf("  [setjmp2] 正常路径: 调用 layer_a...\n");
        layer_a();
    } else {
        /* 直接从 layer_c 跳回，跳过 layer_b 和 layer_a */
        printf("  [setjmp2] 错误回跳! longjmp 回传错误码 = %d\n", ret);
        switch (ret) {
            case 1: printf("  [setjmp2] → 错误来源: layer_a\n"); break;
            case 2: printf("  [setjmp2] → 错误来源: layer_b/c\n"); break;
            default: printf("  [setjmp2] → 未知错误码\n"); break;
        }
    }
}

/* ============================================================
   Section 4: 错误回调链 (Error Callback Chains)
   ============================================================ */

/**
 * 错误回调链: 注册一组回调函数，在错误发生时依次调用。
 *
 * 类比: 就像医院的多级转诊——基层医生处理不了，转诊给专科医生，
 * 专科医生也处理不了，转诊给上级医院。每一级都有机会处理或继续传递。
 *
 * 应用场景:
 *   - 日志记录 + 用户提示 + 资源清理，各走各的回调
 *   - 可扩展的错误处理管道: 新模块注册新回调即可
 */

#define MAX_ERROR_CALLBACKS 8

typedef void (*error_callback_fn)(int error_code, const char *message, void *user_data);

typedef struct {
    error_callback_fn fn;
    void             *user_data;
    const char       *name;
} ErrorCallback;

typedef struct {
    ErrorCallback callbacks[MAX_ERROR_CALLBACKS];
    int           count;
} ErrorHandler;

static ErrorHandler g_handler = { .count = 0 };

/**
 * 注册一个错误回调
 * 返回 0 = 成功, -1 = 回调链已满
 */
static int register_error_callback(error_callback_fn fn, void *user_data,
                                   const char *name)
{
    if (g_handler.count >= MAX_ERROR_CALLBACKS) {
        fprintf(stderr, "  [callback] 错误: 回调链已满 (%d)\n", MAX_ERROR_CALLBACKS);
        return -1;
    }

    ErrorCallback *cb = &g_handler.callbacks[g_handler.count];
    cb->fn        = fn;
    cb->user_data = user_data;
    cb->name      = name;
    g_handler.count++;

    printf("  [callback] 注册回调: %s\n", name);
    return 0;
}

/**
 * 触发错误回调链: 依次调用所有已注册的回调
 */
static void trigger_error_chain(int error_code, const char *message)
{
    printf("\n  [callback] === 触发错误回调链 (code=%d, msg=\"%s\") ===\n",
           error_code, message);

    for (int i = 0; i < g_handler.count; i++) {
        ErrorCallback *cb = &g_handler.callbacks[i];
        printf("  [callback] → 调用: %s\n", cb->name);
        cb->fn(error_code, message, cb->user_data);
    }
}

/**
 * 清理回调链
 */
static void clear_error_callbacks(void)
{
    g_handler.count = 0;
    memset(g_handler.callbacks, 0, sizeof(g_handler.callbacks));
}

/* --- 回调函数实现 --- */

/** 回调 1: 日志记录 */
static void log_callback(int error_code, const char *message, void *user_data)
{
    (void)user_data;
    fprintf(stderr, "  [callback|LOG] 错误 #%d: %s\n", error_code, message);
}

/** 回调 2: 用户通知 */
static void notify_callback(int error_code, const char *message, void *user_data)
{
    const char *username = (const char *)user_data;
    if (username != NULL) {
        printf("  [callback|NOTIFY] 通知用户 \"%s\": 发生了错误 #%d — %s\n",
               username, error_code, message);
    }
}

/** 回调 3: 资源清理 */
static void cleanup_callback(int error_code, const char *message, void *user_data)
{
    (void)error_code;
    (void)message;
    (void)user_data;
    printf("  [callback|CLEANUP] 正在执行资源清理...\n");
    /* 在实际项目中: 关闭文件、释放内存、断开连接等 */
}

/** 回调 4: 条件拦截 — 错误码 >= 100 时中止程序 */
static void critical_callback(int error_code, const char *message, void *user_data)
{
    (void)user_data;
    printf("  [callback|CRITICAL] 严重错误 #%d: %s\n", error_code, message);
    if (error_code >= 100) {
        printf("  [callback|CRITICAL] 错误码 >= 100, 中止程序\n");
        exit(EXIT_FAILURE);
    }
    printf("  [callback|CRITICAL] 错误码 < 100, 允许继续\n");
}

static void demo_callback_chain(void)
{
    printf("\n  [callback] === 错误回调链演示 ===\n");

    /* 清理上一次演示的残留 */
    clear_error_callbacks();

    /* 注册回调 */
    register_error_callback(log_callback,     NULL,            "log");
    register_error_callback(notify_callback,  (void *)"Alice", "notify");
    register_error_callback(cleanup_callback, NULL,            "cleanup");

    /* 触发: 普通错误 */
    trigger_error_chain(42, "配置文件解析失败");

    /* 动态注册新回调 */
    register_error_callback(critical_callback, NULL, "critical");

    /* 触发: 带 critical 回调 */
    trigger_error_chain(10, "配置使用默认值");
}

/* ============================================================
   公共入口
   ============================================================ */

int main_error_handling_sample(void)
{
    printf("==== 错误处理 (Error Handling) ====\n\n");

    /* Section 1: errno */
    printf("--- errno 模式 ---\n");
    demo_errno();

    /* Section 1b: Error-first */
    printf("\n--- 错误优先 (Error-first) ---\n");
    demo_error_first();

    /* Section 2: perror / strerror */
    printf("\n--- perror / strerror ---\n");
    demo_perror_strerror();

    /* Section 3: setjmp/longjmp */
    printf("\n--- setjmp / longjmp ---\n");
    demo_setjmp_longjmp();
    demo_setjmp_multilayer();

    /* Section 4: Callback chains */
    demo_callback_chain();

    printf("\nerror handling sample done.\n");
    return 0;
}
