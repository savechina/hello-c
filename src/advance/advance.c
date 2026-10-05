#include <stdio.h>
#include "advance/advance.h"

int main_advance_sample(void) {
    printf("========================================\n");
    printf("  C 进阶教程 (Advance C Tutorial)\n");
    printf("========================================\n\n");

    printf("--- 错误处理 (Error Handling) ---\n");
    main_error_handling_sample();
    printf("\n");

    /* volatile ≠ atomic — prelude: runs first to set the contract before atomics */
    printf("--- volatile (易变限定符) ---\n");
    main_volatile_sample();
    printf("\n");

    printf("--- 原子类型 (Atomic Types) ---\n");
    main_atomic_types_sample();
    printf("\n");

    printf("--- 透明指针 (Smart Pointers) ---\n");
    main_smart_pointers_sample();
    printf("\n");

    printf("--- 异步与线程 (Async & Threads) ---\n");
    main_async_thread_sample();
    printf("\n");

    printf("--- 同步原语 (Sync Primitives) ---\n");
    main_async_sync_sample();
    printf("\n");

    printf("--- 线程池 (Thread Pool) ---\n");
    main_async_pool_sample();
    printf("\n");

    printf("--- I/O 多路复用 (I/O Multiplexing) ---\n");
    main_async_iomux_sample();
    printf("\n");

    printf("--- 系统编程 (System Programming) ---\n");
    main_system_file_sample();
    printf("\n");

    printf("--- 信号与异步事件 (Signals) ---\n");
    main_system_signal_sample();
    printf("\n");

    printf("--- 内存映射 (Memory Mapping) ---\n");
    main_system_mmap_sample();
    printf("\n");

    printf("--- 进程管理 (Processes) ---\n");
    main_system_process_sample();
    printf("\n");

    printf("--- 进程间通信 (IPC) ---\n");
    main_system_ipc_sample();
    printf("\n");

    printf("--- 命令行工具 (CLI Tools) ---\n");
    main_system_cli_sample();
    printf("\n");

    printf("--- 数据结构遍历 (Iterators) ---\n");
    main_iterators_sample();
    printf("\n");

    printf("--- 高级多态 (Advanced Traits) ---\n");
    main_advanced_traits_sample();
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

    printf("--- HTTP 服务器 - Socket 与 HTTP 协议 ---\n");
    main_web_socket_sample();
    printf("\n");

    printf("--- HTTP 服务器 - 并发模型 ---\n");
    main_web_concurrent_sample();
    printf("\n");

    printf("--- 严格别名与有效类型 (Strict Aliasing & Effective Type) ---\n");
    main_strict_aliasing_sample();
    printf("\n");

    printf("--- restrict 指针契约 (Pointer Qualifier Contract) ---\n");
    main_restrict_sample();
    printf("\n");

    printf("--- 内存安全工程 (Memory Safety Engineering) ---\n");
    main_raii_sample();
    printf("\n");

    printf("进阶教程完毕。\n");
    return 0;
}
