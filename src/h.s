# hello:
    .section __TEXT,__text
    .global _main
    .align 2

_main:
        sub     sp, sp, #16             ; Allocate 16 bytes on the stack

        adrp    x0, hello_str@PAGE      ; Load page address of "Hello World"
        add     x0, x0, hello_str@PAGEOFF ; Add offset to get the full address

        bl      _printf                 ; Call the printf function

        mov     w0, #0                  ; Set exit code to 0

        ; Call the exit system call
        mov     x16, #1                 ; Syscall number for exit (macOS ARM64)
        svc     #0x80                  ; Invoke system call

        add     sp, sp, #16             ; Deallocate stack (this might not be reached)
        ret                             ; Return (this might not be reached)

        .section __DATA,__rodata
hello_str:
        .asciz "Hello World\n"
