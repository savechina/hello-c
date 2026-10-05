#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "basic/static_assert_sample.h"

/* 特性门: _Static_assert 是 C11 (201112L) 关键字, 晚于 C11 的翻译单元才有 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
/* 本项目 -std=c17, 门常开 */
#else
#error "_Static_assert requires C11 or later (use -std=c11 / -std=c17)"
#endif

/* ---- 文件作用域断言: 把平台假设写成编译期契约 ---- */

_Static_assert(sizeof(int) >= 4, "int 至少 4 字节 — 本教程整型演示的前提");
_Static_assert(sizeof(int32_t) == 4, "int32_t 必须恰好 4 字节 (stdint.h 的承诺)");
_Static_assert(sizeof(long) >= 4, "long 至少 4 字节 (ILP32 与 LP64 都满足)");

/* ❌ 错例 (取消注释即编译失败, 请勿启用):
 * _Static_assert(sizeof(long) == 4, "错误假设: long 恒为 4 字节");
 * macOS/Linux 64 位是 LP64, long = 8 字节 → static assertion failed。
 * 教训: 断言写下的是你的假设; 平台不满足时立刻编译报错,
 * 而不是带着错误假设编译通过、在运行期静默出错。
 * 正确姿势: 要么改用 stdint.h 定宽类型, 要么断言跨平台都成立的下界 (>= 4)。
 */

typedef struct {
    int32_t x;
    int32_t y;
} SaPoint;

_Static_assert(sizeof(SaPoint) == 2 * sizeof(int32_t), "两个 int32_t 成员之间不应有填充");
_Static_assert(offsetof(SaPoint, y) == sizeof(int32_t), "y 紧跟 x, 偏移量等于 x 的宽度");

/* <assert.h> 在 C11 后定义 static_assert 宏 — 与关键字 _Static_assert 同义,
 * 给从 C++ 或旧代码迁移来的读者一条熟悉的拼写 */
static_assert(sizeof(int32_t) == 4, "assert.h 宏版: 同一条断言, 两种拼写");

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const int sa_probe[5] = {10, 20, 30, 40, 50};

_Static_assert(ARRAY_SIZE(sa_probe) == 5, "增删数组元素时, 编译期提醒同步更新假设");
/* ARRAY_SIZE 只对真数组成立: 传入指针会衰变成 sizeof(T*)/sizeof(T),
 * 编译器不报错但结果与元素个数无关 — 用它之前先确认拿到的是数组 */

int sa_clamp_u8(int v)
{
    /* 块作用域断言: 把本函数的隐含假设钉在使用点旁边 */
    _Static_assert(255 <= INT_MAX, "u8 上界必须放得进 int, 否则比较本身会溢出");

    if (v < 0) {
        return 0;
    }
    if (v > 255) {
        return 255;
    }
    return v;
}

static void demo_sizeof_guarantees(void)
{
    printf("=== sizeof 保证 (文件作用域断言) ===\n");
    printf("  sizeof(int)      = %zu  (断言 >= 4)\n", sizeof(int));
    printf("  sizeof(int32_t)  = %zu  (断言 == 4)\n", sizeof(int32_t));
    printf("  sizeof(long)     = %zu  (断言 >= 4)\n", sizeof(long));
    printf("  这些数字编译器已经核对过 — 断言失败根本走不到运行期\n\n");
}

static void demo_struct_layout(void)
{
    SaPoint p = {1, 2};

    printf("=== 结构体布局断言 ===\n");
    printf("  sizeof(SaPoint) = %zu  (断言 == 2 * sizeof(int32_t))\n", sizeof(SaPoint));
    printf("  offsetof(y)     = %zu  (断言 == sizeof(int32_t))\n", offsetof(SaPoint, y));
    printf("  p = {x=%d, y=%d} — 序列化/网络协议依赖这种布局假设\n\n", p.x, p.y);
}

static void demo_array_size_guard(void)
{
    int copy[ARRAY_SIZE(sa_probe)];
    size_t i;

    printf("=== ARRAY_SIZE 模式 ===\n");
    for (i = 0; i < ARRAY_SIZE(sa_probe); i++) {
        copy[i] = sa_probe[i];
    }
    printf("  ARRAY_SIZE(sa_probe) = %zu (断言 == 5)\n", ARRAY_SIZE(sa_probe));
    printf("  copy[0]=%d, copy[4]=%d — 长度与遍历用同一个宏, 增删元素零成本\n\n",
           copy[0], copy[4]);
}

static void demo_runtime_confirmation(void)
{
    int v;

    printf("=== 运行期确认 + 块作用域断言 ===\n");
    printf("  __STDC_VERSION__ = %ldL (特性门: >= 201112L)\n", (long)__STDC_VERSION__);
    v = sa_clamp_u8(999);
    printf("  sa_clamp_u8(999) = %d\n", v);
    v = sa_clamp_u8(-5);
    printf("  sa_clamp_u8(-5)  = %d\n", v);
    v = sa_clamp_u8(42);
    printf("  sa_clamp_u8(42)  = %d\n", v);
    /* 分工: 运行期 assert 查逻辑 (可被 NDEBUG 裁掉), _Static_assert 查平台事实 (永远在) */
    assert(sa_clamp_u8(999) == 255);
    printf("  assert(sa_clamp_u8(999) == 255) → 通过 (运行期检查)\n");
    printf("  对照: <assert.h> 在 C11 后也提供 static_assert 宏名, 语义相同\n\n");
}

int main_static_assert_sample(void)
{
    printf("--- 编译期断言 (_Static_assert) ---\n");
    printf("C11 关键字: 条件为假时拒绝编译, 把假设变成契约.\n\n");

    demo_sizeof_guarantees();
    demo_struct_layout();
    demo_array_size_guard();
    demo_runtime_confirmation();

    printf("_Static_assert 演示完毕.\n");
    return 0;
}
