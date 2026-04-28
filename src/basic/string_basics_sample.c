#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "basic/string_basics_sample.h"

/* ── buffer sizes for demos ── */
#define STR_BUF_SMALL  8
#define STR_BUF_MED    32
#define STR_BUF_LARGE  128

/* ── 1. char[] vs char* initialization ── */
static void demo_char_array_vs_pointer(void)
{
    printf("=== 1. char[] 还是 char*？这是个好问题 ===\n\n");

    /* char[]: 栈上可修改 */
    char greeting[STR_BUF_MED] = "Hello";
    printf("  char greeting[] = \"Hello\";\n");
    printf("    sizeof: %zu bytes, content: \"%s\"\n", sizeof(greeting), greeting);

    greeting[0] = 'h';
    printf("    greeting[0] = 'h' → \"%s\" ✅ 可以修改\n\n", greeting);

    /* char*: 指向只读字面量 */
    const char *literal = "World";
    printf("  const char *literal = \"World\";\n");
    printf("    sizeof: %zu bytes (指针大小!), content: \"%s\"\n",
           sizeof(literal), literal);
    printf("    literal[0] = 'w' → ❌ 段错误！字面量在只读段\n\n");

    /* 用 sizeof 看本质区别 */
    printf("  sizeof(greeting) = %zu  (整个数组)\n", sizeof(greeting));
    printf("  sizeof(literal)  = %zu  (指针本身, 不是字符串!)\n", sizeof(literal));
    printf("\n");
}

/* ── 2. null terminator 的重要性 ── */
static void demo_null_terminator(void)
{
    printf("=== 2. '\\0' — 字符串的句号 ===\n\n");

    /* 正确的字符串 */
    char correct[6] = {'H', 'e', 'l', 'l', 'o', '\0'};
    printf("  正确的字符串: {'H','e','l','l','o','\\0'}\n");
    printf("    strlen  = %zu\n", strlen(correct));
    printf("    printf: \"%s\"\n\n", correct);

    /* 缺少 '\0' 的字符数组 */
    char no_null[5] = {'H', 'e', 'l', 'l', 'o'};
    printf("  缺少 '\\0': {'H','e','l','l','o'}\n");
    printf("    strlen  = %zu  (可能越界! 直到偶然撞到 '\\0')\n", strlen(no_null));
    printf("    编译器不报错, 运行时出乱码 💣\n\n");

    /* 字符串字面量自动加 '\0' */
    char auto_null[] = "Hi";
    printf("  自动加 '\\0': char auto_null[] = \"Hi\";\n");
    printf("    sizeof: %zu (2字符 + '\\0')\n", sizeof(auto_null));
    printf("    auto_null[2] 的值为: %d\n\n", auto_null[2]);

    /* 手动确保 '\0' */
    char manual[STR_BUF_SMALL];
    manual[0] = 'C';
    manual[1] = '\0';
    printf("  手动设置 '\\0': manual[1] = '\\0';\n");
    printf("    strlen: %zu, content: \"%s\"\n\n", strlen(manual), manual);
}

/* ── 3. ASCII 内存图演示 ── */
static void demo_memory_layout(void)
{
    printf("=== 3. ASCII 内存图 ===\n\n");

    char name[8] = "C Prog";

    printf("  char name[8] = \"C Prog\";\n\n");
    printf("  地址偏移  +0    +1    +2    +3    +4    +5    +6    +7\n");
    printf("          ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐\n");
    printf("  name → │ 'C' │ ' ' │ 'P' │ 'r' │ 'o' │ 'g' │ '\\0' │ '?' │\n");
    printf("          │ 67  │ 32  │ 80  │ 114 │ 111 │ 103 │  0   │ ??  │\n");
    printf("          └─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┘\n\n");
    printf("  ① strlen(\"C Prog\") = 6 (不算 '\\0')\n");
    printf("  ② sizeof(name)     = 8 (整个数组)\n");
    printf("  ③ '%c' 和 \"%c\" 的区别: 单引号=char, 双引号=字符串\n\n", name[0], name[0]);

    /* 验证输出 */
    printf("  实际值验证:\n");
    printf("    name[0]='%c' (ASCII %d)\n", name[0], (int)(unsigned char)name[0]);
    printf("    name[5]='%c' (ASCII %d)\n", name[5], (int)(unsigned char)name[5]);
    printf("    name[6]='\\0' (ASCII %d)\n", name[6]);
    printf("\n");
}

/* ── 4. Python str vs C string 对比 ── */
static void demo_python_vs_c(void)
{
    printf("=== 4. Python str vs C string ===\n\n");

    printf("  ┌────────────────────────────────────────────┐\n");
    printf("  │              Python str                      │\n");
    printf("  │                                              │\n");
    printf("  │  s = \"Hello\"                                 │\n");
    printf("  │  ┌──────────────────────┐                    │\n");
    printf("  │  │ PyObject_HEAD        │                    │\n");
    printf("  │  │ Py_ssize_t length=5  │                    │\n");
    printf("  │  │ char data[]=\"Hello\"  │                    │\n");
    printf("  │  │ hash, flags ...      │                    │\n");
    printf("  │  └──────────────────────┘                    │\n");
    printf("  │  len(s) → O(1), 自动管理, 不可变             │\n");
    printf("  └────────────────────────────────────────────┘\n\n");

    printf("  ┌────────────────────────────────────────────┐\n");
    printf("  │              C string                        │\n");
    printf("  │                                              │\n");
    printf("  │  char s[] = \"Hello\";                         │\n");
    printf("  │  ┌────┬────┬────┬────┬────┬───┐             │\n");
    printf("  │  │ H  │ e  │ l  │ l  │ o  │\\0 │             │\n");
    printf("  │  └────┴────┴────┴────┴────┴───┘             │\n");
    printf("  │  strlen(s) → O(n), 手动管理, char[]可改      │\n");
    printf("  └────────────────────────────────────────────┘\n\n");

    printf("  Python 把长度藏在对象里 — 你看不见但安全。\n");
    printf("  C 把长度甩给你算 — 你看得见但危险。\n\n");
}

/* ── 5. 字符串字面量的本质 ── */
static void demo_string_literals(void)
{
    printf("=== 5. 字符串字面量 (String Literals) ===\n\n");

    /* 字面量是只读的 */
    const char *s1 = "Hello";
    const char *s2 = "Hello";
    printf("  const char *s1 = \"Hello\";\n");
    printf("  const char *s2 = \"Hello\";\n");
    printf("    s1 地址: %p\n", (void *)s1);
    printf("    s2 地址: %p\n", (void *)s2);

    if (s1 == s2) {
        printf("    s1 == s2 → ✅ 编译器可能合并相同字面量\n");
    } else {
        printf("    s1 == s2 → 不合并 (取决于编译器优化)\n");
    }
    printf("\n");

    /* 转义字符 */
    char escapes[] = "Tab\tNewline\nBackslash\\\\\n";
    printf("  转义字符: \"Tab\\tNewline\\nBackslash\\\\\\\\\"\n");
    printf("    → %s", escapes);
    printf("\n");

    /* 多行字符串字面量 */
    const char *multiline = "Line 1 "
                            "Line 2 "
                            "Line 3";
    printf("  多行字面量拼接: \"%s\"\n\n", multiline);
}

/* ── coordinator ── */
int main_string_basics_sample(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║         字符串基础 (String Basics)                      ║\n");
    printf("║         字符数组、空终止符、字符串字面量                ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    demo_char_array_vs_pointer();
    demo_null_terminator();
    demo_memory_layout();
    demo_python_vs_c();
    demo_string_literals();

    printf("string_basics_sample 演示完毕。\n");
    return 0;
}
