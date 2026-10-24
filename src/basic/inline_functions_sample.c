#include <stdio.h>
#include "basic/inline_functions_sample.h"

/*
 * 内联函数 (inline) (C99 引入, C17 §6.7.4 沿用)
 *
 * 错误先行: 先看注释里把 inline 当"保证"的两个错例,
 * 再看可运行的正确用法 — static inline 头文件模式 + 取地址证明。
 */

/* inline 是【提示】不是【命令】:
 *   - 编译器可以忽略它 (典型: -O0, 或函数体太大)
 *   - 编译器也可以不写 inline 就内联 (优化器/ LTO 自行决定)
 * 取了地址的函数一定会有一份真实体 (out-of-line copy), 否则 &f 无处可指。 */

/* ❌ 错例 1 (仅示意, 不编译): C99 的"外部链接 inline"陷阱
 *   // TU-A: inline int f(void) { return 1; }
 *   // TU-B: int main(void) { return f(); }   // 链接错误: undefined f
 * C99 规定: 不带 static/extern 的 inline 定义只是"inline 定义",
 * 不提供外部定义; 其他编译单元要用, 必须有一个 TU 写 extern inline。
 * 单文件教程里一律用 static inline, 规避这个坑。
 *
 * ❌ 错例 2: 以为写了 inline 就一定没有函数调用
 *   static inline int big(int x);  // 优化器拒绝内联时, 照样产生 call 指令
 */

int inl_clamp_via_call(int v, int lo, int hi)
{
    /* 直接调用: 编译器【可以】把这里替换成内联代码 */
    return inl_clamp(v, lo, hi);
}

int inl_same_result_via_address(void)
{
    int (*fp)(int, int, int) = inl_clamp; /* 取地址 → 编译器必须生成真实函数体 */
    int direct = inl_clamp(5, 0, 10);
    int via_addr = fp(5, 0, 10);

    /* 两种调用路径算的是同一个函数, 结果必然一致 — 这就是"提示"的边界:
     * inline 影响【是否生成调用开销】, 从不改变【语义和结果】。 */
    return direct == via_addr ? 1 : 0;
}

static void inline_demo(void)
{
    int (*fp)(int, int, int) = inl_clamp;

    printf("=== inline: 提示不是命令 ===\n");
    printf("  inl_clamp_via_call(5, 0, 10)     = %d\n", inl_clamp_via_call(5, 0, 10));
    printf("  inl_clamp_via_call(-3, 0, 10)    = %d\n", inl_clamp_via_call(-3, 0, 10));
    printf("  inl_clamp_via_call(99, 0, 10)    = %d\n", inl_clamp_via_call(99, 0, 10));
    printf("  inl_same_result_via_address()    = %d (1=直接调用与取地址调用一致)\n",
           inl_same_result_via_address());
    printf("  &inl_clamp = %p ← 取地址迫使编译器生成 out-of-line 函数体\n",
           (void *)fp);
    printf("  ❌ 把 inline 当保证 / 跨 TU 的外部链接 inline, 见注释错例\n\n");
}

/* ── public entry ── */

int main_inline_functions_sample(void)
{
    printf("--- 内联函数 (Inline Functions) ---\n");
    printf("C99 §6.7.4: inline 是优化提示; static inline 是头文件惯用法。\n\n");

    inline_demo();

    printf("内联函数演示完毕.\n");
    return 0;
}
