#include <shell/shell.h>
#include <drivers/keyboard.h>
#include <drivers/vga.h>
#include <klib/kprintf.h>
#include <klib/string.h>
#include <tools/debug.h>

#include <cpu/io.h>
#include <mm/pmm.h>
#include <mm/heap.h>

#define INPUT_MAX 256

static char input[INPUT_MAX];

/* ... shell_readline implementation ... */
static void shell_readline(void) {
        int len = 0;       /* Current length of input */
        int cursor = 0;    /* Current cursor position (0 to len) */
        
        memset(input, 0, INPUT_MAX);

        for (;;) {
                char c = keyboard_getchar();
                if (!c) {
                        __asm__ volatile("hlt");
                        continue;
                }

                if ((unsigned char)c == KEY_LEFT) {
                        if (cursor > 0) {
                                cursor--;
                                kprintf("\b");
                        }
                        continue;
                }

                if ((unsigned char)c == KEY_RIGHT) {
                        if (cursor < len) {
                                kprintf("%c", input[cursor]);
                                cursor++;
                        }
                        continue;
                }

                if (c == '\n') {
                        kprintf("\n");
                        input[len] = '\0';
                        return;
                }

                if (c == '\b') {
                        if (cursor > 0) {
                                /* Shift all chars left */
                                for (int i = cursor; i < len; i++) {
                                        input[i - 1] = input[i];
                                }
                                len--;
                                cursor--;
                                input[len] = '\0';

                                /* Backspace visually, rewrite rest of line, clear last char */
                                kprintf("\b");
                                for (int i = cursor; i < len; i++) {
                                        kprintf("%c", input[i]);
                                }
                                kprintf(" "); /* Clear trailing char */
                                
                                /* Move cursor back to correct visual position */
                                for (int i = cursor; i <= len; i++) {
                                        kprintf("\b");
                                }
                        }
                        continue;
                }

                if (len < INPUT_MAX - 1) {
                        /* Shift chars right to make space */
                        for (int i = len; i > cursor; i--) {
                                input[i] = input[i - 1];
                        }
                        input[cursor] = c;
                        len++;
                        
                        /* Print inserted char and rewrite rest of line */
                        kprintf("%c", c);
                        for (int i = cursor + 1; i < len; i++) {
                                kprintf("%c", input[i]);
                        }
                        
                        /* Move cursor back to correct visual position (after the inserted char) */
                        for (int i = len; i > cursor + 1; i--) {
                                kprintf("\b");
                        }
                        
                        cursor++;
                }
        }
}

/* ===== Built-in commands ===== */

static void cmd_help(void) {
        kprintf("Available commands:\n");
        kprintf("  help     - Show this message\n");
        kprintf("  clear    - Clear the screen\n");
        kprintf("  echo     - Print text (usage: echo <text>)\n");
        kprintf("  info     - Show OS info\n");
        kprintf("  debug    - Debugging tools (table, info, alloc, crash)\n");
        kprintf("  reboot   - Reboot the system\n");
        kprintf("  poweroff - Shutdown the system\n");
}

static void cmd_clear(void) {
        vga_clear();
}

static void cmd_info(void) {
        kprintf("LOS - A hobby operating system\n");
        kprintf("Architecture: i386\n");
        kprintf("VGA: 80x25 text mode\n");
        kprintf("Memory: %d MB total, %d KB used\n", 
                pmm_get_total_memory() / 1024 / 1024,
                pmm_get_used_memory() / 1024);
}

static void cmd_reboot(void) {
        kprintf("Rebooting...\n");
        uint8_t good = 0x02;
        while (good & 0x02)
                good = inb(0x64);
        outb(0x64, 0xFE);
        __asm__ volatile("hlt");
}

static void cmd_poweroff(void) {
        kprintf("Shutting down...\n");
        outw(0x604, 0x2000);  /* QEMU */
        outw(0xB004, 0x2000); /* Bochs */
        kprintf("Poweroff failed (ACPI not supported yet).\n");
        __asm__ volatile("cli; hlt");
        for (;;) {}
}

static void cmd_crash(void) {
        kprintf("Crashing... Accessing 0xC0000000\n");
        uint32_t *ptr = (uint32_t*)0xC0000000;
        uint32_t val = *ptr;
        kprintf("Read: 0x%x\n", val);
}

static void cmd_alloc_test(void) {
        kprintf("Testing Heap Allocation...\n");
        
        void *ptr1 = kmalloc(100);
        kprintf("Allocated 100 bytes at 0x%x\n", (uint32_t)ptr1);
        if (ptr1) {
                strcpy((char *)ptr1, "Hello Heap!");
                kprintf("Content: %s\n", (char *)ptr1);
        }
        
        void *ptr2 = kmalloc(50);
        kprintf("Allocated 50 bytes at 0x%x\n", (uint32_t)ptr2);
        
        if (ptr1) kfree(ptr1);
        kprintf("Freed ptr1\n");
        
        void *ptr3 = kmalloc(20);
        kprintf("Allocated 20 bytes at 0x%x (Should reuse ptr1 space)\n", (uint32_t)ptr3);
        
        kfree(ptr2);
        kfree(ptr3);
        kprintf("Freed all.\n");
        
        /* heap_print_info(); needs to be exposed in header if we want to call it here, 
           assuming it's in header as 'void heap_print_info(void);' */
}



static void cmd_debug(const char *args) {
        if (!args) {
                kprintf("Usage: debug <command>\n");
                kprintf("Commands:\n");
                kprintf("  table    - Show heap allocation table\n");
                kprintf("  info     - Show heap summary info\n");
                kprintf("  alloc    - Run heap allocation test\n");
                kprintf("  crash    - Trigger a Page Fault\n");
                return;
        }
        
        if (strcmp(args, "table") == 0) {
                heap_print_table();
        } else if (strcmp(args, "info") == 0) {
                heap_print_info();
        } else if (strcmp(args, "alloc") == 0) {
                cmd_alloc_test();
        } else if (strcmp(args, "crash") == 0) {
                cmd_crash();
        } else {
                kprintf("Unknown debug command: %s\n", args);
        }
}

static void cmd_echo(const char *args) {
        if (args) {
                kprintf("%s\n", args);
        } else {
                kprintf("\n");
        }
}

/* ===== Command dispatch ===== */

static void shell_execute(void) {
        /* Skip leading spaces */
        char *cmd = input;
        while (*cmd == ' ') {
                cmd++;
        }

        /* Empty input */
        if (*cmd == '\0') {
                return;
        }

        /* Find end of command word */
        char *args = cmd;
        while (*args && *args != ' ') {
                args++;
        }

        /* Split command and arguments */
        if (*args) {
                *args = '\0';
                args++;
                /* Skip spaces before arguments */
                while (*args == ' ') {
                        args++;
                }
                if (*args == '\0') {
                        args = (char *)0;
                }
        } else {
                args = (char *)0;
        }

        if (strcmp(cmd, "help") == 0) {
                cmd_help();
        } else if (strcmp(cmd, "clear") == 0) {
                cmd_clear();
        } else if (strcmp(cmd, "info") == 0) {
                cmd_info();
        } else if (strcmp(cmd, "echo") == 0) {
                cmd_echo(args);
        } else if (strcmp(cmd, "reboot") == 0) {
                cmd_reboot();
        } else if (strcmp(cmd, "poweroff") == 0) {
                cmd_poweroff();
        } else if (strcmp(cmd, "debug") == 0) {
                cmd_debug(args);
        } else {
                kprintf("Unknown command: %s\n", cmd);
        }
}

/* ===== Main shell loop ===== */

static void shell_prompt(void) {
        vga_setcolor(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE);
        kprintf("LOS");
        vga_setcolor(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
        kprintf("> ");
        vga_setcolor(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLUE);
}

void shell_run(void) {
        debug_ok("Shell Initialized");
        kprintf("\nType 'help' for available commands.\n\n");
        for (;;) {
                shell_prompt();
                shell_readline();
                shell_execute();
        }
}
