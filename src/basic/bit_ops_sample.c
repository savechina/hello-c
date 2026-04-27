#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <strings.h>
#include "basic/bit_ops_sample.h"

/*
 * 位运算与内存操作演示 (Bitwise Operations & Memory Ops)
 *
 * 本节演示 C 语言中的位运算和内存操作：
 *  - 基本位运算: AND/OR/XOR/NOT
 *  - 移位运算: 左移/右移
 *  - Bitmask: 设置/清除/翻转/检查位
 *  - Struct bit fields
 *  - memcpy/memmove/memset
 *  - Endianness 检测
 *  - 实用模式: 标志位、权限系统、字节打包
 */

/* ---- Section 1: 基本位运算 AND/OR/XOR/NOT ---- */

static uint32_t print_bits8(uint8_t v)
{
    for (int32_t k = 7; k >= 0; k--) {
        printf("%d", (v >> k) & 1);
    }
    return 0;
}

static uint32_t print_bits32(uint32_t v)
{
    for (int32_t k = 31; k >= 0; k--) {
        printf("%d", (v >> k) & 1);
    }
    return 0;
}

static void demo_basic_bitwise(void)
{
    uint8_t a = 0b11001010;
    uint8_t b = 0b10100101;

    printf("=== 基本位运算 ===\n");
    printf("  a = "); print_bits8(a); printf("  (0x%02" PRIx8 ")\n", a);
    printf("  b = "); print_bits8(b); printf("  (0x%02" PRIx8 ")\n", b);
    printf("\n");
    printf("  a & b = "); print_bits8(a & b); printf("  (AND: 同 1 为 1)\n");
    printf("  a | b = "); print_bits8(a | b); printf("  (OR:  有 1 为 1)\n");
    printf("  a ^ b = "); print_bits8(a ^ b); printf("  (XOR: 不同为 1)\n");
    printf("  ~a    = "); print_bits8((uint8_t)~a); printf("  (NOT: 逐位取反)\n");
    printf("\n");
}

/* ---- Section 2: 移位运算与溢出错误 ---- */

static void demo_shifts(void)
{
    uint32_t val = 0x00000001;

    printf("=== 移位运算 ===\n");
    printf("  原始: "); print_bits32(val); printf("  (0x%08" PRIx32 ")\n", val);

    val = val << 8;
    printf("  << 8: "); print_bits32(val); printf("  (0x%08" PRIx32 ")\n", val);

    val = val << 8;
    printf("  << 8: "); print_bits32(val); printf("  (0x%08" PRIx32 ")\n", val);

    val = 0xFF000000;
    printf("  >> 8: "); print_bits32(val >> 8); printf(" (0x%08" PRIx32 ")\n", val >> 8);

    printf("\n");
    printf("  ❌ 错误: 移位位数 >= 位宽 = 未定义行为 (UB)\n");
    printf("    uint32_t x = 1; x << 32;  // UB! 结果未定义\n");
    printf("    uint32_t y = 1; x << 33;  // UB! 结果未定义\n");
    printf("\n");
    printf("  ✅ 修复: 移位前检查边界\n");
    uint32_t x = 1;
    int32_t shift = 32;
    if (shift >= 0 && shift < 32) {
        printf("    x << %d = 0x%08" PRIx32 "\n", shift, x << shift);
    } else {
        printf("    shift=%" PRId32 " 超出范围 [0..31], 跳过\n", shift);
    }
    printf("\n");
}

/* ---- Section 3-10: 位运算与权限/内存操作 ---- */

#define FLAG_READ    (1u << 0)
#define FLAG_WRITE   (1u << 1)
#define FLAG_EXECUTE (1u << 2)
#define FLAG_DELETE  (1u << 3)

static void demo_bitmask_patterns(void)
{
    uint32_t flags = 0;

    printf("=== Bitmask 模式 (权限系统) ===\n");
    printf("  初始标志: "); print_bits32(flags);
    printf("  (无权限)\n");

    flags |= FLAG_READ;
    printf("  添加 READ:        "); print_bits32(flags); printf("\n");

    flags |= FLAG_WRITE;
    printf("  添加 WRITE:       "); print_bits32(flags); printf("\n");

    flags &= ~FLAG_READ;
    printf("  清除 READ:        "); print_bits32(flags); printf("\n");

    flags ^= FLAG_EXECUTE;
    printf("  翻转 EXECUTE:     "); print_bits32(flags); printf("\n");

    if (flags & FLAG_WRITE) {
        printf("  ✅ WRITE 已设置\n");
    }
    if (!(flags & FLAG_READ)) {
        printf("  ❌ READ 未设置\n");
    }

    flags &= ~0u;
    printf("  全部清除:         "); print_bits32(flags); printf("\n\n");
}

/* ---- Section 4: Struct bit fields ---- */

struct Permissions {
    uint32_t can_read    : 1;
    uint32_t can_write   : 1;
    uint32_t can_execute : 1;
    uint32_t can_delete  : 1;
    uint32_t reserved    : 4;
    uint32_t user_id     : 16;
};

static void demo_bit_fields(void)
{
    printf("=== Struct bit fields ===\n");

    struct Permissions p;
    p.can_read    = 1;
    p.can_write   = 0;
    p.can_execute = 1;
    p.can_delete  = 0;
    p.reserved    = 0;
    p.user_id     = 42;

    printf("  sizeof(struct Permissions) = %zu bytes\n", sizeof(p));
    printf("  can_read    = %u\n", p.can_read);
    printf("  can_write   = %u\n", p.can_write);
    printf("  can_execute = %u\n", p.can_execute);
    printf("  can_delete  = %u\n", p.can_delete);
    printf("  user_id     = %u\n", p.user_id);
    printf("\n");
}

/* ---- Section 5: memcpy / memmove / memset ---- */

static void demo_memory_ops(void)
{
    uint8_t src[8]  = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    uint8_t dst[8]  = {0};
    uint8_t overlap[8];

    printf("=== memcpy / memmove / memset ===\n");

    memcpy(dst, src, 8);
    printf("  memcpy:  ");
    for (int32_t k = 0; k < 8; k++) printf("0x%02" PRIx8 " ", dst[k]);
    printf("\n");

    memset(dst, 0, 8);

    memcpy(overlap, src, 8);
    memmove(overlap + 2, overlap, 6);
    printf("  memmove (overlap shift): ");
    for (int32_t k = 0; k < 8; k++) printf("0x%02" PRIx8 " ", overlap[k]);
    printf("\n");

    memset(dst, 0xFF, 8);
    printf("  memset 0xFF: ");
    for (int32_t k = 0; k < 8; k++) printf("0x%02" PRIx8 " ", dst[k]);
    printf("\n");
    printf("\n");
}

/* ---- Section 6: Endianness 检测 ---- */

static void demo_endianness(void)
{
    uint32_t val = 0x01020304;
    uint8_t *bytes = (uint8_t *)&val;

    printf("=== Endianness (字节序) ===\n");
    printf("  uint32_t = 0x01020304\n");
    printf("  内存中字节顺序: ");
    for (int32_t k = 0; k < 4; k++) printf("[%d]=0x%02" PRIx8 " ", k, bytes[k]);
    printf("\n");

    if (bytes[0] == 0x04) {
        printf("  → Little Endian (低位字节在前)\n");
    } else {
        printf("  → Big Endian (高位字节在前)\n");
    }

    printf("\n  ASCII 示意图: 0x01020304 在内存中\n");
    printf("  Little Endian:  [04] [03] [02] [01]\n");
    printf("  Big Endian:     [01] [02] [03] [04]\n");
    printf("\n");
}

/* ---- Section 7: 字节打包与解包 ---- */

static uint32_t pack_bytes(uint8_t b3, uint8_t b2, uint8_t b1, uint8_t b0)
{
    return ((uint32_t)b3 << 24) |
           ((uint32_t)b2 << 16) |
           ((uint32_t)b1 <<  8) |
           (uint32_t)b0;
}

static void unpack_bytes(uint32_t val, uint8_t *out)
{
    out[0] = (uint8_t)val;
    out[1] = (uint8_t)(val >> 8);
    out[2] = (uint8_t)(val >> 16);
    out[3] = (uint8_t)(val >> 24);
}

static void demo_pack_unpack(void)
{
    printf("=== 字节打包/解包 ===\n");

    uint32_t packed = pack_bytes(0xDE, 0xAD, 0xBE, 0xEF);
    printf("  pack(0xDE, 0xAD, 0xBE, 0xEF) = 0x%08" PRIx32 "\n", packed);

    uint8_t unpacked[4];
    unpack_bytes(packed, unpacked);
    printf("  unpack = [0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 ", 0x%02" PRIx8 "]\n",
           unpacked[0], unpacked[1], unpacked[2], unpacked[3]);
    printf("\n");
}

/* ---- Section 8: 实用 bitmask 模式 ---- */

static void demo_practical_bitmasks(void)
{
    printf("=== 实用 bitmask 模式 ===\n");

    uint8_t color = 0;

    color |= (1u << 0) | (1u << 2);
    printf("  设置 bit 0 和 2: "); print_bits8(color); printf("\n");

    color &= ~(1u << 0);
    printf("  清除 bit 0:       "); print_bits8(color); printf("\n");

    color ^= (1u << 0) | (1u << 1);
    printf("  翻转 bit 0 和 1:  "); print_bits8(color); printf("\n");

    uint32_t mask = 0b11110000000000000000000000000000u;
    uint32_t bits32 = 0b10100000000000000000000000000000u;
    uint32_t result = bits32 & mask;
    printf("  高4位掩码提取:     "); print_bits32(bits32); printf("\n");
    printf("                   & "); print_bits32(mask); printf("\n");
    printf("                   = "); print_bits32(result); printf("\n");
    printf("\n");
}

/* ---- Section 9: Python int.bit_length() vs C 位操作 ---- */

static void demo_vs_python_bitlength(void)
{
    uint32_t val = 1023;

    printf("=== Python int.bit_length() vs C 位操作 ===\n");
    printf("\n");
    printf("  Python:\n");
    printf("    n = 1023\n");
    printf("    n.bit_length()  # → 10 (需要 10 位表示)\n");
    printf("\n");
    printf("  C (手动计算):\n");

    uint32_t temp = val;
    int32_t bits = 0;
    while (temp > 0) {
        temp >>= 1;
        bits++;
    }
    printf("    1023 需要 %" PRId32 " 位\n", bits);

    printf("\n  Python:          bin(1023)  # '0b1111111111'\n");
    printf("  C (打印二进制):   ");
    print_bits32(val);
    printf("\n");
    printf("\n");
}

/* ---- Section 10: ASCII 位图演示 ---- */

static void demo_ascii_bit_diagram(void)
{
    printf("=== ASCII 位图演示 ===\n");
    printf("\n");
    printf("  位运算 AND (0b1100 & 0b1010 = 0b1000):\n");
    printf("    bit 3 2 1 0\n");
    printf("    a =   1 1 0 0\n");
    printf("    b =   1 0 1 0\n");
    printf("    ----------\n");
    printf("    a&b = 1 0 0 0   ← 仅 bit 3 两者都为 1\n");
    printf("\n");
    printf("  位运算 XOR (0b1100 ^ 0b1010 = 0b0110):\n");
    printf("    bit 3 2 1 0\n");
    printf("    a =   1 1 0 0\n");
    printf("    b =   1 0 1 0\n");
    printf("    ----------\n");
    printf("    a^b = 0 1 1 0   ← bit 不同的位置为 1\n");
    printf("\n");
}

/* ================================================================
 * Coordinator entry
 * ================================================================ */

int main_bit_ops_sample(void)
{
    printf("========================================\n");
    printf("  位运算与内存操作\n");
    printf("========================================\n\n");

    demo_basic_bitwise();
    demo_shifts();
    demo_bitmask_patterns();
    demo_bit_fields();
    demo_memory_ops();
    demo_endianness();
    demo_pack_unpack();
    demo_practical_bitmasks();
    demo_vs_python_bitlength();
    demo_ascii_bit_diagram();

    printf("位运算与内存操作演示完毕。\n");
    return 0;
}
