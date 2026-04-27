#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>
#include "basic/strings_sample.h"

/* ── buffer sizes for demos ── */
#define STR_SMALL_BUF  5
#define STR_MED_BUF    32
#define STR_LARGE_BUF  128
#define CSV_MAX_FIELD  64
#define CSV_MAX_FIELDS 8

/* ============================================================
 * Section 1: String Initialization (char[] vs char*)
 * ============================================================ */

static void demo_string_init(void)
{
    printf("==== [1] String Initialization (字符初始化) ====\n\n");

    /* ── char[]: modifiable, stored on stack ── */
    char greeting[20] = "Hello, C!";
    printf("char greeting[20] = \"Hello, C!\";\n");
    printf("  content:  \"%s\"\n", greeting);
    printf("  length:   %zu\n", strlen(greeting));
    printf("  size of:  %zu bytes (includes space for 20 chars)\n", sizeof(greeting));

    /* Modify individual character in char[] */
    greeting[7] = 'c';
    printf("  after greeting[7]='c': \"%s\"\n", greeting);

    printf("\n");

    /* ── char*: pointer to read-only literal ── */
    const char *literal = "Immutable String";
    printf("const char *literal = \"Immutable String\";\n");
    printf("  content:  \"%s\"\n", literal);
    printf("  length:   %zu\n", strlen(literal));
    printf("  size of:  %zu bytes (pointer size, NOT string size!)\n", sizeof(literal));

    printf("\n");

    /* ── char[] initialized from string literal ── */
    char copy[20];
    strncpy(copy, "From Literal", sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';
    printf("char copy[20]; strncpy(copy, \"From Literal\", ...);\n");
    printf("  content:  \"%s\"\n", copy);

    printf("\n");
    printf("💡 关键区别：char[] 分配完整内存（可修改），char* 指向只读字面量（不可修改）\n\n");
}

/* ============================================================
 * Section 2: strcpy vs strncpy — Safety Comparison
 * ============================================================ */

static void demo_strcpy_vs_strncpy(void)
{
    char small[STR_SMALL_BUF];

    printf("==== [2] strcpy vs strncpy (安全复制对比) ====\n\n");

    /* ── ❌ UNSAFE: strcpy overflow demo (EXPLANATORY ONLY) ── */
    printf("❌ 危险示例（不要在实际代码中这样做！）:\n");
    printf("   char small[5];\n");
    printf("   strcpy(small, \"Hello World!\");  // 写入13字节到5字节缓冲区！\n");
    printf("   → Buffer Overflow! 覆盖栈上相邻内存 → Undefined Behavior\n\n");

    printf("   memcpy(small, \"Hello\", 5);      // 恰好填满，但没有空间放 \\0\n");
    printf("   → 字符串没有终止符 → strlen() 越界读取\n\n");

    /* ── ✅ SAFE: strncpy with proper null termination ── */
    printf("✅ 安全做法：strncpy + 手动 null terminator:\n");

    strncpy(small, "Hello World!", sizeof(small) - 1);
    small[sizeof(small) - 1] = '\0';
    printf("   strncpy(small, \"Hello World!\", sizeof(small) - 1);\n");
    printf("   small[sizeof(small) - 1] = '\\0';\n");
    printf("   → 结果: \"%s\" (截断到 %zu 个字符)\n\n", small, sizeof(small) - 1);

    /* ── ✅ SAFE: short string fits ── */
    strncpy(small, "Hi", sizeof(small) - 1);
    small[sizeof(small) - 1] = '\0';
    printf("   strncpy(small, \"Hi\", sizeof(small) - 1);\n");
    printf("   → 结果: \"%s\" (原始长度足够)\n\n", small);

    /* ── strncpy pitfall: not null-terminating ── */
    printf("⚠️ strncpy 陷阱：src 长度 >= n 时不会自动加 \\0\n");
    char buf[5];
    strncpy(buf, "ABCD", sizeof(buf));        /* exactly fits, no room for \0 */
    printf("   strncpy(buf, \"ABCD\", 5);      → 需要手动: buf[4] = '\\0';\n");
    buf[4] = '\0';                            /* always null-terminate */
    printf("   修复后 buf = \"%s\"\n\n", buf);
}

/* ============================================================
 * Section 3: strlen — Manual vs Library Implementation
 * ============================================================ */

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
    printf("==== [3] strlen — 手动实现 vs 库函数 ====\n\n");

    const char *texts[] = {
        "Hello",
        "",
        "C Strings!",
        "🌍 (multi-byte UTF-8)"
    };
    int count = 4;

    printf("  %-20s | %-10s | %-10s | 说明\n", "字符串", "my_strlen", "strlen()");
    printf("  --------------------|------------|------------|------\n");

    for (int i = 0; i < count; i++) {
        size_t manual = my_strlen(texts[i]);
        size_t lib    = strlen(texts[i]);
        printf("  %-20s | %-10zu | %-10zu | %s\n",
               texts[i], manual, lib,
               (manual == lib) ? "一致 ✓" : "不同! bytes≠codepoints");
    }

    printf("\n");
    printf("💡 strlen() 返回的是**字节数**（不包括 \\0），不是字符数。\n");
    printf("   UTF-8 多字节字符会被计为多个字节。\n");
    printf("   例如 '🌍' 在 UTF-8 中是 4 字节，strlen 返回 4。\n\n");
}

/* ============================================================
 * Section 4: 原理解析 — ASCII 内存图 & Python str vs C char*
 * ============================================================ */

static void demo_ascii_memory_diagram(void)
{
    printf("==== [4] ASCII 内存图 & Python vs C ====\n\n");

    /* ── ASCII Memory Diagram ── */
    printf("  ┌── String in Stack Memory ───────────────────────┐\n");
    printf("  │                                                  │\n");
    printf("  │  char name[6] = \"Hello\";                       │\n");
    printf("  │                                                  │\n");
    printf("  │  Address  +0  +1  +2  +3  +4  +5               │\n");
    printf("  │          ┌───┬───┬───┬───┬───┬───┐            │\n");
    printf("  │  name →  │'H'│'e'│'l'│'l'│'o'│'\\0'│            │\n");
    printf("  │          │ 72│101│108│108│111│  0│            │\n");
    printf("  │          └───┴───┴───┴───┴───┴───┘            │\n");
    printf("  │                                                  │\n");
    printf("  │  ① 每个字符 = 1 字节 (ASCII)                    │\n");
    printf("  │  ② \\0 = null terminator（字符串结束标记）       │\n");
    printf("  │  ③ strlen(\"Hello\") = 5 (不算 \\0)              │\n");
    printf("  │  ④ sizeof(name) = 6 (包括 \\0 的空间)           │\n");
    printf("  └──────────────────────────────────────────────────┘\n\n");

    /* ── Python str vs C char* ── */
    printf("  ┌── Python str vs C char* ────────────────────────┐\n");
    printf("  │                                                  │\n");
    printf("  │  ┌──── Python ────┐  ┌──── C ─────────┐       │\n");
    printf("  │  │ s = \"Hello\"    │  │ char s[]=\"Hello\"│       │\n");
    printf("  │  │                │  │                │       │\n");
    printf("  │  │ struct {       │  │ ['H','e','l',  │       │\n");
    printf("  │  │   char *data;  │  │  'l','o','\\0'] │       │\n");
   printf("  │  │   size_t len;  │  │                │       │\n");
    printf("  │  │ }              │  │ 无长度信息！    │       │\n");
    printf("  │  │                │  │ 靠 \\0 找结尾   │       │\n");
    printf("  │  │ len(s) → O(1)  │  │ strlen() → O(n)│       │\n");
    printf("  │  │ 内存自动管理    │  │ 手动边界检查   │       │\n");
    printf("  │  └────────────────┘  └────────────────┘       │\n");
    printf("  │                                                  │\n");
    printf("  │  C 的代价：你需要自己管理一切                   │\n");
    printf("  │  C 的好处：完全的控制力和性能                  │\n");
    printf("  └──────────────────────────────────────────────────┘\n\n");
}

/* ============================================================
 * Section 5: strcmp — String Comparison
 * ============================================================ */

static void demo_strcmp(void)
{
    printf("==== [5] strcmp — 字符串比较 ====\n\n");

    struct cmp_case {
        const char *a;
        const char *b;
    };

    struct cmp_case cases[] = {
        {"hello", "hello"},
        {"hello", "world"},
        {"world", "hello"},
        {"Hello", "hello"},     /* case-sensitive */
        {"abc",  "abcd"},       /* prefix */
        {"",     "empty"},      /* empty string */
    };
    int count = 6;

    printf("  %-12s | %-12s | result | 含义\n", "str1", "str2");
    printf("  --------------|--------------|--------|------------------\n");

    for (int i = 0; i < count; i++) {
        int result = strcmp(cases[i].a, cases[i].b);
        const char *meaning;
        if (result == 0)       meaning = "完全相等 (=0)";
        else if (result < 0)   meaning = "str1 < str2 (<0)";
        else                   meaning = "str1 > str2 (>0)";

        printf("  %-12s | %-12s |   %4d | %s\n",
               cases[i].a, cases[i].b, result, meaning);
    }

    printf("\n");
    printf("💡 strcmp 返回：\n");
    printf("   0  →  两个字符串完全相同\n");
    printf("   <0 →  str1 字典序小于 str2\n");
    printf("   >0 →  str1 字典序大于 str2\n");
    printf("   ⚠️ 大小写敏感（'H' 的 ASCII 值 = 72, 'h' = 104）\n\n");
}

/* ============================================================
 * Section 6: String Search — strchr, strstr
 * ============================================================ */

static void demo_string_search(void)
{
    printf("==== [6] 字符串搜索 (strchr, strstr) ====\n\n");

    const char *text = "Hello, C Programming!";

    /* ── strchr: find single character ── */
    printf("  strchr — 查找单个字符:\n");
    printf("  text = \"%s\"\n\n", text);

    char targets[] = {',', 'C', 'x', '!', '\0'};
    for (int i = 0; targets[i] != '\0'; i++) {
        char *found = strchr(text, targets[i]);
        if (found) {
            printf("    strchr(text, '%c') → found at index %ld, rest: \"%s\"\n",
                   targets[i], found - text, found);
        } else {
            printf("    strchr(text, '%c') → not found\n", targets[i]);
        }
    }

    printf("\n");

    /* ── strstr: find substring ── */
    printf("  strstr — 查找子串:\n");

    const char *substrings[] = {"C", "Program", "xyz", "Hello", "!", NULL};
    for (int i = 0; substrings[i] != NULL; i++) {
        char *found = strstr(text, substrings[i]);
        if (found) {
            printf("    strstr(text, \"%s\") → found at index %ld, rest: \"%s\"\n",
                   substrings[i], found - text, found);
        } else {
            printf("    strstr(text, \"%s\") → not found\n", substrings[i]);
        }
    }

    printf("\n");
    printf("💡 这两个函数都返回指向找到位置的指针，找不到返回 NULL。\n");
    printf("   用 ptr - original 可以计算偏移量（如上所示）。\n\n");
}

/* ============================================================
 * Section 7: snprintf vs sprintf — Safe Formatting
 * ============================================================ */

static void demo_snprintf_vs_sprintf(void)
{
    printf("==== [7] snprintf vs sprintf (安全格式化) ====\n\n");

    char buf[STR_MED_BUF];
    const char *name = "World";
    int year = 2026;

    /* ── ❌ UNSAFE: sprintf (no bounds checking) ── */
    printf("❌ sprintf（无边界检查）:\n");
    printf("   char buf[5];\n");
    printf("   sprintf(buf, \"Hello, %%s!\", \"World\");\n");
    printf("   → 写入 13 字节到 5 字节缓冲区 → Buffer Overflow!\n\n");

    /* ── ✅ SAFE: snprintf — fits ── */
    int ret = snprintf(buf, sizeof(buf), "Hello, %s! (%d)", name, year);
    printf("✅ snprintf（有边界检查）— 足够空间:\n");
    printf("   snprintf(buf, %zu, \"Hello, %%s! (%%d)\", \"World\", 2026);\n", sizeof(buf));
    printf("   → 结果: \"%s\" (需要 %d 字符)\n\n", buf, ret);

    /* ── ✅ SAFE: snprintf — truncation warning ── */
    char tiny[10];
    ret = snprintf(tiny, sizeof(tiny), "Hello, %s!", name);
    printf("🔧 snprintf — 空间不足（截断）:\n");
    printf("   char tiny[10];\n");
    printf("   snprintf(tiny, 10, \"Hello, %%s!\", \"World\");\n");
    printf("   → 结果: \"%s\" (截断到 %zu 字符)\n", tiny, sizeof(tiny) - 1);
    printf("   → snprintf 返回 %d（完整输出需要这么多字符）\n", ret);
    if (ret >= (int)sizeof(tiny)) {
        printf("   → ret >= sizeof → 发生了截断！\n");
    }

    printf("\n");
    printf("💡 始终使用 snprintf 而非 sprintf！\n");
    printf("   snprintf 的返回值很有用：正数表示完整输出需要的字符数。\n");
    printf("   如果返回值 >= 缓冲区大小，说明发生了截断。\n\n");
}

/* ============================================================
 * Section 8: strtok — Tokenizing Strings
 * ============================================================ */

static void demo_strtok(void)
{
    printf("==== [8] strtok — 分隔字符串 ====\n\n");

    /* ── Basic: split by space ── */
    char sentence[] = "Hello World from C Language";
    printf("  strtok 基本用法 — 按空格分隔:\n");
    printf("  text: \"%s\"\n", sentence);

    char *token = strtok(sentence, " ");
    int idx = 0;
    printf("  tokens: ");
    while (token != NULL) {
        printf("[%s]", token);
        if (idx < 4) printf(", ");  /* show up to 5 */
        idx++;
        token = strtok(NULL, " ");
    }
    printf(" (%d tokens)\n\n", idx);

    /* ── Split by multiple delimiters ── */
    char data[] = "apple,banana;cherry:date|elderberry";
    printf("  多种分隔符: \"%s\"\n", data);
    printf("  delimiters: \",;:|\"\n");

    token = strtok(data, ",;:|");
    idx = 0;
    printf("  tokens: ");
    while (token != NULL) {
        printf("[%s]", token);
        if (idx < 6) printf(", ");
        idx++;
        token = strtok(NULL, ",;:|");
    }
    printf(" (%d tokens)\n\n", idx);

    printf("⚠️ strtok 会修改原字符串（插入 \\0），且不是线程安全的。\n");
    printf("   如果需要保留原字符串，先用 strncpy 复制到缓冲区。\n");
    printf("   线程安全替代：strtok_r（POSIX）。\n\n");
}

/* ============================================================
 * Section 9: Practical — Parsing CSV
 * ============================================================ */

static void demo_csv_parsing(void)
{
    printf("==== [9] 实战：安全解析 CSV 字符串 ====\n\n");

    const char *raw_csv = "John,25,Engineer,New York";

    /* Always work on a copy — strtok modifies the string */
    char work_buf[STR_LARGE_BUF];
    strncpy(work_buf, raw_csv, sizeof(work_buf) - 1);
    work_buf[sizeof(work_buf) - 1] = '\0';

    printf("  原始 CSV: \"%s\"\n", raw_csv);
    printf("  字段分隔符: \",\"\n\n");

    char field[CSV_MAX_FIELD];
    int field_num = 1;
    char *token = strtok(work_buf, ",");

    printf("  解析结果:\n");
    printf("  ┌──────┬──────────────────────┐\n");
    printf("  │ #    │ 字段内容              │\n");
    printf("  ├──────┼──────────────────────┤\n");

    while (token != NULL && field_num <= CSV_MAX_FIELDS) {
        /* Safe copy into field buffer with truncation check */
        size_t tlen = strlen(token);
        strncpy(field, token, sizeof(field) - 1);
        field[sizeof(field) - 1] = '\0';

        if (tlen >= sizeof(field)) {
            printf("  │ %4d │ %-20s │ (截断! 原长 %zu)\n",
                   field_num, field, tlen);
        } else {
            printf("  │ %4d │ %-20s │\n",
                   field_num, field);
        }

        field_num++;
        token = strtok(NULL, ",");
    }

    printf("  └──────┴──────────────────────┘\n\n");

    /* ── Robust: with count ── */
    printf("  关键安全措施总结:\n");
    printf("  ✅ 1. 复制原始字符串（strtok 修改源数据）\n");
    printf("  ✅ 2. strncpy + 手动 \\0（防止字段超长）\n");
    printf("  ✅ 3. 检查 strtok 返回值 != NULL（防止越界）\n");
    printf("  ✅ 4. 限制最大字段数（防止畸形输入）\n\n");
}

/* ============================================================
 * Section 10: 常见错误 & 三级别练习
 * ============================================================ */

static void demo_common_errors(void)
{
    printf("==== [10] 常见错误总结 & 练习提示 ====\n\n");

    printf("  ❌ 错误 1: 忘记 null terminator\n");
    printf("     char buf[5]; strncpy(buf, \"abcd\", 5);\n");
    printf("     → buf[5] 未设置 '\\0' → strlen(buf) 可能越界\n");
    printf("     ✅ 修复: strncpy 后始终 buf[sizeof(buf)-1] = '\\0';\n\n");

    printf("  ❌ 错误 2: 用 == 比较字符串\n");
    printf("     char *a = \"hello\", *b = \"hello\";\n");
    printf("     if (a == b)  // ❌ 比较指针地址，不是内容！\n");
    printf("     ✅ 修复: if (strcmp(a, b) == 0)\n\n");

    printf("  ❌ 错误 3: 缓冲区不够大\n");
    printf("     char buf[5]; strcpy(buf, \"hello\");\n");
    printf("     → \"hello\" 需要 6 字节（含 \\0）, buf 只有 5\n");
    printf("     ✅ 修复: char buf[6] 或更大; 使用 strncpy\n\n");

    printf("  ❌ 错误 4: 修改字符串字面量\n");
    printf("     char *s = \"hello\"; s[0] = 'H';  // ❌ UB!\n");
    printf("     ✅ 修复: char s[] = \"hello\"; s[0] = 'H';  // char[] 可修改\n\n");

    printf("  💡 练习建议:\n");
    printf("     🟢 入门: 用 strlen 计算多个字符串长度\n");
    printf("     🟡 中级: 用 strncpy 实现安全的字符串复制函数\n");
    printf("     🔴 挑战: 用 strtok 安全解析 CSV，处理转义和空白\n\n");
}

/* ============================================================
 * Coordinator: main_strings_sample
 * ============================================================ */

int main_strings_sample(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║         字符串深度 (Strings Deep Dive)                   ║\n");
    printf("║         安全模式：bounded string operations only          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    demo_ascii_memory_diagram();   /* Section 4 in docs */
    demo_string_init();            /* Section 1 */
    demo_strcpy_vs_strncpy();      /* Section 2 */
    demo_strlen();                 /* Section 3 */
    demo_strcmp();                 /* Section 5 */
    demo_string_search();          /* Section 6 */
    demo_snprintf_vs_sprintf();    /* Section 7 */
    demo_strtok();                 /* Section 8 */
    demo_csv_parsing();            /* Section 9 */
    demo_common_errors();          /* Section 10 */

    printf("  strings sample done.\n");
    return 0;
}
