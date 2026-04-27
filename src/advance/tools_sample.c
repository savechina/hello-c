/**
 * @file tools_sample.c
 * @brief 工具链 (Toolchain) — Advance tutorial chapter
 *
 * Demonstrates:
 *   1. Makefile best practices for C projects
 *   2. Code coverage with gcov/lcov
 *   3. Static analysis with cppcheck
 *   4. CI pipeline patterns
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "advance/tools_sample.h"

/* ============================================================
   Section 1: Error-First — "it compiles" != "it is correct"
   ============================================================ */

static void demo_problem(void)
{
    printf("=== Section 1: 「它编译通过了」的陷阱 ===\n");

    int divisor = 0;
    (void)divisor;
    printf("  问题1: 除零 — 编译器不报错，运行时崩溃\n");

    char buf[4];
    (void)buf;
    printf("  问题2: 缓冲区溢出 — 编译器只给警告\n");

    FILE *fp_tmp = fopen("/dev/null", "r");
    if (fp_tmp == NULL) {
        printf("  问题3: fopen 可能失败（如只读模式）\n");
    } else {
        fclose(fp_tmp);
    }

    printf("\n  结论: 编译通过只保证了「语法正确」。\n");
    printf("  「语义正确」需要: 测试 + 覆盖率 + 静态分析 + CI。\n");
}

/* ============================================================
   Section 2: Makefile 最佳实践 — C 项目的标准结构
   ============================================================ */

static void print_makefile_structure(void)
{
    printf("\n=== Section 2: Makefile 最佳实践 ===\n");

    printf("  标准 Makefile 结构:\n");
    printf("\n");
    printf("  # 编译器与标志\n");
    printf("  CC       := gcc\n");
    printf("  CFLAGS   := -Wall -Wextra -Werror -std=c17 -g -O2\n");
    printf("  LDFLAGS  :=\n");
    printf("\n");
    printf("  # 覆盖率标志 (与 CFLAGS 分离)\n");
    printf("  COV_FLAGS := -fprofile-arcs -ftest-coverage\n");
    printf("\n");
    printf("  # 源文件与对象文件\n");
    printf("  SRCS  := $(wildcard src/**/*.c)\n");
    printf("  OBJS  := $(patsubst src/%%.c,build/obj/%%.o,$(SRCS))\n");
    printf("\n");
    printf("  # 默认目标\n");
    printf("  all: build\n");
    printf("\n");
    printf("  # 构建\n");
    printf("  build: $(OBJS)\n");
    printf("  \t$(CC) $^ -o build/bin/hello\n");
    printf("\n");
    printf("  # 编译规则\n");
    printf("  build/obj/%%.o: src/%%.c\n");
    printf("  \t$(CC) $(CFLAGS) -MMD -c $< -o $@\n");
    printf("\n");
    printf("  # 清理\n");
    printf("  clean:\n");
    printf("  \trm -rf build/\n");
    printf("\n");
    printf("  # 测试\n");
    printf("  test: build\n");
    printf("  \t./build/bin/hello\n");
    printf("\n");
    printf("  # 静态分析\n");
    printf("  lint:\n");
    printf("  \tcppcheck --enable=all --std=c17 src/\n");
    printf("\n");
    printf("  # 覆盖率\n");
    printf("  coverage: CFLAGS += $(COV_FLAGS)\n");
    printf("  coverage: LDFLAGS += $(COV_FLAGS)\n");
    printf("  coverage: test\n");
    printf("  \tgcov -b src/**/*.c\n");
    printf("  \tlcov --capture -d build/ -o coverage.info\n");
    printf("  \tgenhtml coverage.info --output-directory coverage-html\n");
    printf("\n");
    printf("  # 依赖文件\n");
    printf("  -include $(OBJS:.o=.d)\n");
    printf("  .PHONY: all build clean test lint coverage\n");
}

/* ============================================================
   Section 3: 被测函数 — 设计有覆盖率的代码
   ============================================================ */

static int classify_number(int n)
{
    if (n < 0) {
        return -1;
    } else if (n == 0) {
        return 0;
    } else if (n < 100) {
        return 1;
    } else {
        return 2;
    }
}

static int count_letter(const char *str, char letter)
{
    int count = 0;

    if (str == NULL) {
        return -1;
    }

    while (*str != '\0') {
        if (*str == letter) {
            count++;
        }
        str++;
    }

    return count;
}

static int safe_add(int64_t a, int64_t b, int64_t *result)
{
    if (result == NULL) {
        return -1;
    }

    *result = a + b;

    if ((*result > a && b > 0) || (*result < a && b < 0)) {
        return -2;
    }

    return 0;
}

/* ============================================================
   Section 4: 覆盖率分析 — gcov 工作原理
   ============================================================ */

static void demo_gcov(void)
{
    printf("\n=== Section 4: gcov 覆盖率分析 ===\n");

    printf("  执行分类测试:\n");
    int r1 = classify_number(-5);
    int r2 = classify_number(0);
    int r3 = classify_number(42);
    int r4 = classify_number(200);

    printf("  -5 -> %d (期望 -1)\n", r1);
    printf("  0 -> %d (期望 0)\n", r2);
    printf("  42 -> %d (期望 1)\n", r3);
    printf("  200 -> %d (期望 2)\n", r4);

    int rc = count_letter("hello", 'l');
    printf("  'hello' 中 'l' 的个数 = %d (期望 2)\n", rc);

    int64_t sum = 0;
    rc = safe_add(100, 200, &sum);
    printf("  100 + 200 = %" PRId64 " (返回码: %d)\n", sum, rc);

    printf("\n  gcov 命令:\n");
    printf("    gcc -fprofile-arcs -ftest-coverage -o hello src/tools_sample.c\n");
    printf("    ./hello  (生成 .gcda 文件)\n");
    printf("    gcov -b src/tools_sample.c  (读取覆盖率数据)\n");
    printf("\n  gcov 输出示例:\n");
    printf("    File 'tools_sample.c'\n");
    printf("    Lines executed:85.00%% of 20\n");
    printf("    Branches executed:75.00%% of 16\n");
    printf("    Taken at least once:50.00%% of 16\n");
    printf("    No branches\n");
}

/* ============================================================
   Section 5: lcov — HTML 可视化报告
   ============================================================ */

static void demo_lcov(void)
{
    printf("\n=== Section 5: lcov HTML 报告 ===\n");

    printf("  lcov 命令链:\n");
    printf("    # 1. 收集覆盖率数据\n");
    printf("    lcov --capture -d build/obj/ -o coverage.info\n");
    printf("\n");
    printf("    # 2. 移除不关心的文件 (系统头文件等)\n");
    printf("    lcov --remove coverage.info '/usr/*' -o coverage.info\n");
    printf("\n");
    printf("    # 3. 生成 HTML 报告\n");
    printf("    genhtml coverage.info --output-directory coverage-html\n");
    printf("\n");
    printf("    # 4. 查看报告\n");
    printf("    open coverage-html/index.html\n");
    printf("\n");
    printf("  HTML 报告显示:\n");
    printf("    - 目录级: 每个文件/目录的覆盖率百分比\n");
    printf("    - 文件级: 源码每行是否被覆盖 (绿/红)\n");
    printf("    - 函数级: 每个函数是否被调用\n");
    printf("    - 分支级: 每个 if/else 分支是否被覆盖\n");
}

/* ============================================================
   Section 6: 静态分析常见问题 — cppcheck 能发现什么
   ============================================================ */

static void demo_static_analysis_problems(void)
{
    printf("\n=== Section 6: 静态分析问题演示 ===\n");

    printf("  cppcheck --enable=all src/ 可以检测:\n");
    printf("\n");

    printf("  1. 内存泄漏\n");
    printf("     int *p = malloc(100);  // 没有 free!\n");
    printf("     修复: free(p);\n");
    printf("\n");

    printf("  2. 空指针解引用\n");
    printf("     char *p = NULL;\n");
    printf("     // printf('%%c', *p);  -> cppcheck: nullPointer\n");
    printf("     修复: if (p != NULL) { /* ... */ }\n");
    printf("\n");

    printf("  3. 数组越界\n");
    printf("     int arr[5];\n");
    printf("     arr[5] = 42;  // cppcheck: arrayIndexOutOfBounds\n");
    printf("\n");

    printf("  4. 未使用的变量\n");
    printf("     int x = 5;    // cppcheck: unusedVariable\n");
    printf("\n");

    printf("  5. 未初始化的变量\n");
    printf("     int x;\n");
    printf("     // printf('%%d', x);  -> cppcheck: uninitvar\n");
    printf("\n");

    printf("  6. 格式化字符串不匹配\n");
    printf("     // printf('%%s', 42);  -> cppcheck: invalidscanf\n");
    printf("\n");

    printf("  7. 资源泄漏 (文件)\n");
    printf("     FILE *f = fopen(\"x\", \"r\"); // 没有 fclose!\n");
    printf("\n");
}

/* ============================================================
   Section 7: 运行 cppcheck
   ============================================================ */

static void demo_cppcheck_usage(void)
{
    printf("\n=== Section 7: cppcheck 运行指南 ===\n");

    printf("  基本用法:\n");
    printf("    cppcheck --enable=all --std=c17 .\n");
    printf("\n");

    printf("  常用选项:\n");
    printf("    --enable=all         启用所有检查\n");
    printf("    --std=c17           C17 标准\n");
    printf("    --inconclusive       包含不确定性警告\n");
    printf("    --suppress=missingInclude  忽略头文件缺失\n");
    printf("    --verbose           详细输出\n");
    printf("    --error-exitcode=1  发现错误时退出码为 1 (CI 友好)\n");
    printf("    --xml               XML 格式输出 (便于 CI 解析)\n");
    printf("\n");

    printf("  输出示例:\n");
    printf("    [src/main.c:12]: (error) Memory leak: p\n");
    printf("    [src/main.c:15]: (error) Null pointer dereference\n");
    printf("    [src/main.c:20]: (warning) Uninitialized variable: x\n");
    printf("    [src/main.c:25]: (style) The function 'foo' is never used\n");
    printf("\n");

    printf("  严重级别:\n");
    printf("    error   — 确定的 bug，必须修复\n");
    printf("    warning — 很可能有 bug，应该修复\n");
    printf("    style   — 代码风格问题，建议优化\n");
    printf("    performance — 性能问题\n");
    printf("    portability — 跨平台问题\n");
}

/* ============================================================
   Section 8: 编译器警告即静态分析 — -Wall -Wextra -Werror
   ============================================================ */

static void demo_compiler_warnings(void)
{
    printf("\n=== Section 8: 编译器警告 = 免费静态分析 ===\n");

    printf("  你的项目已经在做: gcc -Wall -Wextra -Werror -std=c17\n");
    printf("\n");

    printf("  -Wall  捕获:\n");
    printf("    - 未使用的变量 (-Wunused-variable)\n");
    printf("    - 未初始化的变量 (-Wuninitialized)\n");
    printf("    - 隐式函数声明 (-Wimplicit-function-declaration)\n");
    printf("    - 符号/无符号比较 (-Wsign-compare)\n");
    printf("    - 格式字符串不匹配 (-Wformat)\n");
    printf("\n");

    printf("  -Wextra 额外捕获:\n");
    printf("    - 未使用的参数 (-Wunused-parameter)\n");
    printf("    - 多余的符号 (-Wunused-result)\n");
    printf("    - 比较总是真/假 (-Wtype-limits)\n");
    printf("\n");

    printf("  -Werror 的行为:\n");
    printf("    warning -> error，编译失败。\n");
    printf("    好处: 不允许「带警告交付」\n");
    printf("    缺点: 第三方库可能有警告，可用 -isystem 排除\n");
}

/* ============================================================
   Section 9: 集成覆盖率到 Makefile
   ============================================================ */

static void demo_coverage_makefile(void)
{
    printf("\n=== Section 9: 覆盖率 Makefile 集成 ===\n");

    printf("  在 Makefile 末尾添加:\n");
    printf("\n");
    printf("  # ---- Code Coverage Targets ----\n");
    printf("  .PHONY: coverage coverage-clean\n");
    printf("\n");
    printf("  coverage-clean:\n");
    printf("  \tfind build/obj/ -name '*.gcda' -delete\n");
    printf("  \tfind build/obj/ -name '*.gcno' -delete\n");
    printf("  \trm -rf coverage.info coverage-html/\n");
    printf("\n");
    printf("  coverage: coverage-clean\n");
    printf("  \t@echo '=== Building with coverage flags ==='\n");
    printf("  \t@make build\n");
    printf("  \t@./build/bin/hello\n");
    printf("  \t@lcov --capture -d build/obj/ -o coverage.info \\\\\n");
    printf("  \t    --rc lcov_branch_coverage=1 2>/dev/null || \\\\\n");
    printf("  \t    echo '未安装 lcov, 跳过'\n");
    printf("  \t@gcov -b build/obj/ **/*.gcda 2>/dev/null || true\n");
    printf("  \t@echo '覆盖率报告已生成!'\n");
    printf("\n");
    printf("  然后运行: make coverage\n");
}

/* ============================================================
   Section 10: 集成 cppcheck 到 Makefile
   ============================================================ */

static void demo_cppcheck_makefile(void)
{
    printf("\n=== Section 10: cppcheck Makefile 集成 ===\n");

    printf("  Makefile 中添加:\n");
    printf("\n");
    printf("  # ---- Static Analysis Targets ----\n");
    printf("  .PHONY: lint lint-strict\n");
    printf("\n");
    printf("  lint:\n");
    printf("  \t@echo '=== 静态分析 (cppcheck) ==='\n");
    printf("  \t@cppcheck --enable=all --std=c17 \\\\\n");
    printf("  \t  --suppress=missingInclude \\\\\n");
    printf("  \t  --error-exitcode=0 \\\\\n");
    printf("  \t  src/ 2>&1 || true\n");
    printf("\n");
    printf("  lint-strict:\n");
    printf("  \t@echo '=== 静态分析 (严格模式) ==='\n");
    printf("  \t@cppcheck --enable=all --std=c17 \\\\\n");
    printf("  \t  --inconclusive \\\\\n");
    printf("  \t  --error-exitcode=1 \\\\\n");
    printf("  \t  src/\n");
    printf("\n");
    printf("  然后运行: make lint\n");
}

/* ============================================================
   Section 11: GitHub Actions CI 流水线
   ============================================================ */

static void demo_ci_pipeline(void)
{
    printf("\n=== Section 11: GitHub Actions CI 流水线 ===\n");

    printf("  .github/workflows/ci.yml:\n");
    printf("\n");
    printf("  name: C CI\n");
    printf("  on: [push, pull_request]\n");
    printf("\n");
    printf("  jobs:\n");
    printf("    build:\n");
    printf("      runs-on: ubuntu-latest\n");
    printf("      steps:\n");
    printf("        - uses: actions/checkout@v4\n");
    printf("        - name: Install dependencies\n");
    printf("          run: sudo apt-get update && sudo apt-get install -y \\\\\n");
    printf("               gcc cppcheck lcov\n");
    printf("\n");
    printf("        # Step 1: Build\n");
    printf("        - name: Build\n");
    printf("          run: make build\n");
    printf("\n");
    printf("        # Step 2: Test\n");
    printf("        - name: Test\n");
    printf("          run: make run\n");
    printf("\n");
    printf("        # Step 3: Static Analysis\n");
    printf("        - name: Static Analysis\n");
    printf("          run: make lint\n");
    printf("\n");
    printf("        # Step 4: Coverage\n");
    printf("        - name: Coverage\n");
    printf("          run: make coverage\n");
    printf("\n");
    printf("        # Step 5: Upload to Codecov\n");
    printf("        - name: Upload Coverage\n");
    printf("          uses: codecov/codecov-action@v3\n");
    printf("          with:\n");
    printf("            file: coverage.info\n");
    printf("\n");
    printf("  流水线阶段:\n");
    printf("    1. Build  ->  编译必须通过\n");
    printf("    2. Test   ->  所有测试必须通过\n");
    printf("    3. Lint   ->  静态分析无 error/warning\n");
    printf("    4. Coverage -> 覆盖率不低于阈值 (如 80%%)\n");
    printf("    5. Deploy ->   以上全部通过才部署\n");
}

/* ============================================================
   Section 12: 多平台编译
   ============================================================ */

static void demo_cross_platform(void)
{
    printf("\n=== Section 12: 多平台编译 ===\n");

    printf("  CI 矩阵编译 (多平台测试):\n");
    printf("\n");
    printf("  strategy:\n");
    printf("    matrix:\n");
    printf("      os: [ubuntu-latest, macos-latest]\n");
    printf("      cc: [gcc, clang]\n");
    printf("    exclude:\n");
    printf("      - os: macos-latest\n");
    printf("        cc: gcc\n");
    printf("\n");
    printf("  等价于: 在 Ubuntu+GCC, Ubuntu+Clang, macOS+Clang 上分别编译。\n");
    printf("\n");
    printf("  Windows (MinGW):\n");
    printf("    - os: windows-latest\n");
    printf("      run: make build CC=gcc\n");
}

/* ============================================================
   Section 13: 发布模式 — Release Builds
   ============================================================ */

static void demo_release_build(void)
{
    printf("\n=== Section 13: 发布模式 (Release Builds) ===\n");

    printf("  Debug vs Release:\n");
    printf("\n");
    printf("  Debug:\n");
    printf("    CFLAGS := -Wall -Wextra -Werror -std=c17 -g -O0\n");
    printf("    特点: 完整调试信息，无优化，便于 gdb 调试\n");
    printf("\n");
    printf("  Release:\n");
    printf("    CFLAGS := -Wall -Wextra -Werror -std=c17 -O2 -DNDEBUG\n");
    printf("    特点: 优化，断言被移除，体积更小\n");
    printf("\n");
    printf("  Makefile 集成:\n");
    printf("    .PHONY: debug release\n");
    printf("    debug: CFLAGS += -g -O0 -DDEBUG\n");
    printf("    debug: build\n");
    printf("    release: CFLAGS += -O2 -DNDEBUG\n");
    printf("    release: build\n");
    printf("\n");
    printf("    然后运行: make release\n");
}

/* ============================================================
   Section 14: 代码质量门禁
   ============================================================ */

static void demo_quality_gates(void)
{
    printf("\n=== Section 14: 代码质量门禁 ===\n");

    printf("  在 CI 中设置质量门禁:\n");
    printf("\n");
    printf("    # 覆盖率必须 >= 80%%\n");
    printf("    lcov --summary coverage.info | \\\\\n");
    printf("      grep 'lines' | awk '{ print $2 }' | head -1\n");
    printf("\n");
    printf("    # cppcheck 必须无 error\n");
    printf("    cppcheck --enable=all --error-exitcode=1 src/\n");
    printf("\n");
    printf("    # 编译器警告计数必须为 0\n");
    printf("    make build 2>&1 | grep -c 'warning' || true\n");
    printf("\n");
    printf("  质量门禁 = 代码的「出厂检验标准」。\n");
    printf("  不达标 -> 拒绝合并，直到修复。\n");
}

/* ============================================================
   Section 15: 工具链总结 — 完整工作流
   ============================================================ */

static void demo_toolchain_summary(void)
{
    printf("\n=== Section 15: 工具链完整工作流 ===\n");

    printf("  开发者的日常工具链:\n");
    printf("\n");
    printf("  开发中:\n");
    printf("    make build  (编译检查)\n");
    printf("    make run    (运行测试)\n");
    printf("\n");
    printf("  提交前:\n");
    printf("    make lint   (静态分析)\n");
    printf("    make coverage (覆盖率检查)\n");
    printf("\n");
    printf("  CI 自动执行:\n");
    printf("    git push -> CI 触发:\n");
    printf("      1. make build (编译)\n");
    printf("      2. make run   (测试)\n");
    printf("      3. make lint  (静态分析)\n");
    printf("      4. make coverage (覆盖率)\n");
    printf("      5. 全部通过 -> 合并到 main\n");
    printf("\n");
    printf("  这就是工厂的「全流程质检体系」:\n");
    printf("    工人自检(编译) -> 班组互查(测试) -> 质检员巡检(lint) ->\n");
    printf("    抽检报告(coverage) -> 出厂检验(CI门禁)\n");
}

/* ============================================================
   公共入口
   ============================================================ */

int main_tools_sample(void)
{
    printf("============================================\n");
    printf("  工具链 (Toolchain) 🟢\n");
    printf("============================================\n");

    demo_problem();
    print_makefile_structure();
    demo_gcov();
    demo_lcov();
    demo_static_analysis_problems();
    demo_cppcheck_usage();
    demo_compiler_warnings();
    demo_coverage_makefile();
    demo_cppcheck_makefile();
    demo_ci_pipeline();
    demo_cross_platform();
    demo_release_build();
    demo_quality_gates();
    demo_toolchain_summary();

    printf("\ntools sample done.\n");
    return 0;
}
