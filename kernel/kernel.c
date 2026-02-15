#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <drivers/keyboard.h>
#include <drivers/pit.h>
#include <drivers/vga.h>
#include <klib/kprintf.h>
#include <shell/shell.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <tools/debug.h>

void kernel_main(uint32_t mb2_magic, uint32_t mb2_info_ptr) {
        (void)mb2_magic;
        (void)mb2_info_ptr;

        vga_init();
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
        vga_clear();
        vga_write("Welcome to LOS.\n\n");

        gdt_init();
        idt_init();

        pit_init(100);

        keyboard_init();

        pmm_init(mb2_info_ptr);
        
        /* Initialize Heap at 4MB, size 1MB */
        heap_init((void *)0x400000, 1024 * 1024);

        /* Enable hardware interrupts */
        __asm__ volatile("sti");

        shell_run();
}