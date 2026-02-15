#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KB_DATA_PORT    0x60
#define KB_STATUS_PORT  0x64

#define KB_BUFFER_SIZE  256

void keyboard_init(void);
char keyboard_getchar(void);

/* Special keys */
#define KEY_UP    0x80
#define KEY_DOWN  0x81
#define KEY_LEFT  0x82
#define KEY_RIGHT 0x83

void keyboard_echo_debug(void);

#endif
