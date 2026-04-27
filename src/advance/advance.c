#include <stdio.h>
#include "advance/advance.h"

int main_advance(void) {
    printf("========================================\n");
    printf("  C 进阶教程 (Advance C Tutorial)\n");
    printf("========================================\n\n");

    printf("--- 错误处理 (Error Handling) ---\n");
    main_error_handling_sample();
    printf("\n");

    printf("--- 原子类型 (Atomic Types) ---\n");
    main_atomic_types_sample();
    printf("\n");

    printf("--- 透明指针 (Smart Pointers) ---\n");
    main_smart_pointers_sample();
    printf("\n");

    printf("--- 异步与线程 (Async & Threads) ---\n");
    main_async_sample();
    printf("\n");

    printf("--- 数据结构遍历 (Iterators) ---\n");
    main_iterators_sample();
    printf("\n");

    printf("--- 高级多态 (Advanced Traits) ---\n");
    main_advanced_traits_sample();
    printf("\n");

    printf("--- 系统调用 (System Calls) ---\n");
    main_system_sample();
    printf("\n");

    printf("--- 测试框架 (Testing) ---\n");
    main_testing_sample();
    printf("\n");

    printf("--- 工具链 (Tools) ---\n");
    main_tools_sample();
    printf("\n");

    printf("--- 数据库 (Database) ---\n");
    main_database_sample();
    printf("\n");

    printf("--- HTTP 服务器 (Web) ---\n");
    main_web_sample();
    printf("\n");

    printf("进阶教程完毕。\n");
    return 0;
}
