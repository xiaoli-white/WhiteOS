.section .boot, "ax", @progbits
.global _start
.code64

_start:
    cld
    cli

    lea stack_top(%rip), %rsp

    call kernel_main

halt:
    hlt
    jmp halt

.section .bss, "aw", @nobits
.align 16
.skip 16384
stack_top: