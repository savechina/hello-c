#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "basic/cli_args_sample.h"

/* ============================================================
 * Section 1: argc / argv — Simulated Command-Line Parsing
 * ============================================================ */

static void demo_argc_argv(void)
{
    printf("==== [1] argc / argv — 命令行参数解析 ====\n\n");

    /* ── 模拟 argv 数组（因为在 coordinator 中调用，无法接收真正的 cli args） ── */
    char *argv[] = {"hello-c", "-v", "--mode=fast", "input.txt", NULL};
    int argc = 4; /* not counting NULL */

    printf("  argc = %d\n", argc);
    printf("  argv[0] = \"%s\" (程序名)\n", argv[0]);
    for (int i = 1; i < argc; i++) {
        printf("  argv[%d] = \"%s\"\n", i, argv[i]);
    }

    printf("\n");

    /* ── 解析模拟参数 ── */
    int verbose = 0;
    const char *mode = "default";
    const char *input_file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
            printf("  解析: '-v' → verbose 模式启用\n");
        } else if (strncmp(argv[i], "--mode=", 7) == 0) {
            mode = argv[i] + 7;
            printf("  解析: '--mode=%s'\n", mode);
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
            printf("  解析: input_file = \"%s\"\n", input_file);
        }
    }

    printf("\n  结果: verbose=%d, mode=\"%s\", input=\"%s\"\n\n",
           verbose, mode, input_file ? input_file : "(null)");

    printf("💡 argv[0] 总是程序名。有效参数从 argv[1] 开始。\n\n");
}

/* ============================================================
 * Section 2: argc 检查 — 防止越界访问
 * ============================================================ */

static void demo_argc_check(void)
{
    printf("==== [2] argc 检查 — 防止越界 ====\n\n");

    /* ── ❌ 危险：不检查 argc 直接访问 argv[1] ── */
    printf("❌ 危险做法:\n");
    printf("   int main(int argc, char *argv[]) {\n");
    printf("       printf(\"%%s\\\\n\", argv[1]);  // argc==1 时 → 越界! 💥\n");
    printf("   }\n");
    printf("   → Segment Fault! argv[1] 不存在，访问非法内存。\n\n");

    /* ── ✅ 正确：始终检查 argc ── */
    printf("✅ 正确做法:\n");

    char *test_argv[] = {"hello-c", NULL};  /* 只有一个参数（程序名）*/
    int test_argc = 1;

    printf("   int argc = %d;\n", test_argc);
    printf("   if (argc < 2) {\n");
    printf("       fprintf(stderr, \"用法: %%s <filename>\\\\n\", argv[0]);\n");
    printf("       return 1;\n");
    printf("   }\n\n");

    if (test_argc < 2) {
        fprintf(stderr, "   → 用法: %s <filename>\n", test_argv[0]);
        printf("   → argc 检查成功拦截了越界访问。\n\n");
    }

    printf("💡 经验法则：每次使用 argv[i] 前，先验证 argc > i。\n\n");
}

/* ============================================================
 * Section 3: stdin / stdout / stderr 概念
 * ============================================================ */

static void demo_stdio_streams(void)
{
    printf("==== [3] stdin / stdout / stderr — 标准流 ====\n\n");

    printf("  ┌── 三个标准流 ───────────────────────────────┐\n");
    printf("  │                                              │\n");
    printf("  │  流            文件描述符  默认方向           │\n");
    printf("  │  ────────────  ──────────  ────────          │\n");
    printf("  │  stdin        0           键盘 → 程序        │\n");
    printf("  │  stdout       1           程序 → 终端        │\n");
    printf("  │  stderr       2           程序 → 终端        │\n");
    printf("  │                                              │\n");
    printf("  │  ┌── keyboard ──┐                           │\n");
    printf("  │  │              │ stdin (fd 0)               │\n");
    printf("  │  │   [键盘输入] ───→  ┌──────────────┐      │\n");
    printf("  │  │              │      │              │      │\n");
    printf("  │  └──────────────┘      │    程序       │      │\n");
    printf("  │              ┌──────→  │              │      │\n");
    printf("  │              │         └───┬──────┬───┘      │\n");
    printf("  │         stdout(1)     │        │            │\n");
    printf("  │         stderr (2)    │        │            │\n");
    printf("  │                       ↓        ↓            │\n");
    printf("  │                 正常输出  错误输出             │\n");
    printf("  │                                              │\n");
    printf("  │  stdout 和 stderr 都默认输出到终端，           │\n");
    printf("  │  但可以通过重定向分开（见 Section 6）         │\n");
    printf("  └──────────────────────────────────────────────┘\n\n");

    /* ── 演示：stdout vs stderr ── */
    printf("  stdout 输出: ");
    printf("这里是正常输出（printf → stdout）\n");

    fprintf(stderr, "  stderr 输出: 这里是错误信息（fprintf(stderr, ...)）\n");
    fprintf(stderr, "  错误信息不经过 stdout 缓冲，立即显示。\n\n");

    printf("💡 printf() 写入 stdout。用 fprintf(stderr, ...) 写错误信息。\n");
    printf("   这样用户可以重定向 stdout 到文件，但仍看到错误信息。\n\n");
}

/* ============================================================
 * Section 4: stdin 读取 — fgets from stdin
 * ============================================================ */

static void demo_stdin_reading(void)
{
    printf("==== [4] stdin 读取 — fgets 从标准输入 ====\n\n");

    /* ── 模拟 stdin 读取（无法在 coordinator 中真正等待用户输入） ── */
    printf("  ┌── 用 fgets 从 stdin 读取 ──────────────────┐\n");
    printf("  │                                              │\n");
    printf("  │  char buf[256];                              │\n");
    printf("  │  printf(\"请输入你的名字: \");                   │\n");
    printf("  │  fgets(buf, sizeof(buf), stdin);             │\n");
    printf("  │                                              │\n");
    printf("  │  → buf 中包含用户输入的内容（含 \\n）          │\n");
    printf("  │  → 遇到 EOF（Ctrl+D）时 fgets 返回 NULL       │\n");
    printf("  │  → fgets 安全：最多读 sizeof(buf)-1 字符      │\n");
    printf("  │                                              │\n");
    printf("  │  与 gets() 的区别:                             │\n");
    printf("  │    gets(buf)       → 无限输入 → 溢出 💥       │\n");
    printf("  │    fgets(buf,s,stdin)→ 限制输入 → 安全 ✓      │\n");
    printf("  └──────────────────────────────────────────────┘\n\n");

    /* ── 展示模拟交互 ── */
    printf("  ── 模拟用户交互 ──\n");

    /* 模拟 fgets 从 stdin 读取一行 */
    const char *simulated_input = "Hello, world!";
    printf("  [模拟用户输入] \"%s\"\n", simulated_input);

    char buf[256];
    strncpy(buf, simulated_input, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* 去除末尾换行 */
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
    printf("  [程序收到] \"%s\" (长度 %zu)\n\n", buf, strlen(buf));

    printf("💡 实际运行时，fgets(buf, sizeof(buf), stdin) 会阻塞等待用户输入。\n");
    printf("   本教程以模拟方式展示概念。\n\n");
}

/* ============================================================
 * Section 5: Python sys.argv vs C argc/argv
 * ============================================================ */

static void demo_python_vs_c_args(void)
{
    printf("==== [5] Python sys.argv vs C argc/argv ====\n\n");

    printf("  ┌── 对比 ──────────────────────────────────┐\n");
    printf("  │                                          │\n");
    printf("  │  Python:                                 │\n");
    printf("  │    import sys                            │\n");
    printf("  │    print(sys.argv[1])   # 'hello'        │\n");
    printf("  │    len(sys.argv)        # 2              │\n");
    printf("  │    sys.argv[0]          # 'program.py'   │\n");
    printf("  │                                          │\n");
    printf("  │  C:                                      │\n");
    printf("  │    int main(int argc, char *argv[]) {    │\n");
    printf("  │        printf(\"%%s\\\\n\", argv[1]);  // 'hello'│\n");
    printf("  │        printf(\"%%d\", argc);          // 2    │\n");
    printf("  │    }                                    │\n");
    printf("  │                                          │\n");
    printf("  │  差异:                                    │\n");
    printf("  │  ① Python 自动提供 sys.argv, C 在 main() │\n");
    printf("  │  ② Python 数组越界 → IndexError,        │\n");
    printf("  │     C 数组越界 → Segmentation Fault!     │\n");
    printf("  │  ③ Python 全是字符串, C 也是字符串       │\n");
    printf("  │     (C 需要 atoi/stdstrtol 转数字)       │\n");
    printf("  └──────────────────────────────────────────┘\n\n");

    /* ── atoi 转换演示 ── */
    printf("  ── C 中数字参数解析 ──\n");
    const char *num_str = "42";
    int value = 0;
    for (int i = 0; num_str[i] != '\0'; i++) {
        if (num_str[i] >= '0' && num_str[i] <= '9') {
            value = value * 10 + (num_str[i] - '0');
        }
    }
    printf("  手动解析: \"%s\" → %d\n", num_str, value);
    printf("  实际开发请用 strtol() 替代手动解析。\n\n");
}

/* ============================================================
 * Section 6: I/O 重定向与管道概念
 * ============================================================ */

static void demo_redirection_pipe(void)
{
    printf("==== [6] I/O 重定向与管道概念 ====\n\n");

    printf("  ┌── Shell 重定向操作符 ──────────────────────┐\n");
    printf("  │                                              │\n");
    printf("  │  操作符    含义         示例                 │\n");
    printf("  │  ────────  ──────────  ──────────────────── │\n");
    printf("  │  >         覆盖输出    ./prog > out.txt     │\n");
    printf("  │  >>        追加输出    ./prog >> out.txt    │\n");
    printf("  │  <         文件输入    ./prog < input.txt   │\n");
    printf("  │  |         管道        ./prog | grep \"foo\" │\n");
    printf("  │  2>        错误重定向  ./prog 2> err.log    │\n");
    printf("  │  &>        全重定向    ./prog &> all.log    │\n");
    printf("  │                                              │\n");
    printf("  │  ┌── stdout(1) ──→ out.txt                   │\n");
    printf("  │  │ 程序 →  │                                 │\n");
    printf("  │  └── stderr(2) ──→ err.log                   │\n");
    printf("  │                                              │\n");
    printf("  │  管道 | 把前一个程序的 stdout 连到           │\n");
    printf("  │  后一个程序的 stdin                          │\n");
    printf("  │                                              │\n");
    printf("  │  cat data.txt | grep \"ERR\" | wc -l           │\n");
    printf("  │  → cat.stdout → grep.stdin                   │\n");
    printf("  │  → grep.stdout → wc.stdin                    │\n");
    printf("  │  → wc.stdout → 终端                          │\n");
    printf("  └──────────────────────────────────────────────┘\n\n");

    printf("💡 C 程序无需知晓这些操作 —— 操作系统在程序启动前\n");
    printf("   就重定向了文件描述符。程序照常 printf/fgets，\n");
    printf("   数据自动流向重定向的目标。\n\n");
}

/* ============================================================
 * Section 7: 交互式输入模拟
 * ============================================================ */

static void demo_interactive_simulation(void)
{
    printf("==== [7] 交互式输入模式演示 ====\n\n");

    printf("  ── 交互式 vs 批处理 ──\n\n");

    printf("  交互式模式（等待用户输入）:\n");
    printf("    while (1) {\n");
    printf("        printf(\"> \");\n");
    printf("        if (fgets(buf, sizeof(buf), stdin) == NULL) break;\n");
    printf("        // 处理 buf...\n");
    printf("    }\n\n");

    printf("  批处理模式（从 stdin 读取所有数据）:\n");
    printf("    while (fgets(buf, sizeof(buf), stdin) != NULL) {\n");
    printf("        // 处理每一行\n");
    printf("    }\n\n");

    /* ── 模拟交互式循环 ── */
    const char *sim_commands[] = {"add 10 20", "sub 50 30", "quit", NULL};
    int sim_count = 3;

    printf("  ┌── 模拟交互式 REPL ─────────────────┐\n");
    for (int i = 0; i < sim_count; i++) {
        const char *cmd = sim_commands[i];
        printf("  │ > %s\n", cmd);

        if (strcmp(cmd, "quit") == 0) {
            printf("  │   退出交互式模式\n");
            break;
        } else if (strncmp(cmd, "add ", 4) == 0) {
            int a = 10, b = 20;  /* 简化模拟 */
            printf("  │   结果: %d\n", a + b);
        } else if (strncmp(cmd, "sub ", 4) == 0) {
            int a = 50, b = 30;
            printf("  │   结果: %d\n", a - b);
        } else {
            printf("  │   未知命令: %s\n", cmd);
        }
    }
    printf("  └──────────────────────────────────────┘\n\n");

    printf("💡 fgets + stdin 是 C 中最简单的交互方式。\n");
    printf("   需要更复杂的终端交互（方向键、历史记录）时，使用 readline 库。\n\n");
}

/* ============================================================
 * Section 8: 错误模式 — argc/argv 常见陷阱
 * ============================================================ */

static void demo_common_arg_errors(void)
{
    printf("==== [8] 常见 CLI 错误模式 ====\n\n");

    printf("  ❌ 错误 1: 不检查 argc\n");
    printf("     printf(\"%%s\\\\n\", argv[1]);  // argc 可能为 1!\n");
    printf("     ✅ 修复: if (argc < 2) { /* 打印用法 */ }\n\n");

    printf("  ❌ 错误 2: 用 atoi 不检查转换结果\n");
    printf("     int n = atoi(\"abc\");  // 返回 0, 不报错!\n");
    printf("     ✅ 修复: 用 strtol() 检查 errno\n\n");

    printf("  ❌ 错误 3: 混淆 argv 和 stdin\n");
    printf("     argv[1] = \"hello.txt\"  // 这是文件名，不是文件内容!\n");
    printf("     需要先 fopen(argv[1], \"r\") 才能读取内容\n\n");

    printf("  ❌ 错误 4: argv 元素是 char*，不要修改\n");
    printf("     argv[1][0] = 'A';  // ❌ 某些系统上 argv 在只读段\n");
    printf("     ✅ 修复: 需要修改时先 strncpy 到自己的缓冲区\n\n");

    printf("  💡 练习建议:\n");
    printf("     🟢 入门: 写程序打印 argc 和所有 argv\n");
    printf("     🟡 中级: 支持 -v (verbose) 和 -o (输出文件) 参数\n");
    printf("     🔴 挑战: 实现简易 grep: 从 stdin 读取并匹配字符串\n");
}

/* ============================================================
 * Section 9: getopt 概念（选学）
 * ============================================================ */

static void demo_getopt_concept(void)
{
    printf("==== [9] getopt — 标准参数解析（选学） ====\n\n");

    printf("  当命令行参数变多时，手动 strcmp 会非常繁琐。\n");
    printf("  POSIX 提供了 getopt() 函数来简化解析：\n\n");

    printf("  ┌── getopt 概念演示 ───────────────────────┐\n");
    printf("  │                                          │\n");
    printf("  │  /* 伪代码 - 在真实程序中编译 */          │\n");
    printf("  │  #include <unistd.h>                     │\n");
    printf("  │                                          │\n");
    printf("  │  int opt;                                │\n");
    printf("  │  while ((opt = getopt(argc, argv, \"vh:\"))│\n");
    printf("  │         != -1) {                         │\n");
    printf("  │      switch (opt) {                      │\n");
    printf("  │      case 'v': verbose = 1; break;      │\n");
    printf("  │      case 'h': usage(); break;           │\n");
    printf("  │      case ':': /* 缺少参数值 */ break;   │\n");
    printf("  │      case '?': /* 未知选项 */ break;     │\n");
    printf("  │      }                                    │\n");
    printf("  │  }                                        │\n");
    printf("  │                                          │\n");
    printf("  │  \"vh:\" 含义: v=无参数标志, h:=需要参数   │\n");
    printf("  │  ./prog -v -o out.txt input.txt          │\n");
    printf("  └──────────────────────────────────────────┘\n\n");

    printf("💡 本章使用手动 strcmp 演示原理。实际项目中\n");
    printf("   推荐用 getopt()（POSIX）或 argparse 库。\n\n");
}

/* ============================================================
 * Coordinator: main_cli_args_sample
 * ============================================================ */

int main_cli_args_sample(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     命令行参数与 I/O 重定向 (CLI Args & I/O Redirect)   ║\n");
    printf("║     argc/argv, stdin/stdout/stderr, 重定向 & 管道        ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    demo_argc_argv();              /* Section 1 */
    demo_argc_check();             /* Section 2 (error-first) */
    demo_stdio_streams();          /* Section 3 */
    demo_stdin_reading();          /* Section 4 */
    demo_python_vs_c_args();       /* Section 5 */
    demo_redirection_pipe();       /* Section 6 */
    demo_interactive_simulation(); /* Section 7 */
    demo_common_arg_errors();      /* Section 8 */
    demo_getopt_concept();         /* Section 9 (选学) */

    printf("  cli_args_sample done.\n");
    return 0;
}
