BITS 32
SECTION .text
global _start
extern kernel_main

_start:
    push ebx
    push eax
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
