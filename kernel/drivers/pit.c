#include <drivers/pit.h>
#include <cpu/irq.h>
#include <klib/kprintf.h>
#include <tools/debug.h>

static inline void outb(uint16_t port, uint8_t val) {
        __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static uint32_t tick = 0;

static void timer_callback(struct registers *r) {
        (void)r;
        tick++;
}

void pit_init(uint32_t frequency) {
        uint16_t divisor = PIT_FREQ / frequency;

        /* Command: channel 0, lobyte/hibyte, mode 2 (rate generator), binary */
        outb(PIT_CMD, 0x36);

        /* Send divisor low byte then high byte */
        outb(PIT_CHANNEL0, divisor & 0xFF);
        outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);

        /* Register our timer handler on IRQ 0 */
        irq_register_handler(0, timer_callback);
        
        initial_debug("PIT Initialized (%d Hz)", frequency);
}

uint32_t pit_get_ticks(void) {
        return tick;
}
