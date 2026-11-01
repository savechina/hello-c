#include "basic/datatype_sample.h"
#include <float.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* <uchar.h> 可移植性探测 (portable feature probe):
 * char16_t / char32_t 在 C 里不是关键字, 只能由 C11 的 <uchar.h> 定义。
 * Apple SDK 至今不提供该头文件, 而 Linux/glibc 有 — 用 __has_include
 * (clang/GCC 均支持的预处理运算符) 探测, 两边都能编译。 */
#if defined(__has_include)
  #if __has_include(<uchar.h>)
    #include <uchar.h>
    #define DATATYPE_HAS_UCHAR 1
  #endif
#endif

#ifndef DATATYPE_HAS_UCHAR
/* macOS 降级路径: 标准本来就规定 char16_t ≡ uint_least16_t,
 * char32_t ≡ uint_least32_t — 手工复刻这两个 typedef,
 * 让本章演示在没有 <uchar.h> 的平台上同样可运行。
 * ⚠️ 隐患备忘: C23 把 char16_t/char32_t 变成关键字, bump std 前必须本文件重审。
 *   (eng-review 2026-10-31 D6=A 决议: 保留演示保真度 + 记录触发条件) */
typedef uint_least16_t char16_t;
typedef uint_least32_t char32_t;
#endif

/*
 * 数据类型全景 (Datatype Landscape) — C17
 *
 * 十个板块:
 *   1. 整数家族 (basic integer family, limits.h)
 *   2. 定宽整数 (fixed-width integers, inttypes.h)
 *   3. 字符类型 (char / signed char / wchar_t / char16_t / char32_t)
 *   4. 浮点家族 (float / double / long double, float.h)
 *   5. 布尔类型 (_Bool / stdbool.h)
 *   6. size_t / ptrdiff_t / offsetof
 *   7. 复合类型速览 (array / struct / union / enum / pointer / func ptr)
 *   8. 类型限定符 (const / volatile / restrict — 点到为止)
 *   9. 日期与时间 (time_t / clock_t / struct tm / timespec / strftime)
 *  10. sizeof 汇总表 (summary table)
 *
 * 约定: src/basic/ 只用 ISO C 头文件 (<time.h> <uchar.h> <inttypes.h> 等),
 * POSIX 头 (<sys 目录下那些) 属于 src/advance/ 的地盘。
 */

/* --- 文件作用域 helper: 第 7 节的函数指针演示用 --- */
static int datatype_add_sample(int a, int b) { return a + b; }

/* ======================= 各板块实现 ======================= */

/* 第 1 节: short / int / long / long long (signed & unsigned) + limits.h */
static void datatype_integer_family_sample(void) {
  printf("=== 1. 整数家族 (Basic Integer Family) ===\n\n");

  /* 所有边界值一律引用 <limits.h> 宏, 绝不硬编码 — 跨平台时它们会变。 */
  printf("  short              sizeof=%zu  SHRT_MIN=%d  SHRT_MAX=%d\n",
         sizeof(short), SHRT_MIN, SHRT_MAX);
  printf("  unsigned short     sizeof=%zu  USHRT_MAX=%u  (无符号下界恒为 0)\n",
         sizeof(unsigned short), (unsigned)USHRT_MAX);
  printf("  int                sizeof=%zu  INT_MIN=%d  INT_MAX=%d\n",
         sizeof(int), INT_MIN, INT_MAX);
  printf("  unsigned int       sizeof=%zu  UINT_MAX=%u\n", sizeof(unsigned int),
         UINT_MAX);
  printf("  long               sizeof=%zu  LONG_MIN=%ld  LONG_MAX=%ld\n",
         sizeof(long), LONG_MIN, LONG_MAX);
  printf("  unsigned long      sizeof=%zu  ULONG_MAX=%lu\n",
         sizeof(unsigned long), ULONG_MAX);
  printf("  long long          sizeof=%zu  LLONG_MIN=%lld  LLONG_MAX=%lld\n",
         sizeof(long long), LLONG_MIN, LLONG_MAX);
  printf("  unsigned long long sizeof=%zu  ULLONG_MAX=%llu\n",
         sizeof(unsigned long long), ULLONG_MAX);

  printf("  注: sizeof(long) 平台相关 — LP64 (macOS/Linux 64位) = 8,\n");
  printf("      LLP64 (Windows 64位) = 4。跨平台代码别假设位宽, 用 "
         "<stdint.h>。\n");
  printf("  注: 字面量后缀标类型 — 42=int, 42L=long, 42LL=long long, "
         "42u=unsigned。\n");
  printf("  注: 有符号溢出是 UB (不是回绕) — 深入见 integer_safety 章, "
         "本章绝不演示它。\n");
  printf("\n");
}

/* 第 2 节: int8_t..int64_t + intmax_t/uintmax_t + intptr_t/uintptr_t */
static void datatype_fixed_width_sample(void) {
  int8_t mi8 = -42;
  uint8_t mu8 = 200;
  int16_t mi16 = -12345;
  uint16_t mu16 = 65535;
  int32_t mi32 = 12345;
  uint32_t mu32 = 4000000000u;
  int64_t mi64 = 9223372036854775807LL;
  uint64_t mu64 = UINT64_MAX;
  int sample = 7;

  printf("=== 2. 定宽整数 (Fixed-Width Integers, <inttypes.h>) ===\n\n");

  /* PRId 与 PRIu 系列宏保证在任何平台上格式都正确 (Windows 的 int32_t 是 int,
   * int64_t 却是 long long — 直接写 %d/%lld 必然踩坑)。 */
  printf("  int8_t   = %" PRId8 "   (range: %" PRId8 " ~ %" PRId8 ")\n", mi8,
         INT8_MIN, INT8_MAX);
  printf("  uint8_t  = %" PRIu8 "  (range: 0 ~ %" PRIu8 ")\n", mu8, UINT8_MAX);
  printf("  int16_t  = %" PRId16 "  (range: %" PRId16 " ~ %" PRId16 ")\n", mi16,
         INT16_MIN, INT16_MAX);
  printf("  uint16_t = %" PRIu16 "  (range: 0 ~ %" PRIu16 ")\n", mu16,
         UINT16_MAX);
  printf("  int32_t  = %" PRId32 "  (range: %" PRId32 " ~ %" PRId32 ")\n", mi32,
         INT32_MIN, INT32_MAX);
  printf("  uint32_t = %" PRIu32 "  (range: 0 ~ %" PRIu32 ")\n", mu32,
         UINT32_MAX);
  printf("  int64_t  = %" PRId64 "  (range: %" PRId64 " ~ %" PRId64 ")\n", mi64,
         INT64_MIN, INT64_MAX);
  printf("  uint64_t = %" PRIu64 "  (range: 0 ~ %" PRIu64 ")\n", mu64,
         UINT64_MAX);

  /* intmax_t / uintmax_t: 本实现支持的最宽有符号 / 无符号整数类型。
   * 万能兜底: 不知道数据多宽时用它 (vsnprintf 返回值也是 intmax 语义)。 */
  printf("  intmax_t  = %" PRIdMAX "  (INTMAX_MIN ~ INTMAX_MAX)\n",
         (intmax_t)INTMAX_MAX);
  printf("  uintmax_t = %" PRIuMAX "  (0 ~ UINTMAX_MAX)\n",
         (uintmax_t)UINTMAX_MAX);

  /* intptr_t / uintptr_t: 宽度足以容纳指针的整数 — 用于把指针当整数存
   * (句柄表 / 哈希)。打印地址用 PRIxPTR, 比 %p 更适合先存后印的场景。 */
  printf("  &sample       = 0x%" PRIxPTR
         "  (uintptr_t 装指针, cast (uintptr_t)p)\n",
         (uintptr_t)&sample);
  printf(
      "  sizeof(intptr_t)=%zu  sizeof(uintptr_t)=%zu  sizeof(intmax_t)=%zu\n",
      sizeof(intptr_t), sizeof(uintptr_t), sizeof(intmax_t));
  printf("\n");
}

/* 第 3 节: plain char / signed char / unsigned char / wchar_t / char16_t /
 * char32_t */
static void datatype_char_types_sample(void) {
  char c = 'A';
  signed char sc = -1;
  unsigned char uc = 255;
  wchar_t wc = L'A';
  char16_t c16 = u'中';
  char32_t c32 = U'A';

  printf("=== 3. 字符类型 (Character Types) ===\n\n");

  /* plain char 的 signedness 是 implementation-defined — 这是 C 里最容易
   * 被忽视的坑: ARM Linux ABI 规定 char = unsigned, x86/Apple 规定 signed。
   * 需要明确符号时永远显式写 signed char / unsigned char。 */
  printf(
      "  char           '%c' value=%d  sizeof=%zu  CHAR_MIN=%d  CHAR_MAX=%d\n",
      c, c, sizeof(char), CHAR_MIN, CHAR_MAX);
  printf("  signed char    value=%d  sizeof=%zu  SCHAR_MIN=%d  SCHAR_MAX=%d\n",
         sc, sizeof(signed char), SCHAR_MIN, SCHAR_MAX);
  printf("  unsigned char  value=%u  sizeof=%zu  UCHAR_MAX=%u\n", uc,
         sizeof(unsigned char), UCHAR_MAX);
  printf("  → 本平台 plain char %s (CHAR_MIN < 0 即带符号)\n",
         CHAR_MIN < 0 ? "默认带符号 (signed)" : "默认不带符号 (unsigned)");

  /* wchar_t: 宽字符, 由 <stddef.h>/<wchar.h> 定义 typedef (C 里不是关键字)。
   * sizeof(wchar_t) 平台相关: Windows=2 (UTF-16), macOS/Linux=4 (UCS-4)。 */
  printf("  wchar_t  L'A'  = U+%04X  sizeof=%zu\n", (unsigned)wc,
         sizeof(wchar_t));

  /* char16_t / char32_t (C11 <uchar.h>): 分别恒为 16/32 位无符号整数,
   * 专用于 UTF-16 / UTF-32 码点 (code point)。
   * 打印十六进制码点而不是 %c — 避免 locale / 执行字符集干扰。 */
  printf("  char16_t u'中' = U+%04X  sizeof=%zu\n", (unsigned)c16,
         sizeof(char16_t));
  printf("  char32_t U'A'  = U+%04X  sizeof=%zu\n", (unsigned)c32,
         sizeof(char32_t));
#ifndef DATATYPE_HAS_UCHAR
  printf("  (本平台无 <uchar.h>, 上面两个类型由本文件 typedef 复刻 — "
         "见文件头注释)\n");
#endif

  /* 类型 ≠ 编码: 同一个"中"字, UTF-8 下 char[] 占 3 字节, char16_t 占 2,
   * char32_t 占 4。这里假设源文件/执行字符集为 UTF-8 (本教程默认)。 */
  printf("  \"中\" 存进 char[] (UTF-8) = %zu 字节; 同一字在 char16_t 是 1 个 "
         "16-bit 单元\n",
         strlen("中"));
  printf("\n");
}

/* 第 4 节: float / double / long double + float.h */
static void datatype_float_family_sample(void) {
  float f = 3.14f;
  double d = 3.14159265358979;
  long double ld = 2.71828182845904523536L;
  float f01 = 0.1f;

  printf("=== 4. 浮点家族 (Floating-Point Family) ===\n\n");

  /* %g 自动在 %f / %e 间选更紧凑的表示 — 浮点极限这种"很大/很小"的数最适用 */
  printf("  float       sizeof=%zu  sample=%g\n", sizeof(float), (double)f);
  printf("  double      sizeof=%zu  sample=%.14f\n", sizeof(double), d);
  printf("  long double sizeof=%zu  sample=%Lg\n", sizeof(long double), ld);

  printf("  FLT_MIN=%g  FLT_MAX=%g  FLT_EPSILON=%g\n", FLT_MIN, FLT_MAX,
         FLT_EPSILON);
  printf("  DBL_MIN=%g  DBL_MAX=%g  DBL_EPSILON=%g\n", DBL_MIN, DBL_MAX,
         DBL_EPSILON);
  printf("  LDBL_MIN=%Lg  LDBL_MAX=%Lg  LDBL_EPSILON=%Lg\n", LDBL_MIN, LDBL_MAX,
         LDBL_EPSILON);

  printf("  注: FLT_MIN 是最小正规化 (normalized) 正数, 不是「最小非零值」;\n");
  printf("      更小的 subnormal 数不在此宏语义内。\n");
  printf("  注: long double 位宽看 ABI — x86_64 Linux=80位扩展(存16字节),\n");
  printf(
      "      Apple Silicon ABI 直接让 long double 退化成 double (8字节)。\n");

  /* 精度陷阱 (precision pitfall): 0.1 是二进制下的循环小数, float 存不下。
   * 不写 f01 == 0.1 做演示 — 两个都是常量, 编译器直接判恒 false 并触发
   * -Wliteral-range (本仓库 -Werror 会拦), 这本身就证明了"永远别用 =="。 */
  printf("  精度陷阱: 0.1f 实际存的是 %.9g, 而 0.1 (double) 是 %.17g → 0.1f != "
         "0.1\n",
         (double)f01, 0.1);
  printf("  结论: 浮点不要用 == 精确比较, 用 |a-b| <= 容差 (epsilon) 判断。\n");
  printf("\n");
}

/* 第 5 节: _Bool / <stdbool.h> */
static void datatype_bool_sample(void) {
  _Bool raw = 0;
  bool flag = true;   /* stdbool.h: bool 是 _Bool 的宏, true/false 也是宏 */
  bool cmp = (3 > 2); /* 关系表达式的结果正是 1 (true) / 0 (false) */

  printf("=== 5. 布尔类型 (_Bool / <stdbool.h>) ===\n\n");

  printf("  sizeof(_Bool)=%zu — 不是 1 bit, 是最小可寻址单元 (1 byte)\n",
         sizeof(_Bool));
  /* _Bool 赋值时归一化 (normalization): 任何非 0 → 1, 不会"保留原值" */
  raw = 42;
  printf("  _Bool b = 42 → %d (赋值时非0归一化为1)\n", (int)raw);
  raw = 0;
  printf("  _Bool b = 0  → %d\n", (int)raw);
  printf("  bool flag = true → %d, (3 > 2) 存进 bool → %d\n", (int)flag,
         (int)cmp);
  printf("\n");
}

/* 第 6 节: size_t / ptrdiff_t / offsetof */
static void datatype_size_types_sample(void) {
  int nums[5] = {10, 20, 30, 40, 50};
  int* p_first = &nums[1];
  int* p_last = &nums[4];
  ptrdiff_t dist = p_last - p_first;
  struct Layout {
    char tag;      /* offset 0 */
    double weight; /* 通常 offset 8 — 编译器插 padding 对齐到 8 */
    int id;        /* 通常 offset 16 */
  };

  printf("=== 6. size_t / ptrdiff_t / offsetof ===\n\n");

  printf("  sizeof(size_t)=%zu  sizeof(ptrdiff_t)=%zu\n", sizeof(size_t),
         sizeof(ptrdiff_t));
  printf("  size_t     = 无符号, 专门装 sizeof 的结果和长度/下标 (绝不为负)\n");
  printf("  ptrdiff_t  = 有符号, 专门装「指针相减」的结果\n");

  /* 指针相减得到的是元素个数, 不是字节数; 两个指针必须指向同一数组 */
  printf("  &nums[4] - &nums[1] = %td 个元素 (ptrdiff_t, 格式串用 %%td)\n",
         dist);

  /* offsetof: 字段相对结构体起始地址的字节偏移 — 序列化/协议解析的常客 */
  printf("  offsetof(Layout, tag)=%zu  (weight)=%zu  (id)=%zu\n",
         offsetof(struct Layout, tag), offsetof(struct Layout, weight),
         offsetof(struct Layout, id));
  printf("  sizeof(struct Layout)=%zu > 三字段之和 %zu — 差值就是 padding\n",
         sizeof(struct Layout), sizeof(char) + sizeof(double) + sizeof(int));
  printf("\n");
}

/* 第 7 节: 复合类型速览 — 每样只给 sizeof + 一个最小演示 (深入见各专章) */
static void datatype_composites_sample(void) {
  /* [array] 连续内存, 编译期定长 */
  int arr[5] = {10, 20, 30, 40, 50};
  /* [struct] 各字段各占其位; char name[32] 是本仓库的嵌入缓冲区约定 */
  struct Person {
    char name[32];
    int age;
    double height;
  } p;
  /* [union] 全部成员共享同一偏移 — 至多一个成员"活着" */
  union Probe {
    uint32_t word;
    uint8_t byte[4];
  } probe;
  /* [enum] 一组具名整数常量; 未显式赋值的枚举器自动 +1 */
  enum TrafficLight { LIGHT_RED = 0, LIGHT_YELLOW = 2, LIGHT_GREEN = 5 };
  /* [pointer] 存地址; void* 是"通用指针" (C 里的 erase type) */
  int value = 42;
  int* ip = &value;
  void* vp = &value;
  /* [function pointer] 指向函数而非数据 — 见 function_pointers 专章 */
  int (*op)(int, int) = datatype_add_sample;
  size_t i;
  int written;

  printf("=== 7. 复合类型速览 (Composite Types Quick Tour) ===\n\n");

  printf("  [array]    sizeof(arr)=%zu = %zu × sizeof(int), 元素个数=%zu\n",
         sizeof arr, sizeof arr / sizeof arr[0], sizeof arr / sizeof arr[0]);
  printf("             arr[2]=%d; 惯用法 sizeof arr / sizeof arr[0] 求长度\n",
         arr[2]);

  /* 有界写入: 永远 snprintf, 永远传 sizeof buf — 见 safe_strings 章 */
  written = snprintf(p.name, sizeof p.name, "%s", "Ada Lovelace");
  p.age = 36;
  p.height = 1.67;
  printf("  [struct]   sizeof(struct Person)=%zu, name=\"%s\" (snprintf 写入 "
         "%d 字节)\n",
         sizeof p, p.name, written);
  printf("             char name[32] 内嵌数组约定 — 避免 char* 悬垂 (见 basic/ "
         "各章)\n");

  /* 同址探针: 写 word、读 byte[] — 看 union 成员共享同一段字节 */
  probe.word = 0x01020304u;
  printf(
      "  [union]    sizeof(union Probe)=%zu (= 最大成员 %zu), word=0x%08" PRIx32
      " → byte[] =",
      sizeof probe, sizeof(uint32_t), probe.word);
  for (i = 0; i < sizeof probe.byte / sizeof probe.byte[0]; i++) {
    printf(" %02X", (unsigned)probe.byte[i]);
  }
  printf("  ← 字节顺序 (endian) 决定读出方向\n");

  printf("  [enum]     RED=%d YELLOW=%d GREEN=%d, sizeof=%zu (至少 int)\n",
         LIGHT_RED, LIGHT_YELLOW, LIGHT_GREEN, sizeof(enum TrafficLight));

  printf("  [pointer]  sizeof(int*)=%zu sizeof(void*)=%zu, *ip=%d, &value=%p\n",
         sizeof(int*), sizeof(void*), *ip, (void*)vp);

  printf("  [func ptr] int (*op)(int,int) = datatype_add_sample → op(2,3)=%d, "
         "sizeof=%zu\n",
         op(2, 3), sizeof op);

  printf("  → 深入: 各类型均有专章 (arrays / structs / unions / enums / "
         "pointers /\n");
  printf("     function_pointers), 本节只做地图不展开。\n");
  printf("\n");
}

/* 第 8 节: 类型限定符 — 只点名, 深入内容全在 advance/ */
static void datatype_qualifiers_sample(void) {
  const int MAX_ITEMS = 100;
  const double E = 2.71828;
  volatile int heartbeat = 1;

  printf("=== 8. 类型限定符 (Type Qualifiers) ===\n\n");

  /* const: 编译期只读承诺 — 违反是约束违规 (constraint violation), 编译器会拦
   */
  printf("  const int    MAX_ITEMS = %d\n", MAX_ITEMS);
  printf("  const double E         = %.5f\n", E);
  printf(
      "  → const 是「承诺不写」, 不是「零开销」 (它同样影响优化与接口契约)\n");

  /* volatile: 每次访问都真实读写内存, 禁止优化掉/合并 — MMIO 寄存器、
   * 信号处理器共享变量、setjmp/longjmp 场景。深入见 advance/ volatile 专章。 */
  heartbeat = 2;
  printf("  volatile int heartbeat = %d (每次访问不被优化掉; 深入见 advance/ "
         "章)\n",
         heartbeat);

  /* restrict: 只能出现在指针形参上, 承诺"本指针独占该内存, 不与其他指针重叠"
   * — 编译器据此敢做向量化。完整示例: src/advance/restrict_sample.c */
  printf("  restrict: 见 src/advance/restrict_sample.c (缓冲区不重叠契约), "
         "本章不展开\n");
  printf("\n");
}

/* 第 9 节: 日期与时间 — 全部 ISO C (<time.h>), 不引入 POSIX 的 localtime_r 等
 */
static void datatype_datetime_sample(void) {
  time_t now = time(NULL);
  struct tm* tm_ptr;
  struct timespec ts;
  clock_t c0;
  clock_t c1;
  char buf[64];
  size_t len;
  volatile double sink = 0.0; /* volatile: 防止 -O2 把整个忙等循环优化掉 */
  int i;

  printf("=== 9. 日期与时间 (Date & Time) ===\n\n");

  /* --- time_t: 纯整数秒数 (Unix timestamp) --- */
  printf("  [time_t]   now = %lld  ← 显式 cast 成 (long long) 配 %%lld\n",
         (long long)now);
  printf(
      "  ⚠ 永远不要 printf(\"%%ld\", time_t): time_t 的底层类型是实现定义的\n");
  printf("      (Linux/macOS 64位 → long, Windows → long long, 老32位系统 → "
         "int),\n");
  printf("      直接 %%ld 在 time_t=long long 的平台上是 format mismatch → UB "
         "+ 警告。\n");
  printf("      万无一失写法: (long long)now + %%lld, 或 (intmax_t)now + "
         "PRIdMAX。\n");

  /* --- clock_t: 进程 CPU 时间 (不是墙上时间) --- */
  c0 = clock();
  for (i = 0; i < 5000000; i++) {
    sink += 1.0 / (double)(i + 1); /* 忙等微基准 */
  }
  c1 = clock();
  printf("  [clock_t]  忙等循环 = %lld ticks ≈ %.2f ms (CLOCKS_PER_SEC=%lld)\n",
         (long long)(c1 - c0),
         (double)(c1 - c0) * 1000.0 / (double)CLOCKS_PER_SEC,
         (long long)CLOCKS_PER_SEC);

  /* --- struct tm: 拆解时间 (broken-down time), localtime() 是 ISO C --- */
  tm_ptr =
      localtime(&now); /* basic/ 目录只用 ISO C, 不用 POSIX 的 localtime_r */
  if (tm_ptr != NULL) {
    printf("  [struct tm] localtime → %04d-%02d-%02d %02d:%02d:%02d\n",
           tm_ptr->tm_year + 1900, tm_ptr->tm_mon + 1, tm_ptr->tm_mday,
           tm_ptr->tm_hour, tm_ptr->tm_min, tm_ptr->tm_sec);
    printf("             tm_year=%d (要 +1900), tm_mon=%d (要 +1), tm_wday=%d "
           "(0=周日)\n",
           tm_ptr->tm_year, tm_ptr->tm_mon, tm_ptr->tm_wday);
  }

  /* --- struct timespec (C11): 秒 + 纳秒, 比 time_t 精细 --- */
  if (timespec_get(&ts, TIME_UTC) == TIME_UTC) {
    printf("  [timespec] timespec_get(TIME_UTC) → tv_sec=%lld, tv_nsec=%ld "
           "(C11)\n",
           (long long)ts.tv_sec, (long)ts.tv_nsec);
  }

  /* --- strftime: struct tm → 自定义格式字符串, 有界 (第二个参数是 buf 大小)
   * --- */
  if (tm_ptr != NULL) {
    len = strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", tm_ptr);
    if (len > 0) {
      printf("  [strftime] \"%%Y-%%m-%%d %%H:%%M:%%S\" → \"%s\" (%zu bytes)\n",
             buf, len);
    }

    /* --- asctime: struct tm → 固定格式字符串 (ISO C)。
     * 注意: 返回静态缓冲区, 非线程安全, 打印时自带 '\n' --- */
    {
      char* asc = asctime(tm_ptr);
      if (asc != NULL) {
        printf("  [asctime] %s", asc);
      }
    }
  }
  printf("\n");
}

/* 第 10 节: sizeof 汇总表 — 名称 + 大小成对, 循环打印 */
static void datatype_sizeof_table_sample(void) {
  static const struct SizeRow {
    const char* name;
    size_t size;
  } rows[] = {
      {"char", sizeof(char)},
      {"short", sizeof(short)},
      {"int", sizeof(int)},
      {"long", sizeof(long)},
      {"long long", sizeof(long long)},
      {"float", sizeof(float)},
      {"double", sizeof(double)},
      {"long double", sizeof(long double)},
      {"_Bool", sizeof(_Bool)},
      {"void*", sizeof(void*)},
      {"size_t", sizeof(size_t)},
      {"time_t", sizeof(time_t)},
  };
  size_t i;

  printf("=== 10. sizeof 汇总表 (Summary Table) ===\n\n");
  printf("  %-14s = %s\n", "type", "bytes");
  for (i = 0; i < sizeof rows / sizeof rows[0]; i++) {
    printf("  %-14s = %zu\n", rows[i].name, rows[i].size);
  }
  printf("  ← 全部单位是字节; 位宽随平台/ABI 变化 (LP64 vs LLP64 vs ILP32),\n");
  printf("     跨平台序列化请用 <stdint.h> 定宽类型, 不要用 long/size_t。\n\n");
}

/**
 * @brief Standalone datatype demo — follows hello-rust `_sample` convention.
 * Compiles into the unified `hello` binary.
 * 十个板块依次打印, 详见文件头的目录注释。
 */
int main_datatype_sample(void) {
  printf("=== 数据类型全景 (Datatype Landscape) — C17 ===\n\n");
  datatype_integer_family_sample();
  datatype_fixed_width_sample();
  datatype_char_types_sample();
  datatype_float_family_sample();
  datatype_bool_sample();
  datatype_size_types_sample();
  datatype_composites_sample();
  datatype_qualifiers_sample();
  datatype_datetime_sample();
  datatype_sizeof_table_sample();
  return 0;
}
