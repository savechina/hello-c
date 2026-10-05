/**
 * @file restrict_sample.c
 * @brief restrict 指针契约 (Pointer Qualifier Contract) — Advance tutorial chapter
 *
 * 内容:
 *   1. restrict 的契约语义 (C11 §6.7.3.1): 块内对象只经该 lvalue 访问
 *   2. 合规示例: 分离缓冲区向量加法 add_arrays (编译器可自动向量化)
 *   3. 违规调用: dst 与源重叠 — 仅注释反例, 永不执行 (违反契约 = UB)
 *   4. 何时不用: 重叠语义 (memmove) 的缓冲区
 *   5. 与严格别名 (strict-aliasing) 章的关系 — 同族优化契约
 *
 * CERT: EXP43-C / DCL33-C. 测试: test/advance/test_restrict_sample.c
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "advance/restrict_sample.h"

/* ---------------------------------------------------------
   合规示例 (non-static — 单元测试直接调用)
   --------------------------------------------------------- */

void add_arrays(double *restrict dst, const double *restrict a,
                const double *restrict b, size_t n)
{
    /* restrict 契约 (C11 §6.7.3.1): 本函数执行期间, dst/a/b 各自指向的
     * 对象只能通过这三个 lvalue 访问 —— 即三者存储两两不重叠。
     * 编译器可据此假设写 dst 不影响后续读 a/b, 进而展开/向量化循环。 */
    for (size_t i = 0; i < n; i++) {
        dst[i] = a[i] + b[i];
    }
}

/* ---------------------------------------------------------
   Demo 1: 契约语义 — restrict 说了什么
   --------------------------------------------------------- */

static void restrict_contract_sample(void)
{
    printf("=== Demo 1: restrict 契约 (C11 §6.7.3.1) ===\n");
    printf("  restrict 是写给编译器的承诺书, 不是运行时检查:\n");
    printf("    「在本块执行期间, 该指针指向的对象,\n");
    printf("      只能通过**这一个** lvalue (及其算术衍生) 访问」\n");
    printf("  推论: 两个 restrict 指针指向的对象必然不重叠 (否则两条承诺互相\n");
    printf("        矛盾 → 违反契约 → UB, 标准不给任何诊断义务)。\n");
    printf("  换来的好处: 编译器可做激进优化 —— 循环展开、软件流水、SIMD 向量化。\n");
    printf("  注意: 它和 const 无关 (restrict 管别名, const 管只读), 也和\n");
    printf("        volatile 无关。\n\n");
}

/* ---------------------------------------------------------
   Demo 2: 合规调用 — 分离缓冲区向量加法
   --------------------------------------------------------- */

static void restrict_compliant_sample(void)
{
    printf("=== Demo 2: 合规调用 (三缓冲分离) ===\n");

    double dst[8];
    const double a[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    const double b[8] = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0};

    add_arrays(dst, a, b, 8);

    printf("  dst[] =");
    for (size_t i = 0; i < 8; i++) {
        printf(" %g", dst[i]);
    }
    printf("\n  期望  = 11 22 33 44 55 66 77 88 — 三个数组在栈上各自独立,\n");
    printf("        契约成立, 编译器可放心向量化此循环。\n\n");
}

/* ---------------------------------------------------------
   Demo 3: 违规调用 — 仅注释反例, 永不执行
   --------------------------------------------------------- */

static void restrict_violation_sample(void)
{
    printf("=== Demo 3: 违规调用 (仅注释反例, 永不执行) ===\n");

    /*
     * ❌ 反例 (Counter-example — 不要执行):
     *
     *     double buf[8] = {...};
     *     add_arrays(buf, buf + 1, buf, 7);   // dst 与 a、b 都重叠!
     *
     * 为什么是 UB (C11 §6.7.3.1):
     *   add_arrays 内部, dst 与 a/b 的 restrict 承诺互相矛盾 —— 同一对象
     *   在块内经两个不兼容的 restrict lvalue 访问, 契约被破坏, 行为未定义。
     * 症状为什么隐蔽:
     *   编译器可能把循环展开成 2/4/4 交错的批处理 (a[0],a[1] 先读,
     *   再写 dst[0],dst[1]), 重叠时「后面的写污染了还没读的源」——
     *   -O0 正常、-O2 结果错, 和严格别名章一个症状模式。
     */
    printf("  反例见本函数注释: add_arrays(buf, buf+1, buf, 7) 重叠 → 契约违反 → UB。\n");
    printf("  规避: 调用前保证三缓冲分离; 或换用支持重叠的接口 (见 Demo 4)。\n\n");
}

/* ---------------------------------------------------------
   Demo 4: 何时不该用 — 重叠语义 (memmove)
   --------------------------------------------------------- */

static void restrict_overlap_sample(void)
{
    printf("=== Demo 4: 何时不用 restrict — 重叠缓冲区 ===\n");

    /* 若接口语义本身就允许源与目标重叠 (滑动搬移), restrict 就是撒谎。
     * 标准库的 memmove/copy 处理重叠, memcpy 不处理 —— 选对工具:
     * 允许重叠 → memmove (或不加 restrict 的接口); 保证分离 → restrict 接口。 */
    char text[16] = "memory";
    memmove(text + 2, text, 6); /* 重叠复制: memmove 明确定义了重叠行为 */
    printf("  memmove(\"memory\"+2, \"memory\", 6) → \"%s\" (重叠, 合法)\n", text);

    printf("  判断口诀:\n");
    printf("    - 调用方**能**保证不重叠, 且要快 → restrict 接口 (add_arrays);\n");
    printf("    - 调用方**可能**重叠 → 不加 restrict 的接口 + 自己处理重叠;\n");
    printf("    - 给可能重叠的参数加 restrict = 给编译器喂假承诺 = UB。\n\n");
}

/* ---------------------------------------------------------
   Demo 5: 与严格别名章的关系 — 同族优化契约
   --------------------------------------------------------- */

static void restrict_vs_aliasing_sample(void)
{
    printf("=== Demo 5: restrict vs 严格别名 (同族契约) ===\n");
    printf("  两条契约约束的是优化器的不同权限, 违反都 = UB:\n");
    printf("  +------------------+---------------------------+-------------------------+\n");
    printf("  | 契约             | 约束什么                  | 违反时编译器可假设      |\n");
    printf("  +------------------+---------------------------+-------------------------+\n");
    printf("  | 严格别名 (§6.5)  | 访问用的**类型**兼容      | 不兼容写不碰此对象      |\n");
    printf("  | restrict (§6.7)  | 指针指向的**存储**不重叠  | 两指针访问的是不同对象  |\n");
    printf("  +------------------+---------------------------+-------------------------+\n");
    printf("  共同点: 都是「声明时承诺、违反即 UB、-O2 下以错误结果惩罚」。\n");
    printf("  详见同章 docs/src/advance/strict-aliasing.md。\n\n");
}

/* ---------------------------------------------------------
   入口函数
   --------------------------------------------------------- */

int main_restrict_sample(void)
{
    printf("========================================\n");
    printf("  restrict 指针契约 (Pointer Contract)\n");
    printf("========================================\n\n");

    restrict_contract_sample();
    restrict_compliant_sample();
    restrict_violation_sample();
    restrict_overlap_sample();
    restrict_vs_aliasing_sample();

    printf("restrict 演示完毕.\n");
    return 0;
}
