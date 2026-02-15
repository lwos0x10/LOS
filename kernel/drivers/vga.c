#include <drivers/vga.h>
#include <klib/string.h>
#include <stddef.h>

static uint16_t* VGA = (uint16_t*)0xB8000;

uint8_t ROW;
uint8_t COLUMN;
uint8_t COLOR;

static inline void outb(uint16_t port, uint8_t val) {
        __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
        uint8_t ret;
        __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

void vga_setcolor(uint8_t fg, uint8_t bg) {
        COLOR = fg | (bg << 4);
}

uint16_t vga_entry(char c) {
        return (uint16_t)c | (uint16_t)(COLOR << 8);
}

void vga_update_cursor(int x, int y) {
        uint16_t pos = y * VGA_COLUMN + x;

        outb(0x3D4, 0x0F);
        outb(0x3D5, (uint8_t)(pos & 0xFF));
        outb(0x3D4, 0x0E);
        outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_enable_cursor(uint8_t start, uint8_t end) {
        outb(0x3D4, 0x0A);
        outb(0x3D5, (inb(0x3D5) & 0xC0) | start);

        outb(0x3D4, 0x0B);
        outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}

void vga_clear() {
        for (int i = 0; i < VGA_ROW * VGA_COLUMN; i++) {
                VGA[i] = vga_entry(' ');
        }
        ROW = 0;
        COLUMN = 0;
        vga_update_cursor(COLUMN, ROW);
}

void vga_init() {
        ROW = 0;
        COLUMN = 0;
        COLOR = 0x0F;
        vga_clear();
        vga_enable_cursor(14, 15); /* Scanlines 14-15 = underscore cursor */
}

static void vga_scroll(void) {
        /* Move every row up by one */
        for (int i = 0; i < (VGA_ROW - 1) * VGA_COLUMN; i++) {
                VGA[i] = VGA[i + VGA_COLUMN];
        }
        /* Clear the last row */
        for (int i = (VGA_ROW - 1) * VGA_COLUMN; i < VGA_ROW * VGA_COLUMN; i++) {
                VGA[i] = vga_entry(' ');
        }
        ROW = VGA_ROW - 1;
}

void vga_putc(char c) {
        if (c == '\n') {
                COLUMN = 0;
                ROW++;
                if (ROW >= VGA_ROW) {
                        vga_scroll();
                }
        } else if (c == '\b') {
                if (COLUMN > 0) {
                        COLUMN--;
                } else if (ROW > 0) {
                        ROW--;
                        COLUMN = VGA_COLUMN - 1;
                }
        } else {
                const size_t index = ROW * VGA_COLUMN + COLUMN;
                VGA[index] = vga_entry(c);
                COLUMN++;

                if (COLUMN >= VGA_COLUMN) {
                        COLUMN = 0;
                        ROW++;
                        if (ROW >= VGA_ROW) {
                                vga_scroll();
                        }
                }
        }
        
        vga_update_cursor(COLUMN, ROW);
}

void vga_write(const char* str) {
        while (*str) {
                vga_putc(*str);
                str++;
        }
}
