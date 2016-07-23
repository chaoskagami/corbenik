#include <common.h>

void *fcram_temp = (void *)0x23000000;

void *fcram_static_mem = (void*)FCRAM_STATIC_ALLOC_LOC;
void *static_allocate(size_t bytes) {
    size_t aligned_bytes = bytes + (4 - (bytes % 4)); // Align to integer size (for ARM processor)
    void *ret = fcram_static_mem;
    fcram_static_mem = (uint8_t*)fcram_static_mem + aligned_bytes;
    return ret;
}
