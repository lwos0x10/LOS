; ============================================================
; ISR stubs and IDT flush — NASM, 32-bit
; ============================================================

[BITS 32]

; ------ IDT flush (loads the IDTR) ------
[GLOBAL idt_flush]
idt_flush:
    mov eax, [esp+4]
    lidt [eax]
    ret

; ------ Macros for ISR stubs ------
%macro ISR_NOERRCODE 1
[GLOBAL isr%1]
isr%1:
    push dword 0        ; dummy error code
    push dword %1       ; interrupt number
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
[GLOBAL isr%1]
isr%1:
    ; CPU already pushed the error code
    push dword %1       ; interrupt number
    jmp isr_common_stub
%endmacro

; ------ 32 CPU exception stubs ------
ISR_NOERRCODE 0    ; Division By Zero
ISR_NOERRCODE 1    ; Debug
ISR_NOERRCODE 2    ; Non Maskable Interrupt
ISR_NOERRCODE 3    ; Breakpoint
ISR_NOERRCODE 4    ; Overflow
ISR_NOERRCODE 5    ; Bound Range Exceeded
ISR_NOERRCODE 6    ; Invalid Opcode
ISR_NOERRCODE 7    ; Device Not Available
ISR_ERRCODE   8    ; Double Fault
ISR_NOERRCODE 9    ; Coprocessor Segment Overrun
ISR_ERRCODE   10   ; Invalid TSS
ISR_ERRCODE   11   ; Segment Not Present
ISR_ERRCODE   12   ; Stack-Segment Fault
ISR_ERRCODE   13   ; General Protection Fault
ISR_ERRCODE   14   ; Page Fault
ISR_NOERRCODE 15   ; Reserved
ISR_NOERRCODE 16   ; x87 Floating-Point Exception
ISR_ERRCODE   17   ; Alignment Check
ISR_NOERRCODE 18   ; Machine Check
ISR_NOERRCODE 19   ; SIMD Floating-Point Exception
ISR_NOERRCODE 20   ; Virtualization Exception
ISR_ERRCODE   21   ; Control Protection Exception
ISR_NOERRCODE 22   ; Reserved
ISR_NOERRCODE 23   ; Reserved
ISR_NOERRCODE 24   ; Reserved
ISR_NOERRCODE 25   ; Reserved
ISR_NOERRCODE 26   ; Reserved
ISR_NOERRCODE 27   ; Reserved
ISR_NOERRCODE 28   ; Reserved
ISR_ERRCODE   29   ; VMM Communication Exception
ISR_ERRCODE   30   ; Security Exception
ISR_NOERRCODE 31   ; Reserved

; ------ Common stub ------
[EXTERN isr_handler]

isr_common_stub:
    pusha               ; push edi, esi, ebp, esp, ebx, edx, ecx, eax

    mov ax, ds
    push eax            ; save data segment descriptor

    mov ax, 0x10        ; load kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp            ; pass pointer to registers struct
    call isr_handler
    add esp, 4          ; pop argument

    pop eax             ; restore original data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa                ; pop edi, esi, ebp, esp, ebx, edx, ecx, eax
    add esp, 8          ; pop int_no and err_code
    iret                ; return from interrupt
