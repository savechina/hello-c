# Quickstart: C Advance Tutorial — Subcommand Support#

## Prerequisites#

- Completed: All 27 basic C chapters
- Build tools: `gcc` (macOS) or `clang` (Linux)
- Makefile: `make build`, `make test`, `make run` already working

## New Feature: Subcommand Support#

The `hello` binary now accepts topic-level subcommands, matching `../hello-rust` behavior.

### Usage#

```bash
# Run all sample chapters (default behavior, backward compatible)
./build/bin/hello
# or explicitly:
./build/bin/hello all

# Run all basic chapters only
./build/bin/hello basic

# Run all advance chapters only
./build/bin/hello advance

# List available topics
./build/bin/hello list

# Show usage
./build/bin/hello help
```

### Example Output (`./build/bin/hello list`)#

```
Available topics:
  basic     - 27 basic C chapters (variables, data types, functions, ...)
  advance   - 12 advance C chapters (threads, pointers, error handling, ...)
  module1   - print_hello example
  module2   - print_util example
```

## Implementation Steps#

1. Modify `src/main.c` to parse `argv[1]` and dispatch to subcommands
2. Update `src/basic/basic.c` — add `main_basic_sample(void)` coordinator
3. Update `src/advance/advance.c` — add `main_advance_sample(void)` coordinator
4. Update Makefile help target to show new subcommands
5. Update `docs/src/SUMMARY.md` with subcommand documentation

## Quality Checklist#

- [ ] `make build` zero warnings
- [ ] `./build/bin/hello basic` runs all basic chapters
- [ ] `./build/bin/hello advance` runs all advance chapters
- [ ] `./build/bin/hello list` prints available topics
- [ ] `./build/bin/hello all` / no args runs everything (backward compatible)
- [ ] `./build/bin/hello help` prints usage
- [ ] Bilingual output (Chinese + English terms)

## Add Test with Unity — 添加 Unity 测试

本项目使用 **Unity** (v2.6.1) 作为测试框架，所有测试通过 `make test` 自动编译和执行。

### 运行测试

```bash
make test       # 编译并运行所有 Unity 测试（自动发现 test/**/*.c）
```

`make test` 会自动扫描 `test/` 目录下的所有 `test_*.c` 文件，编译链接后统一执行，输出 PASS/FAIL 结果（彩色高亮）。

### 测试目录结构

测试目录与 `src/` 镜像对齐，方便模块对应：

```
test/
├── advance/         # 对应 src/advance/ 的测试
│   ├── test_calc_add.c       # 测试 calc_add()
│   ├── test_calc_multiply.c  # 测试 calc_multiply()
│   └── test_calc_is_valid.c  # 测试 calc_is_valid()
├── basic/           # 对应 src/basic/ 的测试
├── module1/         # 对应 src/module1/ 的测试
└── module2/         # 对应 src/module2/ 的测试
```

### 示例：`test_calc_add.c`

以 `test/advance/test_calc_add.c` 为例，它测试 `src/advance/calc.c` 中的 `calc_add()` 函数：

```c
#include "unity.h"
#include "advance/calc.h"

void setUp(void) {}      // (可选) 每个 test case 前的初始化
void tearDown(void) {}   // (可选) 每个 test case 后的清理

void test_calc_add_basic(void)
{
    TEST_ASSERT_EQUAL_INT(5, calc_add(2, 3));   // 正数加法
    TEST_ASSERT_EQUAL_INT(0, calc_add(0, 0));   // 零值边界
    TEST_ASSERT_EQUAL_INT(0, calc_add(-1, 1));  // 正负抵消
}

void test_calc_add_negative(void)
{
    TEST_ASSERT_EQUAL_INT(-1, calc_add(-2, 1));  // 混合符号
    TEST_ASSERT_EQUAL_INT(-5, calc_add(-2, -3)); // 双负值
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_calc_add_basic);
    RUN_TEST(test_calc_add_negative);
    return UNITY_END();
}
```

**要点：**
- 测试文件命名规则：`test_<module>.c`
- 每个 `.c` 必须包含 `unity.h`（Unity 框架头）和对应模块头文件
- 通过 `RUN_TEST(func)` 注册测试用例，`TEST_ASSERT_EQUAL_INT` 等宏进行断言
- 运行 `make test` 后，Makefile 自动编译所有 `test/**/*.c` 并执行

### 添加新测试

1. 在 `test/` 下对应子目录创建 `test_<module>.c`
2. `#include "unity.h"` + 对应模块头文件
3. 实现 `setUp()` / `tearDown()`（可选）
4. 编写 `void test_xxx(void)` 测试函数，使用 `TEST_ASSERT_*` 系列宏
5. `main()` 中调用 `UNITY_BEGIN()` / `RUN_TEST()` / `UNITY_END()`
6. 执行 `make test` 验证
