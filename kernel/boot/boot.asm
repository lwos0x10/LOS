; multiboot2_header.asm
BITS 32
SECTION .multiboot
align 8

MB2_MAGIC    equ 0xE85250D6
MB2_ARCH     equ 0          ; 0 = i386
MB2_HDR_LEN  equ (mb2_hdr_end - mb2_hdr_start)
MB2_CHECKSUM equ -(MB2_MAGIC + MB2_ARCH + MB2_HDR_LEN)

global mb2_hdr_start
mb2_hdr_start:
    dd MB2_MAGIC
    dd MB2_ARCH
    dd MB2_HDR_LEN
    dd MB2_CHECKSUM

    dw 0
    dw 0
    dd 8
mb2_hdr_end:
