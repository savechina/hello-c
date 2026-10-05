/**
 * @file strict_aliasing_sample.c
 * @brief 严格别名 (Strict Aliasing)、有效类型 (Effective Type) 与合法类型双关
 *
 * 内容:
 *   1. 有效类型规则 (malloc 内存经 lvalue 写入后获得有效类型, C11 §6.5 / §6.2.6.1)
 *   2. 合法双关: memcpy 逐字节搬运 (uint32_t ↔ uint8_t[4] / float)
 *   3. 非法双关: 指针强制转换 — 仅注释反例, 永不执行
 *   4. union 双关: 实现定义 (C11 §6.5.2.3 脚注) — 讲解但不依赖
 *   5. -O2 为何会误编译别名违规 (TBAA / load hoisting)
 *
 * CERT: EXP39-C (类型双关). 测试: test/advance/test_strict_aliasing_sample.c
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "advance/strict_aliasing_sample.h"

/* ---------------------------------------------------------
   合法双关辅助函数 (non-static — 单元测试直接调用)
   --------------------------------------------------------- */

void pun_u32_to_bytes(uint32_t v, uint8_t out[4])
{
    /* memcpy 不违反别名规则: 逐字符访问是 §6.5p7 明文许可的例外 */
    memcpy(out, &v, sizeof v);
}

uint32_t pun_bytes_to_u32(const uint8_t in[4])
{
    uint32_t v = 0;
    memcpy(&v, in, sizeof v);
    return v;
}

/* ---------------------------------------------------------
   Demo 1: 有效类型 (Effective Type)
   --------------------------------------------------------- */

static void strict_aliasing_effective_type_sample(void)
{
    printf("=== Demo 1: 有效类型 (Effective Type) ===\n");

    /* malloc 出来的块没有声明类型 (no declared type) — §6.5p6 的适用前提 */
    int *slot = malloc(sizeof *slot);
    if (slot == NULL) {
        printf("  malloc 失败, 跳过演示\n\n");
        return;
    }

    *slot = 42; /* 写入后, 该对象的有效类型变为 int — 读 int 也合法 */
    printf("  *slot = %d (写入后有效类型 = int)\n", *slot);
    free(slot);

    printf("  规则 (C11 §6.5p6): 无声明类型的对象被某类型 lvalue 写入后,\n");
    printf("    该 lvalue 的类型成为对象的有效类型 (effective type)。\n");
    printf("  规则 (C11 §6.5p7): 通过不兼容类型的 lvalue 访问 → UB;\n");
    printf("    例外 1: 字符类型 (char/unsigned char) 访问永远合法;\n");
    printf("    例外 2: union 成员访问见 §6.5.2.3 (实现定义, Demo 4 讲解)。\n");
    printf("  表示层面另有 §6.2.6.1 (对象表示/位型) 约束整数的位级布局。\n\n");
}

/* ---------------------------------------------------------
   Demo 2: 合法双关 — memcpy 逐字节搬运
   --------------------------------------------------------- */

static void strict_aliasing_legal_pun_sample(void)
{
    printf("=== Demo 2: 合法双关 (memcpy) ===\n");

    uint32_t magic = 0x01020304u;
    uint8_t bytes[4];
    pun_u32_to_bytes(magic, bytes);
    printf("  uint32 0x%" PRIX32 " → bytes: %02X %02X %02X %02X\n",
           magic, bytes[0], bytes[1], bytes[2], bytes[3]);
    printf("  回读: 0x%" PRIX32 " (roundtrip 一致)\n",
           pun_bytes_to_u32(bytes));

    /* 逐字节观察原对象 — 字符类型访问, §6.5p7 永久豁免, 与端序无关 */
    const unsigned char *raw = (const unsigned char *)&magic;
    printf("  char* 逐字节观察 uint32: %02X %02X %02X %02X (宿主端序)\n",
           raw[0], raw[1], raw[2], raw[3]);

    /* uint32 位模式 ↔ float: 只用 IEEE-754 有定义的位型, 不造野值 */
    if (sizeof(float) == sizeof(uint32_t)) {
        uint32_t bits = 0x3F800000u; /* IEEE-754 中 1.0f 的编码 */
        float f = 0.0f;
        memcpy(&f, &bits, sizeof f); /* 合法: 搬运的是表示, 不经不兼容 lvalue 读 */
        printf("  bits 0x3F800000 → float = %g\n", (double)f);

        float g = 2.0f;
        uint32_t gbits = 0;
        memcpy(&gbits, &g, sizeof gbits); /* 反向同理: 把 float 的位型拿出来 */
        printf("  float 2.0f 的位型 = 0x%" PRIX32 " (IEEE-754)\n", gbits);
    } else {
        printf("  sizeof(float) != 4, 跳过 float 双关演示\n");
    }
    printf("\n");
}

/* ---------------------------------------------------------
   Demo 3: 非法双关 — 指针强制转换 (仅注释, 永不执行)
   --------------------------------------------------------- */

static void strict_aliasing_illegal_pun_sample(void)
{
    printf("=== Demo 3: 非法双关 (仅注释反例, 永不执行) ===\n");

    /*
     * ❌ 反例 (Counter-example — 不要执行):
     *
     *     uint32_t bits = 0x3F800000u;
     *     float *pf = (float *)&bits;    // 指针强制转换
     *     if (*pf == 1.0f) { ... }       // 经 float lvalue 读 int 对象
     *
     * 为什么是 UB (C11 §6.5p7): float 与 uint32_t 不兼容 (incompatible),
     * 通过 float lvalue 访问 uint32_t 有效类型的对象 → 未定义行为。
     * -fstrict-aliasing (-O2 默认开启) 让编译器可以假设这段代码不存在。
     */
    printf("  反例见本函数注释: (float *)&uint32_t 变量再解引用 = UB (§6.5p7)。\n");
    printf("  规避: 位模式搬运用 memcpy —— 就是 Demo 2 的做法。\n\n");
}

/* ---------------------------------------------------------
   Demo 4: union 双关 — 实现定义, 讲解但不依赖
   --------------------------------------------------------- */

static void strict_aliasing_union_sample(void)
{
    printf("=== Demo 4: union 双关 — 实现定义, 不依赖 ===\n");

    /*
     * ❌ 不依赖 (讲解用, 本章不执行):
     *
     *     union { uint32_t u; float f; } pun;
     *     pun.u = 0x3F800000u;          // 写入成员 u
     *     printf("%f", pun.f);          // 改经成员 f 读出 —— 换成员了
     *
     * C11 §6.5.2.3 脚注: 读取所用成员与上次写入的成员不同时, 行为按
     * 「对值的表示是合适的 (appropriate for the representation)」处理 ——
     * 即实现定义 (implementation-defined), 标准不强制统一结果。
     * GCC/Clang 明确定义了该行为, 但可移植代码不应依赖它。
     */
    printf("  反例见本函数注释: union 换成员读写是实现定义 (§6.5.2.3 脚注)。\n");
    printf("  可移植代码的唯一选择仍是 memcpy (Demo 2)。\n\n");
}

/* ---------------------------------------------------------
   Demo 5: -O2 与别名违规 — 编译器如何「合法地」算错
   --------------------------------------------------------- */

static void strict_aliasing_miscompile_sample(void)
{
    printf("=== Demo 5: -O2 与别名违规 ===\n");
    printf("  设违规代码: p 是 uint32_t*, q 实际指向同一个对象 (float*)\n");
    printf("    x = *p;      // 第一次读\n");
    printf("    *q = 1.0f;   // float 写\n");
    printf("    y = *p;      // 第二次读\n");
    printf("  基于类型的别名分析 (TBAA) 允许编译器推导:\n");
    printf("    「float* 的写不可能改变 uint32_t 对象」→ 第二次读可复用第一次\n");
    printf("    伪代码: r1 = load p; store q; y = r1;   (load 被提升/消除)\n");
    printf("  若 q 真的指向 p 的存储, y 就读到陈旧值 —— 源码语义被优化破坏。\n");
    printf("  这不是编译器 bug: 违规代码是 UB, 标准把裁量权交给了编译器。\n");
    printf("  规避: 别名不兼容的访问一律走 memcpy / 字符类型 / 正确声明类型。\n\n");
}

/* ---------------------------------------------------------
   入口函数
   --------------------------------------------------------- */

int main_strict_aliasing_sample(void)
{
    printf("========================================\n");
    printf("  严格别名与有效类型 (Strict Aliasing)\n");
    printf("========================================\n\n");

    strict_aliasing_effective_type_sample();
    strict_aliasing_legal_pun_sample();
    strict_aliasing_illegal_pun_sample();
    strict_aliasing_union_sample();
    strict_aliasing_miscompile_sample();

    printf("strict-aliasing 演示完毕.\n");
    return 0;
}
