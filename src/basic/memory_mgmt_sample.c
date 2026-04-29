#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "basic/memory_mgmt_sample.h"

/* ============================================================
   Memory Management Demo: malloc/free, heap vs stack,
   safe allocation patterns, valgrind-clean.
   ============================================================ */

/* ────────────────────────────────────────────────────────────
   Section 1: Basic malloc + free with NULL check
   ──────────────────────────────────────────────────────────── */

static void demo_basic_malloc_free(void) {
    printf("=== 1. Basic malloc + free ===\n");

    int32_t *p = malloc(sizeof(int32_t));
    if (p == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }

    *p = 42;
    printf("  *p = %" PRId32 "\n", *p);
    printf("  (address: %p)\n", (void *)p);

    free(p);
    p = NULL;
    printf("  free(p); p = NULL;\n\n");
}

/* ────────────────────────────────────────────────────────────
   Section 2: malloc array
   ──────────────────────────────────────────────────────────── */

static void demo_malloc_array(void) {
    printf("=== 2. malloc array ===\n");

    size_t n = 5;
    int32_t *arr = malloc(n * sizeof(int32_t));
    if (arr == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }

    for (size_t i = 0; i < n; i++) {
        arr[i] = (int32_t)((i + 1) * 10);
    }

    printf("  Array contents: ");
    for (size_t i = 0; i < n; i++) {
        printf("%" PRId32 " ", arr[i]);
    }
    printf("\n");

    free(arr);
    arr = NULL;
    printf("  free(arr); arr = NULL;\n\n");
}

/* ────────────────────────────────────────────────────────────
   Section 3: calloc vs malloc
   ──────────────────────────────────────────────────────────── */

static void demo_calloc_vs_malloc(void) {
    printf("=== 3. calloc vs malloc ===\n");

    size_t count = 4;
    printf("  Malloc (uninitialized):\n");
    int32_t *a = malloc(count * sizeof(int32_t));
    if (a == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }
    printf("    Before writing: ");
    for (size_t i = 0; i < count; i++) {
        printf("%" PRId32 " ", a[i]);
    }
    printf("\n");
    free(a);
    a = NULL;

    printf("  Calloc (zero-initialized):\n");
    int32_t *b = calloc(count, sizeof(int32_t));
    if (b == NULL) {
        fprintf(stderr, "  Error: calloc failed\n");
        return;
    }
    printf("    Before writing: ");
    for (size_t i = 0; i < count; i++) {
        printf("%" PRId32 " ", b[i]);
    }
    printf("\n");

    free(b);
    b = NULL;
    printf("  calloc guarantees 0. malloc gives garbage.\n\n");
}

/* ────────────────────────────────────────────────────────────
   Section 4: realloc (grow)
   ──────────────────────────────────────────────────────────── */

static void demo_realloc_grow(void) {
    printf("=== 4. realloc (grow) ===\n");

    int32_t *buf = malloc(3 * sizeof(int32_t));
    if (buf == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }
    buf[0] = 10;
    buf[1] = 20;
    buf[2] = 30;
    printf("  Before:  buf[0..2] = %" PRId32 " %" PRId32 " %" PRId32 "\n",
           buf[0], buf[1], buf[2]);

    /* Safe realloc: use temp to avoid losing original on failure */
    int32_t *tmp = realloc(buf, 6 * sizeof(int32_t));
    if (tmp == NULL) {
        free(buf);
        fprintf(stderr, "  Error: realloc failed, original freed\n");
        return;
    }
    buf = tmp;

    buf[3] = 40;
    buf[4] = 50;
    buf[5] = 60;

    printf("  After:   buf[0..5] = %" PRId32 " %" PRId32 " %" PRId32
           " %" PRId32 " %" PRId32 " %" PRId32 "\n",
           buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);

    free(buf);
    buf = NULL;
    printf("  Key: use temp ptr for realloc! free(buf); buf = NULL;\n\n");
}

/* ────────────────────────────────────────────────────────────
   Section 5: Struct allocation
   ──────────────────────────────────────────────────────────── */

typedef struct {
    char name[32];
    int32_t age;
} Person;

static void demo_struct_allocation(void) {
    printf("=== 5. Struct allocation ===\n");

    Person *p = malloc(sizeof(Person));
    if (p == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }

    strncpy(p->name, "Alice", sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    p->age = 25;

    printf("  name = %s, age = %" PRId32 "\n", p->name, p->age);
    printf("  (struct at: %p)\n", (void *)p);

    free(p);
    p = NULL;
    printf("  free(p); p = NULL;\n\n");
}

/* ────────────────────────────────────────────────────────────
   Section 6: Memory leak demo (shown, then fixed)
   ──────────────────────────────────────────────────────────── */

static void demo_memory_leak_concept(void) {
    printf("=== 6. Memory leak concept (demonstrated safely) ===\n");

    /*
     * BAD pattern (conceptual only — not executed to avoid actual leak):
     *
     *   char *buf = malloc(256);  // allocated
     *   strncpy(buf, "data!", 255);
     *   buf[255] = '\0';
     *   // forgot to free(buf) → leak!
     *   // buf goes out of scope, but the 256 bytes are gone forever.
     */

    /* FIXED version — we do both: */
    char *buf = malloc(256);
    if (buf == NULL) {
        fprintf(stderr, "  Error: malloc failed\n");
        return;
    }
    strncpy(buf, "data!", 255);
    buf[255] = '\0';

    printf("  Allocated and used buf.\n");
    printf("  GOOD: free(buf) is called here.\n");

    free(buf);
    buf = NULL;
    printf("  free(buf); buf = NULL;  ← no leak!\n\n");
}

/* ────────────────────────────────────────────────────────────
   Section 7: Safe allocation pattern (factory function)
   ──────────────────────────────────────────────────────────── */

static Person *create_person(const char *name, int32_t age) {
    Person *p = malloc(sizeof(Person));
    if (p == NULL) {
        return NULL;
    }

    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    p->age = age;
    return p;
}

static void demo_safe_alloc_pattern(void) {
    printf("=== 7. Safe alloc pattern (factory function) ===\n");

    printf("  Safe pattern:\n");
    printf("    1. malloc + NULL check\n");
    printf("    2. strncpy (not strcpy)\n");
    printf("    3. Caller controls free\n\n");

    Person *p = create_person("Bob", 30);
    if (p == NULL) {
        fprintf(stderr, "  Error: create_person failed\n");
        return;
    }

    printf("  Created: name = %s, age = %" PRId32 "\n", p->name, p->age);

    free(p);
    p = NULL;
    printf("  free(p); p = NULL;  ← caller's responsibility\n\n");
}

/* ────────────────────────────────────────────────────────────
   Entry point
   ──────────────────────────────────────────────────────────── */

int main_memory_mgmt_sample(void) {
    printf("========================================\n");
    printf("  内存管理 (Memory Management: malloc/free)\n");
    printf("========================================\n\n");

    demo_basic_malloc_free();
    demo_malloc_array();
    demo_calloc_vs_malloc();
    demo_realloc_grow();
    demo_struct_allocation();
    demo_memory_leak_concept();
    demo_safe_alloc_pattern();

    printf("内存管理演示完毕。\n");
    return 0;
}
