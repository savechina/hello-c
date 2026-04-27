/**
 * @file logging_sample.c
 * @brief 日志与格式化输出 (Logging & Formatted Output)
 *
 * Demonstrates:
 *   1. printf family (printf, fprintf, sprintf, snprintf, vprintf)
 *   2. Variadic functions with va_list (custom printf-like)
 *   3. Log macros with levels (DEBUG/INFO/WARN/ERROR)
 *   4. __FILE__, __LINE__, __func__ in debug macros
 *   5. Practical: timestamped log function
 */

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "basic/logging_sample.h"

/* ============================================================
   Section 1: printf 家族 — stdout 与 stderr
   ============================================================ */

static void demo_printf_family(void) {
    /* printf → stdout（标准输出，正常信息） */
    printf("[INFO] printf: 正常输出到 stdout\n");

    /* fprintf → 指定的 FILE* 流（stderr 用于错误/警告） */
    fprintf(stderr, "[ERROR] fprintf: 输出到 stderr（错误流）\n");
    fprintf(stdout, "[INFO] fprintf(stdout): 显式输出到 stdout\n");

    /* sprintf → 写入字符数组（危险！无边界检查） */
    char buf_sprintf[32];
    sprintf(buf_sprintf, "Hello, %s! Score: %d", "Alice", 95);
    printf("[INFO] sprintf: %s\n", buf_sprintf);

    /* snprintf → 写入字符数组（安全！指定最大长度） */
    char buf_snprintf[32];
    int used = snprintf(buf_snprintf, sizeof(buf_snprintf), "%s has %d points", "Bob", 30000);
    printf("[INFO] snprintf: %s (%d chars used)\n", buf_snprintf, used);

    /* snprintf 的安全之处：超出缓冲区时截断，不会溢出 */
    char tiny_buf[10];
    int truncated = snprintf(tiny_buf, sizeof(tiny_buf), "This is a long string!");
    printf("[INFO] snprintf truncation: \"%s\" (%d chars needed, %zu buffer)\n",
           tiny_buf, truncated, sizeof(tiny_buf));
}

/* ============================================================
   Section 2: 可变参数函数 (va_list)
   ============================================================ */

/**
 * 自定义 printf-like 函数：使用 va_list 处理可变参数
 * 模拟一个简单的日志函数
 */
static void custom_log(const char *level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stdout, "[%s] ", level);
    vfprintf(stdout, fmt, args);
    va_end(args);
}

/**
 * 用 vsnprintf 将格式化输出写入缓冲区（安全方式）
 */
static void format_to_buffer(char *dst, size_t dst_size, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(dst, dst_size, fmt, args);
    va_end(args);
}

static void demo_variadic(void) {
    printf("\n=== Variadic Functions ===\n");

    custom_log("INFO", "Server started on port %d\n", 8080);
    custom_log("WARN", "Disk usage at %.1f%%\n", 85.5);
    custom_log("ERROR", "Failed to open file \"%s\"\n", "/tmp/config.ini");

    /* vsnprintf 版本 */
    char buffer[128];
    format_to_buffer(buffer, sizeof(buffer), "User %s logged in from %s", "Alice", "192.168.1.100");
    printf("[INFO] format_to_buffer: %s\n", buffer);
}

/* ============================================================
   Section 3: 日志宏（Log Macros with Levels）
   ============================================================ */

/* 模拟日志级别：通过编译期宏控制 */
#ifndef LOG_LEVEL
#define LOG_LEVEL 3  /* 0=OFF, 1=ERROR, 2=WARN, 3=INFO, 4=DEBUG */
#endif

#define LOG_ERROR(fmt, ...) \
    do { if (LOG_LEVEL >= 1) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__); } while (0)

#define LOG_WARN(fmt, ...) \
    do { if (LOG_LEVEL >= 2) fprintf(stderr, "[WARN]  " fmt "\n", ##__VA_ARGS__); } while (0)

#define LOG_INFO(fmt, ...) \
    do { if (LOG_LEVEL >= 3) fprintf(stdout, "[INFO]  " fmt "\n", ##__VA_ARGS__); } while (0)

#define LOG_DEBUG(fmt, ...) \
    do { if (LOG_LEVEL >= 4) fprintf(stdout, "[DEBUG] " fmt "\n", ##__VA_ARGS__); } while (0)

static void demo_log_macros(void) {
    printf("\n=== Log Macros ===\n");
    printf("  Current LOG_LEVEL = %d (DEBUG=%d)\n\n", LOG_LEVEL, 4);

    LOG_ERROR("Database connection failed: %s", "timeout");
    LOG_WARN("Retry attempt %d of %d", 2, 5);
    LOG_INFO("Processing request from %s:%d", "127.0.0.1", 9999);

    /* DEBUG 级别：当前 LOG_LEVEL=3 时不会输出 */
    LOG_DEBUG("Variable x = %d, y = %d, result = %f", 42, 100, 3.14159);
}

/* ============================================================
   Section 4: __FILE__, __LINE__, __func__ 调试宏
   ============================================================ */

/* 带位置信息的 debug 宏 */
#define DEBUG_POSITION(fmt, ...) \
    fprintf(stdout, "[DEBUG] %s:%d in %s(): " fmt "\n", \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__)

/* 带位置和时间的完整 log 宏 */
#define LOG_FULL(level, fmt, ...) \
    fprintf(stdout, "[%s] %s:%d %s() " fmt "\n", \
            level, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

static void demo_location_macros(void) {
    printf("\n=== __FILE__, __LINE__, __func__ ===\n");

    DEBUG_POSITION("Entering %s, x = %d", "function_name", 99);
    DEBUG_POSITION("Buffer size = %zu bytes", (size_t)1024);
    LOG_FULL("INFO", "Configuration loaded from %s", "./app.conf");
}

/* ============================================================
   Section 5: 带时间戳的日志函数（实战）
   ============================================================ */

static void logged_printf(const char *level, const char *msg, ...) {
    /* 获取当前时间 */
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char time_buf[24];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", tm_info);

    /* 格式化输出到 stderr */
    va_list args;
    va_start(args, msg);
    fprintf(stderr, "%s [%-5s] ", time_buf, level);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
}

static void demo_timestamped_log(void) {
    printf("\n=== Timestamped Logging ===\n");

    logged_printf("INFO", "Application starting up");
    logged_printf("INFO", "Loading configuration from %s", "/etc/myapp.conf");
    logged_printf("WARN", "Deprecated function called: init_legacy()");
    logged_printf("ERROR", "Cannot connect to database at %s:%d", "localhost", 5432);
}

/* ============================================================
Section 6: snprintf 安全示例 — 对比 sprintf 风险
 ============================================================ */

static void demo_snprintf_safety(void) {
    printf("\n=== snprintf Safety vs sprintf Risk ===\n");

    /* 危险：sprintf 没有边界检查，缓冲区过小会溢出 */
    char unsafe_buf[8];
    printf("  snprintf 安全版本（指定最大长度 = %zu）：\n", sizeof(unsafe_buf));
    int needed = snprintf(unsafe_buf, sizeof(unsafe_buf), "Hello, World!");
    printf("    实际写入: \"%s\"\n", unsafe_buf);
    printf("    实际需要: %d 字符，缓冲区只有 %zu\n", needed, sizeof(unsafe_buf));
    printf("    没有内存越界！snprintf 保护了缓冲区。\n");
}

/* ============================================================
   公共入口
   ============================================================ */

int main_logging_sample(void) {
    printf("==== 日志与格式化输出 (Logging & Formatted Output) ====\n\n");

    printf("--- printf 家族 ---\n");
    demo_printf_family();

    printf("\n--- 可变参数函数 ---\n");
    demo_variadic();

    printf("\n--- 日志级别宏 ---\n");
    demo_log_macros();

    printf("\n--- __FILE__ / __LINE__ / __func__ ---\n");
    demo_location_macros();

    printf("\n--- 带时间戳的日志 ---\n");
    demo_timestamped_log();

    printf("\n--- snprintf 安全对比 ---\n");
    demo_snprintf_safety();

    printf("\nlogging sample done.\n");
    return 0;
}
