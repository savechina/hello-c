#ifndef TYPE_GENERIC_SAMPLE_H
#define TYPE_GENERIC_SAMPLE_H

/**
 * @brief Entry of the _Generic type-generic selection chapter.
 */
int main_type_generic_sample(void);

/* Non-static on purpose (repo convention, see src/advance/calc.h):
 * Unity tests assert these pure helpers directly. */
int tg_max_int(int a, int b);
int tg_min_int(int a, int b);

#endif /* TYPE_GENERIC_SAMPLE_H */
