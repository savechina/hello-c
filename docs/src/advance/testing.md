# 测试框架 (Testing Framework) 🟡

## 开篇故事

想象你是一家工厂的质量检验员。流水线上生产的产品需要逐个检验——尺寸对不对、颜色对不对、结构严不严。你不可能只看一眼就说「不合格」，你得告诉生产线：**哪一号工位、第几件产品、哪里不合格**。

C 语言的测试就是这条质检线。每个 `ASSERT` 就是一个检验点——如果产品（函数返回值）不合格，检验员不仅要停下来，还要报告精确的位置和原因。

但如果你写的断言只说「test failed」——就像质检员只喊一声「不行！」就下班了。没人知道该修哪里，生产线照样出次品。

**测试的本质不是证明代码正确，而是证明代码有错误**。好的测试框架，就是让你快速定位错误的那把放大镜。

## 本章适合谁

本章适合已经理解 C 语言函数、指针、宏的读者。如果你还没掌握以下知识：
- 函数声明与定义 → 先看 [函数](../basic/functions.md)
- 指针与函数指针 → 先看 [指针](../basic/pointers.md) 和 [函数指针](../basic/function_pointers.md)
- 预处理宏 → 先看 [预处理器与宏](../basic/preprocessor.md)

请先补习这些前置知识。

## 你会学到什么

1. 如何用 `__FILE__` 和 `__LINE__` 让断言精准定位
2. 如何从零搭建一个测试框架：Test Runner + Test Case 注册
3. 如何通过函数指针注入 Mock，隔离被测代码的依赖
4. Test Fixture（setup/teardown）、参数化测试、测试分组
5. 测试报告的结构化输出（TAP 格式）

## 前置要求

- GCC 或 Clang 编译器
- 理解 C 语言的宏机制（`#define`）
- 理解函数指针（`void (*fn)(void)`）
- 运行源码：`make run` 查看效果

## 问题引入 — 「test failed」等于没说

假设你写了这样一个断言：

```c
/* ❌ 反面教材 */
#define ASSERT(cond) do {                \
    if (!(cond)) {                       \
        printf("test failed\n");         \
    }                                    \
} while (0)

int a = 3, b = 5;
ASSERT(a == b);  /* 输出: test failed */
```

读完这行输出，你的反应是什么？
- 哪个文件失败了？不知道。
- 哪一行失败了？不知道。
- 期望什么值、实际什么值？不知道。

这就像质检员喊了一声「不行！」，但没说哪个工位、哪件产品、什么毛病。生产线的人看了只会原地发呆。

**错误是第一堂课**：没有位置的断言 = 没有价值的断言。

## 自定义 ASSERT 宏 — __FILE__ + __LINE__

C 标准库提供了两个编译时预定义宏：

| 宏 | 含义 | 示例值 |
|----|------|--------|
| `__FILE__` | 当前源文件名（字符串） | `"testing_sample.c"` |
| `__LINE__` | 当前行号（整数） | `85` |
| `__func__` | 当前函数名（字符串） | `"test_add"` |

用它们改造断言：

```c
#define ASSERT_EQ(actual, expected) do {                \
    int _a = (int)(actual);                             \
    int _e = (int)(expected);                           \
    if (_a != _e) {                                     \
        printf("FAIL %s:%d: 期望 %d, 实际 %d\n",        \
               __FILE__, __LINE__, _e, _a);             \
    }                                                   \
} while (0)

ASSERT_EQ(3 + 2, 5);
/* 如果失败: FAIL testing_sample.c:85: 期望 5, 实际 4 */
```

现在质检员报告精确了：`testing_sample.c:85`，期望 5 实际 4。你可以直接跳到那一行修改。

## 彩色输出增强

给输出加颜色，让「PASS」和「FAIL」一目了然：

```c
#define COLOR_GREEN  "\033[0;32m"
#define COLOR_RED    "\033[0;31m"
#define COLOR_RESET  "\033[0m"

#define ASSERT_EQ(actual, expected) do {                \
    int _a = (int)(actual);                             \
    int _e = (int)(expected);                           \
    if (_a != _e) {                                     \
        printf(COLOR_RED "❌ FAIL %s:%d\n" COLOR_RESET,  \
               __FILE__, __LINE__);                     \
        printf("期望 %d, 实际 %d\n", _e, _a);           \
    } else {                                            \
        printf(COLOR_GREEN "✅ PASS" COLOR_RESET "\n"); \
    }                                                   \
} while (0)
```

编译运行后，终端里绿色 PASS 和红色 FAIL 一目了然。

## 测试框架 — Test Runner

有了断言，下一步是组织它们——这就是 **测试框架** 要解决的问题。

### 核心结构

```c
typedef struct {
    const char *name;
    void (*func)(void);
} TestCase;

typedef struct {
    int total;
    int passed;
    int failed;
    TestCase cases[64];
} TestRunner;
```

- `TestCase` = 一张质检卡（名字 + 函数）
- `TestRunner` = 自动传送带 + 计数器
- `runner_add()` = 往传送带上放卡
- `runner_run_all()` = 启动传送带，依次执行

### 运行器实现

```c
static TestRunner g_runner;
static int g_current_case_failed;

static void runner_run_all(void)
{
    for (int i = 0; i < g_runner.total; i++) {
        g_current_case_failed = 0;
        g_runner.cases[i].func();
        if (g_current_case_failed) {
            g_runner.failed++;
        } else {
            g_runner.passed++;
        }
    }
}
```

每个测试函数内部用 `ASSERT_EQ_RUN` 判断 pass/fail。如果任何断言失败，就设置 `g_current_case_failed = 1`，runner 据此计数。

## 被测函数与测试用例

被测代码：

```c
static int calc_add(int a, int b)
{
    return a + b;
}
```

测试代码：

```c
static void test_calc_add_basic(void)
{
    ASSERT_EQ_RUN(calc_add(2, 3), 5);
    ASSERT_EQ_RUN(calc_add(0, 0), 0);
    ASSERT_EQ_RUN(calc_add(-1, 1), 0);
}

runner_add("加法基础", test_calc_add_basic);
runner_run_all();
```

运行结果：
```
运行 [1/3]: 加法基础 ... PASS
```

**要点**：一个测试函数可以有多个断言。任何一个 Assert 失败，整个测试用例算失败。

## Unity 测试框架 (Unity Testing Framework) 🟢

### 什么是 Unity？

Unity 是轻量级的 C 语言测试框架，和 CMock 同属 ThrowTheSwitch 生态。它只有 3 个源文件（`unity.c`、`unity.h`、`unity_internals.h`），零外部依赖。我选择它作为项目的测试框架，因为它：
- **极简** —— 三个文件就能跑，不需要安装任何额外工具
- **够用** —— 覆盖了我日常需要的所有断言类型
- **可集成** —— 配合 Makefile 自动发现测试文件，零配置运行

### 和自定义 ASSERT_EQ_RUN 的对比

我在前面搭建了自定义 Test Runner + `ASSERT_EQ_RUN` 宏。那套方案帮我理解了测试框架的底层原理，但 Unity 是生产级方案。看同一组加法测试，两种写法的区别：

| 维度 | 自定义 `ASSERT_EQ_RUN` | Unity `TEST_ASSERT_EQUAL_INT` |
|------|------------------------|-------------------------------|
| 调用方式 | `ASSERT_EQ_RUN(calc_add(2, 3), 5)` | `TEST_ASSERT_EQUAL_INT(5, calc_add(2, 3))` |
| 参数顺序 | (actual, expected) | (expected, actual) |
| 失败输出 | 手动格式化的 `printf` | 内置标准格式，含表达式、文件、行号 |
| 失败后行为 | 设置 `g_current_case_failed = 1` 继续跑 | 默认继续（可配置 `abort`） |
| runner 集成 | 需手写 `runner_add()`/`runner_run_all()` | `UNITY_BEGIN()`/`RUN_TEST()`/`UNITY_END()` 三件套 |

**代码对比** —— 同样测试 `calc_add`，自定义版 vs Unity 版：

```c
/* ── 自定义框架版 ── */
#include "framework.h"  /* 包含 ASSERT_EQ_RUN、runner 相关 */

static void test_calc_add_basic(void)
{
    ASSERT_EQ_RUN(calc_add(2, 3), 5);    /* (actual, expected) */
    ASSERT_EQ_RUN(calc_add(0, 0), 0);
    ASSERT_EQ_RUN(calc_add(-1, 1), 0);
}

/* main 中手动注册 */
int main(void) {
    runner_add("加法基础", test_calc_add_basic);
    runner_run_all();
    return g_runner.failed > 0 ? 1 : 0;
}
```

```c
/* ── Unity 版 ── */
#include "unity.h"
#include "advance/calc.h"

void setUp(void) {}    /* 每个测试前执行（可选）*/
void tearDown(void) {} /* 每个测试后执行（可选）*/

static void test_calc_add_basic(void)
{
    TEST_ASSERT_EQUAL_INT(5, calc_add(2, 3));    /* (expected, actual) */
    TEST_ASSERT_EQUAL_INT(0, calc_add(0, 0));
    TEST_ASSERT_EQUAL_INT(0, calc_add(-1, 1));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_calc_add_basic);
    return UNITY_END();
}
```

我自己的感悟：**自定义框架教了我原理，Unity 让我专注写测试**。从 `ASSERT_EQ_RUN` 切换到 Unity 后，最大的感受是不用再手写 Runner 的注册和计数逻辑了——`UNITY_BEGIN()`/`RUN_TEST()`/`UNITY_END()` 三行就把框架搭好了。

### 示例：test/advance/test_calc_add.c

项目中实际的 Unity 测试文件在 `test/advance/test_calc_add.c`，它是和 `src/` 目录结构一一镜像的。来读一下完整代码：

```c
/**
 * @file test_calc_add.c
 * @brief Unit tests for calc_add() function using Unity test framework.
 *
 * Tests basic addition, edge cases (zero), and negative number scenarios.
 */

#include "unity.h"
#include "advance/calc.h"

/**
 * @brief Setup function called before each test.
 *
 * Currently unused — no test-specific setup required.
 */
void setUp(void)
{
}

/**
 * @brief Teardown function called after each test.
 *
 * Currently unused — no cleanup required.
 */
void tearDown(void)
{
}

/**
 * @brief Test basic calc_add functionality.
 *
 * Verifies:
 * - Positive addition: calc_add(2, 3) == 5
 * - Zero addition:  calc_add(0, 0) == 0
 * - Sign neutralization: calc_add(-1, 1) == 0
 */
void test_calc_add_basic(void)
{
    TEST_ASSERT_EQUAL_INT(5, calc_add(2, 3));
    TEST_ASSERT_EQUAL_INT(0, calc_add(0, 0));
    TEST_ASSERT_EQUAL_INT(0, calc_add(-1, 1));
}

/**
 * @brief Test calc_add with negative numbers.
 *
 * Verifies:
 * - Mixed sign: calc_add(-2, 1) == -1
 * - Both negative: calc_add(-2, -3) == -5
 */
void test_calc_add_negative(void)
{
    TEST_ASSERT_EQUAL_INT(-1, calc_add(-2, 1));
    TEST_ASSERT_EQUAL_INT(-5, calc_add(-2, -3));
}

/**
 * @brief Main entry point for the test runner.
 *
 * Initializes Unity, registers all test cases, and returns the result.
 *
 * @return int Unity test result (0 = all passed, non-zero = failures)
 */
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_calc_add_basic);
    RUN_TEST(test_calc_add_negative);
    return UNITY_END();
}
```

**关键结构**：
1. **`setUp()` / `tearDown()`** —— Unity 要求这两个函数必须存在。即使为空也得写上（可以暂时不实现，留空编译通过）。
2. **`TEST_ASSERT_EQUAL_INT(expected, actual)`** —— 注意参数顺序：**期望值在前，实际值在后**。我自定义的 `ASSERT_EQ_RUN` 用的是 (actual, expected) 顺序，方向相反。这是我刚切换时最常写反的地方。
3. **`UNITY_BEGIN()` / `RUN_TEST()` / `UNITY_END()`** —— 固定三部曲：初始化 → 逐个注册并运行 → 结束并返回结果。

### 运行 Unity 测试 —— make test

项目的 `Makefile` 已经集成了完整的测试构建和运行流程。`make test` 的工作原理：

1. **自动发现** —— `wildcard` 捕获 `test/**/*.c`（排除 `vendor/`）
2. **编译每个测试** —— 每个 `test_*.c` 编译为独立的测试二进制
3. **带 Unity 编译 flags** —— `-DUNITY_OUTPUT_COLOR`（彩色输出）+ `-DUNITY_SUPPORT_VARIADIC_MACROS`
4. **链接被测代码** —— 测试二进制链接对应的 `.o`（如 `build/obj/advance/calc.o`）
5. **顺序执行** —— 遍历所有二进制，逐个运行

```bash
# 一键运行所有测试
make test
```

如果添加新的测试文件（比如 `test/advance/test_calc_multiply.c`），不需要修改 Makefile——`wildcard` 自动捕获。保持目录镜像结构（`test/advance/` 对应 `src/advance/`）就行。

**执行流程示意**：
```
make test
  → 发现 test/advance/test_calc_add.c, test/advance/test_calc_multiply.c, ...
  → 编译 test/advance/test_calc_add.c → build/test/advance/test_calc_add
  → 编译 test/advance/test_calc_multiply.c → build/test/advance/test_calc_multiply
  → 运行 build/test/advance/test_calc_add
  → 运行 build/test/advance/test_calc_multiply
  → 全部通过 → exit code 0
```

## Mock 函数 — 函数指针注入

Mock 是测试中最重要的概念之一：**用假数据替换真实依赖**。

### 为什么需要 Mock？

假设你要测试一个传感器系统：
```c
int temp = read_sensor();  // 读真实硬件
if (temp > 50) { ... }
```

你不能每次都找一台温度 80°C 的机器来测试高温场景。你需要 Mock：

```c
typedef int (*ReadSensorFn)(void);
static ReadSensorFn g_read_sensor = real_read_sensor;

/* 测试时注入 Mock */
g_read_sensor = mock_read_sensor;  /* mock 返回固定值 99 */
```

### 工厂类比

生产线上的温度传感器坏了，质检员用「模拟传感器」提供标准信号（比如模拟高温 80°C），继续测试后续执行器是否正确响应。

### Mock 设置

```c
static int mock_read_sensor_value = 99;

static int mock_read_sensor(void)
{
    return mock_read_sensor_value;
}

static void test_mock_high_temp(void)
{
    mock_read_sensor_value = 80;
    g_read_sensor = mock_read_sensor;
    int rc = system_process();
    ASSERT_EQ_RUN(rc, 0);
    ASSERT_EQ_RUN(mock_write_actuator_last_value, 100);
}
```

## Mock 测试用例 — 多场景验证

一个 Mock 可以模拟多种场景：

```c
static void test_mock_high_temp(void)  /* temp=80 → 执行器设为 100 */
static void test_mock_mid_temp(void)   /* temp=40 → 执行器设为 50 */
static void test_mock_low_temp(void)   /* temp=10 → 执行器设为 0 */
```

三组测试，同一个被测函数，不同的 Mock 输入。不依赖硬件，验证所有逻辑分支。

## CMock 自动化 Mock 生成 (CMock Automated Mock Generation)

### 什么是 CMock？
CMock 是一个基于 Ruby 的自动化 Mock 生成工具，和 Unity 同属 ThrowTheSwitch 生态。它可以根据 C 头文件自动生成 Mock 函数的实现，省去手写 Mock 的麻烦。

**Unity vs CMock 的核心区别**：
| 工具 | 角色 | 作用 |
|------|------|------|
| Unity | 测试运行器 + 断言库 | 提供测试用例结构、断言宏、结果输出 |
| CMock | Mock 生成器 | 根据头文件自动生成 Mock 函数 |

### 快速上手
假设我们有一个传感器头文件 `src/advance/sensor.h`：
```c
// src/advance/sensor.h
#ifndef SENSOR_H
#define SENSOR_H

int read_sensor_temperature(void);
int read_sensor_humidity(void);

#endif
```

运行 CMock 生成 Mock：
```bash
ruby test/vendor/cmock/lib/cmock.rb -otest/mocks src/advance/sensor.h
```

会在 `test/mocks/` 目录下生成 `mock_sensor.h`，里面包含了自动生成的 Mock 函数实现，支持：
- 期望调用次数设置（`expect_call_count`）
- 返回值设置（`set_return_value`）
- 参数校验（`check_argument`）

### 和自定义 Mock 的对比
之前我们学习了手写函数指针 Mock，CMock 的优势是：
1. 自动生成，减少手写错误
2. 支持丰富的校验规则（参数匹配、调用顺序等）
3. 和 Unity 无缝集成

手写 Mock 的优势是：
1. 适合教学，理解 Mock 底层原理
2. 无额外工具依赖（不需要 Ruby）

### 运行要求
CMock 是 Ruby 脚本，需要系统安装 Ruby 才能运行生成命令。但生成的 Mock 头文件是纯 C 代码，编译时不需要 Ruby。

## 测试夹具 (Test Fixtures) — Setup / Teardown

多个测试共用同一个准备/清理流程：

```c
static void fixture_setup(void)
{
    g_fixture_initialized = 1;
    g_resource_count = 0;
}

static void fixture_teardown(void)
{
    g_fixture_initialized = 0;
}

static void test_fixture_resource(void)
{
    fixture_setup();
    int id = fixture_allocate();
    ASSERT_EQ_RUN(id, 1);
    fixture_teardown();
}
```

工厂类比：每批质检前校准仪器（setup），质检后清理工作台（teardown）。

## 测试分组

将测试按功能分组，每组独立统计：

```c
/* 分组: 数学运算 */
runner_add("加法", test_calc_add);
runner_add("乘法", test_calc_multiply);
runner_run_all();

/* 分组: Mock 场景 */
runner_add("高温", test_mock_high);
runner_add("低温", test_mock_low);
runner_run_all();
```

## 参数化测试

同一逻辑，多输入验证：

```c
typedef struct { int a; int b; int expected; } AddCase;

AddCase cases[] = {
    { 0,  0,  0 },
    { 1,  2,  3 },
    { -5, 5,  0 },
};

for (int i = 0; i < 3; i++) {
    int result = calc_add(cases[i].a, cases[i].b);
    printf("✅/❌ [%d] %s\n", i + 1,
           result == cases[i].expected ? "PASS" : "FAIL");
}
```

## 测试报告 — TAP 格式

```
TAP version 13
1..3
ok 1 - 加法基础
ok 2 - 乘法基础
not ok 3 - 无效范围 (期望 1, 实际 0)
```

TAP (Test Anything Protocol) 是标准化测试输出。CI 系统可解析生成仪表盘。

## 故障排查 (FAQ)

**Q：assert.h 里的 assert() 不够用吗？**

A：标准 `assert()` 在 `NDEBUG` 模式下失效，且只有文件名/行号。自定义宏更灵活：彩色输出、Runner 集成、自定义消息。

**Q：为什么测试函数用 `static void`？**

A：`static` 限制为文件内可见，避免符号冲突。这是 C 的「私有函数」模式。

## 小结

本章学习了 C 语言测试框架的核心：

- **ASSERT 宏**：`__FILE__` + `__LINE__` 精准定位
- **Test Runner**：TestCase + 计数器自动执行
- **Unity 框架**：轻量级测试运行器，标准化断言宏
- **Mock 函数**：函数指针注入，隔离外部依赖
- **CMock 生成器**：自动化 Mock 函数生成，减少手写成本
- **Test Fixtures**：setup/teardown 统一管理
- **TAP 报告**：结构化输出，CI 友好

**核心术语**：Assert / Test Runner / Test Case / Unity / Mock / CMock / Fixture / TAP

## 术语表

| 英文 | 中文 |
|------|------|
| Assertion | 断言 |
| Test Framework | 测试框架 |
| Test Runner | 测试运行器 |
| Test Case | 测试用例 |
| Mock Function | 模拟函数 |
| Test Fixture | 测试夹具 |
| TAP | 测试协议 |

## 继续学习

- [上一章](./database.md)：数据库
- [下一章](./tools.md)：工具链
