#include <float.h>
#include "basic/datatype_sample.h"
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>

/**
 * @brief Standalone datatype demo — follows hello-rust `_sample` convention.
 * Compiles into the unified `hello` binary.
 */
int main_datatype_sample(void) {
    /* --- integer types --- */
    int8_t  mi8  = -42;
    uint8_t mu8  = 200;
    int32_t mi32 = 12345;
    int64_t mi64 = 9223372036854775807LL;
    printf("  int8_t  = %" PRId8 "   (range: %" PRId8 " ~ %" PRId8 ")\n", mi8, INT8_MIN, INT8_MAX);
    printf("  uint8_t = %" PRIu8 "  (range: 0 ~ %" PRIu8 ")\n", mu8, UINT8_MAX);
    printf("  int32_t = %" PRId32 "  (range: %" PRId32 " ~ %" PRId32 ")\n", mi32, INT32_MIN, INT32_MAX);
    printf("  int64_t = %" PRId64 "\n", mi64);

    /* --- float / double --- */
    float  fval = 3.14f;
    double dval = 3.14159265358979;
    printf("  float  = %.6f  (sizeof=%zu)\n", fval, sizeof(float));
    printf("  double = %.14f  (sizeof=%zu)\n", dval, sizeof(double));

    /* --- char --- */
    char ch = 'A';
    unsigned char uch = 255;
    printf("  char            = '%c'  (value=%d)\n", ch, ch);
    printf("  unsigned char   = %u  (max=%u)\n", uch, UCHAR_MAX);

    /* --- sizeof --- */
    printf("  sizeof(char)    = %zu\n", sizeof(char));
    printf("  sizeof(short)   = %zu\n", sizeof(short));
    printf("  sizeof(int)     = %zu\n", sizeof(int));
    printf("  sizeof(long)    = %zu\n", sizeof(long));
    printf("  sizeof(float)   = %zu\n", sizeof(float));
    printf("  sizeof(double)  = %zu\n", sizeof(double));

    /* --- const --- */
    const int MAX_ITEMS = 100;
    const double E = 2.71828;
    printf("  const int MAX_ITEMS = %d\n", MAX_ITEMS);
    printf("  const double E      = %.5f\n", E);

    return 0;
}

/* Stubs for backward compatibility with basic.c calls */

void string_sample(void) {
    printf("  [string_sample] 安全字符串拷贝 (strncpy)\n");
    char src[] = "Hello, C!";
    char dest[20];
    strncpy(dest, src, sizeof(dest) - 1);
    dest[sizeof(dest) - 1] = '\0';
    printf("  dest = \"%s\"\n", dest);
}

void date_sample(void) {
    printf("  [date_sample] 当前时间戳 (time_t)\n");
    time_t now = time(NULL);
    printf("  当前时间戳: %ld\n", (long)now);
}
