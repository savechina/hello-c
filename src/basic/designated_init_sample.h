#ifndef DESIGNATED_INIT_SAMPLE_H
#define DESIGNATED_INIT_SAMPLE_H

/**
 * @brief Entry of the designated initializer / compound literal chapter.
 */
int main_designated_init_sample(void);

/* Non-static on purpose (repo convention, see src/advance/calc.h):
 * Unity asserts this — it proves partial init zero-fills the rest. */
int di_sum_of_defaults(int fps);

#endif /* DESIGNATED_INIT_SAMPLE_H */
