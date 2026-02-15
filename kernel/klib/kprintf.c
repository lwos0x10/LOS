#include <klib/kprintf.h>
#include <drivers/vga.h>

#include <stdarg.h>

char* itoa(int value) {
        static char str[64];

        int count = 0;
        int is_neg = 0;
        
        if (value < 0) {
                is_neg = 1;
                value *= -1;
        }
        
        do {
                str[count] = value % 10  + '0';
                value /= 10;
                count++;
        } while(value != 0);

        if (is_neg) {
                str[count] = '-';
                count++;
        }

        str[count] = '\0';

        int i = 0;
        int j = count - 1;
         
        while (i < j) {
                char temp = str[i];
                str[i] = str[j];
                str[j] = temp;
                i++;
                j--;
        }

        return str;
}

char* htoa(unsigned int value) {
    static char str[64];
    int count = 0;
    
    do {
        int digit = value % 16;
        if (digit < 10)
            str[count] = digit + '0';
        else
            str[count] = (digit - 10) + 'A';
        value /= 16;
        count++;
    } while(value != 0);
    
    str[count] = '\0';
    
    int i = 0;
    int j = count - 1;
    
    while (i < j) {
        char temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
    
    return str;
}

void vkprintf(const char *fmt, va_list args) {
        while (*fmt != '\0') {
                if (*fmt == '%') {
                        fmt++;
                        switch (*fmt) {
                                case 'd':
                                        vga_write(itoa(va_arg(args, int)));
                                        break;
                                case 'x':
                                        vga_write(htoa(va_arg(args, unsigned int)));
                                        break;
                                case 's':
                                        vga_write(va_arg(args, char*));
                                        break;
                                case 'c':
                                        vga_putc((char)va_arg(args, int));
                                        break;
                                case '%':
                                        vga_putc('%');
                                        break;
                                default:
                                        vga_putc('%');
                                        vga_putc(*fmt);
                                        break;
                        }
                } 
                else {
                        vga_putc(*fmt);
                }
                fmt++;
        }
}

void kprintf(const char *fmt, ...) {
        va_list args;
        va_start(args, fmt);
        vkprintf(fmt, args);
        va_end(args);
}
