#include "basic/vla_flexible_sample.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * VLA 与柔性数组成员 (Variable-Length Array & Flexible Array Member)
 *
 * 错误先行: 先看注释里的错例 (大 VLA 炸栈、漏 sizeof 的 malloc),
 * 再看可运行的正确用法。n 全程夹在 [1,64], ASan 下绝对安全。
 */

/* ── VLA: 运行时确定长度的栈数组 (C99 引入, C17 保留, C23 设为可选) ── */

#define VLA_MAX_N 64

/* ❌ 错例 (仅示意, 勿模仿):
 *   size_t n = 100 * 1024 * 1024;
 *   int huge[n];              // 400MB 栈数组 → 栈溢出 (segmentation fault)
 *   int neg[-1];              // 负长度 → 约束违规, 编译不过
 *   int arr[4] = {1,2,3,4};   // ✅ 固定数组可用初始化列表
 *   int vla[n] = {1,2,3,4};   // ❌ VLA 不能带初始化列表 → 约束违规
 * 栈空间很小 (主线程通常 1-8MB, 子线程默认 512KB-2MB), 大数据一律走堆。
 */

int vla_sum_n(int n) {
  if (n < 1) {
    n = 1;
  }
  if (n > VLA_MAX_N) {
    n = VLA_MAX_N;
  }
  {
    int values[n]; /* n 是运行时值 → 真 VLA; sizeof(values) 此刻才计算 */
    int sum = 0;

    for (int i = 0; i < n; i++) {
      values[i] = i + 1;
    }
    for (int i = 0; i < n; i++) {
      sum += values[i];
    }
    return sum; /* sum = n*(n+1)/2, 返回前 values 已随栈帧销毁 */
  }
}

int vla_sizeof_n(int n) {
  if (n < 1) {
    n = 1;
  }
  if (n > VLA_MAX_N) {
    n = VLA_MAX_N;
  }
  {
    int values[n];
    return (int)sizeof(values); /* VLA 的 sizeof 在【运行时】求值 */
  }
}

static void vla_demo(void) {
  printf("=== VLA: 运行时定长栈数组 ===\n");
  printf("  vla_sum_n(10)   = %d (期望 55 = 1+2+...+10)\n", vla_sum_n(10));
  printf("  vla_sum_n(64)   = %d (期望 2080, 演示上限)\n", vla_sum_n(64));
  printf("  vla_sizeof_n(10)= %d (运行时 sizeof, 固定数组是编译期常量)\n",
         vla_sizeof_n(10));
  printf("  sizeof(int)=%zu → 10*%zu=%zu 与上面一致\n", sizeof(int),
         sizeof(int), (size_t)vla_sizeof_n(10));
  printf("  ❌ 大 VLA 直接炸栈: int huge[100*1024*1024] ≈ 400MB, 栈容不下\n");
  printf(
      "  ❌ VLA 长度必须是正的运行时值, 且不能带初始化列表 (见注释错例)\n\n");
}

/* ── 柔性数组成员: struct 末尾的零长数组, 配合一次 malloc ── */

struct IntRecord {
  int count;
  int values[]; /* 柔性数组成员: 必须是最后一个成员, 前面至少一个具名字段 */
};

/* ❌ 错例 (仅示意, 勿模仿):
 *   struct IntRecord r;              // 栈上的柔性数组成员是未完成类型,
 * 不能这样用 malloc(sizeof(struct IntRecord)); // 只分配了头部, values[0]
 * 就是堆越界! malloc(sizeof(struct IntRecord) + count); // ❌ 漏乘 sizeof(int)
 * — ASan 当场抓
 */

int fam_sum_n(int n) {
  struct IntRecord* rec;
  int sum = 0;

  if (n < 1) {
    n = 1;
  }
  if (n > VLA_MAX_N) {
    n = VLA_MAX_N;
  }

  /* 安全公式: sizeof(头部) + n * sizeof(元素) — 一次 malloc 包含全部。
   * malloc 返回值必须检查, NULL 解引用是段错误。 */
  rec = malloc(sizeof(*rec) + (size_t)n * sizeof(rec->values[0]));
  if (rec == NULL) {
    return -1;
  }

  rec->count = n;
  for (int i = 0; i < n; i++) {
    rec->values[i] = i + 1;
  }
  for (int i = 0; i < rec->count; i++) {
    sum += rec->values[i];
  }

  free(rec); /* 单次 free — 头部和 values 在同一块内存 */
  return sum;
}

static void fam_demo(void) {
  struct IntRecord probe; /* 完整类型, 栈上可声明 — 但 FAM 部分长度为 0 */
  size_t header = sizeof(struct IntRecord);

  probe.count = 0;
  printf("=== 柔性数组成员 (Flexible Array Member) ===\n");
  printf("  sizeof(struct IntRecord) = %zu (只含 count + 对齐填充)\n", header);
  printf("  offsetof(values)          = %zu (flexible array 不计入 sizeof)\n",
         offsetof(struct IntRecord, values));
  printf("  栈上 probe.count = %d — FAM 在栈对象上长度为 0, 数据须走堆\n",
         probe.count);
  printf("  fam_sum_n(10) = %d (期望 55, 一次 malloc + 一次 free)\n",
         fam_sum_n(10));
  printf(
      "  ❌ 漏乘 sizeof 的 malloc 只给头部字节数, 写 values[0] 即堆溢出\n\n");
}

/* ── public entry ── */

int main_vla_flexible_sample(void) {
  printf("--- VLA 与柔性数组成员 (VLA & Flexible Array Members) ---\n");
  printf("VLA: 栈上运行时定长, 小而短; FAM: 堆上一次 malloc, 头尾可变长。\n\n");

  vla_demo();
  fam_demo();

  printf("VLA 与柔性数组成员演示完毕.\n");
  return 0;
}
