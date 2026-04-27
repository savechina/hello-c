#include <stdio.h>
#include <stdint.h>
#include "basic/conditional_comp_sample.h"

static void platform_detection_sample(void) {
    printf("----basic ... platform_detection_sample.....start ----\n");
#ifdef __APPLE__
    printf("  当前平台: macOS (Darwin)\n");
    printf("  __APPLE__ 已定义 ✓\n");
#elif defined(__linux__)
    printf("  当前平台: Linux\n");
    printf("  __linux__ 已定义 ✓\n");
#elif defined(__FreeBSD__)
    printf("  当前平台: FreeBSD\n");
    printf("  __FreeBSD__ 已定义 ✓\n");
#else
    printf("  当前平台: 未知 (其他类 Unix 系统)\n");
#endif
    printf("----basic ... platform_detection_sample.....done ----\n");
}

static void debug_release_sample(void) {
    printf("----basic ... debug_release_sample..... start ----\n");

#ifndef NDEBUG
    printf("  编译模式: DEBUG (NDEBUG 未定义)\n");
    printf("  调试信息: 已启用\n");
#else
    printf("  编译模式: RELEASE (NDEBUG 已定义)\n");
    printf("  调试信息: 已禁用\n");
#endif

    /* 自定义调试宏 */
#ifdef DEBUG_VERBOSE
    printf("  详细调试: verbose 模式已启用\n");
#else
    printf("  详细调试: 关闭 (定义 -DDEBUG_VERBOSE 启用)\n");
#endif

    printf("----basic ... debug_release_sample.....done ----\n");
}

static void feature_test_sample(void) {
    printf("----basic ... feature_test_sample.....start ----\n");

#ifdef _GNU_SOURCE
    printf("  _GNU_SOURCE: 已定义 (GNU 扩展可用)\n");
#else
    printf("  _GNU_SOURCE: 未定义 (标准 POSIX 模式)\n");
#endif

#ifdef _POSIX_C_SOURCE
    printf("  _POSIX_C_SOURCE: %ld\n", (long)_POSIX_C_SOURCE);
#else
    printf("  _POSIX_C_SOURCE: 未定义\n");
#endif

    printf("----basic ... feature_test_sample.....done ----\n");
}

static void conditional_compile_sample(void) {
    printf("----basic ... conditional_compile_sample.....start ----\n");

    /* 条件编译: 根据平台选择不同代码路径 */
    int val = 42;
    (void)val;
#if defined(__LP64__) || defined(_LP64)
    printf("  指针宽度: 64 位 (LP64)\n");
    printf("  long = %zu 位\n", sizeof(long) * 8);
#else
    printf("  指针宽度: 32 位\n");
    printf("  long = %zu 位\n", sizeof(long) * 8);
#endif

    /* 编译时常量检查 */
#if INT_MAX > 2147483640
    printf("  int 范围: 足够大 (INT_MAX = %d)\n", INT_MAX);
#else
    printf("  警告: int 范围较小\n");
#endif

    printf("----basic ... conditional_compile_sample.....done ----\n");
}

int main_conditional_comp_sample(void) {
    printf("=== 条件编译（Conditional Compilation）演示 ===\n\n");
    platform_detection_sample();
    printf("\n");
    debug_release_sample();
    printf("\n");
    feature_test_sample();
    printf("\n");
    conditional_compile_sample();
    printf("\n");
    return 0;
}
