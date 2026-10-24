#ifndef VLA_FLEXIBLE_SAMPLE_H
#define VLA_FLEXIBLE_SAMPLE_H

#include <stddef.h>

/**
 * @brief Entry of the VLA + flexible array member chapter.
 */
int main_vla_flexible_sample(void);

/* Non-static on purpose (repo convention, see src/advance/calc.h):
 * Unity tests assert these directly. */

/* Builds a runtime-sized VLA (n clamped to [1,64], ASan-safe) and
 * returns the sum of its elements — proof the size was truly dynamic. */
int vla_sum_n(int n);

/* Returns sizeof(the VLA) — evaluated at RUNTIME, unlike fixed arrays. */
int vla_sizeof_n(int n);

/* Heap record with a trailing flexible array member: one malloc for
 * header + payload. Returns the sum, or -1 on allocation failure. */
int fam_sum_n(int n);

#endif /* VLA_FLEXIBLE_SAMPLE_H */
