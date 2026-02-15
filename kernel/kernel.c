#include <drivers/vga.h>
#include <klib/kprintf.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>

void kernel_main(uint32_t mb2_magic, uint32_t mb2_info_ptr) {
        (void)mb2_magic;
        (void)mb2_info_ptr;

        gdt_init();
        idt_init();

        vga_init();
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
        vga_clear();
        vga_write("Welcome to LOS.\n");

        int x = 10 / 0;
        kprintf("%d", x);
        for (;;) { __asm__ volatile("hlt"); }
}
