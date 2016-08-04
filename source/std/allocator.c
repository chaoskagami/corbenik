#include <common.h>

// 16 <- AES block size.
#define SALLOC_ALIGN 16

struct alloc_info* first_mem = NULL;

void* sbrk(int incr) {
  extern uint32_t __end__; /* Defined by the linker */
  static uint32_t *heap_end;
  uint32_t        *prev_heap_end;

  if (heap_end == 0) {
    heap_end = &__end__;
  }

  prev_heap_end = heap_end;
  if (heap_end + incr > stack_ptr)
    abort("Heap overflowed!\n");

  heap_end += incr;
  return (void*) prev_heap_end;
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

    block = (free_block*)sbrk(size);
    block->size = size;

    return ((char*)block) + sizeof(free_block);
}

void free(void* ptr) {
	if (ptr == NULL) return;

    free_block* block = (free_block*)(((char*)ptr) - sizeof(free_block ));
    block->next = free_block_list_head.next;
    free_block_list_head.next = block;
}
