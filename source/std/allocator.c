#include <common.h>

// 16 <- AES block size.
#define SALLOC_ALIGN 16

struct alloc_info* first_mem = NULL;
static uint32_t *heap_end = NULL;
extern uint32_t __end__; /* Defined by the linker */

void* sbrk(size_t incr) {
  uint32_t        *prev_heap_end;

  if (heap_end == NULL) {
    heap_end = &__end__;
  }

  // FIXME - Make sure heap isn't leaking into stack here. That would be bad.

  prev_heap_end = heap_end;

  heap_end += incr;
  return (void*) prev_heap_end;
}

// This is an incredibly crappy and inefficient implementation of malloc/free nicked from stackoverflow.

typedef struct free_block {
    size_t size;
    size_t real_size;
#ifdef MALLOC_DEBUG
    const char* info;
#endif
    struct free_block* next;

    uint32_t canary;
    uint32_t pad[3]; // Otherwise, not a 16-multiple
} free_block;

static free_block free_block_list_head = {
    0,
    0,
#ifdef MALLOC_DEBUG
    NULL,
#endif
    0,
    0,
    {0}
};

static const size_t align_to = 64;

#ifdef MALLOC_DEBUG
static size_t alloc_count      = 0;
static size_t free_count       = 0;
static size_t allocated_memory = 0;
#endif

#ifdef MALLOC_DEBUG
void* malloc_chkd(size_t size, const char* info) {
#else
void* malloc(size_t size) {
#endif
    size_t bsize = (size + sizeof(free_block) + (align_to - 1)) & ~ (align_to - 1);

    free_block* block = free_block_list_head.next;
    free_block** head = &(free_block_list_head.next);

    while (block != 0) {
        if (block->size >= bsize) {
            *head = block->next;

            block->real_size = size;
#ifdef MALLOC_DEBUG
            block->info = info;

            ++alloc_count;
            allocated_memory += block->size;
#endif

            return ((char*)block) + sizeof(free_block);
        }
        head = &(block->next);
        block = block->next;
    }

    block = (free_block*)sbrk(bsize);
    block->size = bsize;
    block->real_size = size;
    block->canary = 0x1337d00d; // Arbitrary. No special meaning.

#ifdef MALLOC_DEBUG
    block->info = info;

    ++alloc_count;
    allocated_memory += bsize;
#endif

    return ((char*)block) + sizeof(free_block);
}

void* malloc_zero(size_t size) {
    void* ret = malloc(size);

    if (ret)
        memset(ret, 0, size);

    return ret;
}

#ifdef MALLOC_DEBUG
void free_chkd(void* ptr, const char* info) {
#else
void free(void* ptr) {
#endif
    if (ptr == NULL) return;

    free_block* block = (free_block*)(((char*)ptr) - sizeof(free_block ));

#ifdef MALLOC_DEBUG
    if (block->canary != 0x1337d00d) {
        panic("%s: Attempt free non-pointer.\n", info);
    }

    ++free_count;
    if (allocated_memory < block->size) {
        fprintf(stderr, "%s: Invalid free detected.\n"
                        "  Allocated at: %s\n",
                        info, block->info);
    }
    allocated_memory -= block->size;
#endif

    block->next = free_block_list_head.next;
    free_block_list_head.next = block;
}

#ifdef MALLOC_DEBUG
void print_alloc_stats() {
	fprintf(stderr, "[A] %u [F] %u [M] %u [B] %lu\n", alloc_count, free_count, allocated_memory, (uint32_t)heap_end - (uint32_t)&__end__);
}
#endif

void *realloc(void* ptr, size_t size) {
    if (ptr == NULL)
        return malloc(size);

    free_block* current = (free_block*)(((char*)ptr) - sizeof(free_block));

    if (size < current->size || size < current->real_size)
        return ptr;

    void* new_ptr = malloc(size);

    memcpy(new_ptr, ptr, current->real_size);

    free(ptr);

    return new_ptr;
}
