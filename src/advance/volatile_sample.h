#ifndef VOLATILE_SAMPLE_H
#define VOLATILE_SAMPLE_H

/**
 * @brief volatile 章节 (volatile — 正确用法与危险误解).
 *
 * Demonstrates:
 *   1. 误解先行: volatile 共享计数器仍是 data race / UB (C11 §5.1.2.4)
 *   2. 正确用法: volatile sig_atomic_t 信号标志 (signal/raise, 标准 C)
 *   3. 内存映射 I/O (MMIO) 占位讲解 — 本平台无真实设备
 *   4. volatile 计数循环 vs 普通循环 — 不依赖优化器行为
 *
 * CERT: EXP32-C. Test: test/advance/test_volatile_sample.c
 */
int main_volatile_sample(void);

/**
 * @brief 有界地运行 signal()/raise() 演示: 安装处理器 → 触发 → 轮询标志。
 *
 * 轮询上限固定 (1000 次) 且不 sleep, 绝不无限等待; 结束时恢复 SIG_DFL。
 *
 * @return 1 = 信号处理器已置位 volatile 标志; 0 = 安装失败或未置位
 */
int volatile_demo_poll(void);

#endif /* VOLATILE_SAMPLE_H */
