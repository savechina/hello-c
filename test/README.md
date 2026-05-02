# test/ 目录说明

## 概述
本目录存放 Unity/CMock 测试用例，目录结构与 `src/` 镜像一致，方便对应模块的测试管理。

## 目录结构
```
test/
├── vendor/          # 第三方测试依赖
│   ├── unity/       # Unity v2.6.1 (3 source files)
│   └── cmock/       # CMock v2.6.0 (Ruby-based mock generator)
├── advance/         # 对应 src/advance/ 的测试
│   └── test_calc_add.c  # calc_add() Unity 测试示例
├── basic/           # 对应 src/basic/ 的测试
├── module1/         # 对应 src/module1/ 的测试
├── module2/         # 对应 src/module2/ 的测试
├── mocks/           # CMock 生成的 Mock 文件 (gitignored)
└── README.md        # 本文件
```

## 快速开始

### 运行所有测试
```bash
make test  # 自动编译并运行所有 test/**/*.c 测试用例
```

### 添加新测试用例
1. 在对应模块的测试目录创建 `test_*.c`（如 `test/advance/test_calc_multiply.c`）
2. 包含对应头文件（`#include "advance/calc.h"`）和 Unity 头文件（`#include "unity.h"`）
3. 实现 `setUp()`/`tearDown()`（可选）、测试用例、`main()` 调用 `UNITY_BEGIN()`/`RUN_TEST()`/`UNITY_END()`
4. 运行 `make test` 自动编译执行

### Unity 编译 flags
Makefile 使用以下 flags 编译测试：
- `-DUNITY_OUTPUT_COLOR`：彩色输出（PASS 绿色，FAIL 红色）
- `-DUNITY_SUPPORT_VARIADIC_MACROS`：支持变参宏
- `-I$(UNITY_DIR)`：Unity 头文件路径

## CMock 使用
CMock 是 Ruby 脚本，可根据头文件自动生成 Mock 函数：
```bash
# 生成 sensor.h 的 Mock（需 Ruby 环境）
ruby test/vendor/cmock/lib/cmock.rb -otest/mocks src/advance/sensor.h
```
生成的 Mock 文件位于 `test/mocks/mock_sensor.h`。

## 注意事项
- 测试文件会自动被 Makefile 的 `wildcard` 捕获，无需手动修改 Makefile
- `test/mocks/` 目录已加入 `.gitignore`，生成的 Mock 无需提交
- CMock 仅用于教学演示，需 Ruby 环境才能生成 Mock
