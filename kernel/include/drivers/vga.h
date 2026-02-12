#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_ROW 25
#define VGA_COLUMN 80

typedef enum {
    VGA_COLOR_BLACK         = 0x0,
    VGA_COLOR_BLUE          = 0x1,
    VGA_COLOR_GREEN         = 0x2,
    VGA_COLOR_CYAN          = 0x3,
    VGA_COLOR_RED           = 0x4,
    VGA_COLOR_MAGENTA       = 0x5,
    VGA_COLOR_BROWN         = 0x6,
    VGA_COLOR_LIGHT_GREY    = 0x7,
    VGA_COLOR_DARK_GREY     = 0x8,
    VGA_COLOR_LIGHT_BLUE    = 0x9,
    VGA_COLOR_LIGHT_GREEN   = 0xA,
    VGA_COLOR_LIGHT_CYAN    = 0xB,
    VGA_COLOR_LIGHT_RED     = 0xC,
    VGA_COLOR_LIGHT_MAGENTA = 0xD,
    VGA_COLOR_LIGHT_BROWN   = 0xE,  // Yellow
    VGA_COLOR_WHITE         = 0xF
} vga_color_t;

void vga_init();
void vga_setcolor(uint8_t fg, uint8_t bg);
uint16_t vga_entry(char c);
void vga_clear();
void vga_putc(char c);
void vga_write(const char* str);

#endif
