/**
 * @file headers_sample.c
 * @brief 头文件与模块系统 (Headers & Module System)
 * 
 * Demonstrates:
 *   1. static vs extern visibility
 *   2. Translation units concept
 *   3. Include guard mechanics (demonstrated via header design)
 *   4. Circular dependency avoidance (forward declarations)
 *   5. Practical header organization (public API vs internal helpers)
 */

#include <stdio.h>
#include <stdint.h>
#include "basic/headers_sample.h"

/* ============================================================
   Section 1: 内部辅助函数 — static linkage (不可见)
   ============================================================ */

/**
 * 这个函数只在当前 .c 文件内可见。
 * 即使其他文件用 extern 声明也无法链接到它。
 * 这就是「隐藏实现细节」的核心手段。
 */
static int internal_helper(int x) {
    /* 仅本文件可用的内部工具函数 */
    return x * x + 2 * x + 1;  /* (x+1)^2 */
}

/**
 * 另一个内部工具：验证输入范围
 */
static int validate_input(int value) {
    if (value < 0 || value > 1000) {
        return 0;  /* invalid */
    }
    return 1;  /* valid */
}

/* ============================================================
   Section 2: 公开接口 — extern linkage（默认）
   ============================================================ */

/**
 * 公开函数：在头文件中声明，其他 .c 文件可通过 #include 调用
 * 
 * 注意：虽然我们没有在其他文件中实际引用它，
 * 但它的可见性是 extern（因为没加 static）。
 */
int compute_result(int n) {
    if (!validate_input(n)) {
        printf("  [compute_result] 输入 %d 超出有效范围 [0-1000]\n", n);
        return -1;
    }
    return internal_helper(n);
}

/* ============================================================
   Section 3: 翻译单元 (Translation Unit) 概念演示
   ============================================================ */

/**
 * 翻译单元 = 一个 .c 文件 + 它 #include 的所有头文件
 * 
 * 每个 .c 文件独立编译为 .o，然后链接器把它们拼到一起。
 * 理解翻译单元是理解 include guard 和多文件编译的基础。
 */
static void demo_translation_unit(void) {
    printf("  [translation unit] 当前文件: %s\n", __FILE__);
    printf("  [translation unit] 编译时间: %s %s\n", __DATE__, __TIME__);
    printf("  [translation unit] 每个 .c = 一个翻译单元\n");
    printf("  [translation unit] 预处理: 展开 #include/#define → 翻译单元\n");
    printf("  [translation unit] 编译:   翻译单元 → .o 目标文件\n");
    printf("  [translation unit] 链接:   多个 .o → 可执行文件\n");
}

/* ============================================================
   Section 4: 前向声明 (Forward Declaration) — 避免循环依赖
   ============================================================ */

/**
 * 前向声明：先告诉编译器有这样一个函数，具体实现在后面
 * 
 * 场景：如果 utils.c 需要调用 core.c 的函数，
 * 而 core.c 又需要调用 utils.c 的函数，就会形成循环依赖。
 * 解决方法：用头文件做前向声明，打破循环。
 */

/* 前向声明：声明但暂不定义 */
void process_data(void);

static void init_config(void) {
    printf("  [forward decl] init_config() 调用 process_data()...\n");
    process_data();  /* 合法：前面已前向声明 */
}

/* 后定义 */
void process_data(void) {
    printf("  [forward decl] process_data() 正在处理数据...\n");
    /* 模拟数据处理 */
    int buffer[3] = {10, 20, 30};
    for (int i = 0; i < 3; i++) {
        printf("    buffer[%d] = %d\n", i, buffer[i]);
    }
}

/* ============================================================
   Section 5: include guard 机制演示
   ============================================================ */

/**
 * include guard 防止头文件被重复包含
 * 
 * 即使你在一个 .c 文件中写了两行：
 *   #include "my.h"
 *   #include "my.h"
 * 
 * 实际内容只会被包含一次，因为第二次遇到时 _H 宏已定义。
 * 
 * 这里我们用实际输出演示 guard 的效果。
 */
static void demo_include_guard(void) {
    printf("  [include guard] 这个演示通过实际编译体现\n");
    printf("  [include guard] 本文件头部已包含 headers_sample.h\n");
    printf("  [include guard] 如果再次 #include \"basic/headers_sample.h\"\n");
    printf("  [include guard] → HEADERS_SAMPLE_H 已定义 → 跳过内容\n");
    printf("  [include guard] → 不会重复声明 main_headers_sample()\n");
    printf("  [include guard]\n");
    printf("  [include guard] #pragma once 是等效的现代写法：\n");
    printf("  [include guard]   #pragma once  ← 一行搞定，但非标准\n");
    printf("  [include guard]   #ifndef ...   ← 传统写法，标准保证\n");
}

/* ============================================================
   Section 6: 头文件组织最佳实践
   ============================================================ */

/**
 * 实践中的头文件组织原则：
 * 
 * 1. 公开 API 声明放在 .h（导出给用户）
 * 2. 内部 helper 用 static 隐藏在 .c 中
 * 3. 数据结构如果只需要指针操作，只需前向声明
 * 4. 常量/宏如果跨文件共享，放 .h；仅本文件用，放 .c
 */
#define MODULE_VERSION 1   /* 模块版本：仅本文件用，所以 #define 在 .c */

static void demo_header_organization(void) {
    printf("  [organization] 本模块版本: %d\n", MODULE_VERSION);
    printf("  [organization] public API: compute_result() → 在 .h 中声明\n");
    printf("  [organization]   internal: internal_helper() → static 在 .c 中\n");
    printf("  [organization]   internal: validate_input()  → static 在 .c 中\n");
    printf("  [organization] \n");
    printf("  [organization] 头文件只暴露「需要什么」，隐藏「怎么实现」。\n");
    printf("  [organization] 这就是 C 语言的「信息隐藏」哲学。\n");
}

/* ============================================================
   公共入口 — main_headers_sample
   ============================================================ */

int main_headers_sample(void) {
    printf("==== 头文件与模块系统 (Headers & Module System) ====\n\n");

    /* Section 1: static vs extern visibility */
    printf("--- static vs extern 可见性 ---\n");
    printf("  internal_helper() 是 static，只能在本文件调用：\n");
    printf("  internal_helper(4) = %d\n", internal_helper(4));
    printf("  compute_result(5) = %d\n", compute_result(5));
    printf("  compute_result(-1) = %d (超出范围检测)\n", compute_result(-1));

    /* Section 2: translation unit */
    printf("\n--- 翻译单元 (Translation Unit) ---\n");
    demo_translation_unit();

    /* Section 3: include guard */
    printf("\n--- 头文件卫士 (Include Guard) ---\n");
    demo_include_guard();

    /* Section 4: forward declaration */
    printf("\n--- 前向声明 (Forward Declaration) ---\n");
    init_config();

    /* Section 5: header organization */
    printf("\n--- 头文件组织最佳实践 ---\n");
    demo_header_organization();

    printf("\nheaders sample done.\n");
    return 0;
}
