#ifndef RESTRICT_SAMPLE_H
#define RESTRICT_SAMPLE_H

#include <stddef.h>

/**
 * @brief restrict 指针契约章节 (Pointer Qualifier Contract).
 *
 * Demonstrates:
 *   1. restrict 的契约语义 (C11 §6.7.3.1): 块内对象只经该 lvalue 访问
 *   2. 合规示例: 分离缓冲区向量加法 add_arrays (编译器可自动向量化)
 *   3. 违规调用: dst 与源重叠 — 仅注释反例, 永不执行 (违反契约 = UB)
 *   4. 何时不用: 重叠语义 (memmove) 的缓冲区
 *   5. 与严格别名 (strict-aliasing) 章的关系 — 同族优化契约
 *
 * CERT: EXP43-C / DCL33-C. 测试: test/advance/test_restrict_sample.c
 */
int main_restrict_sample(void);

/**
 * @brief 逐元素向量加法: dst[i] = a[i] + b[i], 三缓冲互不重叠 (restrict 契约).
 *
 * 实际定义 (src/advance/restrict_sample.c) 把三个指针参数声明为
 * restrict 限定 —— 本头文件给出不带限定的兼容声明 (C11 §6.7.6.3p15:
 * 限定与无限定参数版本在类型判定中视为同一类型), 使纯 C++ 解析器
 * (如无编译数据库的 clangd) 也能读入此头文件。
 *
 * 调用方保证 dst、a、b 指向的存储两两不重叠; 违反则行为未定义 (UB)。
 *
 * @param dst 输出缓冲 (不得与 a/b 重叠)
 * @param a   加数缓冲 (不得与 dst/b 重叠)
 * @param b   加数缓冲 (不得与 dst/a 重叠)
 * @param n   元素个数 (0 合法, 任何指针此时都不得解引用)
 */
void add_arrays(double *dst, const double *a, const double *b, size_t n);

#endif /* RESTRICT_SAMPLE_H */
