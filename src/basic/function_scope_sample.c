#include <stdio.h>
#include "basic/function_scope_sample.h"

/* ============================================================
 * Module-level (file-scope) variables for demo
 * ============================================================ */

int g_global_value = 100;  /* Global variable: visible to all functions in this file */

/* ============================================================
 * Section 1: Local vs Global variables + shadowing
 * ============================================================ */

static void demo_local_global_variables(void) {
    printf("==== [sample] Local Variable vs Global Variable ====\n");

    printf("  global_value = %d (defined at file scope)\n", g_global_value);

    {
        int local_value = 50;  /* Local variable: only visible in this block */
        printf("  local_value = %d (defined in { } block)\n", local_value);
        printf("  global_value still accessible = %d\n", g_global_value);
    }
    /* local_value is gone here */
    printf("  after block: local_value is destroyed, global_value = %d\n",
           g_global_value);
}

/* Shadowing demo: local variable hides global variable of same name */
static int shadow_demo(int input) {
    int shadow_var = 42;  /* local */
    printf("  inside function: shadow_var = %d (local, hides global)\n",
           shadow_var);
    return shadow_var + input;
}

static void demo_variable_shadowing(void) {
    printf("\n==== [sample] Variable Shadowing ====\n");

    int shadow_var = 100;  /* local scope 1 */
    printf("  outer: shadow_var = %d\n", shadow_var);

    {
        int shadow_var = 200;  /* shadows outer shadow_var */
        printf("  inner block: shadow_var = %d (shadows outer)\n", shadow_var);
    }
    printf("  after inner block: shadow_var = %d (back to outer)\n", shadow_var);

    /* Call function — it has its own local shadow_var */
    int result = shadow_demo(10);
    printf("  function returned: %d (its own local shadow_var = 42)\n", result);
}

/* ============================================================
 * Section 2: Static functions (internal linkage)
 * ============================================================ */

/* These static helpers are only visible within this .c file */
static int clamp_value(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static int double_value(int x) {
    return x * 2;
}

static void demo_static_functions(void) {
    printf("\n==== [sample] static Functions (Internal Linkage) ====\n");

    printf("  clamp_value(-5, 0, 100) = %d\n", clamp_value(-5, 0, 100));
    printf("  clamp_value(150, 0, 100) = %d\n", clamp_value(150, 0, 100));
    printf("  clamp_value(50, 0, 100) = %d\n", clamp_value(50, 0, 100));

    printf("  double_value(7) = %d\n", double_value(7));

    printf("  These static functions are invisible to other .c files!\n");
}

/* ============================================================
 * Section 3: Extern keyword & cross-file visibility
 * ============================================================ */

static void demo_extern_keyword(void) {
    printf("\n==== [sample] extern Keyword & Cross-File Visibility ====\n");

    /* g_global_value is defined at file scope above (external linkage) */
    printf("  g_global_value = %d (external linkage, visible everywhere)\n",
           g_global_value);

    /* Modify it so other demo sections see the change */
    g_global_value += 50;
    printf("  after += 50: g_global_value = %d\n", g_global_value);

    printf("  Other .c files can access this via:\n");
    printf("    extern int g_global_value;\n");
}

/* ============================================================
 * Section 4: Forward declarations
 * ============================================================ */

/* Forward declaration: compute_mean is defined AFTER use_computed_mean */
static int compute_mean(int a, int b, int c);

static void use_computed_mean(void) {
    int mean = compute_mean(10, 20, 30);
    printf("  mean of (10, 20, 30) = %d\n", mean);
}

/* Actual definition appears after the forward declaration + usage */
static int compute_mean(int a, int b, int c) {
    return (a + b + c) / 3;
}

static void demo_forward_declaration(void) {
    printf("\n==== [sample] Forward Declaration ====\n");

    printf("  compute_mean is declared above but defined after use_computed_mean.\n");
    printf("  Without the forward declaration, compile would fail.\n\n");

    use_computed_mean();
}

/* ============================================================
 * Public entry point
 * ============================================================ */

int main_function_scope_sample(void) {
    demo_local_global_variables();
    demo_variable_shadowing();
    demo_static_functions();
    demo_extern_keyword();
    demo_forward_declaration();

    printf("\nfunction_scope sample done.\n");
    return 0;
}
