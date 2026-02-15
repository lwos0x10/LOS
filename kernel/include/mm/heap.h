#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stddef.h>

/* Heap block header structure */
typedef struct heap_block_header {
    size_t size;                    /* Size of the block (excluding header) */
    uint8_t is_free;                /* 1 if free, 0 if used */
    struct heap_block_header *next; /* Pointer to next block */
} heap_block_header_t;

/* Initialize heap with a given start address and size */
void heap_init(void *start, size_t size);

/* Allocate memory */
void *kmalloc(size_t size);

/* Free memory */
void kfree(void *ptr);

/* Debug helper */
void heap_print_info(void);
void heap_print_table(void);

#endif
