#include <drivers/vga.h>
#include <klib/string.h>
#include <stddef.h>

static uint16_t* VGA = (uint16_t*)0xB8000;

uint8_t ROW;
uint8_t COLUMN;
uint8_t COLOR;

void vga_setcolor(uint8_t fg, uint8_t bg) {
        COLOR = fg | (bg << 4);
}

uint16_t vga_entry(char c) {
        return (uint16_t)c | (uint16_t)(COLOR << 8);
}

void vga_clear() {
        for (int i = 0; i < VGA_ROW * VGA_COLUMN; i++) {
                VGA[i] = vga_entry(' ');
        }
}

void vga_init() {
        ROW = 0;
        COLUMN = 0;
        COLOR = 0x0F;
        vga_clear();
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
                return;
        }

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

void vga_write(const char* str) {
        while (*str) {
                vga_putc(*str);
                str++;
        }
}
