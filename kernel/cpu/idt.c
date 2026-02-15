#include <cpu/idt.h>
#include <cpu/isr.h>
#include <cpu/irq.h>

struct idt_entry idt_entries[256];
struct idt_ptr   idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt_entries[num].base_lo = (base & 0xFFFF);
    idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
    idt_entries[num].sel     = sel;
    idt_entries[num].always0 = 0;
    idt_entries[num].flags   = flags;
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt_entries;

    /* Zero out the entire IDT */
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    /* Install the 32 CPU exception handlers */
    isr_install();

    /* Install the 16 IRQ handlers (PIC remapped inside) */
    irq_install();

    /* Load the IDT */
    idt_flush((uint32_t)&idtp);
}

