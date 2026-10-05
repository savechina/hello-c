#include <stdio.h>
#include "basic/designated_init_sample.h"

/*
 * 指定初始化 + 复合字面量 (C99 引入, C17 沿用)
 * 设计器 .member = value 让初始化自带字段名, 顺序不再承载语义。
 */

typedef struct {
    const char *name;
    int width;
    int height;
    int fps;
} DiVideo;

typedef union {
    int i;
    double d;
    char c;
} DiValue;

typedef struct {
    int w;
    int h;
} DiSize;

static int di_area(DiSize s)
{
    return s.w * s.h;
}

/* Unity 直测入口: 只指定 .fps, 其余字段必须被标准清零 (C17 6.7.9p21) */
int di_sum_of_defaults(int fps)
{
    DiVideo v = { .fps = fps };
    return v.width + v.height + v.fps;
}

static void demo_positional_fragility(void)
{
    DiVideo v = {"intro", 1920, 1080, 30};

    printf("=== ❌ 错例: 位置初始化把顺序当契约 ===\n");
    printf("  DiVideo v = {\"intro\", 1920, 1080, 30};\n");
    printf("  当前: name=%s width=%d height=%d fps=%d\n",
           v.name, v.width, v.height, v.fps);
    printf("  隐患: 若有人把字段重排为 {name, fps, width, height}...\n");
    printf("        编译器一声不吭, 1080 被当 fps, 30 被当 height\n");
    printf("        — 初始化值跟着位置走, 静默变成错数据\n\n");
}

static void demo_designated_struct(void)
{
    DiVideo v = {
        .height = 1080,
        .name = "intro",
        .fps = 60,
        .width = 1920,
    };

    printf("=== ✅ 指定初始化: 顺序无关, 自文档 ===\n");
    printf("  写的时候 .height 在前, 打印仍按字段定义顺序:\n");
    printf("  name=%s width=%d height=%d fps=%d\n\n",
           v.name, v.width, v.height, v.fps);
}

static void demo_partial_zero_fill(void)
{
    DiVideo v = { .name = "clip" };

    printf("=== 部分初始化 = 其余字段清零 ===\n");
    printf("  DiVideo v = { .name = \"clip\" };\n");
    printf("  width=%d height=%d fps=%d ← 标准保证为 0, 不是栈上垃圾\n",
           v.width, v.height, v.fps);
    printf("  di_sum_of_defaults(30) = %d ← 只指定 fps, 其余贡献 0\n\n",
           di_sum_of_defaults(30));
}

static void demo_array_designator(void)
{
    int slots[8] = { [2] = 42, [5] = 7 };
    size_t i;

    printf("=== 数组的指定初始化 ===\n");
    printf("  int slots[8] = { [2] = 42, [5] = 7 };\n");
    printf("  slots:");
    for (i = 0; i < sizeof(slots) / sizeof(slots[0]); i++) {
        printf(" [%zu]=%d", i, slots[i]);
    }
    printf("\n  未指定的下标自动为 0 — 稀疏表/查找表的正确姿势\n\n");
}

static void demo_union_active_member(void)
{
    DiValue v = { .d = 3.14 };

    printf("=== union 指定初始化: 决定激活成员 ===\n");
    printf("  DiValue v = { .d = 3.14 }; ← 激活成员是 d\n");
    printf("  v.d = %.2f\n", v.d);
    printf("  ⚠ 此刻读 v.i 是在读同一块内存的另一种解释\n");
    printf("    (实现定义, 可能是陷阱表示) — 记住激活成员是谁\n\n");
}

static void demo_compound_literal_arg(void)
{
    printf("=== 复合字面量直接当实参 ===\n");
    printf("  di_area((DiSize){ .w = 3, .h = 4 }) = %d\n",
           di_area((DiSize){ .w = 3, .h = 4 }));
    printf("  不必先造临时变量 — 一次性对象, 用完即弃 (C99 §6.5.2.5)\n\n");
}

/* ❌ 危险 (仅示意, 不编译): 块作用域的复合字面量是自动存储期,
 *   返回它的地址会悬空 (dangling), 解引用即 use-after-free:
 *       const DiSize *bad(void) { return &(DiSize){ .w = 3, .h = 4 }; }
 *   对比: 文件作用域的复合字面量天然静态存储期, 地址始终有效。
 */

static void demo_lifetime_warning(void)
{
    printf("=== 复合字面量的生命周期 ===\n");
    printf("  文件作用域: 静态存储期, 地址永久有效\n");
    printf("  块作用域 (无 static): 自动存储期, 离开所在作用域即失效\n");
    printf("  ❌ 危险写法见源码注释 — 返回块内复合字面量地址 = 悬空指针\n\n");
}

int main_designated_init_sample(void)
{
    printf("--- 指定初始化与复合字面量 (Designated Init) ---\n");
    printf("C99/C17: .member = value 让初始化自带名字, 部分初始化自动清零.\n\n");

    demo_positional_fragility();
    demo_designated_struct();
    demo_partial_zero_fill();
    demo_array_designator();
    demo_union_active_member();
    demo_compound_literal_arg();
    demo_lifetime_warning();

    printf("指定初始化演示完毕.\n");
    return 0;
}
