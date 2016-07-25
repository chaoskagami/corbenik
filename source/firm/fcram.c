#include <common.h>

// 16 <- AES block size.
#define SALLOC_ALIGN 16

void *fcram_temp = (void *)0x23000000;

void *fcram_static_mem = (void*)FCRAM_STATIC_ALLOC_LOC;

struct alloc_info* first_mem = NULL;

// Low level static allocator / sbrk-like function.
void *fake_sbrk(size_t bytes) {
    void *ret = fcram_static_mem;

    fcram_static_mem = (uint8_t*)fcram_static_mem + bytes;

    return ret;
}

// This is an incredibly crappy and inefficient implementation of malloc/free nicked from stackoverflow.

typedef struct free_block {
    size_t size;
    struct free_block* next;
} free_block;

static free_block free_block_list_head = { 0, 0 };

// static const size_t overhead = sizeof(size_t);

static const size_t align_to = 16;

void* malloc(size_t size) {
    size = (size + sizeof(free_block) + (align_to - 1)) & ~ (align_to - 1);
    free_block* block = free_block_list_head.next;
    free_block** head = &(free_block_list_head.next);
    while (block != 0) {
        if (block->size >= size) {
            *head = block->next;
            return ((char*)block) + sizeof(free_block);
        }
        head = &(block->next);
        block = block->next;
    }

    block = (free_block*)fake_sbrk(size);
    block->size = size;

    return ((char*)block) + sizeof(free_block);
}

void free(void* ptr) {
	if (ptr == NULL) return;

    free_block* block = (free_block*)(((char*)ptr) - sizeof(free_block ));
    block->next = free_block_list_head.next;
    free_block_list_head.next = block;
}
