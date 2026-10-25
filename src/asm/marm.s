; marm.s — ARM64 裸机 (bare-metal) 样例
; ============================================================
; x86 引导扇区 (见 main.s) 的 ARM64 对照版。
;
; 与 x86 的差异 (这是本章教学的对照点):
;   - ARM64 没有固定的引导加载地址 — 起始地址由固件/BootROM 决定,
;     所以不需要 org 0x7C00
;   - 没有 0xAA55 引导签名 / 512 字节扇区约定 — 可执行文件或
;     固件镜像的格式由工具链与启动规范 (如 ARM SystemBoot /
;     AArch64 Runtime ABI) 定义
;
; 这个文件在仓库里作为**只读教学参照**, 不参与 make 构建:
;   - 真正裸机 ARM64 需要交叉工具链 (aarch64-none-elf-as / ld)
;     与 QEMU (`qemu-system-aarch64 -M virt`) 才能验证, 超出本仓
;     toolchain 范围
;   - 注意: 这是 A64 指令集 (64 位裸机), 不能被当成 Linux/macos
;     用户态程序直接执行 — WFI 在用户态是非法特权指令
;
; 逐行讲解:
;   .globl _start     — 暴露入口符号 (裸机由固件跳转到它)
;   wfi               — Wait For Interrupt: 低功耗等待, 类似 x86 HLT
;   b halt_loop       — 无限循环兜底: 若有中断返回, 防止跑飞到未定义区域

.globl _start

_start:
    wfi                           ; Wait For Interrupt — 等待中断, 类似 x86 HLT

halt_loop:
    b halt_loop
