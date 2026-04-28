#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include "basic/void_pointers_sample.h"

/*
 * void* 指针 — 类型擦除、泛型函数
 * void* 是无类型指针，能指向任何数据，但读之前必须转回正确类型。
 */

typedef enum {
    TYPE_INT32 = 1,
    TYPE_DOUBLE = 2,
    TYPE_CHAR = 3
} VpTypeTag;

static void demo_any_type_pointer(void)
{
    int32_t i = 42;
    double d = 3.14;
    char c = 'Z';

    printf("=== void* 可指向任何类型 ===\n");

    void *v1 = &i;
    void *v2 = &d;
    void *v3 = &c;

    printf("  (void*)&i → *(int32_t*)v1 = %" PRId32 "\n", *(int32_t *)v1);
    printf("  (void*)&d → *(double*)v2  = %.2f\n", *(double *)v2);
    printf("  (void*)&c → *(char*)v3   = '%c'\n\n", *(char *)v3);
}

static void demo_generic_swap(void)
{
    printf("=== 泛型 swap (void* + size) ===\n");

    int32_t xi = 10, yi = 20;
    printf("  交换前 int32_t: %" PRId32 ", %" PRId32 "\n", xi, yi);

    unsigned char buf[256];
    size_t sz = sizeof(int32_t);
    memcpy(buf, &xi, sz);
    memcpy(&xi, &yi, sz);
    memcpy(&yi, buf, sz);

    printf("  交换后 int32_t: %" PRId32 ", %" PRId32 "\n", xi, yi);

    double xd = 1.5, yd = 9.9;
    printf("  交换前 double:  %.1f, %.1f\n", xd, yd);

    sz = sizeof(double);
    memcpy(buf, &xd, sz);
    memcpy(&xd, &yd, sz);
    memcpy(&yd, buf, sz);

    printf("  交换后 double:  %.1f, %.1f\n\n", xd, yd);

    printf("  原理: 不按类型操作，只管按字节拷贝\n\n");
}

static void demo_type_tagged_print(VpTypeTag tag, void *data)
{
    printf("  ");
    switch (tag) {
    case TYPE_INT32:
        printf("int32:  %" PRId32, *(int32_t *)data);
        break;
    case TYPE_DOUBLE:
        printf("double: %.3f", *(double *)data);
        break;
    case TYPE_CHAR:
        printf("char:   '%c'", *(char *)data);
        break;
    default:
        printf("unknown(tag=%d)", (int)tag);
        break;
    }
    printf("\n");
}

static void demo_tagged_generic(void)
{
    printf("=== 类型标签模式: 丢失的类型靠 tag 找回 ===\n");

    int32_t iv = 100;
    double dv = 2.718;
    char cv = 'A';

    demo_type_tagged_print(TYPE_INT32, &iv);
    demo_type_tagged_print(TYPE_DOUBLE, &dv);
    demo_type_tagged_print(TYPE_CHAR, &cv);

    printf("\n  ⚠️  类型擦除后必须靠 tag 或协议追回类型!\n\n");
}

static int cmp_int32(const void *a, const void *b)
{
    int32_t va = *(const int32_t *)a;
    int32_t vb = *(const int32_t *)b;
    return (va > vb) - (va < vb);
}

static void demo_qsort_pattern(void)
{
    int32_t nums[] = {33, 10, 75, 42, 5};
    int32_t n = 5;

    printf("=== stdlib.h qsort: void* 的经典用法 ===\n");

    printf("  排序前: ");
    for (int32_t i = 0; i < n; i++) printf("%" PRId32 " ", nums[i]);
    printf("\n");

    qsort(nums, (size_t)n, sizeof(int32_t), cmp_int32);

    printf("  排序后: ");
    for (int32_t i = 0; i < n; i++) printf("%" PRId32 " ", nums[i]);
    printf("\n\n");

    printf("  qsort 的签名:\n");
    printf("    void qsort(void *base, size_t nmemb, size_t size,\n");
    printf("               int (*cmp)(const void*, const void*));\n");
    printf("  核心: void* 抹掉类型，回调函数负责解释内容\n\n");
}

int main_void_pointers_sample(void)
{
    printf("========================================\n");
    printf("  void* 指针 (类型擦除 & 泛型)\n");
    printf("  「void* 是万能适配器，但接对线是你的工作。」\n");
    printf("========================================\n\n");

    demo_any_type_pointer();
    demo_generic_swap();
    demo_tagged_generic();
    demo_qsort_pattern();

    printf("void* 指针演示完毕。\n");
    return 0;
}
