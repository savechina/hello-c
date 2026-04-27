/**
 * @file testing_sample.c
 * @brief 测试框架 (Testing Framework) — Advance tutorial chapter
 *
 * Demonstrates:
 *   1. Custom ASSERT macros with file/line info
 *   2. Test runner, test cases, pass/fail reporting
 *   3. Mock functions via function pointer injection
 *   4. Test organization patterns
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <time.h>
#include "advance/testing_sample.h"

/* ============================================================
   Section 1: Error-First — "test failed" without context
   ============================================================ */

/**
 * 反面教材：一个朴素的断言，只打印 "test failed"。
 * 没有任何文件/行号/期望值/实际值信息。
 * 读者：「哪里失败了？为什么失败？我该怎么修？」
 */
static void demo_naive_assert(void)
{
    printf("=== Section 1: 反面教材 — 无上下文断言 ===\n");

    int actual = 3;
    int expected = 5;

    /* 这就是问题：只有「失败」两个字 */
    if (actual != expected) {
        printf("  反面教材: test failed\n");
        /* ↑ 读完这句话，你知道该改哪一行代码吗？不知道。 */
    }

    /* 对比下面 sections 的断言，差别一目了然。 */
}

/* ============================================================
   Section 2: Custom ASSERT 宏 — __FILE__ + __LINE__
   ============================================================ */

/**
 * 第一个版本的自定义断言——包含文件名和行号。
 * 就像工厂质检员在报告上写「3号工位, 第15件产品」一样精准。
 */
#define ASSERT_EQ_V1(actual, expected) do {                          \
    int64_t _a = (int64_t)(actual);                                  \
    int64_t _e = (int64_t)(expected);                                \
    if (_a != _e) {                                                  \
        printf("  ❌ ASSERT_EQ failed at %s:%d\n", __FILE__, __LINE__); \
        printf("     期望: %" PRId64 ", 实际: %" PRId64 "\n", _e, _a);   \
    } else {                                                         \
        printf("  ✅ ASSERT_EQ at %s:%d PASS\n", __FILE__, __LINE__);   \
    }                                                                \
} while (0)

#define ASSERT_TRUE_V1(cond) do {                                    \
    if (!(cond)) {                                                   \
        printf("  ❌ ASSERT_TRUE failed at %s:%d\n", __FILE__, __LINE__); \
    } else {                                                         \
        printf("  ✅ ASSERT_TRUE at %s:%d PASS\n", __FILE__, __LINE__);  \
    }                                                                \
} while (0)

#define ASSERT_NULL_V1(ptr) do {                                     \
    if ((ptr) != NULL) {                                             \
        printf("  ❌ ASSERT_NULL failed at %s:%d\n", __FILE__, __LINE__); \
    } else {                                                         \
        printf("  ✅ ASSERT_NULL at %s:%d PASS\n", __FILE__, __LINE__);  \
    }                                                                \
} while (0)

/**
 * 演示 V1 断言的效果
 */
static void demo_assert_v1(void)
{
    printf("\n=== Section 2: 自定义 ASSERT 宏 (包含行号) ===\n");

    ASSERT_EQ_V1(3 + 2, 5);
    ASSERT_TRUE_V1(10 > 3);
    ASSERT_NULL_V1(NULL);

    /* 故意失败一个，观察输出的行号信息 */
    ASSERT_EQ_V1(2 * 2, 5);  /* 会失败: 期望5, 实际4 */
}

/* ============================================================
   Section 3: 增强的 ASSERT — 彩色输出 + 自定义消息
   ============================================================ */

/* 终端颜色常量 */
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_RED    "\033[0;31m"
#define COLOR_RESET  "\033[0m"

/*
 * V2: 彩色输出。
 */
#define ASSERT_EQ(actual, expected) do {                             \
    int64_t _a = (int64_t)(actual);                                  \
    int64_t _e = (int64_t)(expected);                                \
    if (_a != _e) {                                                  \
        printf("  " COLOR_RED "❌ FAIL at %s:%d: " COLOR_RESET       \
               "期望 %" PRId64 " != 实际 %" PRId64 "\n",             \
               __FILE__, __LINE__, _e, _a);                          \
    } else {                                                         \
        printf("  " COLOR_GREEN "✅ PASS" COLOR_RESET                \
               " at %s:%d: 值=%" PRId64 "\n",                        \
               __FILE__, __LINE__, _a);                              \
    }                                                                \
} while (0)

/**
 * 演示 V2 效果
 */
static void demo_assert_v2(void)
{
    printf("\n=== Section 3: 增强 ASSERT (彩色 + 消息) ===\n");

    ASSERT_EQ(100, 100);
    ASSERT_EQ(42, 42);
    ASSERT_EQ(-1, -1);
    ASSERT_EQ(0, 1);  /* 故意失败 */
}

/* ============================================================
   Section 4: Test Framework — Test Case 注册 + 计数器
   ============================================================ */

/**
 * Test framework 核心: test case struct + global runner state
 *
 * 类比: 工厂质检线的自动化台——
 *   - TestCase  = 一张质检任务卡（测什么、怎么测）
 *   - TestRunner = 自动传送带（依次执行任务卡）
 *   - 计数器     = 流水线统计看板（多少合格/不合格）
 */

typedef struct {
    const char *name;
    void (*func)(void);
} TestCase;

typedef struct {
    int total;
    int passed;
    int failed;
    TestCase cases[64];  /* 最多 64 个测试用例 */
} TestRunner;

static TestRunner g_runner;

static void runner_init(void)
{
    memset(&g_runner, 0, sizeof(g_runner));
}

/**
 * 注册一个测试用例
 */
static void runner_add(const char *name, void (*func)(void))
{
    if (g_runner.total >= 64) {
        printf("  错误: 测试用例已满 (%d)\n", 64);
        return;
    }
    TestCase *tc = &g_runner.cases[g_runner.total];
    tc->name = name;
    tc->func = func;
    g_runner.total++;
}

/**
 * 运行所有注册的测试用例
 * 在每个测试函数内部，使用 ASSERT_* 判断 pass/fail
 * runner 根据「全局失败标志」判断每个 case 是否 pass
 */
static int g_current_case_failed;

static void runner_run_all(void)
{
    printf("\n  ========== 测试运行器 ==========\n");

    for (int i = 0; i < g_runner.total; i++) {
        TestCase *tc = &g_runner.cases[i];

        g_current_case_failed = 0;
        printf("  运行 [%d/%d]: %s ... ", i + 1, g_runner.total, tc->name);
        fflush(stdout);

        tc->func();

        if (g_current_case_failed) {
            g_runner.failed++;
            printf(COLOR_RED "FAIL" COLOR_RESET "\n");
        } else {
            g_runner.passed++;
            printf(COLOR_GREEN "PASS" COLOR_RESET "\n");
        }
    }

    printf("  ================================\n");
    printf("  总计: %d | 通过: %d | 失败: %d\n",
           g_runner.total, g_runner.passed, g_runner.failed);

    if (g_runner.failed == 0) {
        printf("  " COLOR_GREEN "🎉 所有测试通过!" COLOR_RESET "\n");
    } else {
        printf("  " COLOR_RED "⚠️  %d 个测试失败" COLOR_RESET "\n",
               g_runner.failed);
    }
}

/* ============================================================
   Section 5: 测试框架内的 ASSERT 宏（带 runner 计数）
   ============================================================ */

#define ASSERT_EQ_RUN(actual, expected) do {                         \
    int64_t _a = (int64_t)(actual);                                  \
    int64_t _e = (int64_t)(expected);                                \
    if (_a != _e) {                                                  \
        printf("%s:%d: ASSERT_EQ 失败: 期望 %" PRId64                \
               ", 实际 %" PRId64 "\n",                               \
               __FILE__, __LINE__, _e, _a);                          \
        g_current_case_failed = 1;                                   \
    }                                                                \
} while (0)

#define ASSERT_TRUE_RUN(cond) do {                                   \
    if (!(cond)) {                                                   \
        printf("%s:%d: ASSERT_TRUE 失败\n",                          \
               __FILE__, __LINE__);                                  \
        g_current_case_failed = 1;                                   \
    }                                                                \
} while (0)

/* ============================================================
   Section 6: 被测函数 (Functions Under Test)
   ============================================================ */

/**
 * 一个简单的计算器函数——待测试的代码
 */
static int calc_add(int a, int b)
{
    return a + b;
}

static int calc_multiply(int a, int b)
{
    return a * b;
}

/**
 * 边界检查: 返回值必须在 [0, 10000] 之间（简化验证）
 */
static int calc_is_valid(int result)
{
    return (result >= 0 && result <= 10000) ? 1 : 0;
}

/* ============================================================
   Section 7: 编写测试用例
   ============================================================ */

static void test_calc_add_basic(void)
{
    ASSERT_EQ_RUN(calc_add(2, 3), 5);
    ASSERT_EQ_RUN(calc_add(0, 0), 0);
    ASSERT_EQ_RUN(calc_add(-1, 1), 0);
}

static void test_calc_multiply_basic(void)
{
    ASSERT_EQ_RUN(calc_multiply(3, 4), 12);
    ASSERT_EQ_RUN(calc_multiply(0, 100), 0);
    ASSERT_EQ_RUN(calc_multiply(-2, 3), -6);
}

static void test_calc_valid_range(void)
{
    ASSERT_TRUE_RUN(calc_is_valid(0) == 1);
    ASSERT_TRUE_RUN(calc_is_valid(10000) == 1);
    ASSERT_TRUE_RUN(calc_is_valid(-1) == 0);
    ASSERT_TRUE_RUN(calc_is_valid(10001) == 0);
}

/**
 * 演示 Section 4 + 5 + 7 的集成效果
 */
static void demo_test_runner(void)
{
    printf("\n=== Section 4-7: 测试框架集成演示 ===\n");

    runner_init();
    runner_add("加法基础",     test_calc_add_basic);
    runner_add("乘法基础",     test_calc_multiply_basic);
    runner_add("有效范围检查", test_calc_valid_range);
    runner_run_all();
}

/* ============================================================
   Section 8: Mock 函数 — 函数指针注入
   ============================================================ */

/**
 * Mock: 测试中用假数据替换真实依赖（如 I/O、网络、数据库）
 *
 * 核心思路: 把函数指针存储在全局变量中，生产代码默认指向真实函数。
 * 测试时，把指针指向 mock 函数，注入可控的返回值。
 *
 * 工厂类比: 生产线上的传感器（真实函数）坏了，质检员用
 * 「模拟传感器」(mock) 提供标准信号，继续测试后续流程。
 */

/* --- 被模拟的函数签名 --- */
typedef int (*ReadSensorFn)(void);
typedef int (*WriteActuatorFn)(int value);

/* --- 全局函数指针: 默认指向真实实现 --- */
static ReadSensorFn    g_read_sensor    = NULL;
static WriteActuatorFn g_write_actuator = NULL;

/* --- 真实实现 (生产代码) --- */
static int real_read_sensor(void)
{
    /* 在实际项目中: 读硬件传感器 */
    return 42;
}

static int real_write_actuator(int value)
{
    /* 在实际项目中: 写执行器 */
    printf("    [实际] 执行器设置为 %d\n", value);
    return 0;  /* 0 = OK */
}

/* --- Mock 实现 (测试代码) --- */
static int mock_read_sensor_value = 99;

static int mock_read_sensor(void)
{
    return mock_read_sensor_value;  /* 返回测试设定的值 */
}

static int mock_write_actuator_called = 0;
static int mock_write_actuator_last_value = -1;

static int mock_write_actuator(int value)
{
    mock_write_actuator_called    = 1;
    mock_write_actuator_last_value = value;
    return 0;
}

/* --- 依赖注入: 设置 mock --- */
static void system_set_mock_read(int value)
{
    mock_read_sensor_value      = value;
    g_read_sensor               = mock_read_sensor;
}

static void system_set_mock_write(void)
{
    mock_write_actuator_called     = 0;
    mock_write_actuator_last_value = -1;
    g_write_actuator               = mock_write_actuator;
}

/* --- 恢复到真实实现 --- */
static void system_restore_real(void)
{
    g_read_sensor    = real_read_sensor;
    g_write_actuator = real_write_actuator;
}

/* --- 被测系统: 读取传感器并控制执行器 --- */
static int system_process(void)
{
    int sensor_val = g_read_sensor();

    if (sensor_val > 50) {
        return g_write_actuator(100);  /* 高温 → 全速运行 */
    } else if (sensor_val > 25) {
        return g_write_actuator(50);   /* 中温 → 半速运行 */
    } else {
        return g_write_actuator(0);    /* 低温 → 停止 */
    }
}

/* ============================================================
   Section 9: Mock 测试用例
   ============================================================ */

static void test_mock_high_temp(void)
{
    system_set_mock_read(80);
    system_set_mock_write();

    int rc = system_process();

    ASSERT_EQ_RUN(rc, 0);
    ASSERT_TRUE_RUN(mock_write_actuator_called == 1);
    ASSERT_EQ_RUN(mock_write_actuator_last_value, 100);
}

static void test_mock_mid_temp(void)
{
    system_set_mock_read(40);
    system_set_mock_write();

    int rc = system_process();

    ASSERT_EQ_RUN(rc, 0);
    ASSERT_EQ_RUN(mock_write_actuator_last_value, 50);
}

static void test_mock_low_temp(void)
{
    system_set_mock_read(10);
    system_set_mock_write();

    int rc = system_process();

    ASSERT_EQ_RUN(rc, 0);
    ASSERT_EQ_RUN(mock_write_actuator_last_value, 0);
}

static void demo_mock_injection(void)
{
    printf("\n=== Section 8-9: Mock 函数注入演示 ===\n");

    runner_init();
    runner_add("Mock: 高温场景", test_mock_high_temp);
    runner_add("Mock: 中温场景", test_mock_mid_temp);
    runner_add("Mock: 低温场景", test_mock_low_temp);

    /* 确保函数指针初始化 */
    g_read_sensor    = real_read_sensor;
    g_write_actuator = real_write_actuator;

    runner_run_all();

    /* 恢复到真实实现 */
    system_restore_real();
}

/* ============================================================
   Section 10: 测试夹具 (Test Fixtures) — setup / teardown
   ============================================================ */

/**
 * 测试夹具: 每个测试运行前/后执行的通用代码
 *
 * 工厂类比: 每批质检前都要「校准仪器」(setup),
 * 质检后都要「清理工作台」(teardown)。
 */

static int g_fixture_initialized = 0;
static int g_resource_count      = 0;

static void fixture_setup(void)
{
    g_fixture_initialized = 1;
    g_resource_count = 0;
}

static void fixture_teardown(void)
{
    g_fixture_initialized = 0;
    /* 模拟: 清理所有分配的资源 */
    printf("    [teardown] 清理了 %d 个资源\n", g_resource_count);
    g_resource_count = 0;
}

/* 模拟资源分配 */
static int fixture_allocate(void)
{
    if (!g_fixture_initialized) {
        printf("    错误: 未在 fixture 上下文中分配资源!\n");
        return -1;
    }
    g_resource_count++;
    return g_resource_count;
}

static void test_fixture_resource(void)
{
    fixture_setup();

    int id = fixture_allocate();
    ASSERT_EQ_RUN(id, 1);

    id = fixture_allocate();
    ASSERT_EQ_RUN(id, 2);

    ASSERT_TRUE_RUN(g_fixture_initialized == 1);
    ASSERT_TRUE_RUN(g_resource_count == 2);

    fixture_teardown();
}

static void demo_test_fixtures(void)
{
    printf("\n=== Section 10: 测试夹具 (setup/teardown) ===\n");

    runner_init();
    runner_add("夹具资源分配", test_fixture_resource);
    runner_run_all();
}

/* ============================================================
   Section 11: 测试组织 — 按模块分组运行
   ============================================================ */

/**
 * 将测试按模块/功能分组运行。
 * 每个 group 有自己的 setup/teardown。
 */

typedef void (*TestGroupRunner)(void);

typedef struct {
    const char *group_name;
    void (*setup)(void);
    void (*teardown)(void);
    void (*run_tests)(void);
} TestGroup;

static void demo_test_groups(void)
{
    printf("\n=== Section 11: 测试分组 ===\n");

    /* Group 1: 数学运算 */
    printf("  === 分组: 数学运算 ===\n");
    runner_init();
    runner_add("加法",        test_calc_add_basic);
    runner_add("乘法",        test_calc_multiply_basic);
    runner_add("有效范围",    test_calc_valid_range);
    runner_run_all();

    printf("\n");

    /* Group 2: Mock 场景 */
    printf("  === 分组: Mock 场景 ===\n");
    runner_init();
    runner_add("高温",        test_mock_high_temp);
    runner_add("中温",        test_mock_mid_temp);
    runner_add("低温",        test_mock_low_temp);
    g_read_sensor    = real_read_sensor;
    g_write_actuator = real_write_actuator;
    runner_run_all();
}

/* ============================================================
   Section 12: 性能测试 — 计时基础
   ============================================================ */

static void demo_timing_test(void)
{
    printf("\n=== Section 12: 性能测试 (计时) ===\n");

    clock_t start = clock();

    /* 模拟一段密集计算 */
    volatile int64_t sum = 0;
    for (int64_t i = 0; i < 1000000; i++) {
        sum += i;
    }

    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;

    printf("  计算 sum(0..999999) = %" PRId64 "\n", sum);
    printf("  耗时: %.2f ms\n", elapsed);
    printf("  提示：性能测试需要设定阈值, 超过阈值则 FAIL\n");
}

/* ============================================================
   Section 13: 参数化测试 — 同一测试逻辑, 多组输入
   ============================================================ */

typedef struct {
    int input_a;
    int input_b;
    int expected;
} AddCase;

static void demo_parameterized_test(void)
{
    printf("\n=== Section 13: 参数化测试 ===\n");

    AddCase cases[] = {
        { 0,  0,  0 },
        { 1,  2,  3 },
        { -5, 5,  0 },
        { -10, -20, -30 },
        { 100, 200, 300 },
    };
    int num_cases = sizeof(cases) / sizeof(cases[0]);

    int passed_count = 0;
    int failed_count = 0;

    for (int i = 0; i < num_cases; i++) {
        AddCase *c = &cases[i];
        int result = calc_add(c->input_a, c->input_b);
        if (result == c->expected) {
            passed_count++;
            printf("  ✅ [%d/%d] calc_add(%d, %d) = %d PASS\n",
                   i + 1, num_cases, c->input_a, c->input_b, result);
        } else {
            failed_count++;
            printf("  ❌ [%d/%d] calc_add(%d, %d) = %d, 期望 %d FAIL\n",
                   i + 1, num_cases, c->input_a, c->input_b, result,
                   c->expected);
        }
    }

    printf("  参数化测试: %d 通过, %d 失败 / %d 总计\n",
           passed_count, failed_count, num_cases);
}

/* ============================================================
   Section 14: 跳过条件 — 平台/配置相关的测试
   ============================================================ */

static void demo_conditional_skip(void)
{
    printf("\n=== Section 14: 条件跳过测试 ===\n");

    /* 模拟: 某些测试只在特定平台/配置下运行 */
    const char *platform =
#ifdef __linux__
        "linux";
#else
        "other";
#endif

    printf("  当前平台: %s\n", platform);
    printf("  Linux 特定测试: 跳过 (当前不是 Linux)\n");
    printf("  通用测试: 运行\n");
    printf("  提示: 使用 #ifdef 控制测试编译, 而非运行时跳过\n");
}

/* ============================================================
   Section 15: 测试报告 — 结构化输出
   ============================================================ */

static void demo_test_reporting(void)
{
    printf("\n=== Section 15: 结构化测试报告 ===\n");

    printf("  测试报告格式 (可导出为 TAP/JSON):\n");
    printf("  TAP 格式:\n");
    printf("    TAP version 13\n");
    printf("    1..3\n");
    printf("    ok 1 - 加法基础\n");
    printf("    ok 2 - 乘法基础\n");
    printf("    not ok 3 - 无效范围 (期望 1, 实际 0)\n");
    printf("\n");
    printf("  实际项目中, 可以将上述输出重定向到文件,\n");
    printf("  交给 CI 系统解析 (如 GitHub Actions, Jenkins)。\n");
}

/* ============================================================
   公共入口
   ============================================================ */

int main_testing_sample(void)
{
    printf("============================================\n");
    printf("  测试框架 (Testing Framework) 🟡\n");
    printf("============================================\n");

    demo_naive_assert();
    demo_assert_v1();
    demo_assert_v2();
    demo_test_runner();
    demo_mock_injection();
    demo_test_fixtures();
    demo_test_groups();
    demo_timing_test();
    demo_parameterized_test();
    demo_conditional_skip();
    demo_test_reporting();

    system_restore_real();
    printf("\ntesting sample done.\n");
    return 0;
}
