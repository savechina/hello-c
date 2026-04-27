#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "basic/file_io_sample.h"

/* ── file paths for demos ── */
#define TEXT_DEMO_FILE  "demo_text.txt"
#define BINARY_DEMO_FILE "demo_binary.bin"
#define POSITION_FILE   "demo_position.txt"

/* ── student record struct for binary demo ── */
typedef struct {
    int32_t  id;
    char     name[32];
    double   score;
} StudentRecord;

/* ============================================================
 * Section 1: fopen — Opening a Text File for Writing
 * ============================================================ */

static void demo_fopen_write(void)
{
    printf("==== [1] fopen — 写入文本文件 ====\n\n");

    FILE *fp = fopen(TEXT_DEMO_FILE, "w");
    if (fp == NULL) {
        fprintf(stderr, "❌ 无法打开文件 '%s': %s\n", TEXT_DEMO_FILE, strerror(errno));
        return;
    }

    fprintf(fp, "Hello, File I/O!\n");
    fprintf(fp, "This is line %d.\n", 2);
    fprintf(fp, "C 语言的文件操作并不难。\n");

    fclose(fp);
    printf("✅ 文件 '%s' 已写入并关闭。\n\n", TEXT_DEMO_FILE);
}

/* ============================================================
 * Section 2: fopen + fgets — Reading a Text File Line by Line
 * ============================================================ */

#define READ_BUFFER_SIZE 256

static void demo_fgets_read(void)
{
    printf("==== [2] fgets — 逐行读取文本文件 ====\n\n");

    /* First, ensure the file exists (write it if needed) */
    FILE *fp = fopen(TEXT_DEMO_FILE, "r");
    if (fp == NULL) {
        printf("⚠️ 文件 '%s' 不存在，请先运行 Section 1 创建文件。\n\n", TEXT_DEMO_FILE);
        return;
    }

    char buffer[READ_BUFFER_SIZE];
    int line_num = 0;

    printf("  读取内容:\n");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        line_num++;
        /* Remove trailing newline for display */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        printf("  第 %d 行: %s\n", line_num, buffer);
    }
    printf("  共读取 %d 行。\n\n", line_num);

    fclose(fp);
}

/* ============================================================
 * Section 3: fopen Error Handling — File Does Not Exist
 * ============================================================ */

static void demo_fopen_error_handling(void)
{
    printf("==== [3] fopen 错误处理 — 文件不存在 ====\n\n");

    /* ── ❌ 危险做法：不检查 NULL 直接写入 ── */
    printf("❌ 危险做法（不要在实际代码中这样做！）:\n");
    printf("   fp = fopen(\"nonexistent.dat\", \"r\");\n");
    printf("   fscanf(fp, \"%%s\", buf);  // fp == NULL → Segmentation Fault!\n\n");

    /* ── ✅ 正确做法：始终检查 fopen 返回值 ── */
    printf("✅ 正确做法：检查 NULL 后再操作:\n");

    FILE *fp = fopen("nonexistent.dat", "r");
    if (fp == NULL) {
        /* strerror(errno) 给出人类可读的错误描述 */
        fprintf(stderr, "   fopen 失败: %s (errno=%d)\n", strerror(errno), errno);
        printf("   → 安全地处理了错误，程序没有崩溃。\n\n");
    } else {
        fclose(fp);
    }

    printf("💡 经验法则：每次调用 fopen 后都必须检查返回值是否为 NULL。\n\n");
}

/* ============================================================
 * Section 4: fprintf + fscanf — Formatted I/O
 * ============================================================ */

static void demo_formatted_io(void)
{
    printf("==== [4] fprintf + fscanf — 格式化输入输出 ====\n\n");

    /* ── Write: fprintf to file ── */
    FILE *fp = fopen(TEXT_DEMO_FILE, "w");
    if (fp == NULL) {
        fprintf(stderr, "❌ fopen failed: %s\n", strerror(errno));
        return;
    }

    fprintf(fp, "%-10s %5s %8s\n", "Name", "Age", "Score");
    fprintf(fp, "%-10s %5d %8.2f\n", "Alice",   20, 95.50);
    fprintf(fp, "%-10s %5d %8.2f\n", "Bob",     22, 87.75);
    fprintf(fp, "%-10s %5d %8.2f\n", "Charlie", 21, 92.00);
    fclose(fp);
    printf("✅ fprintf — 表格写入完成。\n\n");

    /* ── Read: fscanf from file ── */
    fp = fopen(TEXT_DEMO_FILE, "r");
    if (fp == NULL) {
        fprintf(stderr, "❌ fopen failed: %s\n", strerror(errno));
        return;
    }

    char header_line[READ_BUFFER_SIZE];
    /* Skip header line */
    if (fgets(header_line, sizeof(header_line), fp) != NULL) {
        printf("  标题行: %s\n", header_line);
    }

    printf("  fscanf 读取数据:\n");
    char name[32];
    int age;
    double score;
    while (fscanf(fp, "%31s %d %lf", name, &age, &score) == 3) {
        printf("    %-10s Age=%3d  Score=%6.2f\n", name, age, score);
    }

    fclose(fp);
    printf("\n");
    printf("💡 fscanf 返回成功匹配的字段数。用 == 3 检查确保完整读取一行。\n\n");
}

/* ============================================================
 * Section 5: fgets vs gets — Safety Comparison
 * ============================================================ */

static void demo_fgets_vs_gets(void)
{
    printf("==== [5] fgets vs gets — 安全对比 ====\n\n");

    /* ── ❌ gets() 已被 C11 标准废弃 ── */
    printf("❌ gets() — 无边界检查，极度危险！\n");
    printf("   char buf[10];\n");
    printf("   gets(buf);  // 用户输入 100 个字符 → 栈溢出! 💥\n");
    printf("   gets() 不知道缓冲区大小，无限写入 → 缓冲区溢出。\n");
    printf("   C11 标准已将 gets() 从标准库中**彻底删除**。\n");
    printf("   永远、永远不要在代码中使用 gets()！\n\n");

    /* ── ✅ fgets() — 安全替代方案 ── */
    printf("✅ fgets() — 指定最大读取字符数\n");

    /* Demonstrate with a file, since we can't simulate stdin easily */
    FILE *fp = fopen(TEXT_DEMO_FILE, "r");
    if (fp != NULL) {
        char line[16];  /* intentionally small buffer */
        printf("   用 fgets(line, 16, fp) 读取文件: (buffer 仅 16 字节)\n");
        while (fgets(line, sizeof(line), fp) != NULL) {
            size_t len = strlen(line);
            if (len > 0 && line[len - 1] == '\n') {
                line[len - 1] = '\0';
                printf("     → \"%s\" (完整行, 含 \\n)\n", line);
            } else if (len == (sizeof(line) - 1)) {
                printf("     → \"%s\" (被截断! buffer 满了)\n", line);
            } else {
                printf("     → \"%s\" (EOF 前剩余内容)\n", line);
            }
        }
        fclose(fp);
    }

    printf("\n💡 fgets(buf, size, stream) 最多读 size-1 个字符，总是 \\0 终止。\n");
    printf("   gets(buf) — 无大小参数 → 溢出。永远用 fgets！\n\n");
}

/* ============================================================
 * Section 6: fwrite + fread — Binary I/O
 * ============================================================ */

static void demo_binary_io(void)
{
    printf("==== [6] fwrite + fread — 二进制文件读写 ====\n\n");

    /* ── Prepare test data ── */
    StudentRecord students[] = {
        {1001, "Alice",   95.5},
        {1002, "Bob",     87.75},
        {1003, "Charlie", 92.0},
    };
    int count = 3;

    /* ── Write: fwrite raw struct to file ── */
    FILE *fp = fopen(BINARY_DEMO_FILE, "wb");
    if (fp == NULL) {
        fprintf(stderr, "❌ fwrite fopen failed: %s\n", strerror(errno));
        return;
    }

    size_t written = fwrite(students, sizeof(StudentRecord), count, fp);
    fclose(fp);
    printf("✅ fwrite: 写入 %zu 条记录到 '%s'\n", written, BINARY_DEMO_FILE);
    printf("   每条记录 %zu 字节 (id=4 + name=%zu + score=8)\n",
           sizeof(StudentRecord), sizeof(((StudentRecord*)0)->name));
    printf("   总写入: %zu 字节\n\n", written * sizeof(StudentRecord));

    /* ── Read: fread raw struct from file ── */
    fp = fopen(BINARY_DEMO_FILE, "rb");
    if (fp == NULL) {
        fprintf(stderr, "❌ fread fopen failed: %s\n", strerror(errno));
        return;
    }

    StudentRecord loaded;
    int idx = 0;
    printf("✅ fread: 从 '%s' 读取:\n", BINARY_DEMO_FILE);
    while (fread(&loaded, sizeof(StudentRecord), 1, fp) == 1) {
        printf("   [%d] id=%d  name='%-10s'  score=%.2f\n",
               idx, loaded.id, loaded.name, loaded.score);
        idx++;
    }
    fclose(fp);
    printf("   共读取 %d 条记录。\n\n", idx);

    printf("💡 fwrite/fread 直接按\"内存布局\"读写，速度极快。\n");
    printf("   ⚠️ 警告：不同平台/编译器对 struct 的 padding 可能不同，\n");
    printf("   跨平台传输时建议使用文本格式或显式序列化。\n\n");
}

/* ============================================================
 * Section 7: Text vs Binary Mode
 * ============================================================ */

static void demo_text_vs_binary(void)
{
    printf("==== [7] 文本模式 vs 二进制模式 ====\n\n");

    printf("  ┌── fopen 模式对比 ─────────────────────────┐\n");
    printf("  │                                              │\n");
    printf("  │  模式  含义    平台差异                      │\n");
    printf("  │  ──    ────    ────                          │\n");
    printf("  │  \"r\"   读     POSIX: 无区别                  │\n");
    printf("  │  \"rb\"  读二进                             │\n");
    printf("  │  \"w\"   写(覆盖) POSIX: 无区别               │\n");
    printf("  │  \"wb\"  写二                              │\n");
    printf("  │  \"a\"   追加    Windows: 换行 \\n → \\r\\n     │\n");
    printf("  │  \"ab\"  追加二                            │\n");
    printf("  │  \"r+\"  读写    文件必须存在                │\n");
    printf("  │  \"w+\"  读写    创建新文件/清空已有          │\n");
    printf("  │                                              │\n");
    printf("  │  ⚠️ Windows 上文本模式会转换 \\n ↔ \\r\\n     │\n");
    printf("  │     二进制模式则原样读写（推荐跨平台时使用） │\n");
    printf("  └──────────────────────────────────────────────┘\n\n");

    /* Demonstrate: write same data, compare in "text" and "binary" modes */
    const char *test_str = "Line1\nLine2\n";

    FILE *fp = fopen(BINARY_DEMO_FILE, "wb");
    fwrite(test_str, 1, strlen(test_str), fp);
    fclose(fp);

    fp = fopen(BINARY_DEMO_FILE, "rb");
    if (fp != NULL) {
        char buf[64];
        size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
        buf[n] = '\0';
        printf("  以 'rb' 读取: %zu 字节\n", n);
        printf("  内容: \"%s\"\n", buf);
        fclose(fp);
    }

    printf("\n");
    printf("💡 在 macOS/Linux 上，文本模式和二进制模式没有区别。\n");
    printf("   在 Windows 上，\"w\" 模式会将 \\n 转换为 \\r\\n。\n");
    printf("   ✅ 通用策略：图片/音频/struct 用 'wb'，日志/配置用 'w'。\n\n");
}

/* ============================================================
 * Section 8: fseek + ftell — File Positioning
 * ============================================================ */

static void demo_file_positioning(void)
{
    printf("==== [8] fseek + ftell — 文件定位 ====\n\n");

    /* Create a test file */
    FILE *fp = fopen(POSITION_FILE, "w");
    if (fp == NULL) {
        fprintf(stderr, "❌ fopen failed: %s\n", strerror(errno));
        return;
    }
    fprintf(fp, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    fclose(fp);

    fp = fopen(POSITION_FILE, "r");
    if (fp == NULL) {
        fprintf(stderr, "❌ fopen failed: %s\n", strerror(errno));
        return;
    }

    /* ── ftell: get current position ── */
    long pos = ftell(fp);
    printf("  ftell 初始位置: %ld\n", pos);

    /* Read first 10 chars */
    char buf[16];
    fgets(buf, sizeof(buf), fp);
    pos = ftell(fp);
    printf("  读取 \"%.10s...\" 后 ftell = %ld\n", buf, pos);

    /* ── fseek: move file position ── */
    fseek(fp, 0, SEEK_SET);  /* rewind to start */
    pos = ftell(fp);
    printf("  fseek(fp, 0, SEEK_SET) → ftell = %ld\n", pos);

    /* Read first 5 chars */
    fgets(buf, sizeof(buf), fp);
    printf("  读取 \"%s\"\n", buf);

    /* Seek to position 10 */
    fseek(fp, 10, SEEK_SET);
    pos = ftell(fp);
    printf("  fseek(fp, 10, SEEK_SET) → ftell = %ld\n", pos);

    char ch = fgetc(fp);
    printf("  fgetc 读取: '%c' (ASCII %d)\n", ch, ch);

    /* Seek relative: move back 5 */
    fseek(fp, -5, SEEK_CUR);
    pos = ftell(fp);
    printf("  fseek(fp, -5, SEEK_CUR) → ftell = %ld\n", pos);

    /* Seek to end */
    fseek(fp, 0, SEEK_END);
    pos = ftell(fp);
    printf("  fseek(fp, 0, SEEK_END) → ftell = %ld (文件大小)\n", pos);

    fclose(fp);

    printf("\n💡 SEEK_SET=文件开头, SEEK_CUR=当前位置, SEEK_END=文件末尾。\n");
    printf("   ftell 返回值是相对于文件开头的字节偏移量。\n\n");
}

/* ============================================================
 * Section 9: ferror — Error Checking
 * ============================================================ */

static void demo_error_checking(void)
{
    printf("==== [9] ferror — 文件操作错误检测 ====\n\n");

    /* ── Write valid file ── */
    FILE *fp = fopen(TEXT_DEMO_FILE, "w");
    if (fp == NULL) {
        fprintf(stderr, "❌ fopen failed: %s\n", strerror(errno));
        return;
    }
    fprintf(fp, "Test data for ferror\n");
    fclose(fp);

    /* ── Normal read — no error ── */
    fp = fopen(TEXT_DEMO_FILE, "r");
    if (fp == NULL) {
        fprintf(stderr, "❌ fopen failed: %s\n", strerror(errno));
        return;
    }

    char buffer[128];
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  ✅ 正常读取: \"%s\"\n", buffer);
    }

    if (ferror(fp)) {
        printf("  ferror = 1 (读取出错)\n");
        clearerr(fp);
    } else {
        printf("  ferror = 0 (正常)\n");
    }

    /* ── Try writing to a read-only file → error ── */
    fprintf(fp, "This write should fail\n");
    if (ferror(fp)) {
        printf("  ferror = 1 (写入只读文件 → 触发错误!)\n");
        clearerr(fp);
    }

    fclose(fp);

    printf("\n");
    printf("  ┌── 文件 I/O 错误处理模式 ───────────────┐\n");
    printf("  │                                          │\n");
    printf("  │  fopen  →  检查返回 NULL                 │\n");
    printf("  │  fread  →  检查返回值 < 预期数量         │\n");
    printf("  │  ferror →  检测读写错误                  │\n");
    printf("  │  feof   →  检测是否到达文件末尾          │\n");
    printf("  │  clearerr→ 清除错误标志                  │\n");
    printf("  │                                          │\n");
    printf("  │  program → FILE* buffer → disk ↕         │\n");
    printf("  │            ↑  ferror/feof 监控状态        │\n");
    printf("  └──────────────────────────────────────────┘\n\n");
}

/* ============================================================
 * Section 10: fclose — Resource Management
 * ============================================================ */

static void demo_close_and_cleanup(void)
{
    printf("==== [10] fclose — 关闭文件与资源清理 ====\n\n");

    FILE *fp_write = fopen(TEXT_DEMO_FILE, "w");
    if (fp_write == NULL) {
        fprintf(stderr, "❌ fopen failed: %s\n", strerror(errno));
        return;
    }
    fprintf(fp_write, "Resource management demo.\n");
    if (fclose(fp_write) != 0) {
        fprintf(stderr, "fclose failed: %s\n", strerror(errno));
    }
    fp_write = NULL;
}

/* ============================================================
 * Section 11: 常见错误总结
 * ============================================================ */

static void demo_common_errors(void)
{
    printf("\n==== [11] 常见错误总结 ====\n\n");

    printf("  ❌ 错误 1: 不检查 fopen 返回值\n");
    printf("     FILE *fp = fopen(\"missing.txt\", \"r\");\n");
    printf("     fgets(buf, 100, fp);  // fp==NULL → 崩溃!\n");
    printf("     ✅ 修复: if (fp == NULL) { /* handle error */ }\n\n");

    printf("  ❌ 错误 2: 忘记 fclose\n");
    printf("     FILE *fp = fopen(\"data.txt\", \"w\");\n");
    printf("     fprintf(fp, \"data\");\n");
    printf("     // 忘记 fclose → 数据可能还在缓冲区、未写入磁盘!\n");
    printf("     ✅ 修复: 每条执行路径都调用 fclose(fp)\n\n");

    printf("  ❌ 错误 3: 用 gets()\n");
    printf("     gets(buf);  // 无边界检查 → 缓冲区溢出!\n");
    printf("     ✅ 修复: fgets(buf, sizeof(buf), stdin)\n\n");

    printf("  ❌ 错误 4: fread 不检查返回值\n");
    printf("     fread(&data, size, 1, fp);  // 文件可能比预期短\n");
    printf("     ✅ 修复: if (fread(...) != 1) { /* handle short read */ }\n\n");

    printf("  💡 练习建议:\n");
    printf("     🟢 入门: 用 fopen/fprintf/fclose 写 hello.txt\n");
    printf("     🟡 中级: 用 fgets 逐行读取并统计行数\n");
    printf("     🔴 挑战: 用 fread/fwrite 序列化 struct 到文件\n");
}

/* ============================================================
 * Coordinator: main_file_io_sample
 * ============================================================ */

int main_file_io_sample(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║         文件 I/O (File I/O)                              ║\n");
    printf("║         fopen/fclose, fread/fwrite, fgets, fseek         ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    demo_fopen_write();              /* Section 1 */
    demo_fopen_error_handling();     /* Section 3 (error-first) */
    demo_fgets_read();               /* Section 2 */
    demo_formatted_io();             /* Section 4 */
    demo_fgets_vs_gets();            /* Section 5 */
    demo_binary_io();                /* Section 6 */
    demo_text_vs_binary();           /* Section 7 */
    demo_file_positioning();         /* Section 8 */
    demo_error_checking();           /* Section 9 */
    demo_close_and_cleanup();        /* Section 10 */
    demo_common_errors();            /* Section 11 */

    /* ── Cleanup temp files ── */
    remove(TEXT_DEMO_FILE);
    remove(BINARY_DEMO_FILE);
    remove(POSITION_FILE);
    printf("  临时文件已清理。\n");
    printf("  file_io_sample done.\n");
    return 0;
}
