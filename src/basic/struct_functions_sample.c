#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "basic/struct_functions_sample.h"

/* -----------------------------------------------------------
   Shared struct for all demos
   ----------------------------------------------------------- */

struct Rectangle {
    float top_x;
    float top_y;
    float width;
    float height;
};

/* ---- Section 1: pass struct by value (copy) ---- */

static float rect_area_by_value(struct Rectangle r)
{
    return r.width * r.height;
}

static void demo_pass_by_value(void)
{
    struct Rectangle rect = { .width = 10.0f, .height = 5.0f };

    float a = rect_area_by_value(rect);
    printf("  area (by value): %.1f\n", (double)a);
    printf("  <-- whole 16-byte struct was copied onto the stack\n");
}

/* ---- Section 2: pass struct by pointer (no copy) ---- */

static float rect_area_ptr(const struct Rectangle *r)
{
    return r->width * r->height;
}

static void rect_grow(struct Rectangle *r, float delta)
{
    r->width  += delta;
    r->height += delta;
}

static void demo_pass_by_ptr(void)
{
    struct Rectangle rect = { .width = 10.0f, .height = 5.0f };

    printf("  initial area: %.1f\n", (double)rect_area_ptr(&rect));

    rect_grow(&rect, 2.0f);
    printf("  after grow(2.0): %.1f\n", (double)rect_area_ptr(&rect));
    printf("  <-- pointer: no copy, modifications visible to caller\n");
}

/* ---- Section 3: return struct from function ---- */

static struct Rectangle make_rect(float x, float y, float w, float h)
{
    struct Rectangle r;
    r.top_x  = x;
    r.top_y  = y;
    r.width  = w;
    r.height = h;
    return r;
}

static struct Rectangle rect_merge(const struct Rectangle *a,
                                   const struct Rectangle *b)
{
    struct Rectangle merged;
    merged.top_x   = a->top_x   < b->top_x   ? a->top_x   : b->top_x;
    merged.top_y   = a->top_y   < b->top_y   ? a->top_y   : b->top_y;
    float a_right  = a->top_x  + a->width;
    float b_right  = b->top_x  + b->width;
    float a_bottom = a->top_y  + a->height;
    float b_bottom = b->top_y  + b->height;
    merged.width   = (a_right  > b_right  ? a_right  : b_right)  - merged.top_x;
    merged.height  = (a_bottom > b_bottom ? a_bottom : b_bottom) - merged.top_y;
    return merged;
}

static void demo_return_struct(void)
{
    struct Rectangle r1 = make_rect(0.0f, 0.0f, 10.0f, 10.0f);
    struct Rectangle r2 = make_rect(5.0f, 5.0f, 10.0f, 10.0f);

    struct Rectangle merged = rect_merge(&r1, &r2);
    printf("  r1: (%.0f,%.0f) %.0fx%.0f\n",
           (double)r1.top_x, (double)r1.top_y,
           (double)r1.width, (double)r1.height);
    printf("  r2: (%.0f,%.0f) %.0fx%.0f\n",
           (double)r2.top_x, (double)r2.top_y,
           (double)r2.width, (double)r2.height);
    printf("  merged: (%.0f,%.0f) %.0fx%.0f — area=%.0f\n",
           (double)merged.top_x, (double)merged.top_y,
           (double)merged.width, (double)merged.height,
           (double)(merged.width * merged.height));
}

/* ---- Section 4: array of structs + pointer arithmetic ---- */

static float total_area(const struct Rectangle rects[], int32_t count)
{
    float sum = 0.0f;
    for (int32_t i = 0; i < count; i++) {
        sum += rects[i].width * rects[i].height;
    }
    return sum;
}

static void demo_array_of_structs(void)
{
    struct Rectangle pool[3] = {
        { .width = 1.0f, .height = 1.0f },
        { .width = 2.0f, .height = 3.0f },
        { .width = 4.0f, .height = 5.0f },
    };

    float tot = total_area(pool, 3);
    printf("  total area of %d rects: %.1f\n", 3, (double)tot);

    /* Pointer arithmetic equivalent */
    const struct Rectangle *p = pool;
    printf("  via pointer: (%p)[0] area = %.1f\n", (void *)p,
           (double)(p[0].width * p[0].height));
}

/* ---- Public entry ---- */

int main_struct_functions_sample(void)
{
    printf("-- pass struct by value (copy) --\n");
    demo_pass_by_value();

    printf("-- pass struct by pointer (no copy) --\n");
    demo_pass_by_ptr();

    printf("-- return struct from function --\n");
    demo_return_struct();

    printf("-- array of structs + pointer arithmetic --\n");
    demo_array_of_structs();

    return 0;
}
