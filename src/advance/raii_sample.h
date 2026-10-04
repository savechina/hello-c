#ifndef RAII_SAMPLE_H
#define RAII_SAMPLE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int main_raii_sample(void);

/* --- Arena (Demo 3) ---
 * Exposed for unit testing: the overflow and alignment guarantees are the whole
 * point of the abstraction and must not regress silently. */
typedef struct {
    unsigned char *base;
    size_t cap;
    size_t used;
} arena_t;

bool     raii_arena_init(arena_t *a, size_t cap);
void    *raii_arena_alloc(arena_t *a, size_t n);
void     raii_arena_destroy(arena_t *a);

/* --- Option<T> (Demo 4) --- */
typedef enum { RAII_NONE, RAII_SOME } raii_tag_t;

typedef struct {
    raii_tag_t tag;
    int        value;
} raii_opt_int_t;

raii_opt_int_t raii_some(int v);
raii_opt_int_t raii_none(void);
bool           raii_value(raii_opt_int_t o, int *out);

/* --- Fixed-capacity buffer (Demo 5) --- */
#define RAII_BUF_CAP 64

typedef struct {
    unsigned char data[RAII_BUF_CAP];
    size_t        len;
} raii_buf_t;

void     raii_buf_init(raii_buf_t *b);
bool     raii_buf_append(raii_buf_t *b, const void *p, size_t n);
bool     raii_buf_append_str(raii_buf_t *b, const char *s);
size_t   raii_buf_len(const raii_buf_t *b);

#endif /* RAII_SAMPLE_H */
