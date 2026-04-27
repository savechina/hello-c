#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "advance/system_cli_sample.h"

/* ====================================================================
 * CLI 类比: CLI 像餐馆点单系统 — argv[1] 是主菜, -v 是加辣,
 *           --help 是菜单
 * ==================================================================== */

/* ── 1. getopt Option Parsing ── */

static void demo_getopt(void)
{
    printf("=== 1. getopt 选项解析 (demo_getopt) ===\n");
    printf("  类比: getopt 是自动点单机 — 帮你解析 -v, -o, --version 等\n\n");

    printf("  短选项格式 (getopt):\n");
    printf("    -v          verbose 模式\n");
    printf("    -o filename 输出到文件\n");
    printf("    -n count    执行次数\n\n");

    printf("  long options 需手动解析 argv (getopt_long 需要 <getopt.h>):\n");
    printf("    --verbose   = -v\n");
    printf("    --output f  = -o f\n");
    printf("    --version   输出版本\n");
    printf("    --help      显示帮助\n\n");

    printf("  getopt 字符串: \"vo:n:\" 解析规则:\n");
    printf("    v      — 不需要参数 (-v)\n");
    printf("    o:     — 需要参数 (-o filename)\n");
    printf("    n:     — 需要参数 (-n 3)\n\n");

    printf("  代码模式:\n");
    printf("    while ((ch = getopt(argc, argv, \"vo:n:\")) != -1) {\n");
    printf("        switch (ch) {\n");
    printf("            case 'v': verbose = 1; break;\n");
    printf("            case 'o': output  = optarg; break;\n");
    printf("            case 'n': count   = atoi(optarg); break;\n");
    printf("            case '?': usage(); exit(1);\n");
    printf("        }\n");
    printf("    }\n");
    printf("    // 剩余非选项参数: argv[optind .. argc-1]\n\n");

    printf("  错误-first 教训:\n");
    printf("    ❌ 忘记 optarg 可能为 NULL (如 -o 后无参数)\n");
    printf("    ❌ 不处理 '?' 导致非法选项被忽略\n");
    printf("    ❌ 不检查 optind 导致剩余参数被遗漏\n");
    printf("    ✅ 每次用 optarg 前检查: if (!optarg) { error(); }\n\n");
}

/* ── 2. Exit Codes ── */

static void demo_exit_codes(void)
{
    printf("=== 2. 退出码 (demo_exit_codes) ===\n");
    printf("  类比: 退出码像医生的诊断结果 — 0=健康, 非0=不同病症\n\n");

    printf("  标准退出码约定:\n");
    printf("    EXIT_SUCCESS (0) — 成功\n");
    printf("    EXIT_FAILURE (1) — 通用失败\n");
    printf("      2 — 用法错误 (wrong usage)\n");
    printf("      2 — 输入文件不存在\n");
    printf("      3 — 权限不足\n");
    printf("      4 — 内存分配失败\n");
    printf("      5 — 网络错误\n");
    printf("      6 — 配置错误\n");
    printf("     126 — 命令找到但不可执行\n");
    printf("     127 — 命令未找到\n");
    printf("     128+N — 被信号 N 终止 (如 SIGKILL=9, 128+9=137)\n\n");

    printf("  代码模式:\n");

    printf("    int main(int argc, char **argv) {\n");
    printf("        if (argc < 2) {\n");
    printf("            fprintf(stderr, \"用法: %%s <file>\\n\", argv[0]);\n");
    printf("            return 2;  // 用法错误\n");
    printf("        }\n");
    printf("\n");
    printf("        FILE *f = fopen(argv[1], \"r\");\n");
    printf("        if (!f) {\n");
    printf("            fprintf(stderr, \"错误: 无法打开文件 '%%s'\\n\", argv[1]);\n");
    printf("            return 2;  // 文件不存在\n");
    printf("        }\n");
    printf("\n");
    printf("        // 主逻辑...\n");
    printf("        fclose(f);\n");
    printf("        return EXIT_SUCCESS;  // 0\n");
    printf("    }\n\n");

    printf("  实用技巧:\n");
    printf("    Shell 中用 $? 检查上一个命令的退出码\n");
    printf("    $ mycmd && echo OK || echo FAIL\n");
    printf("    仅当 mycmd 返回 0 时才打印 OK\n\n");
}

/* ── 3. Args Validation + Usage ── */

static void demo_args_validation(void)
{
    printf("=== 3. 参数验证 (demo_args_validation) ===\n");
    printf("  类比: 参数验证像服务员检查点单 — 必须有主食，可以加辣\n\n");

    printf("  常见 CLI 参数模式:\n");
    printf("    %s [options] <required_arg> [optional_arg]\n\n", "mytool");

    printf("  required args 验证:\n");
    printf("    if (argc < 2) {\n");
    printf("        fprintf(stderr, \"错误: 缺少必需参数 <file>\\n\");\n");
    printf("        fprintf(stderr, \"用法: %%s <file> [--verbose]\\n\", argv[0]);\n");
    printf("        return EXIT_FAILURE;\n");
    printf("    }\n\n");

    printf("  完整 usage 模板:\n");
    printf("    void usage(const char *prog) {\n");
    printf("        fprintf(stderr, \"用法: %%s [选项] <文件>\\n\", prog);\n");
    printf("        fprintf(stderr, \"\\n选项:\\n\");\n");
    printf("        fprintf(stderr, \"  -v, --verbose    显示详细信息\\n\");\n");
    printf("        fprintf(stderr, \"  -o, --output F   输出到文件 F\\n\");\n");
    printf("        fprintf(stderr, \"  -n, --count N    处理 N 行\\n\");\n");
    printf("        fprintf(stderr, \"  -h, --help       显示此帮助信息\\n\");\n");
    printf("        fprintf(stderr, \"  -V, --version    显示版本号\\n\");\n");
    printf("    }\n\n");

    printf("  参数验证清单:\n");
    printf("    [✓] argc 是否满足最少参数\n");
    printf("    [✓] 必需参数是否存在\n");
    printf("    [✓] 可选参数格式是否正确\n");
    printf("    [✓] 数值参数的范围 (如 n > 0)\n");
    printf("    [✓] 路径参数是否可访问\n");
    printf("    [✓] 互斥选项是否冲突\n\n");

    printf("  互斥选项示例:\n");
    printf("    if (input_file && input_stdin) {\n");
    printf("        fprintf(stderr, \"错误: -f 和 --stdin 不能同时使用\\n\");\n");
    printf("        usage(argv[0]);\n");
    printf("        return 2;\n");
    printf("    }\n\n");

    printf("  CLI 最佳实践:\n");
    printf("    1. 错误信息写 stderr (不是 stdout)\n");
    printf("    2. 正常输出写 stdout (管道友好)\n");
    printf("    3. --verbose 控制额外输出\n");
    printf("    4. --help 写 stdout, 错误写 stderr\n");
    printf("    5. 支持 -V/--version 和 -h/--help\n");
    printf("    6. 遵循 POSIX/GNU 选项约定\n\n");
}

/* ── Coordinator entry ── */

int main_system_cli_sample(void)
{
    printf("========================================\n");
    printf("  CLI 开发模式 (Command-Line Interface)\n");
    printf("========================================\n\n");

    printf("  类比: CLI 像餐馆点单系统 — argv[1] 是主菜\n\n");

    demo_getopt();
    demo_exit_codes();
    demo_args_validation();

    printf("CLI 开发模式演示完毕。\n");
    return 0;
}
