#include <cpu/irq.h>
#include <cpu/idt.h>
#include <drivers/pic.h>
#include <klib/kprintf.h>
#include <stddef.h>

static irq_handler_t irq_handlers[16] = { NULL };

void irq_register_handler(int irq, irq_handler_t handler) {
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = handler;
    }
}

void irq_install(void) {
    /* Remap PIC: IRQ 0-7 -> INT 32-39, IRQ 8-15 -> INT 40-47 */
    pic_remap(0x20, 0x28);

    idt_set_gate(32, (uint32_t)irq0,  0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq1,  0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq2,  0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq3,  0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq4,  0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq5,  0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq6,  0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq7,  0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq8,  0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq9,  0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E);
}

void irq_handler(struct registers *r) {
    uint32_t irq_no = r->int_no - 32;

    if (irq_no < 16 && irq_handlers[irq_no] != NULL) {
        irq_handlers[irq_no](r);
    }

    /* Send End-of-Interrupt to PIC */
    pic_send_eoi(irq_no);
}
