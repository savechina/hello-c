#ifndef INLINE_FUNCTIONS_SAMPLE_H
#define INLINE_FUNCTIONS_SAMPLE_H

/**
 * @brief Entry of the C99 inline functions chapter.
 */
int main_inline_functions_sample(void);

/* Non-static on purpose (repo convention, see src/advance/calc.h):
 * Unity tests assert these directly. */

/* Header-style static inline: lives in this .h, every including TU gets
 * its own internal-linkage copy. Proof that inline is only a hint: taking
 * the address forces the compiler to emit a real out-of-line body. */
static inline int inl_clamp(int v, int lo, int hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}

/* Returns the clamped value — computed proof the inline helper works. */
int inl_clamp_via_call(int v, int lo, int hi);

/* Proof-by-address helper: returns 1 when the direct-call result and the
 * address-taken (function pointer) result agree, 0 otherwise. */
int inl_same_result_via_address(void);

#endif /* INLINE_FUNCTIONS_SAMPLE_H */
