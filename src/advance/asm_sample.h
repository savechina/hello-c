#ifndef ASM_SAMPLE_H
#define ASM_SAMPLE_H

/**
 * @brief 内联汇编 (Inline Assembly)
 *
 * GCC/Clang 扩展 `__asm__` (extended asm) 语法:
 *   1. 语法解剖 + 约束速查表 — 输出/输入/ clobber 如何与编译器对话
 *   2. 基础示例: 用汇编做加法 (x86_64 `addl` / AArch64 `add` 条件编译)
 *   3. volatile 限定符 — 防优化的正确姿势与经典陷阱
 *   4. 架构示例: x86 `cpuid` 读 CPU 能力 / AArch64 `mrs` 读计时器
 *      (其它平台走 #else 兜底, 普通 C 实现)
 *
 * Called from main_advance() → advance.c → hello.c → main.c
 */
int main_asm_sample(void);

#endif /* ASM_SAMPLE_H */
