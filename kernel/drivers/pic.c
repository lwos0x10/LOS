#include <drivers/pic.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

void pic_remap(uint8_t offset1, uint8_t offset2) {
    uint8_t mask1, mask2;

    /* Save existing masks */
    mask1 = inb(PIC1_DATA);
    mask2 = inb(PIC2_DATA);

    /* ICW1: begin initialization sequence (cascade mode, ICW4 needed) */
    outb(PIC1_CMD, 0x11);
    io_wait();
    outb(PIC2_CMD, 0x11);
    io_wait();

    /* ICW2: set vector offsets */
    outb(PIC1_DATA, offset1);  /* Master PIC: IRQ 0-7  -> offset1..offset1+7 */
    io_wait();
    outb(PIC2_DATA, offset2);  /* Slave PIC:  IRQ 8-15 -> offset2..offset2+7 */
    io_wait();

    /* ICW3: tell Master that Slave is on IRQ2 (0x04), tell Slave its cascade identity (0x02) */
    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    /* ICW4: 8086/88 mode */
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    /* Restore saved masks */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);
    }
    outb(PIC1_CMD, PIC_EOI);
}
