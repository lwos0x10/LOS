#ifndef PIT_H
#define PIT_H

#include <stdint.h>

#define PIT_FREQ       1193182
#define PIT_CHANNEL0   0x40
#define PIT_CMD        0x43

void pit_init(uint32_t frequency);
uint32_t pit_get_ticks(void);

#endif
