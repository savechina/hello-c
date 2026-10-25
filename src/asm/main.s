; main.s — x86 实模式引导扇区样例 (Boot Sector, NASM 语法)
; ============================================================
; 这是「裸机 (bare-metal)」教学样例: 没有 libc, 没有操作系统,
; 它是 BIOS 可启动的 512 字节引导扇区镜像。
;
; 这个文件**不是**在本仓库 Makefile/clang toolchain 中构建的:
;   它需要 NASM (Intel 语法, `nasm -f bin main.s -o boot.img`)
;   并用 QEMU 验证:  `qemu-system-i386 -drive format=raw,file=boot.img`
; 仓库的 toolchain (Apple clang, Mach-O) 无法产出这种扁平二进制。
;
; 逐行讲解:
;   org 0x7C00        — BIOS 把引导扇区加载到 0x7C00, NASM 据此算绝对地址
;   bits 16           — 实模式 16 位 (CPU 复位后的初始状态)
;   hlt               — 停机指令: 等 NMI/复位, 立即号之后永不返回
;   times 510-($-$$)  — 用 0 填充到第 510 字节 ($$=段起始, $=当前)
;   dw 0xAA55         — 第 511/512 字节的引导签名, BIOS 校验通过才启动

org 0x7C00
bits 16

main:
        hlt                      ; 停机 — 本样例的"Hello World"就是「活着到达这里」

.halt:
        jmp .halt                ; 保险: 万一 hlt 被唤醒 (如 NMI), 无限循环兜底

times 510-($-$$) db 0           ; 填充到 510 字节
dw 0AA55h                        ; 引导签名 0xAA55 (小端在磁盘上表现为 55 AA)
