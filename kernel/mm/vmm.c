#include <mm/vmm.h>
#include <mm/pmm.h>
#include <mm/heap.h>
#include <klib/kprintf.h>
#include <klib/string.h>
#include <tools/debug.h>

/* 
 * Page Directory: 
 * - Must be 4KB aligned.
 * - Contains 1024 entries, each pointing to a Page Table (or 4MB page).
 */
static uint32_t page_directory[ENTRIES_PER_PD] __attribute__((aligned(4096)));

/* 
 * First Page Table:
 * - Must be 4KB aligned.
 * - Contains 1024 entries, mapping 4MB of memory (0x0 - 0x3FFFFF).
 * - Used for Identity Mapping the kernel and lower memory.
 */
static uint32_t first_page_table[ENTRIES_PER_PT] __attribute__((aligned(4096)));

void load_page_directory(uint32_t* pd_addr) {
        __asm__ volatile("mov %0, %%cr3" :: "r"(pd_addr));
}

void enable_paging(void) {
        uint32_t cr0;
        __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
        cr0 |= 0x80000000;
        __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
}

static inline void invlpg(uint32_t addr) {
        __asm__ volatile("invlpg (%0)" :: "r" (addr) : "memory");
}

void vmm_map_page(uint32_t phys, uint32_t virt, uint32_t flags) {
        uint32_t pd_index = virt >> 22;
        uint32_t pt_index = (virt >> 12) & 0x03FF;

        uint32_t* pt_virt;

        /* Check if Page Table exists */
        if (!(page_directory[pd_index] & PAGE_PRESENT)) {
                /* Allocate a new Page Table */
                /* For now, we need a physical page allocator to get a frame for the PT. 
                   But we only have heap (kmalloc) which returns virtual addresses (currently identity mapped).
                   Since we are identity mapped 0-4MB, kmalloc returns address that is also physical.
                   
                   CRITICAL: In real VMM, we need PMM to allocate a frame.
                   Let's assume we can use PMM if we exposed it, or kmalloc for now (since identity).
                   But kmalloc returns virtual address.
                */
                
                /* Temporary Hack: Use kmalloc to get a page for the table. 
                   Since we are identity mapped, virt = phys. */
                uint32_t *new_pt = (uint32_t *)kmalloc(4096);
                if (!new_pt) {
                        debug_err("vmm_map_page: Failed to allocate Page Table");
                        return;
                }
                
                memset(new_pt, 0, 4096);
                
                /* Map the new PT into PD.
                   The physical address of new_pt is needed. Since identity mapped, phys = virt. */
                page_directory[pd_index] = (uint32_t)new_pt | PAGE_PRESENT | PAGE_RW; // User?
                pt_virt = new_pt;
        } else {
                /* Page directory entry contains physical address of PT (masked).
                   Since identity mapped, we can treat it as virtual. */
                pt_virt = (uint32_t*)(page_directory[pd_index] & ~0xFFF);
        }

        /* Set Page Table Entry */
        pt_virt[pt_index] = (phys & ~0xFFF) | (flags & 0xFFF) | PAGE_PRESENT;
        
        /* Invalidate TLB for this address */
        invlpg(virt);
}

void vmm_init(void) {
        /* 1. Clear Page Directory */
        for (int i = 0; i < ENTRIES_PER_PD; i++) {
                /* Attribute: Supervisor, Read/Write, Not Present */
                page_directory[i] = PAGE_RW; 
        }

        /* 2. Setup First Page Table (Identity Map 0x0 - 0x400000) 
           This maps the first 4MB of physical memory to the first 4MB of virtual memory.
           This covers the kernel code, data, BSS, VGA buffer (0xB8000), and initial stack.
        */
        for (int i = 0; i < ENTRIES_PER_PT; i++) {
                /* phys_addr = i * 4096 */
                /* Attribute: Supervisor, Read/Write, Present */
                first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
        }

        /* 3. Add First Page Table to Page Directory
           The first entry of PD (covering 0x0 - 0x3FFFFF) points to our page table.
        */
        page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;

        /* 4. Register Page Fault Handler (ISR 14) 
           (Assuming isr_register_handler or similar exists, or we modify isr.c directly.
            We'll modify isr.c separately to handle the exception)
        */

        /* 5. Load Page Directory (CR3) */
        load_page_directory(page_directory);

        /* 6. Enable Paging (CR0) */
        enable_paging();

        debug_ok("VMM Initialized (Paging Enabled)");
}
