#ifndef KPRINT_F
#define KPRINT_F

#include <stdarg.h>

void vkprintf(const char *fmt, va_list args);
void kprintf(const char *fmt, ...);

#endif
