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
- **Mock 函数**：函数指针注入，隔离外部依赖
- **Test Fixtures**：setup/teardown 统一管理
- **TAP 报告**：结构化输出，CI 友好

**核心术语**：Assert / Test Runner / Test Case / Mock / Fixture / TAP

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

- [上一章](./error_handling.md)：错误处理
- [下一章](./tools.md)：工具链
