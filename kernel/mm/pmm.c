#include <mm/pmm.h>
#include <boot/multiboot2.h>
#include <klib/kprintf.h>
#include <klib/string.h>
#include <tools/debug.h>



/* Symbols from linker script */
extern uint8_t _kernel_start[];
extern uint8_t _kernel_end[];

/* Bitmap logic */
#define BLOCKS_PER_BYTE 8

/* 4GB address space / 4KB blocks = 1,048,576 blocks */
/* Bitmap size = 1,048,576 / 8 = 131,072 bytes (128KB) */
#define MAX_BLOCKS 1048576
#define BITMAP_SIZE (MAX_BLOCKS / BLOCKS_PER_BYTE)

static uint8_t *bitmap = (uint8_t *)_kernel_end; /* Place bitmap after kernel */
static uint32_t total_blocks = 0;
static uint32_t used_blocks = 0;
static uint32_t total_memory_bytes = 0;

static void mmap_set(uint32_t bit) {
        bitmap[bit / 8] |= (1 << (bit % 8));
}

static void mmap_unset(uint32_t bit) {
        bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static int mmap_first_free(void) {
        for (uint32_t i = 0; i < total_blocks / 32; i++) {
                uint32_t *ptr = (uint32_t *)bitmap;
                if (ptr[i] != 0xFFFFFFFF) {
                        for (int j = 0; j < 32; j++) {
                                int bit = 1 << j;
                                if (!(ptr[i] & bit)) {
                                        return i * 32 + j;
                                }
                        }
                }
        }
        return -1;
}

void pmm_init(uint32_t mb2_info_addr) {
        struct multiboot_tag *tag;
        
        /* The Multiboot info structure handles memory map, so we must not overwrite it!
           GRUB loads it right after the kernel. Our bitmap was overwriting it.
           We must place the bitmap AFTER the Multiboot info structure.
        */
        uint32_t mb2_size = *(uint32_t *)mb2_info_addr;
        /* Calculate end of Multiboot info structure */
        uint32_t mb2_end = mb2_info_addr + mb2_size;
        
        /* Align up to next page boundary (4KB) */
        if (mb2_end % PMM_BLOCK_SIZE != 0) {
                mb2_end = (mb2_end + PMM_BLOCK_SIZE) & ~(PMM_BLOCK_SIZE - 1);
        }
        
        /* Ensure we don't start before _kernel_end either (in case mb2 is somewhere else) */
        if (mb2_end < (uint32_t)_kernel_end) {
                mb2_end = (uint32_t)_kernel_end;
        }

        /* Set bitmap to start at the calculated safe location */
        bitmap = (uint8_t *)mb2_end;
        
        /* Initialize bitmap: mark everything as used first */
        memset(bitmap, 0xFF, BITMAP_SIZE);
        
        tag = (struct multiboot_tag *)(mb2_info_addr + 8);
        
        while (tag->type != MULTIBOOT_TAG_TYPE_END) {
                if (tag->type == MULTIBOOT_TAG_TYPE_MMAP) {
                        struct multiboot_tag_mmap *mmap = (struct multiboot_tag_mmap *)tag;
                        struct multiboot_mmap_entry *entry;
                        
                        entry = mmap->entries;
                        while ((uint8_t *)entry < (uint8_t *)mmap + mmap->size) {
                                if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                                        total_memory_bytes += entry->len;
                                        
                                        /* Mark frames as free */
                                        uint32_t start_frame = entry->addr / PMM_BLOCK_SIZE;
                                        uint32_t end_frame = (entry->addr + entry->len) / PMM_BLOCK_SIZE;
                                        
                                        for (uint32_t i = start_frame; i < end_frame; i++) {
                                                if (i < MAX_BLOCKS) {
                                                        mmap_unset(i);
                                                }
                                        }
                                }
                                entry = (struct multiboot_mmap_entry *)((uint8_t *)entry + mmap->entry_size);
                        }
                }
                tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7));
        }

        total_blocks = MAX_BLOCKS; /* We track all 4GB space */

        /* Mark kernel, Multiboot info, and bitmap area as used */
        uint32_t kernel_start_frame = (uint32_t)_kernel_start / PMM_BLOCK_SIZE;
        /* Bitmap ends at bitmap + BITMAP_SIZE */
        uint32_t range_end_frame = ((uint32_t)bitmap + BITMAP_SIZE) / PMM_BLOCK_SIZE + 1;
        
        for (uint32_t i = kernel_start_frame; i < range_end_frame; i++) {
                mmap_set(i);
        }
        
        /* Also mark page 0 as used (null pointer protection) */
        mmap_set(0);

        debug_ok("PMM Initialized (%d MB)", (uint32_t)(total_memory_bytes / 1024 / 1024));
}

void *pmm_alloc_block(void) {
        int frame = mmap_first_free();
        if (frame == -1) {
                return NULL;
        }
        
        mmap_set(frame);
        used_blocks++;
        return (void *)(frame * PMM_BLOCK_SIZE);
}

void pmm_free_block(void *p) {
        uint32_t frame = (uint32_t)p / PMM_BLOCK_SIZE;
        if (frame < total_blocks) {
                mmap_unset(frame);
                used_blocks--;
        }
}

size_t pmm_get_total_memory(void) {
        return total_memory_bytes;
}

size_t pmm_get_used_memory(void) {
        return used_blocks * PMM_BLOCK_SIZE;
}
