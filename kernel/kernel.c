#include <stdint.h>

static uint16_t* VGA = (uint16_t*)0xB8000;

void kernel_main(uint32_t mb2_magic, uint32_t mb2_info_ptr) {
        (void)mb2_magic;
        (void)mb2_info_ptr;
        
        VGA[0] = 0x0F48;


        for (;;) { __asm__ volatile("hlt"); }
}
