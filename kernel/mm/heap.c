#include <mm/heap.h>
#include <mm/pmm.h>
#include <klib/kprintf.h>
#include <tools/debug.h>

static void *heap_start_addr;
static size_t heap_size_bytes;
static heap_block_header_t *head;

void heap_init(void *start, size_t size) {
        heap_start_addr = start;
        heap_size_bytes = size;

        /* Initial block covers the entire heap */
        head = (heap_block_header_t *)start;
        head->size = size - sizeof(heap_block_header_t);
        head->is_free = 1;
        head->next = NULL;

        debug_ok("Heap Initialized at 0x%x (Size: %d KB)", (uint32_t)start, size / 1024);
}

void *kmalloc(size_t size) {
        heap_block_header_t *current = head;
        
        /* Align size to 4 bytes? (Optional but good practice) */
        size = (size + 3) & ~3;

        while (current) {
                if (current->is_free && current->size >= size) {
                        /* Found a suitable block */
                        
                        /* Check if we can split it */
                        if (current->size >= size + sizeof(heap_block_header_t) + 4) {
                                heap_block_header_t *new_block = (heap_block_header_t *)((uint8_t *)current + sizeof(heap_block_header_t) + size);
                                
                                new_block->size = current->size - size - sizeof(heap_block_header_t);
                                new_block->is_free = 1;
                                new_block->next = current->next;
                                
                                current->size = size;
                                current->next = new_block;
                        }
                        
                        current->is_free = 0;
                        return (void *)((uint8_t *)current + sizeof(heap_block_header_t));
                }
                current = current->next;
        }
        
        debug_err("kmalloc failed for size %d", size);
        return NULL;
}

void kfree(void *ptr) {
        if (!ptr) return;
        
        heap_block_header_t *block = (heap_block_header_t *)((uint8_t *)ptr - sizeof(heap_block_header_t));
        block->is_free = 1;
        
        /* Merge with next free block if possible */
        if (block->next && block->next->is_free) {
                block->size += sizeof(heap_block_header_t) + block->next->size;
                block->next = block->next->next;
        }
        
        /* Merge with previous is harder in singly linked list without iterating from head.
           For simplicity in this version, we only merge forward. 
           To merge backward, we'd need a prev pointer or iterate from head.
           Let's iterate from head to merge backward for better efficiency (prevent fragmentation).
        */
        
        /* Optional: Implement full coalescing by iterating from head */
        heap_block_header_t *current = head;
        while (current) {
                if (current->is_free && current->next && current->next->is_free) {
                        current->size += sizeof(heap_block_header_t) + current->next->size;
                        current->next = current->next->next;
                        /* Don't advance current, try to merge again */
                } else {
                        current = current->next;
                }
        }
}

void heap_print_info(void) {
        heap_block_header_t *current = head;
        kprintf("Heap Info:\n");
        while (current) {
                kprintf("  Block at 0x%x: Size %d, %s\n", 
                        (uint32_t)current, current->size, current->is_free ? "Free" : "Used");
                current = current->next;
        }
}

void heap_print_table(void) {
        heap_block_header_t *current = head;
        kprintf("Heap Allocation Table:\n");
        kprintf("Idx  Start       End         Size        Status\n");
        
        int idx = 0;
        while (current) {
                uint32_t start = (uint32_t)current + sizeof(heap_block_header_t);
                uint32_t size = current->size;
                uint32_t end = start + size;
                
                kprintf("[%d] ", idx++);
                kprintf("0x%x    ", start);
                kprintf("0x%x    ", end);
                kprintf("%d          ", size);
                kprintf("%s\n", current->is_free ? "FREE" : "USED");
                        
                current = current->next;
        }
}
