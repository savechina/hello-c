#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "basic/string_processing_sample.h"

/* ── buffer sizes for demos ── */
#define PROC_MED   64
#define PROC_LARGE 128

/* ── 1. strtok — 分隔字符串 ── */
static void demo_strtok(void)
{
    printf("=== 1. strtok — 按分隔符拆分字符串 ===\n\n");

    /* 基本: 按空格分隔 */
    char sentence[] = "Hello World from C Language";
    printf("  按空格分隔:\n");
    printf("    text: \"%s\"\n", sentence);

    char *token = strtok(sentence, " ");
    int idx = 0;
    printf("    tokens: ");
    while (token != NULL) {
        printf("[%s]", token);
        if (idx < 3) printf(", ");
        idx++;
        token = strtok(NULL, " ");
    }
    printf(" (共 %d 个)\n\n", idx);

    /* 多种分隔符 */
    char data[] = "apple,banana;cherry:elderberry";
    printf("  多种分隔符 (\",;:\"):\n");
    printf("    text: \"%s\"\n", data);

    token = strtok(data, ",;:");
    idx = 0;
    printf("    tokens: ");
    while (token != NULL) {
        printf("[%s]", token);
        if (idx < 3) printf(", ");
        idx++;
        token = strtok(NULL, ",;:");
    }
    printf(" (共 %d 个)\n\n", idx);

    printf("  ⚠️ strtok 的警告:\n");
    printf("    1. 修改原字符串(在分隔符处插入 '\\0')\n");
    printf("    2. 不是线程安全(内部用静态变量)\n");
    printf("    3. 多线程用 strtok_r (POSIX)\n\n");
}

/* ── 2. strstr — 查找子串 ── */
static void demo_strstr(void)
{
    printf("=== 2. strstr — 查找子串 ===\n\n");

    const char *text = "The quick brown fox jumps over the lazy dog";

    const char *substrings[] = {"quick", "brown", "fox", "the", "xyz", NULL};
    int i = 0;

    printf("  text = \"%s\"\n\n", text);

    for (i = 0; substrings[i] != NULL; i++) {
        char *found = strstr(text, substrings[i]);
        if (found) {
            ptrdiff_t offset = found - text;
            printf("    strstr(text, \"%s\") → found at offset %td\n",
                   substrings[i], offset);
        } else {
            printf("    strstr(text, \"%s\") → not found\n", substrings[i]);
        }
    }

    printf("\n  💡 strstr 返回 char* 或 NULL\n");
    printf("     found - text 就是偏移量\n");
    printf("     区分大小写: \"the\" ≠ \"The\"\n\n");
}

/* ── 3. strchr + strspn + strcspn ── */
static void demo_char_search(void)
{
    printf("=== 3. 字符搜索与跳过集合 ===\n\n");

    const char *path = "/home/user/docs/readme.txt";

    /* strchr: 找单个字符 */
    printf("  strchr — 查找字符:\n");
    char *last_slash = NULL;
    const char *p = path;
    while ((p = strchr(p, '/')) != NULL) {
        last_slash = (char *)p;
        p++;
    }
    if (last_slash) {
        printf("    最后一个 '/' 位置: \"%s\"\n", last_slash + 1);
        printf("    → 文件名: \"%s\"\n\n", last_slash + 1);
    }

    /* strspn: 计算前缀匹配集合的长度 */
    const char *num_str = "12345abc";
    size_t span = strspn(num_str, "0123456789");
    printf("  strspn — 前缀匹配数字:\n");
    printf("    strspn(\"12345abc\", \"0123456789\") = %zu\n", span);
    printf("    → 前 %zu 个字符都是数字\n\n", span);

    /* strcspn: 计算不匹配集合的长度 */
    const char *mixed = "hello123world";
    size_t cspan = strcspn(mixed, "0123456789");
    printf("  strcspn — 遇到第一个数字前的长度:\n");
    printf("    strcspn(\"hello123world\", \"0123456789\") = %zu\n", cspan);
    printf("    → 前 %zu 个字符不是数字\n\n", cspan);
}

/* ── 4. CSV 解析实战 ── */
static void demo_csv_parsing(void)
{
    printf("=== 4. 实战: 安全 CSV 解析 ===\n\n");

    const char *raw_csv = "Alice,30,Engineer,New York";

    /* 始终在副本上操作 */
    char work[PROC_LARGE];
    strncpy(work, raw_csv, sizeof(work) - 1);
    work[sizeof(work) - 1] = '\0';

    printf("  原始数据: \"%s\"\n\n", raw_csv);

    printf("  ┌─────┬──────────────────────┐\n");
    printf("  │  #  │ 字段                  │\n");
    printf("  ├─────┼──────────────────────┤\n");

    char field[PROC_MED];
    int field_num = 1;
    char *token = strtok(work, ",");

    while (token != NULL && field_num <= 8) {
        strncpy(field, token, sizeof(field) - 1);
        field[sizeof(field) - 1] = '\0';
        printf("  │ %3d │ %-20s │\n", field_num, field);
        field_num++;
        token = strtok(NULL, ",");
    }

    printf("  └─────┴──────────────────────┘\n\n");

    printf("  ✅ 安全措施:\n");
    printf("     1. 复制到缓冲区（strtok 修改原串）\n");
    printf("     2. strncpy + 手动 '\\0'（防止字段超长）\n");
    printf("     3. 限制最大字段数（防畸形输入）\n\n");
}

/* ── 5. 字符串转数字 ── */
static void demo_string_to_number(void)
{
    printf("=== 5. 字符串 → 数字转换 ===\n\n");

    /* strtol: 安全地解析整数 */
    const char *nums[] = {
        "42",
        "-128",
        "0xFF",
        "abc123",
        "700000000000000000000",
        "",
        NULL
    };

    printf("  strtol 安全解析:\n\n");
    printf("  %-20s | %-10s | 剩余   | 说明\n", "输入", "结果");
    printf("  --------------------|------------|--------|---------\n");

    for (int i = 0; nums[i] != NULL; i++) {
        char *endptr = NULL;
        long val = strtol(nums[i], &endptr, 0);

        const char *note = "";
        if (endptr == nums[i]) {
            note = "无法解析";
        } else if (*endptr != '\0') {
            note = "部分解析";
        }

        printf("  %-20s | %-10ld | \"%-6s\" | %s\n",
               nums[i], val, endptr, note);
    }

    printf("\n  💡 strtol 比 atoi 更安全:\n");
    printf("     ✅ 可以检测解析错误 (endptr)\n");
    printf("     ✅ 支持不同进制 (base=0 自动检测)\n");
    printf("     ✅ 不会在溢出时崩溃\n\n");
}

/* ── 6. 实用函数: 去除两端空白 ── */
static void demo_trim_whitespace(void)
{
    printf("=== 6. 实战: 去除两端空白 ===\n\n");

    char test[] = "   Hello, World!   ";
    printf("  原始: \"%s\" (长度 %zu)\n", test, strlen(test));

    /* 找到第一个非空白 */
    char *start = test;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    /* 找到最后一个非空白 */
    char *end = start + strlen(start) - 1;
    while (end > start &&
           (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    *(end + 1) = '\0';

    printf("  去除后: \"%s\" (长度 %zu)\n", start, strlen(start));
    printf("\n  注意: start 是一个指针偏移, 不是新字符串\n");
    printf("     原始缓冲区的前导空白被\"跳过了\"\n\n");
}

/* ── coordinator ── */
int main_string_processing_sample(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║         字符串处理 (String Processing)                   ║\n");
    printf("║         strtok, strstr, strchr, 解析, 转换             ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    demo_strtok();
    demo_strstr();
    demo_char_search();
    demo_csv_parsing();
    demo_string_to_number();
    demo_trim_whitespace();

    printf("string_processing_sample 演示完毕。\n");
    return 0;
}
