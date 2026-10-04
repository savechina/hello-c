#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "advance/raii_sample.h"

/* ============================================================
 * Demo 1: __attribute__((cleanup)) —— 作用域退出自动清理
 * ============================================================
 *
 * 这是 C 里最接近 RAII 的机制。GCC/Clang 扩展（非 ISO C）。
 *
 * 关键点: 属性写在【变量声明】上，不是函数上。
 *   写在函数上会报 -Wignored-attributes。
 *
 * 清理函数收到的是【变量类型的指针】:
 *   变量声明为 void*  → 清理函数必须收 void**
 *   变量声明为 FILE** → 清理函数必须收 FILE***
 */

static void demo1_close_fd(int *fd)
{
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
    printf("    [cleanup] fd 已关闭\n");
}

static void demo1_close_file(FILE **fp)
{
    if (*fp != NULL) {
        fclose(*fp);
        *fp = NULL;
    }
    printf("    [cleanup] FILE* 已关闭\n");
}

static void demo1_free(void **p)
{
    if (*p != NULL) {
        free(*p);
        *p = NULL;
    }
    printf("    [cleanup] 内存已释放\n");
}

/* 提前 return 的场景: 作用域退出时清理依然执行 */
static int demo1_early_return(void)
{
    int fd __attribute__((__cleanup__(demo1_close_fd))) = open("/dev/null", O_RDONLY);
    void *p __attribute__((__cleanup__(demo1_free))) = malloc(64);

    if (fd < 0 || p == NULL) return -1;

    printf("    函数体中间 return，清理仍会触发\n");
    return 0;
}

static void raii_cleanup_attribute_sample(void)
{
    printf("=== Demo 1: __attribute__((cleanup)) ===\n\n");

    printf("  [1. 正常作用域退出 + 逆序析构]\n");
    {
        FILE *f __attribute__((__cleanup__(demo1_close_file))) = fopen("/dev/null", "w");
        void *a __attribute__((__cleanup__(demo1_free))) = malloc(32);
        void *b __attribute__((__cleanup__(demo1_free))) = malloc(32);
        (void)f;
        printf("    声明顺序: f → a → b\n");
    }
    printf("    析构顺序: b → a → f （后声明的先清理）\n\n");

    printf("  [2. 嵌套作用域: 内层先于外层清理]\n");
    {
        void *outer __attribute__((__cleanup__(demo1_free))) = malloc(32);
        {
            void *inner __attribute__((__cleanup__(demo1_free))) = malloc(32);
            (void)outer;
            (void)inner;
            printf("    内层作用域结束\n");
        }
        printf("    外层作用域结束\n");
    }

    printf("\n  [3. 函数中途提前 return]\n");
    int rc = demo1_early_return();
    printf("    demo1_early_return() = %d\n", rc);

    printf("\n  ✅ 对比 src/advance/smart_pointers_sample.c 的 for 循环宏:\n");
    printf("     for 宏遇到 return/break/goto 会跳过清理 → 泄漏\n");
    printf("     cleanup 属性由编译器插入调用 → 不受跳转影响\n\n");

    printf("  ⚠️  两个真实限制:\n");
    printf("     1. exit() / _exit() / longjmp() 绕过栈展开，不触发 cleanup\n");
    printf("     2. 跳进已初始化的作用域会跳过初始化 → 清理函数可能拿到垃圾值\n");
    printf("        （用 -Wjump-misses-init 检查，别用 goto 跨过带 cleanup 的声明）\n\n");
}

/* ============================================================
 * Demo 2: DEFER 宏 —— 基于 cleanup 实现 LIFO 延迟执行
 * ============================================================
 *
 * 标准 C 没有 defer。它目前只是 ISO/DIS TS 25755 草案，
 * 所以下面这个 DEFER 宏是 GNU/Clang 扩展技巧。
 */

struct demo2_defer {
    void (*fn)(void *);
    void *arg;
};

static void demo2_defer_run(struct demo2_defer *d)
{
    d->fn(d->arg);
}

#define demo2_cat_(a, b) a##b
#define demo2_cat(a, b)  demo2_cat_(a, b)

#define defer_to(fn, arg)                                             \
    struct demo2_defer demo2_cat(_demo2_d_, __LINE__)                 \
        __attribute__((__cleanup__(demo2_defer_run))) = { (fn), (arg) }

static void demo2_say(void *s)
{
    printf("    deferred: %s\n", (const char *)s);
}

/* 语句表达式示例用: 变量类型为 int,故清理函数收 int* */
static void demo2_int_cleanup(int *p)
{
    (void)p;
    printf("    语句表达式内的 int 局部变量离开作用域\n");
}

static void raii_defer_sample(void)
{
    printf("=== Demo 2: DEFER 宏 (LIFO) ===\n\n");

    printf("  注册 3 个延迟动作，观察析构顺序:\n");
    {
        defer_to(demo2_say, (void *)"第一个注册");
        defer_to(demo2_say, (void *)"第二个注册");
        defer_to(demo2_say, (void *)"第三个注册");
        printf("    作用域即将结束\n");
    }
    printf("    ↑ 紧跟上面的三行即「第三 → 第二 → 第一个」（后进先出）\n\n");

    printf("  语句表达式（GNU 扩展）也能做作用域级清理:\n");
    int v = ({ int tmp __attribute__((__cleanup__(demo2_int_cleanup))) = 7; tmp + 1; });
    printf("    语句表达式返回值 = %d\n\n", v);
}

/* ============================================================
 * Demo 3: Arena / Region 分配器 —— 一次分配, 一次释放
 * ============================================================
 *
 * 收益: 把 N 次 free()（每次都是 double-free / UAF 的机会）
 *压缩成 1 次生命周期决策。编译器、解释器、游戏引擎常用。
 */

static size_t demo3_align_up(size_t v, size_t a)
{
    return (v + a - 1u) & ~(a - 1u);
}

bool raii_arena_init(arena_t *a, size_t cap)
{
    a->base = malloc(cap);
    if (a->base == NULL) {
        /* 初始化失败时返回全零对象: 失败路径同样置零 cap/used，
         * 调用方拿到的是 zeroed object 而不是 indeterminate 字段。 */
        a->cap = 0;
        a->used = 0;
        return false;
    }
    a->cap = cap;
    a->used = 0;
    return true;
}

/* 真正防住 SIZE_MAX 这类巨大请求的是上面的 n > a->cap；
 * 写成 off > a->cap - n 只是更保险的习惯（两者在此处等价，因为 n <= a->cap 且 off <= a->cap）。 */
void *raii_arena_alloc(arena_t *a, size_t n)
{
    size_t off = demo3_align_up(a->used, _Alignof(max_align_t));
    if (n > a->cap || off > a->cap - n) return NULL;
    void *p = a->base + off;
    a->used = off + n;
    return p;
}

void raii_arena_destroy(arena_t *a)
{
    free(a->base);
    a->base = NULL;
    a->used = 0;
    a->cap = 0;
}

typedef struct {
    int32_t id;
    char    name[16];
} demo3_rec_t;

static void raii_arena_sample(void)
{
    printf("=== Demo 3: Arena / Region 分配器 ===\n\n");

    arena_t arena;
    if (!raii_arena_init(&arena, 512)) {
        printf("  arena 初始化失败\n\n");
        return;
    }

    demo3_rec_t *rec = raii_arena_alloc(&arena, sizeof(*rec));
    int32_t     *nums = raii_arena_alloc(&arena, 3 * sizeof(*nums));
    char        *msg  = raii_arena_alloc(&arena, 32);

    if (rec == NULL || nums == NULL || msg == NULL) {
        printf("  arena 分配失败\n");
        raii_arena_destroy(&arena);
        return;
    }

    rec->id = 7;
    snprintf(rec->name, sizeof(rec->name), "record-%d", (int)rec->id);
    nums[0] = 10; nums[1] = 20; nums[2] = 30;
    snprintf(msg, 32, "%s", "hello from arena");

    printf("  rec  = %s (id=%" PRId32 ")\n", rec->name, rec->id);
    printf("  nums = [%" PRId32 ", %" PRId32 ", %" PRId32 "]\n",
           nums[0], nums[1], nums[2]);
    printf("  msg  = \"%s\"\n", msg);
    printf("  地址连续性: rec=%p nums=%p msg=%p\n",
           (void *)rec, (void *)nums, (void *)msg);
    printf("  已用 %zu / %zu 字节\n\n", arena.used, arena.cap);

    /* O(1) 释放全部 —— 一次调用, 不需要逐个 free */
    raii_arena_destroy(&arena);
    printf("  ✅ 一次 raii_arena_destroy() 释放了全部 3 个对象\n");
    printf("     （对比逐个 free: 每次 free 都是一个出错机会）\n\n");

    printf("  ⚠️  两个经典坑:\n");
    printf("     1. 无法单独释放某个对象 → 生命周期不同的数据要分两个 arena\n");
    printf("     2. reset/销毁会让所有指向 arena 内部的指针失效，必须整体一起丢弃\n\n");
}

/* ============================================================
 * Demo 4: Option<T> —— 用 tagged union 编码「可能没有值」
 * ============================================================
 *
 * 这才是「Rust 风格 datatype 在 C 里」真正对应的东西。
 * 但要说清: 它【不是】编译期强制的，忘记检查 tag 依然是合法 C。
 */

_Static_assert(sizeof(raii_opt_int_t) == 8, "Option<int> 布局变了");

raii_opt_int_t raii_some(int v)
{
    raii_opt_int_t o = { .tag = RAII_SOME, .value = v };
    return o;
}

raii_opt_int_t raii_none(void)
{
    raii_opt_int_t o = { .tag = RAII_NONE, .value = 0 };
    return o;
}

/* 返回 bool 强迫调用者处理「空」的情况 */
bool raii_value(raii_opt_int_t o, int *out)
{
    if (o.tag != RAII_SOME) return false;
    *out = o.value;
    return true;
}

static void raii_option_sample(void)
{
    printf("=== Demo 4: Option<T> (tagged union) ===\n\n");

    raii_opt_int_t some = raii_some(42);
    raii_opt_int_t none = raii_none();

    int v = 0;
    if (raii_value(some, &v)) {
        printf("  raii_some(42)  → 有值: %d\n", v);
    }
    if (raii_value(none, &v)) {
        printf("  raii_none()    → 有值: %d\n", v);
    } else {
        printf("  raii_none()    → 无值（tag == RAII_NONE，调用方必须分支处理）\n");
    }

    printf("\n  对比: 用 -1 / NULL 当哨兵值表示「失败」，调用方容易忘查\n");
    printf("       Option 把「可能没有值」编码进类型，漏检就是逻辑错误\n\n");
    printf("  ⚠️  但这【不是】编译期检查: 忘记判断 tag 依然能编译通过。\n");
    printf("     与 Rust 的 Option<T> 不同，C 没有借用检查也没有穷尽性检查。\n\n");
}

/* ============================================================
 * Demo 5: 定容缓冲区 —— 把容量写进类型
 * ============================================================
 *
 * 溢出在结构上不可能发生: append 里的边界判断用的是 sizeof，
 * 不是调用方传进来的长度。
 */

void raii_buf_init(raii_buf_t *b)
{
    b->len = 0;
}

bool raii_buf_append(raii_buf_t *b, const void *p, size_t n)
{
    if (n > sizeof b->data - b->len) return false;
    memcpy(b->data + b->len, p, n);
    b->len += n;
    return true;
}

bool raii_buf_append_str(raii_buf_t *b, const char *s)
{
    return raii_buf_append(b, s, strlen(s));
}

size_t raii_buf_len(const raii_buf_t *b)
{
    return b->len;
}

static void raii_fixed_buffer_sample(void)
{
    printf("=== Demo 5: 定容缓冲区 (Buf%d) ===\n\n", RAII_BUF_CAP);

    raii_buf_t buf;
    raii_buf_init(&buf);

    bool ok = true;
    ok = ok && raii_buf_append_str(&buf, "Hello");
    ok = ok && raii_buf_append_str(&buf, ", ");
    ok = ok && raii_buf_append_str(&buf, "world");

    printf("  追加 \"Hello, world\": ok=%s, len=%zu\n", ok ? "true" : "false",
           raii_buf_len(&buf));

    /* 故意溢出: 必须被拒绝, 且 len 不变 */
    size_t len_before = raii_buf_len(&buf);
    bool overflow_ok = raii_buf_append_str(&buf,
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789");
    printf("  追加 123 字节: 返回 %s（预期 false）\n",
           overflow_ok ? "true" : "false");
    printf("  len 是否被破坏: %zu → %zu (%s)\n", len_before, raii_buf_len(&buf),
           len_before == raii_buf_len(&buf) ? "未被破坏" : "已损坏");

    printf("\n  ✅ 容量是类型的一部分，append 里用 sizeof 判断，结构上无法越界\n");
    printf("  📖 边界外的调用方: src/basic/logging_sample.c 的 sprintf 反例\n\n");
}

/* ============================================================
 * Coordinator
 * ============================================================ */

int main_raii_sample(void)
{
    printf("========================================\n");
    printf("  C 内存安全工程 (Memory Safety Engineering)\n");
    printf("========================================\n\n");

    raii_cleanup_attribute_sample();
    raii_defer_sample();
    raii_arena_sample();
    raii_option_sample();
    raii_fixed_buffer_sample();

    printf("========================================\n");
    printf("  内存安全工程演示完毕。\n");
    printf("========================================\n\n");
    return 0;
}
