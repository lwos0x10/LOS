#include <tools/debug.h>
#include <drivers/vga.h>
#include <klib/kprintf.h>
#include <stdarg.h>

void debug_ok(const char *fmt, ...) {
        va_list args;
        
        vga_setcolor(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE);
        kprintf("[OK] ");
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
        
        va_start(args, fmt);
        vkprintf(fmt, args);
        va_end(args);
        
        kprintf("\n");
}

void debug_err(const char *fmt, ...) {
        va_list args;
        
        vga_setcolor(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLUE);
        kprintf("[ERROR] ");
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
        
        va_start(args, fmt);
        vkprintf(fmt, args);
        va_end(args);
        
        kprintf("\n");
}
