#include <drivers/keyboard.h>
#include <cpu/irq.h>
#include <klib/kprintf.h>

static inline uint8_t inb(uint16_t port) {
        uint8_t ret;
        __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
}

/* ===== Circular key buffer ===== */

static char buffer[KB_BUFFER_SIZE];
static volatile uint8_t buf_head = 0;
static volatile uint8_t buf_tail = 0;

static void buffer_push(char c) {
        uint8_t next = (buf_head + 1) % KB_BUFFER_SIZE;
        if (next != buf_tail) {
                buffer[buf_head] = c;
                buf_head = next;
        }
}

char keyboard_getchar(void) {
        if (buf_head == buf_tail) {
                return 0;
        }
        char c = buffer[buf_tail];
        buf_tail = (buf_tail + 1) % KB_BUFFER_SIZE;
        return c;
}

/* ===== Modifier state ===== */

static uint8_t shift_held = 0;
static uint8_t caps_on    = 0;

/* ===== US QWERTY scancode set 1 → ASCII ===== */

static const char scancode_normal[128] = {
        0,   27,  '1', '2', '3', '4', '5', '6',  /* 0x00-0x07 */
        '7', '8', '9', '0', '-', '=', '\b', '\t', /* 0x08-0x0F */
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',  /* 0x10-0x17 */
        'o', 'p', '[', ']', '\n', 0,   'a', 's',  /* 0x18-0x1F */
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',  /* 0x20-0x27 */
        '\'', '`', 0,  '\\', 'z', 'x', 'c', 'v', /* 0x28-0x2F */
        'b', 'n', 'm', ',', '.', '/', 0,   '*',   /* 0x30-0x37 */
        0,   ' ', 0,   0,   0,   0,   0,   0,     /* 0x38-0x3F */
        0,   0,   0,   0,   0,   0,   0,   0,     /* 0x40-0x47 */
        0,   0,   0,   0,   0,   0,   0,   0,     /* 0x48-0x4F */
        0,   0,   0,   0,   0,   0,   0,   0,     /* 0x50-0x57 */
        0,   0,   0,   0,   0,   0,   0,   0,     /* 0x58-0x5F */
};

static const char scancode_shifted[128] = {
        0,   27,  '!', '@', '#', '$', '%', '^',    /* 0x00-0x07 */
        '&', '*', '(', ')', '_', '+', '\b', '\t',  /* 0x08-0x0F */
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',  /* 0x10-0x17 */
        'O', 'P', '{', '}', '\n', 0,   'A', 'S',  /* 0x18-0x1F */
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',  /* 0x20-0x27 */
        '"', '~', 0,  '|',  'Z', 'X', 'C', 'V',  /* 0x28-0x2F */
        'B', 'N', 'M', '<', '>', '?', 0,   '*',   /* 0x30-0x37 */
        0,   ' ', 0,   0,   0,   0,   0,   0,     /* 0x38-0x3F */
        0,   0,   0,   0,   0,   0,   0,   0,     /* 0x40-0x47 */
        0,   0,   0,   0,   0,   0,   0,   0,     /* 0x48-0x4F */
        0,   0,   0,   0,   0,   0,   0,   0,     /* 0x50-0x57 */
        0,   0,   0,   0,   0,   0,   0,   0,     /* 0x58-0x5F */
};

/* Scancode constants for modifier keys */
#define SC_LSHIFT_PRESS   0x2A
#define SC_RSHIFT_PRESS   0x36
#define SC_LSHIFT_RELEASE 0xAA
#define SC_RSHIFT_RELEASE 0xB6
#define SC_CAPSLOCK       0x3A

static void keyboard_callback(struct registers *r) {
        (void)r;
        uint8_t scancode = inb(KB_DATA_PORT);

        /* Handle modifier keys */
        if (scancode == SC_LSHIFT_PRESS || scancode == SC_RSHIFT_PRESS) {
                shift_held = 1;
                return;
        }
        if (scancode == SC_LSHIFT_RELEASE || scancode == SC_RSHIFT_RELEASE) {
                shift_held = 0;
                return;
        }
        if (scancode == SC_CAPSLOCK) {
                caps_on = !caps_on;
                return;
        }

        /* Ignore key releases (bit 7 set) */
        if (scancode & 0x80) {
                return;
        }

        /* Look up the character */
        char c;
        if (shift_held) {
                c = scancode_shifted[scancode];
        } else {
                c = scancode_normal[scancode];
        }

        /* Apply caps lock (only affects letters) */
        if (caps_on && c >= 'a' && c <= 'z') {
                c -= 32;
        } else if (caps_on && c >= 'A' && c <= 'Z') {
                c += 32;
        }

        if (c) {
                buffer_push(c);
        }
}

void keyboard_init(void) {
        irq_register_handler(1, keyboard_callback);
}

void keyboard_echo_debug(void) {
        kprintf("Keyboard echo debug mode (press keys to test)\n");
        for (;;) {
                char c = keyboard_getchar();
                if (c) {
                        kprintf("0x%x (%d) (%c)\n", (unsigned char)c, (unsigned char)c, c);
                }
                __asm__ volatile("hlt");
        }
}
