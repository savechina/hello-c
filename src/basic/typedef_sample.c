#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <math.h>
#include "basic/typedef_sample.h"

/* ---- Section 1: basic type aliases ---- */

typedef int32_t Age;
typedef float Score;

typedef struct Student {
    char name[32];
    Age   age;
    Score score;
} Student;

static void demo_basic_typedef(void)
{
    Student s = { .name = "Alice", .age = 20, .score = 95.5f };

    printf("  Student: %s, age=%" PRId32 ", score=%.1f\n",
           s.name, s.age, (double)s.score);
    printf("  <-- typedef removes 'struct' prefix; Student s instead of struct Student s\n");
}

/* ---- Section 2: typedef for arrays and pointers ---- */

typedef int32_t IntArray10[10];
typedef char *StringPtr;

static void demo_array_pointer_typedef(void)
{
    IntArray10 arr = { 1, 2, 3, 4, 5 };
    StringPtr greeting = "hello";

    printf("  arr[0]=%d, arr[4]=%d\n", arr[0], arr[4]);
    printf("  greeting=\"%s\"\n", greeting);
    printf("  <-- typedef for arrays: IntArray10 arr[10] → IntArray10 arr (already sized)\n");
}

/* ---- Section 3: function pointer typedefs ---- */

typedef int32_t (*BinaryOp)(int32_t, int32_t);

static int32_t addition(int32_t a, int32_t b)
{
    return a + b;
}

static int32_t multiplication(int32_t a, int32_t b)
{
    return a * b;
}

static int32_t apply(BinaryOp op, int32_t x, int32_t y)
{
    return op(x, y);
}

static void demo_func_ptr_typedef(void)
{
    BinaryOp ops[2] = { addition, multiplication };

    for (int32_t i = 0; i < 2; i++) {
        int32_t result = apply(ops[i], 6, 7);
        printf("  ops[%" PRId32 "](6, 7) = %" PRId32 "\n", i, result);
    }
    printf("  <-- typedef simplifies: BinaryOp op  vs  int32_t (*op)(int32_t, int32_t)\n");
}

/* ---- Section 4: callback-style typedef with context ---- */

typedef void (*Visitor)(int32_t index, int32_t value, void *context);

static void print_visitor(int32_t index, int32_t value, void *context)
{
    (void)context;
    printf("    [%d] = %" PRId32 "\n", index, value);
}

static void sum_visitor(int32_t index, int32_t value, void *context)
{
    (void)index;
    int32_t *sum = (int32_t *)context;
    *sum += value;
}

static void traverse(int32_t data[], int32_t len, Visitor visit, void *ctx)
{
    for (int32_t i = 0; i < len; i++) {
        visit(i, data[i], ctx);
    }
}

static void demo_visitor_pattern(void)
{
    int32_t numbers[4] = { 10, 20, 30, 40 };

    printf("  print_visitor:\n");
    traverse(numbers, 4, print_visitor, NULL);

    int32_t total = 0;
    traverse(numbers, 4, sum_visitor, &total);
    printf("  sum_visitor total: %" PRId32 "\n", total);
}

/* ---- Public entry ---- */

int main_typedef_sample(void)
{
    printf("-- basic type aliases (struct, int, float) --\n");
    demo_basic_typedef();

    printf("-- typedef for arrays and pointers --\n");
    demo_array_pointer_typedef();

    printf("-- function pointer typedefs --\n");
    demo_func_ptr_typedef();

    printf("-- visitor callback pattern --\n");
    demo_visitor_pattern();

    return 0;
}
