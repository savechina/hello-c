#ifndef VOID_POINTERS_SAMPLE_H
#define VOID_POINTERS_SAMPLE_H

int main_void_pointers_sample(void);

/* qsort 比较器 — 返回 -1/0/1，按 int32_t 升序 */
int cmp_int32(const void *a, const void *b);

#endif /* VOID_POINTERS_SAMPLE_H */
