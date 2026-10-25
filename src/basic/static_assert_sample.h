#ifndef STATIC_ASSERT_SAMPLE_H
#define STATIC_ASSERT_SAMPLE_H

/**
 * @brief Entry of the _Static_assert compile-time invariants chapter.
 */
int main_static_assert_sample(void);

/* Non-static on purpose (repo convention, see src/advance/calc.h):
 * Unity asserts this helper directly — its clamp contract must stay true. */
int sa_clamp_u8(int v);

#endif /* STATIC_ASSERT_SAMPLE_H */
