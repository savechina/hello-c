#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "basic/wide_locale_sample.h"

/*
 * 宽字符与区域设置 (Wide Characters & Locale)
 *
 * 错误先行: 先看注释里"不开 locale 就解析 UTF-8"的错例,
 * 再看 mbrtowc/wcrtomb 往返的正确姿势。
 */

/* setlocale(LC_ALL, "") 从环境变量 (LANG/LC_*) 选 locale。
 * 返回值必须检查 — 失败时 locale 仍停留在 "C", 多字节解析会出错。 */
static int wl_setup_locale(void)
{
    const char *loc = setlocale(LC_ALL, "");

    if (loc == NULL) {
        printf("  ⚠ setlocale(LC_ALL, \"\") 失败, locale 保持 \"C\"\n");
        return 0;
    }
    printf("  setlocale(LC_ALL, \"\") → \"%s\"\n", loc);
    return 1;
}

/* ❌ 错例 (仅示意, 勿模仿):
 *   wchar_t wc;
 *   mbrtowc(&wc, "你", MB_CUR_MAX, NULL); // 未 setlocale 时 locale="C",
 *                                        // 0xE4 不是合法单字节 → (size_t)-1
 *   printf("%lc", wc);                   // 打印垃圾或宽字符转换失败
 * ASCII 不需要 locale: "C" locale 里每个字节都是合法字符;
 * 但 UTF-8 多字节名字必须先 setlocale, 否则 mbrtowc 直接报错。
 */

size_t wl_mbtowc_n(const char *src, wchar_t *dst, size_t cap)
{
    mbstate_t ps;
    size_t count = 0;

    if (src == NULL || dst == NULL || cap == 0) {
        return (size_t)-1;
    }

    memset(&ps, 0, sizeof(ps)); /* 转换状态必须清零 — 多字节序列可能跨调用 */

    while (*src != L'\0') {
        size_t n = mbrtowc(dst, src, MB_CUR_MAX, &ps);

        if (n == (size_t)-1 || n == (size_t)-2) {
            return (size_t)-1; /* 非法序列 / 不完整序列 */
        }
        if (n == 0) {
            src++; /* mbrtowc 遇到 '\0' 返回 0, 消费 1 字节 */
        } else {
            src += n;
        }
        dst++;
        count++;
        if (count >= cap) {
            return (size_t)-1; /* 容量不够 — 拒绝半截结果 */
        }
    }
    *dst = L'\0'; /* 成功时保证 NUL 结尾 */
    return count;
}

int wl_roundtrip_ok(const char *src)
{
    mbstate_t ps_out;
    wchar_t wide[64];
    char back[64];
    size_t n_wide, i;
    size_t back_len = 0;

    n_wide = wl_mbtowc_n(src, wide, sizeof(wide) / sizeof(wide[0]));
    if (n_wide == (size_t)-1) {
        return 0;
    }

    memset(&ps_out, 0, sizeof(ps_out));
    for (i = 0; i <= n_wide; i++) { /* i == n_wide 时转 L'\0' */
        mbstate_t ps_probe = ps_out;
        size_t need = wcrtomb(NULL, wide[i], &ps_probe);

        if (need == (size_t)-1) {
            return 0;
        }
        /* 预检查容量再写入，避免先写 NUL 再判界导致 1 字节溢出 */
        if (need > sizeof(back) - back_len) {
            return 0;
        }
        size_t n = wcrtomb(back + back_len, wide[i], &ps_out);

        if (n == (size_t)-1) {
            return 0;
        }
        back_len += n;
    }

    return memcmp(src, back, back_len) == 0 && back[back_len - 1] == '\0' ? 1 : 0;
}

static void wide_demo(void)
{
    const char *name = "hello";
    wchar_t wide[16];
    size_t n;

    printf("=== wchar_t 基础: 纯 ASCII 无需 locale ===\n");
    printf("  sizeof(wchar_t) = %zu\n", sizeof(wchar_t));
    n = wl_mbtowc_n(name, wide, sizeof(wide) / sizeof(wide[0]));
    /* %lc 形参是 wint_t; FreeBSD 上 wchar_t 是 unsigned int 而 wint_t 是 int,
     * 必须显式转换才能通过 -Wformat (macOS/Linux/illumos 上同为 int 也保持正确) */
    printf("  wl_mbtowc_n(\"%s\") = %zu, wide[0]=%lc (L'%c')\n",
           name, n, (wint_t)wide[0], name[0]);
    printf("  wl_roundtrip_ok(\"%s\") = %d\n", name, wl_roundtrip_ok(name));
    printf("  ❌ 错例: 未 setlocale 就 mbrtowc UTF-8 → (size_t)-1, 见注释\n\n");
}

static void locale_demo(void)
{
    const char *name_utf8 = "caf\xc3\xa9"; /* "café" 的 UTF-8 编码 */
    wchar_t wide[16];
    size_t n;

    printf("=== setlocale + 多字节往返 (名字场景) ===\n");
    if (!wl_setup_locale()) {
        printf("  跳过 UTF-8 演示 (无可用 locale, 例如 LANG 未设置)\n\n");
        return;
    }
    n = wl_mbtowc_n(name_utf8, wide, sizeof(wide) / sizeof(wide[0]));
    if (n == (size_t)-1) {
        printf("  ⚠ \"café\" 转换失败 — 当前 locale 可能不是 UTF-8\n\n");
        return;
    }
    printf("  \"café\" → %zu 个宽字符, roundtrip_ok = %d\n",
           n, wl_roundtrip_ok(name_utf8));
    printf("  为什么名字需要 locale: ASCII 单字节在 \"C\" locale 也合法;\n");
    printf("  但 UTF-8 的 café 每个非 ASCII 字符占 2 字节, 只有 locale\n");
    printf("  知道编码规则, mbrtowc 才能把 2 个字节拼成 1 个 wchar_t。\n\n");
}

/* ── public entry ── */

int main_wide_locale_sample(void)
{
    printf("--- 宽字符与区域设置 (Wide Chars & Locale) ---\n");
    printf("wchar_t + setlocale + mbrtowc/wcrtomb: ASCII 到处可用, 名字需要 locale。\n\n");

    wide_demo();
    locale_demo();

    printf("宽字符与区域设置演示完毕.\n");
    return 0;
}
