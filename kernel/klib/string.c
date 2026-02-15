#include <klib/string.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t n) {
        uint8_t *d = (uint8_t *)dest;
        const uint8_t *s = (const uint8_t *)src;

        for (size_t i = 0; i < n; i++) {
                d[i] = s[i];
        }

        return dest;
}

void *memset(void *dest, int val, size_t n) {
        uint8_t *d = (uint8_t *)dest;
        for (size_t i = 0; i < n; i++) {
                d[i] = (uint8_t)val;
        }
        return dest;
}

size_t strlen(const char *s) {
        size_t len = 0;
        while (s[len]) {
                len++;
        }
        return len;
}

int strcmp(const char *s1, const char *s2) {
        while (*s1 && (*s1 == *s2)) {
                s1++;
                s2++;
        }
        return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char *strcpy(char *dest, const char *src) {
        char *ret = dest;
        while ((*dest++ = *src++));
        return ret;
}