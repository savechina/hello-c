#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include "advance/smart_pointers_sample.h"

/* ================================================================
 * smart-pointers 演示 (Opaque Pointers & RAII-style Patterns in C)
 *
 * 🔴 困难 — 信息隐藏、RAII 宏、工厂模式
 * ================================================================ */

/* ---------------------------------------------------------
   Secret: 真实类型定义 (仅在本文件内可见)
   头文件只暴露 SmartBuffer*, 调用者看不到内部结构
   --------------------------------------------------------- */

#define SB_CAPACITY 256

struct SmartBuffer_Internal {
    uint8_t data[SB_CAPACITY];
    size_t  len;
    int     ref_count;
};

typedef struct SmartBuffer_Internal SmartBuffer_Int;

/* Forward declarations */
static SmartBuffer_Int *smartbuffer_create(void);
static void smartbuffer_destroy(SmartBuffer_Int *buf);
static int smartbuffer_push(SmartBuffer_Int *buf, uint8_t byte);
static size_t smartbuffer_len(const SmartBuffer_Int *buf);
static const uint8_t *smartbuffer_data(const SmartBuffer_Int *buf);

/* ---------------------------------------------------------
   Demo 1: 错误优先 — 原始 malloc/free 导致内存泄漏
   --------------------------------------------------------- */

static void demo_memory_leak(void)
{
    printf("=== Demo 1: 原始指针的内存泄漏问题 ===\n");

    /* ❌ 典型的泄漏场景: malloc 但忘记 free */
    char *leaky = malloc(128);
    if (leaky == NULL) {
        fprintf(stderr, "malloc failed\n");
        return;
    }
    snprintf(leaky, 128, "I was allocated but never freed!");
    printf("  泄漏指针: leaked buffer = \"%s\"\n", leaky);
    /* 没有 free(leaky)! 每次调用 demo_memory_leak 都泄漏 128 字节 */

    printf("  ❌ 问题: malloc 后忘记 free → 内存泄漏\n");
    printf("  ✅ 修复: 每条执行路径都必须配对 free\n\n");

    /* 正确做法 */
    char *safe = malloc(64);
    if (safe == NULL) return;
    snprintf(safe, 64, "allocated and freed");
    printf("  安全指针: \"%s\"\n", safe);
    free(safe);
    safe = NULL;
    printf("  ✅ free 后置 NULL, 防止悬垂指针\n\n");
}

/* ---------------------------------------------------------
   Demo 2: 工厂模式 (Factory Pattern) — opaque pointer
   --------------------------------------------------------- */

static SmartBuffer_Int *smartbuffer_create(void)
{
    SmartBuffer_Int *buf = malloc(sizeof(SmartBuffer_Int));
    if (buf == NULL) return NULL;

    memset(buf->data, 0, SB_CAPACITY);
    buf->len = 0;
    buf->ref_count = 1;
    return buf;
}

static void smartbuffer_destroy(SmartBuffer_Int *buf)
{
    if (buf == NULL) return;

    buf->ref_count--;
    if (buf->ref_count <= 0) {
        memset(buf->data, 0, SB_CAPACITY); /* 销毁前清零 */
        free(buf);
    }
}

static int smartbuffer_push(SmartBuffer_Int *buf, uint8_t byte)
{
    if (buf == NULL || buf->len >= SB_CAPACITY) return -1;

    buf->data[buf->len] = byte;
    buf->len++;
    return 0;
}

static size_t smartbuffer_len(const SmartBuffer_Int *buf)
{
    if (buf == NULL) return 0;
    return buf->len;
}

static const uint8_t *smartbuffer_data(const SmartBuffer_Int *buf)
{
    if (buf == NULL) return NULL;
    return buf->data;
}

static void demo_factory_pattern(void)
{
    printf("=== Demo 2: 工厂模式 + 不透明指针 ===\n");

    /* 调用者只能看见 SmartBuffer*, 看不见内部结构 */
    SmartBuffer_Int *buf = smartbuffer_create();
    if (buf == NULL) {
        fprintf(stderr, "smartbuffer_create failed\n");
        return;
    }

    const char *msg = "Hello Opaque!";
    for (size_t i = 0; i < strlen(msg); i++) {
        smartbuffer_push(buf, (uint8_t)msg[i]);
    }

    printf("  不透明指针缓冲区:\n");
    printf("    长度 = %zu\n", smartbuffer_len(buf));
    printf("    内容 = \"%.*s\"\n", (int)smartbuffer_len(buf),
           (const char *)smartbuffer_data(buf));

    smartbuffer_destroy(buf);
    printf("  ✅ 工厂创建 → 使用 → 销毁, 调用者从未接触内部结构\n\n");
}

/* ---------------------------------------------------------
   Demo 3: RAII-style 宏 —— 作用域结束时自动清理
   --------------------------------------------------------- */

/* 文件 RAII 宏: 在 for 循环中自动关闭文件 */
#define WITH_FILE(var, path, mode) \
    for (FILE *var = fopen(path, mode); \
         var != NULL; \
         fclose(var), var = NULL)

/* malloc RAII 宏: 离开作用域时自动 free */
#define WITH_MALLOC(ptr, type, count) \
    for (type *ptr = calloc(count, sizeof(type)); \
         ptr != NULL; \
         (free(ptr), ptr = NULL))

static void demo_raii_macros(void)
{
    printf("=== Demo 3: RAII-style 宏 ===\n");

    /* 1. 文件 RAII: for 循环结束时自动 fclose */
    printf("  [文件 RAII]\n");
    WITH_FILE(fp, "/tmp/smart_ptr_test.txt", "w") {
        fprintf(fp, "This file will be auto-closed.\n");
        printf("    写入完成, 文件在 for 作用域结束时自动关闭\n");
    }
    printf("  ✅ 文件已关闭 (无需手动 fclose)\n\n");

    /* 2. 内存 RAII: for 循环结束时自动 free */
    printf("  [内存 RAII]\n");
    WITH_MALLOC(arr, int32_t, 5) {
        arr[0] = 10;
        arr[1] = 20;
        arr[2] = 30;
        printf("    arr[0]=%" PRId32 ", arr[1]=%" PRId32 ", arr[2]=%" PRId32 "\n",
               arr[0], arr[1], arr[2]);
        printf("    数组在 for 作用域结束时自动释放\n");
    }
    printf("  ✅ 内存已释放 (无需手动 free)\n\n");

    /* 解释 RAII 宏的原理 */
    printf("  RAII 宏原理:\n");
    printf("    for (T* p = alloc(); p != NULL; cleanup(p), p = NULL) {\n");
    printf("        // 使用 p\n");
    printf("    }\n");
    printf("    // 退出 for 时, 第三个表达式 (cleanup) 自动执行\n");
    printf("    // 等价于 Rust 的 drop / C++ 的 destructor\n\n");
}

/* ---------------------------------------------------------
   Demo 4: void* 类型擦除 — 通用容器
   --------------------------------------------------------- */

typedef struct {
    void **items;
    size_t  count;
    size_t  capacity;
    void (*free_item)(void *);  /* 自定义释放函数 */
} GenericArray;

static GenericArray *generic_array_create(size_t capacity,
                                           void (*free_fn)(void *))
{
    GenericArray *arr = malloc(sizeof(GenericArray));
    if (arr == NULL) return NULL;

    arr->items = calloc(capacity, sizeof(void *));
    if (arr->items == NULL) {
        free(arr);
        return NULL;
    }
    arr->capacity = capacity;
    arr->count = 0;
    arr->free_item = free_fn;
    return arr;
}

static int generic_array_push(GenericArray *arr, void *item)
{
    if (arr == NULL || item == NULL) return -1;
    if (arr->count >= arr->capacity) return -2;

    arr->items[arr->count] = item;
    arr->count++;
    return 0;
}

static void generic_array_destroy(GenericArray *arr)
{
    if (arr == NULL) return;

    for (size_t i = 0; i < arr->count; i++) {
        if (arr->free_item != NULL) {
            arr->free_item(arr->items[i]);
        }
    }
    free(arr->items);
    free(arr);
}

static void demo_generic_array(void)
{
    printf("=== Demo 4: void* 通用容器 ===\n");

    /* 创建整型通用数组 */
    GenericArray *arr = generic_array_create(4, free);
    if (arr == NULL) return;

    for (int32_t i = 0; i < 4; i++) {
        int32_t *val = malloc(sizeof(int32_t));
        if (val == NULL) break;
        *val = i * 10;
        generic_array_push(arr, val);
    }

    printf("  通用数组 (void* 类型擦除):\n");
    for (size_t i = 0; i < arr->count; i++) {
        int32_t *v = (int32_t *)arr->items[i];
        printf("    [%zu] = %" PRId32 "\n", i, *v);
    }

    generic_array_destroy(arr);
    printf("  ✅ destroy 内部依次调用 free_item 和 free(arr)\n\n");
}

/* ---------------------------------------------------------
   Demo 5: 信息隐藏 (Information Hiding) 对比
   --------------------------------------------------------- */

typedef struct {
    /* 公开结构体: 所有字段都可见 */
    int x;
    int y;
    char label[64];
} PublicPoint;

/* 不透明结构体: 在头文件中只有 typedef, 定义在 .c 中 */
typedef struct SmartBuffer_Internal SmartBuffer_Int;

static void demo_public_vs_opaque(void)
{
    printf("=== Demo 5: 公开结构体 vs 不透明结构体 ===\n");

    /* 公开结构体: 调用者可以直接篡改内部字段 */
    PublicPoint pt = {10, 20, "公开点"};
    printf("  [公开结构体 PublicPoint]\n");
    printf("    pt.x = %d, pt.y = %d\n", pt.x, pt.y);
    pt.x = 999;  /* ❌ 调用者随意修改, 破坏不变量 */
    printf("    篡改后 pt.x = %d (破坏了封装!)\n", pt.x);

    /* 不透明结构体: 调用者无法直接访问内部数据 */
    SmartBuffer_Int *buf = smartbuffer_create();
    if (buf != NULL) {
        printf("\n  [不透明结构体 SmartBuffer*]\n");
        printf("    buf->len = %zu (只能通过函数访问)\n",
               smartbuffer_len(buf));
        /* buf->data[0] = 42; ❌ 编译错误! 看不到内部结构 */
        printf("    调用者无法 buf->data = X —— 编译失败!\n");
        printf("    只能通过 smartbuffer_push/pop 等接口修改\n");
        smartbuffer_destroy(buf);
    }

    printf("\n  对比表:\n");
    printf("  +------------------+------------------+------------------+\n");
    printf("  | 特性              | 公开结构体        | 不透明结构体      |\n");
    printf("  +------------------+------------------+------------------+\n");
    printf("  | 字段可见性       | ✅ 完全可见       | ❌ 不可见         |\n");
    printf("  | 直接修改字段     | ✅ 可以           | ❌ 编译错误       |\n");
    printf("  | 二进制兼容       | ❌ 改结构=ABI破裂  | ✅ 改内部不影响   |\n");
    printf("  | API 稳定性       | 低                | 高               |\n");
    printf("  | 适用场景         | 数据传递 (POD)   | 库/模块 API      |\n");
    printf("  +------------------+------------------+------------------+\n\n");
}

/* ---------------------------------------------------------
   Demo 6: RAII + 错误优先 — 资源安全模式
   --------------------------------------------------------- */

static void demo_raii_safety(void)
{
    printf("=== Demo 6: RAII 安全模式 vs 原始模式 ===\n");

    /* 原始模式: 手动管理资源 */
    printf("  [原始模式 — 手动管理]\n");
    char *buf1 = malloc(64);
    if (buf1 == NULL) return;
    snprintf(buf1, 64, "manual management");
    printf("    创建: \"%s\"\n", buf1);
    free(buf1);
    buf1 = NULL;
    printf("    手动 free, 手动置 NULL\n\n");

    /* RAII 模式: 自动管理资源 */
    printf("  [RAII 模式 — 自动管理]\n");
    WITH_MALLOC(buf2, char, 64) {
        if (buf2 == NULL) return;
        snprintf(buf2, 64, "RAII management");
        printf("    创建: \"%s\"\n", buf2);
        printf("    离开 for 作用域时自动 free\n");
    }  /* buf2 自动释放 */
    printf("  ✅ 无需手动 free\n\n");

    printf("  嵌套 RAII 宏: 同时管理多个资源\n");
    WITH_MALLOC(a, int32_t, 3) {
        WITH_MALLOC(b, int32_t, 3) {
            a[0] = 1;
            b[0] = 2;
            printf("    a[0]=%" PRId32 ", b[0]=%" PRId32 "\n", a[0], b[0]);
            printf("    两个资源在各自 for 结束时都自动释放\n");
        }
    }
    printf("  ✅ 嵌套 RAII 安全释放\n\n");
}

/* ---------------------------------------------------------
   Demo 7: 不透明指针的内存布局
   --------------------------------------------------------- */

static void demo_opaque_layout(void)
{
    printf("=== Demo 7: 不透明指针的内存布局 ===\n");

    SmartBuffer_Int *buf = smartbuffer_create();
    if (buf == NULL) return;

    printf("  调用者视角 (头文件中):\n");
    printf("    typedef struct SmartBuffer_Int SmartBuffer_Int;\n");
    printf("    sizeof(SmartBuffer_Int*) = %zu 字节 (指针大小)\n",
           sizeof(buf));
    printf("    sizeof(SmartBuffer_Int)  = 未知! (在 .c 外编译时)\n");

    printf("\n  实现者视角 (本 .c 文件中):\n");
    printf("    内部结构大小: %zu 字节\n",
           (size_t)sizeof(struct SmartBuffer_Internal));
    printf("    布局: [data:%u 字节][len:%zu 字节][ref_count:%zu 字节]\n",
           (unsigned)SB_CAPACITY, sizeof(size_t), sizeof(int));

    printf("\n  内存图:\n");
    printf("    栈:    buf ─────→ 堆: SmartBuffer_Internal\n");
    printf("    %p           %p\n", (void *)&buf, (void *)buf);
    printf("                     ┌────────────────────────┐\n");
    printf("                     │ data[256] bytes        │\n");
    printf("                     │ len = %zu              │\n",
           smartbuffer_len(buf));
    printf("                     │ ref_count = %d         │\n", 1);
    printf("                     └────────────────────────┘\n");

    smartbuffer_destroy(buf);
    printf("\n");
}

/* ---------------------------------------------------------
   入口函数
   --------------------------------------------------------- */

int main_smart_pointers_sample(void)
{
    printf("========================================\n");
    printf("  smart-pointers 透明指针 & RAII 模式\n");
    printf("========================================\n\n");

    demo_memory_leak();        /* 错误优先: 展示内存泄漏 */
    demo_factory_pattern();    /* 工厂模式 + 不透明指针 */
    demo_raii_macros();        /* RAII-style 宏 */
    demo_generic_array();      /* void* 通用容器 */
    demo_public_vs_opaque();   /* 公开 vs 不透明 */
    demo_raii_safety();        /* RAII 安全对比 */
    demo_opaque_layout();      /* 内存布局可视化 */

    printf("smart-pointers 演示完毕.\n");
    return 0;
}
