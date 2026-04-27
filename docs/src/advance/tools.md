# 工具链 (Toolchain) 🟢

## 开篇故事

想象你是一个厨师。你做了一桌子菜，客人还没吃到嘴里，你就已经完成了多次「质检」：
- 买菜时检查食材是否新鲜（静态分析：不打开炉子，阅读代码就能发现问题）
- 做菜时控制火候（编译器警告：语法错误、类型不匹配）
- 出锅后尝一口（测试：运行代码，验证结果）
- 拍照记录（覆盖率报告：证明哪些菜做了、哪些没做）

C 语言的工具链就是厨房里的质检体系。每个工具（gcov、cppcheck、CI）都是不同环节的质检员——有的看食材，有的看火候，有的看成品。

**编译通过 ≠ 代码正确**。你需要一套完整的工具链来保证代码质量。

## 本章适合谁

本章适合已经能独立编写 C 程序、准备开始做真实项目的读者。如果你还在纠结指针的语法——建议先巩固基础，本章内容可以放在后续。

## 你会学到什么

1. Makefile 最佳实践：标准 C 项目的构建结构
2. 代码覆盖率分析：gcov/lcov 的原理与使用
3. 静态分析：cppcheck 能发现哪些问题及如何运行
4. CI 流水线：GitHub Actions 自动化编译/测试/分析

## 前置要求

- 理解 C 语言基本语法
- 知道 Makefile 的基本概念
- 安装了 GCC、git
- （可选）cppcheck、lcov 工具

## 问题引入 — 「它编译通过了」

这段代码编译通过：

```c
int divisor = 0;
printf("%d\n", 100 / divisor);  /* 编译通过！ */
```

运行时呢？除零异常，程序崩溃。编译器没有阻止你——因为它只检查语法，不检查语义。

```
问题1: 除零 — 编译器不报错，运行时崩溃
问题2: 缓冲区溢出 — 编译器只给警告
问题3: fopen 可能失败 — 但你没用 if 检查
```

**结论**：编译通过只保证了「语法正确」。「语义正确」需要测试 + 覆盖率 + 静态分析。

## Makefile 最佳实践

标准 C Makefile 结构：

```makefile
# 编译器与标志
CC       := gcc
CFLAGS   := -Wall -Wextra -Werror -std=c17 -g -O2
LDFLAGS  :=

# 源文件与对象文件
SRCS  := $(wildcard src/**/*.c)
OBJS  := $(patsubst src/%.c,build/obj/%.o,$(SRCS))

# 默认目标
all: build

# 构建
build: $(OBJS)
	$(CC) $^ -o build/bin/hello

# 编译规则
build/obj/%.o: src/%.c
	$(CC) $(CFLAGS) -MMD -c $< -o $@

# 清理
clean:
	rm -rf build/

# 测试
test: build
	./build/bin/hello

.PHONY: all build clean test
```

### 关键规则

| 目标 | 说明 |
|------|------|
| `build` | 编译所有源文件 |
| `clean` | 删除 build 目录 |
| `test` | 运行可执行文件 |
| `-MMD` | 自动生成依赖文件 `.d` |
| `-Wall -Wextra` | 启用所有警告 |
| `-Werror` | 警告提升为错误 |

## 覆盖率分析 — gcov

gcov 是 GCC 内置的覆盖率工具。编译时加 `-fprofile-arcs -ftest-coverage`，运行时收集执行数据：

```bash
# 1. 编译时开启覆盖率
gcc -fprofile-arcs -ftest-coverage -o hello src/tools_sample.c

# 2. 运行测试
./hello  # 生成 .gcda 文件

# 3. 分析覆盖率
gcov -b src/tools_sample.c
```

gcov 输出示例：
```
File 'tools_sample.c'
Lines executed:85.00% of 20
Branches executed:75.00% of 16
Taken at least once:50.00% of 16
```

- **Lines executed**：多少百分比的代码行被执行过
- **Branches executed**：多少百分比的 if/else 分支被执行过

## lcov — HTML 可视化报告

lcov 是 gcov 的前端，生成漂亮的 HTML 报告：

```bash
# 1. 收集覆盖率数据
lcov --capture -d build/obj/ -o coverage.info

# 2. 移除系统头文件
lcov --remove coverage.info '/usr/*' -o coverage.info

# 3. 生成 HTML 报告
genhtml coverage.info --output-directory coverage-html

# 4. 查看报告
open coverage-html/index.html
```

HTML 报告显示：
- 目录级：每个文件的覆盖率百分比
- 文件级：源码每行是否被覆盖（绿/红）
- 函数级：每个函数是否被调用
- 分支级：每个 if/else 分支是否被覆盖

## 静态分析 — cppcheck

静态分析工具不运行代码，而是「阅读」代码，找出潜在错误。

cppcheck 能检测的常见问题：

| 问题 | 示例 | 严重性 |
|------|------|--------|
| 内存泄漏 | `malloc` 后没有 `free` | error |
| 空指针解引用 | `char *p = NULL; printf("%c", *p)` | error |
| 数组越界 | `int arr[5]; arr[5] = 42;` | error |
| 未使用变量 | `int x = 5;` (从未用 x) | style |
| 未初始化变量 | `int x; printf("%d", x)` | warning |
| 格式字符串不匹配 | `printf("%s", 42)` | warning |
| 资源泄漏 | `fopen` 后没有 `fclose` | error |

工厂类比：就像厨师长在做菜前检查食材——没打开炉子，就能发现食材过期、搭配不当等问题。

## 运行 cppcheck

```bash
# 基本用法
cppcheck --enable=all --std=c17 .

# 常用选项
--enable=all          启用所有检查
--std=c17             C17 标准
--inconclusive        包含不确定性警告
--suppress=missingInclude  忽略头文件缺失
--error-exitcode=1    发现错误时退出码为 1（CI 友好）

# 输出示例
[src/main.c:12]: (error) Memory leak: p
[src/main.c:15]: (error) Null pointer dereference
[src/main.c:20]: (warning) Uninitialized variable: x

# 严重级别
error   — 确定的 bug，必须修复
warning — 很可能有 bug，应该修复
style   — 代码风格问题，建议优化
```

## 编译器警告 = 免费静态分析

你的项目已经在做编译器级的静态分析了：

```bash
gcc -Wall -Wextra -Werror -std=c17
```

**`-Wall` 捕获**：
- 未使用变量
- 未初始化变量
- 隐式函数声明
- 符号/无符号比较
- 格式字符串不匹配

**`-Wextra` 额外捕获**：
- 未使用参数
- 多余的类型转换
- 比较总是真/假

**`-Werror` 强制修复**：warning → error，编译失败。不允许「带警告交付」。

## 集成覆盖率到 Makefile

在 Makefile 中添加覆盖率目标：

```makefile
.PHONY: coverage coverage-clean

coverage-clean:
	find build/obj/ -name '*.gcda' -delete
	rm -rf coverage.info coverage-html/

coverage: coverage-clean
	@make build
	@./build/bin/hello
	@lcov --capture -d build/obj/ -o coverage.info
	@gcov -b build/obj/advance/tools_sample.gcda
	@echo '覆盖率报告已生成!'
```

然后运行：`make coverage`

## 集成 cppcheck 到 Makefile

```makefile
.PHONY: lint lint-strict

lint:
	@echo '=== 静态分析 (cppcheck) ==='
	@cppcheck --enable=all --std=c17 \
	  --suppress=missingInclude --error-exitcode=0 src/

lint-strict:
	@cppcheck --enable=all --std=c17 \
	  --inconclusive --error-exitcode=1 src/
```

然后运行：`make lint`

## 质量门禁

在 CI 中不达标就不允许合并：

```bash
# 覆盖率检查
lcov --summary coverage.info | grep 'lines'

# cppcheck 无 error
cppcheck --enable=all --error-exitcode=1 src/

# 编译器警告计数为 0
make build 2>&1 | grep -c 'warning' || true
```

质量门禁 = 代码的「出厂检验标准」。不达标 → 拒绝合并。

## 多平台编译

CI 矩阵编译，在多个平台测试：

```yaml
strategy:
  matrix:
    os: [ubuntu-latest, macos-latest]
    cc: [gcc, clang]
```

等价于在 Ubuntu+GCC、Ubuntu+Clang、macOS+Clang 上分别编译。

## 发布模式

Debug 和 Release 的不同编译配置：

| | Debug | Release |
|--|-------|---------|
| CFLAGS | `-g -O0 -DDEBUG` | `-O2 -DNDEBUG` |
| 特点 | 完整调试信息 | 优化、体积小 |
| 用途 | gdb 调试 | 生产部署 |

Makefile 集成：
```makefile
debug: CFLAGS += -g -O0 -DDEBUG
debug: build

release: CFLAGS += -O2 -DNDEBUG
release: build
```

## GitHub Actions CI 流水线

`.github/workflows/ci.yml` 示例：

```yaml
name: C CI
on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Install dependencies
        run: sudo apt-get install -y gcc cppcheck lcov
      - name: Build
        run: make build
      - name: Test
        run: make run
      - name: Static Analysis
        run: make lint
      - name: Coverage
        run: make coverage
```

流水线阶段：Build → Test → Lint → Coverage → Deploy

## 工具链总结

开发者的日常工具链：

```
开发中:  make build (编译) → make run (测试)
提交前:  make lint (静态分析) → make coverage (覆盖率)
CI 自动: git push → Build → Test → Lint → Coverage → 合并
```

这就是工厂的「全流程质检体系」：
工人自检(编译) → 班组互查(测试) → 质检员巡检(lint) → 抽检报告(coverage) → 出厂检验(CI)

## 故障排查 (FAQ)

**Q：为什么要 `-Werror`？警告不是也可以编译吗？**

A：因为「带警告交付」是一个滑坡。今天放过一个 warning，明天会有十个。`-Werror` 强制你在代码进仓库前修掉所有警告。

**Q：gcc 警告和 cppcheck 有什么不同？**

A：gcc 主要检测语法和类型问题；cppcheck 能发现更深层的逻辑问题（如内存泄漏、空指针解引用）。二者互补。

**Q：覆盖率 100% 就安全了吗？**

A：不是。覆盖率只衡量「哪些代码被执行了」，不衡量「执行的逻辑是否正确」。100% 覆盖率 + 没有断言 = 白测。

## 小结

本章学习了 C 项目的完整工具链：

- **Makefile**：标准构建结构 + 多目标
- **gcov/lcov**：代码覆盖率分析，HTML 可视化
- **cppcheck**：静态分析，发现内存泄漏、空指针等 bug
- **CI 流水线**：GitHub Actions 自动化 Build/Test/Lint
- **质量门禁**：覆盖率阈值 + 静态分析无 error

**核心术语**：Makefile / Coverage / gcov / lcov / cppcheck / CI / Quality Gate

## 术语表

| 英文 | 中文 |
|------|------|
| Makefile | 构建脚本 |
| Code Coverage | 代码覆盖率 |
| gcov | GCC 覆盖率工具 |
| lcov | LTP 覆盖率前端 |
| Static Analysis | 静态分析 |
| cppcheck | C/C++ 静态分析工具 |
| CI | 持续集成 |
| Quality Gate | 质量门禁 |
| Lint | 代码检查 |

## 继续学习

- [上一章](./testing.md)：测试框架
- [下一章](../README.md)：回到课程主页
