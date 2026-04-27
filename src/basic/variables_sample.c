#include <stdio.h>
#include "basic/variables_sample.h"
#include <stdint.h>
#include <string.h>

/* ── const vs #define demo ── */
#ifndef SAMPLE_BUFFER_SIZE
#define SAMPLE_BUFFER_SIZE 256
#endif

/* ── global counter for demo visibility ── */
static int var_call_count = 0;

static void variables_type_sample(void) {
    var_call_count++;
    printf("==== [sample] variables (call #%d) ====\n\n", var_call_count);

    /* ── 1. Integer types ── */
    printf("--- 1. Integer Types ---\n");
    printf("  int8_t   : %d (range: -128 ~ 127)\n", (int8_t)-128);
    printf("  uint8_t  : %u (range: 0 ~ 255)\n", (uint8_t)255);
    printf("  int16_t  : %d\n", (int16_t)-32768);
    printf("  uint16_t : %u\n", (uint16_t)65535);
    printf("  int32_t  : %d\n", INT32_MIN);
    printf("  uint32_t : %u\n", UINT32_MAX);
    printf("  int64_t  : %lld\n", (long long)INT64_MAX);

    /* ── 2. Floating-point ── */
    printf("\n--- 2. Floating-Point Types ---\n");
    float pi_f = 3.14159265f;
    double pi_d = 3.141592653589793;
    printf("  float  PI: %.7f (~7 sig digits)\n", pi_f);
    printf("  double PI: %.15f (~15-16 sig digits)\n", pi_d);
    printf("  diff: %.10f\n", pi_d - (double)pi_f);

    /* ── 3. const ── */
    printf("\n--- 3. const Constants ---\n");
    const double TAX_RATE = 0.08;
    double price = 49.99;
    printf("  price: $%.2f, tax rate: %.0f%%, tax: $%.2f, total: $%.2f\n",
           price, TAX_RATE * 100, price * TAX_RATE, price * (1.0 + TAX_RATE));
}

static void variables_arith_sample(void) {
    printf("\n--- 4. Arithmetic Operators ---\n");
    int x = 17, y = 5;
    printf("  x = %d, y = %d\n", x, y);
    printf("  x + y  = %d\n", x + y);
    printf("  x - y  = %d\n", x - y);
    printf("  x * y  = %d\n", x * y);
    printf("  x / y  = %d (integer division, truncates)\n", x / y);
    printf("  x %% y  = %d (modulo)\n", x % y);
    printf("  (double)x/y = %.4f (float division)\n", (double)x / y);

    /* ── 5. Increment: prefix vs postfix ── */
    printf("\n--- 5. Increment (Prefix vs Postfix) ---\n");
    int m = 5, n = 5;
    int post = m++;
    int pre  = ++n;
    printf("  postfix = m++ => result=%d, m=%d\n", post, m);
    printf("  prefix  = ++n => result=%d, n=%d\n", pre, n);

    /* ── 6. Compound assignment ── */
    printf("\n--- 6. Compound Assignment ---\n");
    int score = 80;
    printf("  score = %d\n", score);
    score += 10;  printf("  score += 10  => %d\n", score);
    score -= 5;   printf("  score -= 5   => %d\n", score);
    score *= 2;   printf("  score *= 2   => %d\n", score);
    score /= 3;   printf("  score /= 3   => %d\n", score);
}

static void variables_scope_sample(void) {
    printf("\n--- 7. Scope & Shadowing ---\n");
    int level = 1;
    printf("  outer: level = %d\n", level);
    {
        int level = 10;
        printf("  inner: level = %d (shadows outer)\n", level);
        {
            int level = 100;
            printf("  nested: level = %d (shadows again)\n", level);
        }
        printf("  exit nested: level = %d (back to inner)\n", level);
    }
    printf("  outermost: level = %d (original)\n", level);
}

static void variables_cast_sample(void) {
    printf("\n--- 8. Type Casting ---\n");
    int a = 7, b = 2;
    printf("  7 / 2        = %.1f (int div first, then cast)\n", (double)(a / b));
    printf("  (double)7/2  = %.1f (cast first, then div)\n", (double)a / b);

    printf("\n  --- Integer Overflow Demo ---\n");
    int32_t max = INT32_MAX;
    printf("  INT32_MAX = %d\n", max);
    printf("  INT32_MAX + 1 = %d (overflow! wraps to negative)\n", max + 1);
}

static void variables_safe_string_sample(void) {
    printf("\n--- 9. Safe String Buffer Demo ---\n");
    char buf[SAMPLE_BUFFER_SIZE];
    const char *msg = "Hello, C Variables!";

    strncpy(buf, msg, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    printf("  buf (safe copy): \"%s\"\n", buf);

    char report[SAMPLE_BUFFER_SIZE];
    int items = 42;
    int ret = snprintf(report, sizeof(report),
                       "processed %d items, buffer size: %d", items, SAMPLE_BUFFER_SIZE);
    if (ret > 0 && (size_t)ret < sizeof(report)) {
        printf("  report: \"%s\"\n", report);
    }
}

/* ── public entry ── */
void variables_all_samples(void) {
    variables_type_sample();
    variables_arith_sample();
    variables_scope_sample();
    variables_cast_sample();
    variables_safe_string_sample();
    printf("\n  variables sample done.\n");
}

/* Coordinator entry for this chapter */
int main_variables_sample(void) {
    variables_all_samples();
    return 0;
}
