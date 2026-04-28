#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <inttypes.h>
#include "basic/string_operations_sample.h"

/* ── buffer sizes for demos ── */
#define BUF_SMALL  8
#define BUF_MED    32
#define BUF_LARGE  128

/* ── 1. strlen — 手动实现 vs 库函数 ── */
static size_t my_strlen(const char *str)
{
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

static void demo_strlen(void)
{
    printf("=== 1. strlen — 字符串有多长？ ===\n\n");

    const char *tests[] = {
        "Hello",
        "",
        "C Programming",
        "🌏",
        NULL
    };

    printf("  %-20s | %-10s | %-10s | 说明\n", "字符串", "my_strlen", "strlen()");
    printf("  --------------------|------------|------------|-----------\n");

    for (int i = 0; tests[i] != NULL; i++) {
        size_t manual = my_strlen(tests[i]);
        size_t lib = strlen(tests[i]);
        printf("  %-20s | %-10zu | %-10zu | %s\n",
               tests[i], manual, lib,
               (manual == lib) ? "一致 ✓" : "字节≠字符!");
    }

    printf("\n  💡 strlen 返回字节数（不含 '\\0'），不是 Unicode 字符数\n");
    printf("     \"🌏\" 在 UTF-8 中是 4 字节，strlen 返回 4\n\n");
}

/* ── 2. strcpy vs strncpy 安全对比 ── */
static void demo_strcpy_vs_strncpy(void)
{
    printf("=== 2. strcpy 的危险 vs strncpy 的安全 ===\n\n");

    char buf[BUF_SMALL];

    /* ❌ strncpy 安全演示 */
    strncpy(buf, "Hello World!", sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    printf("  ✅ strncpy(buf, \"Hello World!\", sizeof(buf)-1);\n");
    printf("     buf[sizeof(buf)-1] = '\\0';\n");
    printf("     → \"%s\" (截断到 %zu 字符)\n\n", buf, sizeof(buf) - 1);

    /* strncpy 的陷阱 */
    char tiny[5];
    strncpy(tiny, "ABCDE", sizeof(tiny));  /* 写满, 没有 \0 的空间 */
    tiny[sizeof(tiny) - 1] = '\0';
    printf("  ⚠️ strncpy(tiny, \"ABCDE\", 5);  → 写满了！需要手动:\n");
    printf("     tiny[4] = '\\0'; → \"%s\"\n\n", tiny);

    /* 短串完整复制 */
    strncpy(buf, "Hi", sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    printf("  ✅ 短串: strncpy(buf, \"Hi\", ...);\n");
    printf("     → \"%s\" (完整复制)\n\n", buf);
}

/* ── 3. strcat — 字符串拼接 ── */
static void demo_strcat(void)
{
    printf("=== 3. strcat — 拼接字符串 ===\n\n");

    char greeting[BUF_MED];

    /* 安全拼接 */
    strncpy(greeting, "Hello", sizeof(greeting) - 1);
    greeting[sizeof(greeting) - 1] = '\0';

    printf("  char greeting[32] = \"Hello\";\n");
    printf("  strlen(greeting) = %zu\n", strlen(greeting));

    strncat(greeting, ", World", sizeof(greeting) - strlen(greeting) - 1);
    printf("  strncat(greeting, \", World\", ...);\n");
    printf("  → \"%s\" (长度 %zu)\n\n", greeting, strlen(greeting));

    /* 拼接受限演示 */
    char shortbuf[10];
    strncpy(shortbuf, "ABC", sizeof(shortbuf) - 1);
    shortbuf[sizeof(shortbuf) - 1] = '\0';
    strncat(shortbuf, "DEFGHIJKLMN", sizeof(shortbuf) - strlen(shortbuf) - 1);
    printf("  短缓冲区拼接:\n");
    printf("    shortbuf[10] = \"ABC\"\n");
    printf("    strncat(..., \"DEFGHIJKLMN\", %zu);\n",
           sizeof(shortbuf) - strlen(shortbuf) - 1);
    printf("    → \"%s\" (剩余 %zu 字符空间)\n\n", shortbuf,
           sizeof(shortbuf) - strlen(shortbuf) - 1);

    printf("  💡 strncat 的第三个参数是**剩余空间**，不是缓冲区总大小！\n\n");
}

/* ── 4. strcmp — 字典序比较 ── */
static void demo_strcmp(void)
{
    printf("=== 4. strcmp — 字符串怎么比较？ ===\n\n");

    struct cmp_pair {
        const char *a;
        const char *b;
    };

    struct cmp_pair cases[] = {
        {"apple", "apple"},
        {"apple", "banana"},
        {"banana", "apple"},
        {"Apple", "apple"},
        {"abc", "abcd"},
        {"", "empty"},
    };
    int count = 6;

    printf("  %-12s | %-12s | result | 含义\n", "str1", "str2");
    printf("  --------------|--------------|--------|---------------\n");

    for (int i = 0; i < count; i++) {
        int result = strcmp(cases[i].a, cases[i].b);
        const char *meaning;
        if (result == 0)       meaning = "相等";
        else if (result < 0)   meaning = "str1 < str2";
        else                   meaning = "str1 > str2";

        printf("  %-12s | %-12s |   %4d | %s\n",
               cases[i].a, cases[i].b, result, meaning);
    }

    printf("\n  ❌ 永远不要用 == 比较字符串!\n");
    printf("     char *a = \"hello\", *b = \"hello\";\n");
    printf("     if (a == b)  ← 比较的是指针地址, 不是内容!\n");
    printf("     ✅ if (strcmp(a, b) == 0)  ← 比较内容\n\n");
}

/* ── 5. strchr — 字符搜索 ── */
static void demo_strchr(void)
{
    printf("=== 5. strchr — 找字符在哪里 ===\n\n");

    const char *text = "Hello, C Programming!";
    char targets[] = {',', 'C', 'P', 'x', '\0'};

    printf("  text = \"%s\"\n\n", text);

    for (int i = 0; targets[i] != '\0'; i++) {
        char *found = strchr(text, targets[i]);
        if (found) {
            ptrdiff_t idx = found - text;
            printf("    strchr(text, '%c') → 索引 %td, 剩余: \"%s\"\n",
                   targets[i], idx, found);
        } else {
            printf("    strchr(text, '%c') → 未找到\n", targets[i]);
        }
    }

    printf("\n  💡 strchr 返回 char*, NULL 表示没找到\n");
    printf("     found - text 就是偏移量\n\n");
}

/* ── 6. 综合演示: 构建路径 ── */
static void demo_build_path(void)
{
    printf("=== 6. 实战: 安全拼接文件路径 ===\n\n");

    char path[BUF_LARGE];

    /* 步骤1: 初始值 */
    strncpy(path, "/home/user", sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    printf("  初始: \"%s\"\n", path);

    /* 步骤2: 追加目录 */
    strncat(path, "/documents", sizeof(path) - strlen(path) - 1);
    printf("  追加目录: \"%s\"\n", path);

    /* 步骤3: 追加文件名 */
    strncat(path, "/report.txt", sizeof(path) - strlen(path) - 1);
    printf("  追加文件: \"%s\"\n", path);

    /* 步骤4: 安全检查 */
    printf("\n  ✅ 使用了 strncpy + strncat + sizeof 边界检查\n");
    printf("  ✅ 每次操作后检查 strlen < sizeof\n");
    printf("  ✅ 缓冲区不会溢出\n\n");
}

/* ── coordinator ── */
int main_string_operations_sample(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║         字符串操作 (String Operations)                   ║\n");
    printf("║         strlen, strcpy, strcat, strcmp, strchr          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    demo_strlen();
    demo_strcpy_vs_strncpy();
    demo_strcat();
    demo_strcmp();
    demo_strchr();
    demo_build_path();

    printf("string_operations_sample 演示完毕。\n");
    return 0;
}
