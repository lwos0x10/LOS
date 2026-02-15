#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PMM_BLOCK_SIZE 4096

void pmm_init(uint32_t mb2_info_addr);
void *pmm_alloc_block(void);
void pmm_free_block(void *p);
size_t pmm_get_total_memory(void);
size_t pmm_get_used_memory(void);

#endif
