# 宽字符与本地化 (wchar_t / setlocale / mbrtowc)

```admonish note
**本章讲什么** — C 的"宽"文本工具链:`setlocale` 选择 locale、`wchar_t` 存单个宽字符、`mbrtowc`/`wcrtomb` 在多字节与宽字符间转换。错误先行:`wchar_t` **不是** UTF-32(Windows 上是 UTF-16)、忽略 `mbrtowc` 返回 `(size_t)-1` 会吞掉非法字节、忘记 `NUL` 终止会越界。示例在无 UTF-8 locale 的环境下自动降级为 ASCII 路径,不炸。
**对应源码** — [`src/basic/wide_locale_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/wide_locale_sample.c)
```

## 开篇故事

`char` 是"一字节"的心智,可世界不只有 ASCII。中文一个字占 3 字节(UTF-8),emoji 占 4 字节 —— 按字节遍历会把一个多字节字符切成碎片。C89 时代给了一条"宽"路:`wchar_t` 一次装一个字符的码点。但 `wchar_t` 有多大、字节流怎么编码,**完全由当前 locale 决定** —— 这就是为什么宽字符程序必须先调 `setlocale`,否则满屏乱码或静默错误。

## 本章适合谁

- 处理过中文乱码、想知道"C 里怎么正确处理 UTF-8"的读者
- 学过字符串,想搞懂 `char*` 与 `wchar_t*` 边界的人
- 要写国际化(i18n)命令行工具的程序员

## 你会学到什么

1. `setlocale(LC_ALL, "")` 为什么是宽字符程序的第一行,以及失败时的降级策略
2. `mbrtowc`(多字节→宽字符)与 `wcrtomb`(宽字符→多字节)的安全用法:计数、终止、错误检测
3. `wchar_t` 的平台差异:它**不是**固定的 32 位;可移植代码只当"足够装下一个编码单元"用

## 错例:最常见的三连错

```c
/* ❌ 错例 (仅示意, 乱码/UB) */
wchar_t w;
mbrtowc(&w, src, 4, &st);      /* 不看返回值 → 非法字节被静默吞掉 */
mbrtowc(&w, "caf\xc3\xa9", 4, &st);  /* 忘了 setlocale, C locale 下 0xC3 非法 */
wchar_t buf[8];
mbsrtowcs(buf, src, 8, NULL);  /* buf 无 NUL 空间 → 越界写 */
```

### 为什么错

1. `mbrtowc` 返回 `(size_t)-1`(非法序列)或 `(size_t)-2`(序列不完整)—— 必须处理
2. 默认 `"C"` locale 只认单字节 ASCII;UTF-8 输入必须先 `setlocale(LC_ALL, "")`
3. 转换结果要手动保证 `NUL` 终止,标准库不替你留位置

## 正确写法:安全的转换包装

```c
size_t wl_mbtowc_n(const char *src, wchar_t *dst, size_t cap)
{
    mbstate_t st;
    memset(&st, 0, sizeof st);          /* 每次转换重置移位状态 */

    size_t n = 0;
    while (*src != '\0') {
        size_t k = mbrtowc(&dst[n], src, MB_CUR_MAX, &st);
        if (k == (size_t)-1 || k == (size_t)-2) return (size_t)-1;  /* 非法/不完整 */
        src += k;
        n++;
        if (n >= cap) return (size_t)-1; /* 溢出保护 */
    }
    dst[n] = L'\0';                      /* ✅ 手动 NUL 终止 */
    return n;
}
```

往返验证(证明"转过去再转回来字节相同"):

```c
int wl_roundtrip_ok(const char *src)
{
    wchar_t wide[64];
    if (wl_mbtowc_n(src, wide, 64) == (size_t)-1) return 0;

    char back[64];
    mbstate_t st;
    memset(&st, 0, sizeof st);

    size_t total = 0;
    for (size_t i = 0; ; i++) {
        size_t k = wcrtomb(&back[total], wide[i], &st);
        if (k == (size_t)-1) return 0;
        total += k;
        if (wide[i] == L'\0') break;     /* 含终止 NUL 一起往返 */
    }
    return memcmp(src, back, total) == 0;
}
```

## 原理解析

### 1. locale 是一切的前提

```c
if (setlocale(LC_ALL, "") == NULL) {
    /* 环境变量无可用 locale → 降级为 ASCII, 不炸 */
}
```

- `setlocale(LC_ALL, "")` 从环境变量(`LANG`/`LC_ALL`)读取用户 locale
- `"C"` locale 下 `mbrtowc` 只接受单字节;UTF-8 字节被判非法
- 失败时**优雅降级**:示例打印提示并跳过 UTF-8 演示,只跑 ASCII

### 2. wchar_t 的真身

| 平台 | `wchar_t` | "一个字符"含义 |
|------|-----------|----------------|
| Linux/macOS | 32 位 | 通常一个 UTF-32 码点 |
| Windows | 16 位 | UTF-16 **编码单元**(一个 emoji = 2 个 wchar_t) |

可移植代码规则:**不要假设 `sizeof(wchar_t) == 4`**,不要拿 `wchar_t` 当 Unicode 码点直接比较 emoji;跨平台库(如 ICU)才是严肃 i18n 的答案。本章教的是 C 标准原语的正确用法,不是完整 Unicode 方案。

### 3. mbstate_t:多字节状态机

UTF-8 之外还有 ISO-2022 这类**状态相关**编码:同一个字节序列在不同状态下含义不同。`mbrtowc` 用 `mbstate_t` 跨字符保存状态 —— 所以:

- 每次**新字符串**转换前 `memset(&st, 0, sizeof st)` 重置
- 同一字符串内**连续**调用必须复用同一个 `st`

## 动手练习

### 🟢 入门:数宽字符个数

用 `wl_mbtowc_n` 把 `"hello"` 转宽字符,断言长度为 5、`wide[5] == L'\0'`。

<details><summary>点击查看答案</summary>

```c
wchar_t wide[16];
size_t n = wl_mbtowc_n("hello", wide, 16);
assert(n == 5 && wide[5] == L'\0');
```

</details>

### 🟡 中级:UTF-8 中文往返

设 `setlocale(LC_ALL, "")` 后,对 `"你好"` 做 `wl_roundtrip_ok` 往返验证;locale 不可用时打印 SKIP。

<details><summary>点击查看答案</summary>

```c
if (setlocale(LC_ALL, "") != NULL) {
    assert(wl_roundtrip_ok("\xe4\xbd\xa0\xe5\xa5\xbd") == 1);
} else {
    printf("SKIP: no UTF-8 locale\n");
}
```

</details>

### 🔴 挑战:非法字节检测器

写 `int wl_is_valid_utf8(const char *s)`:用 `mbrtowc` 逐字符走,任何 `(size_t)-1` 返回 0 —— 验证 `"\xff"` 非法、`"caf\xc3\xa9"` 合法。

<details><summary>点击查看答案</summary>

```c
int wl_is_valid_utf8(const char *s)
{
    wchar_t w[8];
    mbstate_t st;
    memset(&st, 0, sizeof st);
    while (*s) {
        size_t k = mbrtowc(w, s, MB_CUR_MAX, &st);
        if (k == (size_t)-1 || k == (size_t)-2) return 0;
        s += k;
    }
    return 1;
}
```

</details>

## 故障排查 (FAQ)

**Q:为什么打印宽字符全是乱码?**

A:终端是 UTF-8,而你的 `wchar_t` 数据来自另一个编码,或没 `setlocale`。先确认 `setlocale(LC_ALL, "")` 成功,再确认源文件保存为 UTF-8。

**Q:`mbrtowc` 和 `mbtowc` 有什么区别?**

A:`mbtowc` 用全局内部状态(不可重入);`mbrtowc` 用调用者提供的 `mbstate_t`(可重入、线程安全)。新代码一律用 `r` 系列。

**Q:`wchar_t` 能当 `int` 用吗?**

A:能存码点值,但平台宽度不同(Linux 4 字节 / Windows 2 字节)。可移植代码只做转换与比较,不做宽度假设。

## 小结

- 第一行:`setlocale(LC_ALL, "")`,失败则降级 ASCII,不硬闯
- `mbrtowc` 返回值三态:正常字节数 / `(size_t)-1` 非法 / `(size_t)-2` 不完整 —— 必须处理
- 转换结果**手动** `NUL` 终止;`mbstate_t` 每个新字符串前清零
- `wchar_t` 不是 UTF-32;Windows 上是 UTF-16 编码单元
- 本章是 C 原语;严肃跨平台 i18n 用 ICU

## 术语表

| 术语 | 英文 | 解释 |
|------|------|------|
| 本地化 | Locale / setlocale | 影响字符分类、编码、排序的环境配置 |
| 宽字符 | Wide character | `wchar_t` 表示的字符编码单元 |
| 多字节序列 | Multibyte sequence | 一个字符由多个字节表示(如 UTF-8) |
| 转换状态 | mbstate_t | 状态相关编码的跨字符移位状态 |
| 往返转换 | Round-trip | 多字节→宽→多字节后字节完全一致 |

## 延伸阅读

- [cppreference - mbrtowc](https://en.cppreference.com/w/c/string/multibyte/mbrtowc)
- [cppreference - setlocale](https://en.cppreference.com/w/c/locale/setlocale)
- [Unicode UTF-8](https://unicode.org/faq/utf_bom.html)

## 继续学习

- [上一章](./datatype.md):数据类型
- [下一章](./strings.md):字符串

---

> 本章代码位于 [`src/basic/wide_locale_sample.c`](https://github.com/savechina/hello-c/blob/main/src/basic/wide_locale_sample.c),测试位于 [`test/basic/test_wide_locale_sample.c`](https://github.com/savechina/hello-c/blob/main/test/basic/test_wide_locale_sample.c)。
