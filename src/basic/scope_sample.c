#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "basic/scope_sample.h"

/* ============================================================
   Module-level (file-scope) variables
   ============================================================ */

/* File-level extern variable: visible to other .c files via extern */
int global_counter = 0;

/* File-level static variable: hidden from other .c files */
static int file_private_secret = 42;

/* ============================================================
   Extern linkage demonstration
   ============================================================ */

static void demo_extern_linkage(void) {
    printf("==== [sample] Extern Linkage ====\n");

    /* global_counter is a file-scope variable with external linkage */
    global_counter += 10;
    printf("  global_counter (external linkage) = %d\n", global_counter);

    /* file_private_secret is only visible in this file */
    printf("  file_private_secret (internal/static linkage) = %d\n",
           file_private_secret);
    printf("  <-- static at file level hides this from other .c files\n");
}

/* ---- Section 1: Block scope and shadowing ---- */

static void demo_block_scope(void) {
    printf("\n==== [sample] Block Scope & Shadowing ====\n");

    int x = 1;
    printf("  outer: x = %d\n", x);

    {
        int x = 10;
        printf("  inner block 1: x = %d (shadows outer)\n", x);

        {
            int x = 100;
            printf("  inner block 2: x = %d (shadows again)\n", x);
        }
        printf("  exit block 2: x = %d (back to block 1)\n", x);
    }
    printf("  exit all blocks: x = %d (original)\n", x);
}

/* ---- Section 2: static at function level ---- */

static void tick_counter(void) {
    static int tick = 0;
    tick++;
    printf("  tick #%d (static local: lives until program exit)\n", tick);
}

static void demo_static_function_level(void) {
    printf("\n==== [sample] static at Function Level ====\n");

    printf("  Calling tick_counter() 5 times:\n");
    for (int i = 0; i < 5; i++) {
        tick_counter();
    }

    /* Contrast with auto variable */
    printf("\n  Contrast: auto variable resets every call\n");
    {
        int auto_var = 0;
        auto_var++;
        printf("  first time:  auto_var = %d\n", auto_var);
    }
    {
        int auto_var = 0;
        auto_var++;
        printf("  second time: auto_var = %d\n", auto_var);
    }
}

/* ---- Section 3: static at file level ---- */

static void demo_const_correctness(void) {
    printf("\n==== [sample] const Correctness ====\n");

    const int MAX_SIZE = 256;
    printf("  const int MAX_SIZE = %d\n", MAX_SIZE);

    const char *greeting = "Hello, const!";
    printf("  const char *greeting = \"%s\"\n", greeting);

    /* Demonstrating pointer const patterns */
    int value = 10;
    const int *ptr_to_const = &value;
    int *const const_ptr = &value;

    printf("  const int *p:   *p = %d (can't modify *p)\n", *ptr_to_const);
    printf("  int *const p:   *p = %d (can't change p)\n", *const_ptr);
}

/* ---- Section 4: Returning local variable address (conceptual demo) ---- */
/* Deliberately demonstrates UB: returning address of stack local.
 * Warning suppressed because this IS the point of the demo. */

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wreturn-stack-address"
#else
#pragma GCC diagnostic ignored "-Wreturn-local-addr"
#endif
static int *return_local_address(void) {
    int x = 42;
    return &x;  /* intentionally: dangling pointer demo */
}
#pragma GCC diagnostic pop

/*
 * Safe fix 1: use static local variable
 */
static int *return_static_address(void) {
    static int x = 42;
    return &x;
}

/*
 * Safe fix 2: use dynamic allocation (heap)
 */
static int *return_heap_address(void) {
    int *x = malloc(sizeof(int));
    if (x != NULL) {
        *x = 42;
    }
    return x;
}

/*
 * Safe fix 3: let caller allocate
 */
static void fill_by_caller(int *result) {
    *result = 42;
}

static void demo_returning_address(void) {
    printf("\n==== [sample] Returning Address of Local Variable ====\n");

    volatile int *dangling = return_local_address();
    printf("  WARNING: return_local_address() returns a dangling pointer\n");
    printf("           The variable 'x' was on the stack and is now gone.\n");
    printf("           Dereferencing this pointer = undefined behavior.\n");
    printf("           (We do NOT dereference it here.)\n");
    (void)dangling;

    printf("\n  Safe fix 1 - static local:\n");
    int *p1 = return_static_address();
    printf("     *p1 = %d  (safe: static variable lives in .data)\n", *p1);

    printf("  Safe fix 2 - heap allocation:\n");
    int *p2 = return_heap_address();
    if (p2 != NULL) {
        printf("     *p2 = %d  (safe: heap memory persists)\n", *p2);
        free(p2);
        p2 = NULL;
    }

    printf("  Safe fix 3 - caller allocates:\n");
    int caller_var = 0;
    fill_by_caller(&caller_var);
    printf("     caller_var = %d  (safe: caller owns the storage)\n", caller_var);
}

/* ---- Section 5: Lifetime comparison ---- */

static void demo_lifetime_comparison(void) {
    printf("\n==== [sample] Lifetime: Stack vs Heap vs Static ====\n");

    printf("  Stack (auto variable):\n");
    {
        int stack_var = 10;
        printf("    inside block:  stack_var = %d\n", stack_var);
    }
    printf("    after block:     stack_var is destroyed (out of scope)\n");

    printf("  Static variable:\n");
    {
        static int static_var = 10;
        static_var++;
        printf("    inside block:  static_var = %d\n", static_var);
    }
    printf("    after block:     static_var still exists (in .data section)\n");

    printf("  Heap (malloc):\n");
    int *heap_var = malloc(sizeof(int));
    if (heap_var != NULL) {
        *heap_var = 10;
        printf("    before free:   *heap_var = %d\n", *heap_var);
    }
    free(heap_var);
    heap_var = NULL;
    printf("    after free:     heap_var freed and set to NULL\n");

    printf("  Global variable:\n");
    printf("    global_counter = %d (created at startup, lives until exit)\n",
           global_counter);
}

/* ---- Section 6: Dangling pointer explanation ---- */

#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wdangling-pointer"
#endif
static void demo_dangling_pointer_explained(void) {
    printf("\n==== [sample] Dangling Pointer — Explained ====\n");

    int *dangling = NULL;

    {
        int local = 99;
        dangling = &local;
        printf("  local = %d, dangling -> &local\n", local);
    }
    printf("  local is now destroyed.\n");
    printf("  'dangling' still holds the address %p\n", (void *)dangling);
    printf("  BUT: dereferencing (int)*dangling = UNDEFINED BEHAVIOR!\n");
    printf("  Fix: always set pointer to NULL after target is destroyed.\n");
    dangling = NULL;
}
#pragma GCC diagnostic pop

/* ---- Section 7: Use-after-free explanation ---- */

#pragma GCC diagnostic push
#ifndef __clang__
#pragma GCC diagnostic ignored "-Wuse-after-free"
#endif
static void demo_use_after_free_explained(void) {
    printf("\n==== [sample] Use-After-Free — Explained ====\n");

    int *data = malloc(sizeof(int) * 4);
    if (data != NULL) {
        data[0] = 100;
        data[1] = 200;
        printf("  Before free: data[0] = %d, data[1] = %d\n", data[0], data[1]);
    }

    free(data);
    printf("  After free(data):\n");
    printf("    data still holds address %p\n", (void *)data);
    printf("    BUT the memory belongs to the allocator now.\n");
    printf("    Writing data[0] = 99 would be use-after-free (UB!)\n");
    printf("    Safe practice: data = NULL immediately after free.\n");

    data = NULL;
}
#pragma GCC diagnostic pop

/* ---- Section 8: Practical scope management ---- */

static int safe_get_max(int a, int b) {
    return (a > b) ? a : b;
}

static void safe_copy_max(int *dest, int a, int b) {
    *dest = (a > b) ? a : b;
}

static void demo_safe_patterns(void) {
    printf("\n==== [sample] Practical Scope Management Patterns ====\n");

    int result = safe_get_max(10, 20);
    printf("  Pattern 1 (return by value): max(10, 20) = %d\n", result);

    int output = 0;
    safe_copy_max(&output, 30, 15);
    printf("  Pattern 2 (caller allocates): max(30, 15) = %d\n", output);

    char *buffer = malloc(64);
    if (buffer != NULL) {
        strncpy(buffer, "owned by caller", 63);
        buffer[63] = '\0';
        printf("  Pattern 3 (heap + free): %s\n", buffer);
        free(buffer);
        buffer = NULL;
    }

    printf("\n  Summary of safe patterns:\n");
    printf("    1. Return values, not pointers to locals\n");
    printf("    2. Let callers allocate when large data needed\n");
    printf("    3. Heap: always pair malloc/free, NULL after free\n");
}

/* ---- Public entry ---- */

int main_scope_sample(void) {
    demo_extern_linkage();
    demo_block_scope();
    demo_static_function_level();
    demo_const_correctness();
    demo_returning_address();
    demo_lifetime_comparison();
    demo_dangling_pointer_explained();
    demo_use_after_free_explained();
    demo_safe_patterns();

    printf("\nscope sample done.\n");
    return 0;
}
