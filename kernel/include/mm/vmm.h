#ifndef VMM_H
#define VMM_H

#include <stdint.h>

/* Page Table Entry / Directory Entry Flags */
#define PAGE_PRESENT    0x1
#define PAGE_RW         0x2
#define PAGE_USER       0x4
#define PAGE_WRITE_THROUGH 0x8
#define PAGE_CACHE_DISABLE 0x10
#define PAGE_ACCESSED   0x20
#define PAGE_DIRTY      0x40
#define PAGE_SIZE_4MB   0x80

/* Constants */
#define ENTRIES_PER_PT  1024
#define ENTRIES_PER_PD  1024
#define PAGE_SIZE       4096

/* Initialize Virtual Memory Manager */
void vmm_init(void);

/* Map a virtual page to a physical frame */
void vmm_map_page(uint32_t phys, uint32_t virt, uint32_t flags);

#endif
