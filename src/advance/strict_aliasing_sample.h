#ifndef STRICT_ALIASING_SAMPLE_H
#define STRICT_ALIASING_SAMPLE_H

#include <stdint.h>

/**
 * @brief 严格别名 (Strict Aliasing)、有效类型与合法类型双关章节.
 *
 * Demonstrates:
 *   1. 有效类型规则 (effective type, C11 §6.5 / §6.2.6.1)
 *   2. 合法双关: memcpy 逐字节搬运 (uint32_t ↔ uint8_t[4] / float)
 *   3. 非法双关: 指针强制转换 — 仅注释反例, 永不执行
 *   4. union 双关: 实现定义 (C11 §6.5.2.3 脚注) — 讲解但不依赖
 *   5. -O2 为何会误编译别名违规 (TBAA, load hoisting)
 *
 * CERT: EXP39-C. 测试: test/advance/test_strict_aliasing_sample.c
 */
int main_strict_aliasing_sample(void);

/**
 * @brief 把 uint32_t 的内存表示按原样复制到 4 字节 (合法双关, 非 static 供测试).
 *
 * @param v   待拆解的 32 位值
 * @param out 4 字节输出缓冲 (宿主端序, 不做端序转换)
 */
void pun_u32_to_bytes(uint32_t v, uint8_t out[4]);

/**
 * @brief 从 4 字节还原 uint32_t — pun_u32_to_bytes 的逆操作.
 *
 * @param in 4 字节输入缓冲 (宿主端序)
 * @return 还原后的 32 位值
 */
uint32_t pun_bytes_to_u32(const uint8_t in[4]);

#endif /* STRICT_ALIASING_SAMPLE_H */
