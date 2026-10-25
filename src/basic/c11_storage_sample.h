#ifndef C11_STORAGE_SAMPLE_H
 #define C11_STORAGE_SAMPLE_H

/**
 * @brief Entry of the C11 storage-class specifier chapter
 *        (_Alignas / _Noreturn / _Thread_local).
 */
int main_c11_storage_sample(void);

/* Non-static on purpose (repo convention, see src/advance/calc.h):
 * Unity tests assert these directly. */

/* Returns the address (mod 64) of the _Alignas(64) demo buffer.
 * Must be 0 — _Alignas promises the stronger alignment. */
int c11s_cache_line_mod64(void);

/* _Alignof queries — cross-ref: struct_memory_layout chapter owns the deep dive */
int c11s_alignof_int32(void);
int c11s_alignof_int64(void);

/* Runs n pthread workers; each computes sum(1..k) in its OWN
 * _Thread_local counter (k = worker index + 1) and exits with the value.
 * Returns the total of all thread-local results, or -1 on thread error. */
int c11s_tls_thread_sum(int n_threads);

/* Same _Noreturn plumbing, but the worker body is a _Noreturn function
 * that terminates via pthread_exit — computed sum comes back joined. */
int c11s_noreturn_thread_sum(int k);

#endif /* C11_STORAGE_SAMPLE_H */
