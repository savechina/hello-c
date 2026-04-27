#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include "basic/stdlib_sample.h"

static void number_conversion_sample(void) {
    printf("----basic ... number_conversion_sample.....start ----\n");
    printf("  [1] atoi (字符转整数):\n");
    printf("    atoi(\"42\") = %d\n", atoi("42"));
    printf("    atoi(\"-100\") = %d\n", atoi("-100"));

    printf("  [2] atof (字符转浮点):\n");
    printf("    atof(\"3.14159\") = %.5f\n", atof("3.14159"));

    printf("  [3] strtol (安全转换 + 错误检测):\n");
    char *end;
    long val = strtol("123abc", &end, 10);
    printf("    strtol(\"123abc\", &end, 10) = %ld, 剩余 = \"%s\"\n", val, end);

    val = strtol("xyz", &end, 10);
    printf("    strtol(\"xyz\", &end, 10) = %ld (无效输入)\n", val);
    printf("----basic ... number_conversion_sample.....done ----\n");
}

static void random_number_sample(void) {
    printf("----basic ... random_number_sample.....start ----\n");

    /* 正确做法：用时间播种 */
    srand((unsigned int)time(NULL));
    printf("  [4] rand/srand (随机数生成):\n");
    printf("  用 srand(time(NULL)) 播种:");
    for (int i = 0; i < 5; i++) {
        printf(" %d", rand());
    }
    printf("\n");

    /* 常见错误：不播种 → 每次相同序列 */
    printf("  ❌ 不播种 (无 srand) → 每次运行产生相同序列\n");
    printf("  ✅ 正确: srand(time(NULL)) 在程序开头调用一次\n");
    printf("----basic ... random_number_sample.....done ----\n");
}

static void math_functions_sample(void) {
    printf("----basic ... math_functions_sample.....start ----\n");
    printf("  [5] 数学函数:\n");
    printf("    abs(-42) = %d\n", abs(-42));
    printf("    abs(42)  = %d\n", abs(42));
    printf("    floor(3.7) = %.1f\n", floor(3.7));
    printf("    ceil(3.2)  = %.1f\n", ceil(3.2));
    printf("    sqrt(144)  = %.1f\n", sqrt(144.0));
    printf("    pow(2, 10) = %.0f\n", pow(2.0, 10.0));

    /* C99 新增: round, trunc */
    printf("    round(3.5) = %.0f, round(3.7) = %.0f\n", round(3.5), round(3.7));
    printf("----basic ... math_functions_sample.....done ----\n");
}

static void time_functions_sample(void) {
    printf("----basic ... time_functions_sample.....start ----\n");
    printf("  [6] 时间函数:\n");

    time_t now = time(NULL);
    printf("    time(NULL) = %ld (Unix 时间戳)\n", (long)now);

    struct tm *tm_info = localtime(&now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("    当前时间: %s\n", buf);

    /* 计算时间差 */
    time_t later = now + 3600; /* 1 小时后 */
    double diff = difftime(later, now);
    printf("    1 小时后时间差: %.0f 秒\n", diff);
    printf("----basic ... time_functions_sample.....done ----\n");
}

static void char_classification_sample(void) {
    printf("----basic ... char_classification_sample.....start ----\n");
    printf("  [7] 字符分类与转换:\n");
    char test[] = "Hello, 世界!";
    printf("  输入: \"%s\"\n", test);
    printf("  逐字符分类:\n");
    for (int i = 0; test[i] != '\0'; i++) {
        unsigned char c = (unsigned char)test[i];
        printf("    '%c': ", c);
        if (isalpha(c)) printf("字母, ");
        if (isdigit(c)) printf("数字, ");
        if (isspace(c)) printf("空白, ");
        if (ispunct(c)) printf("标点, ");
        if (isupper(c)) printf("大写, ");
        if (islower(c)) printf("小写");
        printf("  → tolower: '%c', toupper: '%c'\n",
               (char)tolower(c), (char)toupper(c));
    }
    printf("----basic ... char_classification_sample.....done ----\n");
}

static void limits_sample(void) {
    printf("----basic ... limits_sample.....start ----\n");
    printf("  [8] 类型极限值 (<limits.h>):\n");
    printf("    CHAR_MIN  = %d, CHAR_MAX  = %d\n", CHAR_MIN, CHAR_MAX);
    printf("    SHRT_MIN  = %d, SHRT_MAX  = %d\n", SHRT_MIN, SHRT_MAX);
    printf("    INT_MIN   = %d, INT_MAX   = %d\n", INT_MIN, INT_MAX);
    printf("    LONG_MIN  = %ld, LONG_MAX  = %ld\n", LONG_MIN, LONG_MAX);
    printf("    LLONG_MIN = %lld, LLONG_MAX = %lld\n", LLONG_MIN, LLONG_MAX);
    printf("    SIZE_MAX  = %zu\n", SIZE_MAX);
    printf("----basic ... limits_sample.....done ----\n");
}

static void stdlib_overview_sample(void) {
    printf("----basic ... stdlib_overview_sample.....start ----\n");
    printf("  [9] C 标准库速查:\n");
    printf("    <stdlib.h>  : malloc, free, atoi, rand, srand, exit, abs\n");
    printf("    <math.h>    : sqrt, pow, sin, cos, floor, ceil\n");
    printf("    <time.h>    : time, localtime, strftime, clock\n");
    printf("    <ctype.h>   : isdigit, isalpha, tolower, toupper\n");
    printf("    <limits.h>  : INT_MAX, CHAR_MIN, LONG_MAX 等\n");
    printf("    <string.h>  : strlen, strcpy, strcat, strcmp, memcmp\n");
    printf("    <stdio.h>   : printf, scanf, fopen, fread, fwrite\n");
    printf("----basic ... stdlib_overview_sample.....done ----\n");
}

int main_stdlib_sample(void) {
    printf("=== C 标准库精要（Standard Library）演示 ===\n\n");
    number_conversion_sample();
    printf("\n");
    random_number_sample();
    printf("\n");
    math_functions_sample();
    printf("\n");
    time_functions_sample();
    printf("\n");
    char_classification_sample();
    printf("\n");
    limits_sample();
    printf("\n");
    stdlib_overview_sample();
    printf("\n");
    return 0;
}
