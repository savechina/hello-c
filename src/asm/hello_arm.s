; hello_arm.s — macOS (AArch64 / Apple Silicon) 用户态 Hello World
; ============================================================
; 这是本目录下「能在仓库 toolchain 里直接构建运行」的汇编样例:
;   make asm        → 构建 build/bin/asm-hello
;   make asm-run    → 构建并运行 (输出 Hello, asm world!)
;
; 与裸机样例 (main.s / marm.s) 的区别:
;   - 用户态程序入口是 Mach-O 的 _main (经 dyld/C runtime 启动),
;     裸机入口是固件跳转的 _start
;   - 用户态不能执行特权指令 (无 wfi), 也不用手写 svc exit —
;     做法: 调 libc printf → 正常 ret 交还 runtime 由它 exit
;
; 声明: 这是 Apple 汇编方言 (GNU 风格语法 + Mach-O 重定位),
;   分号 ';' 是 Apple as 的注释符 (GNU/Linux gas 用 '@' — 不通用!)

    .section __TEXT,__text
    .global _main
    .align 2

_main:
    ; ⚠ 最经典的坑: AArch64 的 ret 分支目标是 LR (x30), 不读栈!
    ;   下面 bl _printf 会 clobber x30 —— 不先保存它,
    ;   然后 ret 就会永远跳回 LR, 程序无限空转 (真实 bug, 已血泪验证)。
    ;   所以标准帧序 prologue: 压栈保存 x29 (FP) + x30 (LR), 对齐到 16 字节
    stp     x29, x30, [sp, #-16]!       ; push x29,x30; sp -= 16
    mov     x29, sp                     ; 帧指针指向当前帧 (调试/回溯需要)

    ; 1. 调用 printf("Hello, asm world!")
    ;    adrp+add 是 AArch64 的取地址惯用法:
    ;    先取符号所在 4KB 页基址, 再加页内偏移 (PC 相对寻址)
    adrp    x0, hello_str@PAGE
    add     x0, x0, hello_str@PAGEOFF
    bl      _printf                     ; C ABI: 第一个参数在 x0 (clobber x0-x18)

    ; 2. main 返回值: w0 = 0 (C 调用约定用 w0 传 32 位返回值)
    mov     w0, #0
    ldp     x29, x30, [sp], #16         ; epilogue: pop x29,x30 (恢复 LR!); sp += 16
    ret                                 ; 此刻 x30 重新指向真正调用 _main 的返回点 →
                                        ; 交还 dyld/C runtime, 由它 exit

    .section __DATA,__rodata
hello_str:
    .asciz "Hello, asm world!\n"
