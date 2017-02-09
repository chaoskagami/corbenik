#include <stdint.h>            // for uint32_t, uint8_t
#include <stdlib.h>            // for NULL
#include <string.h>            // for memcpy, strlen, strncpy

#include <ctr9/ctr_system.h>
#include <ctr9/ctr_cache.h>

#include <structures.h>
#include <std/fs.h>
#include <std/memory.h>


__attribute__((noreturn))
void panic() {
    ctr_system_poweroff();
    while(1);
}

int main()
{
    FILE* f;
    char chain_file[] = PATH_BITS "/corbenik.bin";

    if (crmount()) {
        panic();
    }

    uint8_t* bootstrap = (uint8_t*)0x24F00000;
    uint32_t size = 0;
    uint8_t* chain_data;

    f = cropen(chain_file, "r");
    if (!f) {
        // File missing.
        panic();
    }

    size = crsize(f);
    char* memory = malloc(size);
    crread(memory, 1, size, f);
    crclose(f);

    crumount(); // It doesn't really matter if we clean up or not, but why not

    memmove(bootstrap, memory, size); // Memory is now clobbered. Any memory allocation is unsafe past this point.

    // Set args = 2, { PATH_BITS "/corbenik.bin", "-native" }
    ctr_cache_clean_and_flush_all();

    ((int(*)(int, char**))bootstrap)(0, NULL);

    panic();
}
