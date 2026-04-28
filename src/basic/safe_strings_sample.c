#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "basic/safe_strings_sample.h"

/* ── buffer sizes for demos ── */
#define SAFE_SMALL   8
#define SAFE_MED     32
#define SAFE_LARGE   64
#define SAFE_XLARGE  128

/* ── 1. strncpy 安全复制 — 始终 null-terminate ── */
static void demo_strncpy_safe_pattern(void)
{
    printf("=== 1. strncpy 安全复制模式 ===\n\n");

    char dest[SAFE_SMALL];
    const char *src_long = "This is a very long string";
    const char *src_short = "Hi";

    /* 长串截断 */
    strncpy(dest, src_long, sizeof(dest) - 1);
    dest[sizeof(dest) - 1] = '\0';
    printf("  长串截断: strncpy(dest, \"%s\", %zu);\n", src_long, sizeof(dest) - 1);
    printf("          dest[%zu] = '\\0';\n", sizeof(dest) - 1);
    printf("  → \"%s\" (截断, 原始 %zu 字节)\n\n", dest, strlen(src_long));

    /* 短串完整 */
    strncpy(dest, src_short, sizeof(dest) - 1);
    dest[sizeof(dest) - 1] = '\0';
    printf("  短串完整: strncpy(dest, \"%s\", %zu);\n", src_short, sizeof(dest) - 1);
    printf("  → \"%s\" (完整, 剩余 %zu 字节空间)\n\n", dest,
           sizeof(dest) - strlen(dest) - 1);

    printf("  ❌ 危险模式 (不要这样做!):\n");
    printf("     strncpy(nosafeguard, \"ABCDEFGH\", 8);  // 写满了, 没有 \\0\n");
    printf("     printf(\"%%s\", nosafeguard);  → 越界读取! 💣\n\n");
}

/* ── 2. snprintf 安全格式化 ── */
static void demo_snprintf_safe_pattern(void)
{
    printf("=== 2. snprintf — 永远比 sprintf 安全 ===\n\n");

    char buf[SAFE_MED];

    /* 空间足够 */
    int ret = snprintf(buf, sizeof(buf), "Hello, %s! Score: %d",
                       "World", 95);
    printf("  足够空间:\n");
    printf("    snprintf(buf, %zu, \"Hello, %%s! Score: %%d\", \"World\", 95);\n",
           sizeof(buf));
    printf("    → \"%s\" (需要 %d 字符)\n", buf, ret);
    printf("    → %s\n\n",
           (ret >= 0 && ret < (int)sizeof(buf)) ? "✅ 未截断" : "❌ 被截断");

    /* 空间不足 — 检测截断 */
    char tiny[16];
    ret = snprintf(tiny, sizeof(tiny), "This is over %zu chars", sizeof(tiny) + 20);
    printf("  空间不足:\n");
    printf("    snprintf(tiny, %zu, \"This is over %%zu chars\", %zu);;\n",
           sizeof(tiny), sizeof(tiny) + 20);
    printf("    → \"%s\"\n", tiny);
    printf("    → 返回值 %d 表示完整输出需要 %d 字符\n", ret, ret);
    printf("    → %s\n\n",
           (ret >= (int)sizeof(tiny)) ? "⚠️ 发生了截断!" : "未截断");

    /* 用返回值预判需不需要更大的缓冲区 */
    int needed = snprintf(NULL, 0, "User: %s, Age: %d, City: %s",
                          "Alice", 25, "Shanghai");
    printf("  预判缓冲区大小:\n");
    printf("    snprintf(NULL, 0, ...) → 需要 %d 字符 (+1 for '\\0' = %d 字节)\n",
           needed, needed + 1);
    printf("    → malloc(%d) 就够用了\n\n", needed + 1);

    printf("  💡 snprintf 的黄金规则:\n");
    printf("     ✅ 始终传 sizeof(buf) 作为第二个参数\n");
    printf("     ✅ 检查返回值: 如果 >= buf_size → 发生了截断\n");
    printf("     ✅ 用 snprintf(NULL, 0, ...) 预判所需大小\n\n");
}

/* ── 3. 边界检查模式 ── */
static void demo_bounds_checking(void)
{
    printf("=== 3. 边界检查模式 ===\n\n");

    char name[16];
    const char *input = "AveryLongUserNameStringExceedingBuffer";

    /* 模式 A: strncpy + null-terminate (最常用) */
    strncpy(name, input, sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';
    printf("  模式 A: strncpy + 手动 '\\0'\n");
    printf("    strncpy(name, input, %zu); name[%zu] = '\\0';\n",
           sizeof(name) - 1, sizeof(name) - 1);
    printf("    → \"%s\" (%zu 字符)\n\n", name, strlen(name));

    /* 模式 B: snprintf (更灵活) */
    char formatted[16];
    snprintf(formatted, sizeof(formatted), "User: %s", "BobTheBuilder");
    printf("  模式 B: snprintf\n");
    printf("    snprintf(formatted, %zu, \"User: %%s\", \"BobTheBuilder\");\n",
           sizeof(formatted));
    printf("    → \"%s\" (%zu 字符)\n\n", formatted, strlen(formatted));

    /* 模式 C: 手动长度检查 */
    size_t input_len = strlen(input);
    printf("  模式 C: 手动长度检查\n");
    printf("    input 长度 = %zu, 缓冲区大小 = %zu\n", input_len, sizeof(name));
    if (input_len >= sizeof(name)) {
        printf("    ⚠️ 输入过长! 需要截断\n");
    } else {
        printf("    ✅ 输入刚好\n");
    }
    printf("\n");
}

/* ── 4. 缓冲区溢出防御 ── */
static void demo_overflow_prevention(void)
{
    printf("=== 4. 缓冲区溢出防御 ===\n\n");

    printf("  溢出三件套 (每次字符串操作都要检查):\n\n");

    printf("  1️⃣ 目标缓冲区够大吗?\n");
    printf("     size_t needed = strlen(src) + 1;\n");
    printf("     if (needed > sizeof(dest)) { /* 截断或报错 */ }\n\n");

    printf("  2️⃣ 拼接时剩余空间够吗?\n");
    char buf[20];
    strncpy(buf, "Hello", sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    size_t remaining = sizeof(buf) - strlen(buf) - 1;
    printf("     char buf[20] = \"Hello\";\n");
    printf("     remaining = %zu 字节\n", remaining);
    printf("     if (strlen(addition) > remaining) { /* 截断 */ }\n\n");

    printf("  3️⃣ 格式化后总长度超标吗?\n");
    printf("     int ret = snprintf(buf, sizeof(buf), \"fmt\", args...);\n");
    printf("     if (ret < 0 || ret >= (int)sizeof(buf)) { /* 溢出风险 */ }\n\n");

    /* 实战: 安全拼接用户名和ID */
    char safe_buf[32];
    const char *uname = "alice_wonderland";
    int32_t uid = 42;

    int bytes_used = snprintf(safe_buf, sizeof(safe_buf),
                              "User: %s (id=%" PRId32 ")", uname, uid);
    printf("  实战演示:\n");
    printf("    snprintf(safe_buf, %zu, \"User: %%s (id=%%d)\", \"%s\", %d);\n",
           sizeof(safe_buf), uname, (int)uid);
    printf("    → \"%s\"\n", safe_buf);
    printf("    → 使用 %d/%zu 字节\n\n", bytes_used, sizeof(safe_buf));
}

/* ── 5. 安全函数的组合使用 ── */
static void demo_safe_function_combo(void)
{
    printf("=== 5. 安全函数组合使用 ===\n\n");

    /* 模拟: 从外部输入构建消息 */
    char msg[SAFE_LARGE];
    const char *author = "Bob";
    const char *content = "This is a message that could potentially be very long and exceed the buffer size if we're not careful";

    /* ❌ 错误: 不做边界检查 */
    printf("  ❌ 危险做法 (不要这样做!):\n");
    printf("     sprintf(msg, \"[%%s] %%s\", author, content);\n");
    printf("     → 如果 author + content 太长 → 缓冲区溢出 💥\n\n");

    /* ✅ 安全: 用 snprintf 控制总大小 */
    int total = snprintf(msg, sizeof(msg), "[%s] %s", author, content);
    printf("  ✅ 安全做法:\n");
    printf("     int total = snprintf(msg, %zu, \"[%%s] %%s\", \"%s\", content);\n",
           sizeof(msg), author);
    printf("     → \"%s\"\n", msg);
    printf("     → 需要 %d 字符, 缓冲区 %zu 字节, %s\n\n",
           total, sizeof(msg),
           (total >= (int)sizeof(msg)) ? "⚠️ 被截断" : "✅ OK");

    /* ✅ 分段构建 */
    char safe[SAFE_MED];
    int pos = 0;

    pos += snprintf(safe + pos, sizeof(safe) - pos, "[%s] ", author);
    pos += snprintf(safe + pos, sizeof(safe) - pos, "Hello!");

    printf("  ✅ 分段安全构建:\n");
    printf("    int pos = 0;\n");
    printf("    pos += snprintf(safe + pos, sizeof - pos, …);\n");
    printf("    → \"%s\" (pos = %d of %zu)\n\n", safe, pos, sizeof(safe));
}

/* ── coordinator ── */
int main_safe_strings_sample(void)
{
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║         安全字符串 (Safe Strings)                        ║\n");
    printf("║         strncpy, snprintf, 边界检查, 溢出预防            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    demo_strncpy_safe_pattern();
    demo_snprintf_safe_pattern();
    demo_bounds_checking();
    demo_overflow_prevention();
    demo_safe_function_combo();

    printf("safe_strings_sample 演示完毕。\n");
    return 0;
}
