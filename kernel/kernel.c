#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <drivers/keyboard.h>
#include <drivers/pit.h>
#include <drivers/vga.h>
#include <klib/kprintf.h>

void kernel_main(uint32_t mb2_magic, uint32_t mb2_info_ptr) {
        (void)mb2_magic;
        (void)mb2_info_ptr;

        gdt_init();
        idt_init();

        vga_init();
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
        vga_clear();
        vga_write("Welcome to LOS.\n\n");

        vga_setcolor(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE);
        kprintf("[OK] ");
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
        kprintf("GDT Initialized\n");

        vga_setcolor(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE);
        kprintf("[OK] ");
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
        kprintf("IDT Initialized\n");

        pit_init(100);
        vga_setcolor(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE);
        kprintf("[OK] ");
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
        kprintf("PIT Initialized (100 Hz)\n");

        keyboard_init();
        vga_setcolor(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE);
        kprintf("[OK] ");
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
        kprintf("Keyboard Initialized\n");

        /* Enable hardware interrupts */
        __asm__ volatile("sti");

        keyboard_echo_debug();
}