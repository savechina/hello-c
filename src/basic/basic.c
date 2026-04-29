#include <stdio.h>
#include "basic/basic.h"

int main_basic(void) {
    printf("========================================\n");
    printf("  C 基础入门教程 (Basic C Tutorial)\n");
    printf("========================================\n\n");

    /* === 核心概念 (Core Concepts) === */
    printf("--- 变量与表达式 (Variables & Expressions) ---\n");
    main_variables_sample();
    printf("\n");

    printf("--- 数据类型 (Data Types) ---\n");
    main_datatype_sample();
    printf("\n");

    printf("--- 运算符与表达式 (Operators & Expressions) ---\n");
    main_operators_sample();
    printf("\n");

    printf("--- 控制流 (Control Flow) ---\n");
    main_control_flow_sample();
    printf("\n");

    printf("--- 循环 (Loops) ---\n");
    main_loops_sample();
    printf("\n");

    /* functions: 4 sub-chapters */
    printf("--- 函数基础 (Function Basics) ---\n");
    main_functions_basics_sample();
    printf("\n");

    printf("--- 函数作用域 (Function Scope) ---\n");
    main_function_scope_sample();
    printf("\n");

    printf("--- 递归函数 (Recursion) ---\n");
    main_recursion_sample();
    printf("\n");

    printf("--- 可变参数函数 (Variadic Functions) ---\n");
    main_variadic_functions_sample();
    printf("\n");

    printf("--- 数组基础 (Arrays) ---\n");
    main_arrays_sample();
    printf("\n");

    printf("--- 预处理器与宏 (Preprocessor & Macros) ---\n");
    main_preprocessor_sample();
    printf("\n");

    /* [M2 Moved] Conditional compilation is a core preprocessor topic */
    printf("--- 条件编译 (Conditional Compilation) ---\n");
    main_conditional_comp_sample();
    printf("\n");

    /* === 内存与指针 (Memory & Pointers) === */
    printf("--- 指针基础 (Pointer Basics) ---\n");
    main_pointer_basics_sample();
    printf("\n");

    printf("--- 指针运算 (Pointer Arithmetic) ---\n");
    main_pointer_arith_sample();
    printf("\n");

    printf("--- 指针与数组 (Pointers & Arrays) ---\n");
    main_pointers_and_arrays_sample();
    printf("\n");

    printf("--- 指针与函数 (Pointers & Functions) ---\n");
    main_pointers_and_functions_sample();
    printf("\n");

    printf("--- const 正确性 (Const Correctness) ---\n");
    main_const_correctness_sample();
    printf("\n");

    printf("--- 字符串基础 (String Basics) ---\n");
    main_string_basics_sample();
    printf("\n");

    printf("--- 字符串操作 (String Operations) ---\n");
    main_string_operations_sample();
    printf("\n");

    printf("--- 安全字符串 (Safe Strings) ---\n");
    main_safe_strings_sample();
    printf("\n");

    printf("--- 字符串处理 (String Processing) ---\n");
    main_string_processing_sample();
    printf("\n");

    /* [M1 Moved] void* type erasure requires knowledge of strings/arrays first */
    printf("--- void* 类型擦除 (Void Pointers) ---\n");
    main_void_pointers_sample();
    printf("\n");

    /* === 数据结构 (Data Structures) === */
    printf("--- 结构体基础 (Struct Basics) ---\n");
    main_struct_basics_sample();
    printf("\n");

    printf("--- 嵌套结构体 (Nested Structs) ---\n");
    main_nested_structs_sample();
    printf("\n");

    printf("--- 结构体与函数 (Struct Functions) ---\n");
    main_struct_functions_sample();
    printf("\n");

    printf("--- 结构体内存布局 (Struct Memory Layout) ---\n");
    main_struct_memory_layout_sample();
    printf("\n");

    printf("--- 联合体 (Unions) ---\n");
    main_unions_sample();
    printf("\n");

    printf("--- 类型别名 (Typedef) ---\n");
    main_typedef_sample();
    printf("\n");

    printf("--- 枚举与联合体 (Enums) ---\n");
    main_enums_sample();
    printf("\n");

    printf("--- 作用域与生命周期 (Scope & Lifetime) ---\n");
    main_scope_sample();
    printf("\n");

    /* === 高级模式 (Advanced Patterns) === */
    printf("--- 内存管理 (Memory Management) ---\n");
    main_memory_mgmt_sample();
    printf("\n");

    printf("--- 头文件与模块系统 (Headers & Module System) ---\n");
    main_headers_sample();
    printf("\n");

    printf("--- 日志与格式化输出 (Logging & Formatted Output) ---\n");
    main_logging_sample();
    printf("\n");

    printf("--- 调试与错误处理 (Debugging & Error Handling) ---\n");
    main_debugging_sample();
    printf("\n");

    printf("--- 文件 I/O (File I/O) ---\n");
    main_file_io_sample();
    printf("\n");

    printf("--- 函数指针 (Function Pointers) ---\n");
    main_function_pointers_sample();
    printf("\n");

    printf("--- 回调函数与多态 (Callbacks) ---\n");
    main_callbacks_sample();
    printf("\n");

    printf("--- void* 泛型编程 (Generic Programming) ---\n");
    main_void_generic_sample();
    printf("\n");

    printf("--- 位运算与内存操作 (Bit Operations) ---\n");
    main_bit_ops_sample();
    printf("\n");

    printf("--- 命令行参数与 I/O 重定向 (CLI Args) ---\n");
    main_cli_args_sample();
    printf("\n");

    printf("--- 标准库精要 (Standard Library) ---\n");
    main_stdlib_sample();
    printf("\n");

    printf("基本教程完毕。\n");
    return 0;
}
