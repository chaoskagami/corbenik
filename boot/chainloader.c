#include <corbconf.h>
#if defined(CHAINLOADER) && CHAINLOADER == 1

#include <stdint.h>            // for uint32_t, uint8_t
#include <stdlib.h>            // for NULL
#include <string.h>            // for memcpy, strlen, strncpy
#include <arm11.h>             // for screen_mode
#include <malloc.h>            // for memalign
#include <menu-backend.h>      // for options_s, show_menu, type::option
#include <std/abort.h>         // for panic
#include <std/draw.h>          // for framebuffers, stderr
#include <std/fs.h>            // for crclose, cropen, crread, crsize, recur...
#include <std/memory.h>        // for memfind, strdup_self
#include <structures.h>        // for PATH_BITS, PATH_CHAINS
#include <option.h>        // for get_opt_u32
#include "ctr9/io/fatfs/ff.h"  // for FILINFO, f_stat, ::FR_OK, AM_DIR

uint32_t current_chain_index = 0;
struct options_s *chains = NULL;

// TODO - The near same function is called in different places. It would
//        be better to have a recursive listing that calls a function for
//        each entry (it would cut code density)

__attribute__ ((noreturn))
void chainload_file(void* data)
{
    char* chain_file_data = (char*)data;

    // We copy because it's possible the payload will overwrite us in memory.
    FILE* f;
    char chain_file[256];
    strncpy(chain_file, chain_file_data, 255);

    uint32_t size = 0;
    uint8_t* chain_data = (uint8_t*)0x23F00000;

    f = cropen(chain_file, "r");
    if (!f) {
        // File missing.
        panic("Missing program to chainload?\n");
    }

    size = crsize(f);
    crread(chain_data, 1, size, f);
    crclose(f);

    fprintf(stderr, "Setting argc, argv...\n");

    size = size - (size % 4) + 4;

    uint32_t* off = (uint32_t*) &chain_data[size];

    off[0] = (uint32_t)off + 4; // char**
    off[1] = (uint32_t)off + 8; // char*

    char* arg0 = (char*)&off[1];
    memcpy(arg0, chain_file, strlen(chain_file) + 1);

    fprintf(stderr, "Changing display mode and chainloading...\n");

    screen_mode(1, get_opt_u32(OPTION_BRIGHTNESS)); // Because RGBA8 screeninit is non-standard...ugh

    // Copy CakeHax struct where it is expected (at 0x23FFFE00)
    // It's very very likely we'll corrupt memory with this, but we aren't coming back anyways as of the
    // next call, so not my problem
    memcpy((void*)0x23FFFE00, framebuffers, sizeof(struct framebuffers));

    // TODO - TBH, I should really flush dcache and icache here

    ((void(*)(int, char**))0x23F00000)(1, (char**)off);

    while(1);
}

void chain_file_hdl(char* fpath) {
    FILINFO f2;
    if (f_stat(fpath, &f2) != FR_OK)
        return;

    if (!(f2.fattrib & AM_DIR)) {
        char* basename = &fpath[strlen(fpath) - 1];
        while(basename[0] != '/') basename--;
        basename++;

        char* dup = strdup_self(fpath);

        chains[current_chain_index].name = strdup_self(basename);
        chains[current_chain_index].desc = dup;

        chains[current_chain_index].handle = option;
        chains[current_chain_index].func  = chainload_file;
        chains[current_chain_index].value  = NULL;
        chains[current_chain_index].param = dup;
        chains[current_chain_index].indent = 0;
        chains[current_chain_index].highlight = 0;

        current_chain_index++;
    }
}

// This is dual purpose. When we actually list
// patches to build the cache - desc_is_fname
// will be set to 1.

void
list_chain_build(const char *name)
{
    current_chain_index = 0;

    chains[0].name = "Chainloader Payloads";
    chains[0].desc = "";
    chains[0].param = 0;
    chains[0].func = NULL;
    chains[0].value = NULL;
    chains[0].handle = unselectable;
    chains[0].indent = 0;
    chains[0].highlight = 1;

    current_chain_index += 1;

    recurse_call(name, chain_file_hdl);

    chains[current_chain_index].name = NULL;

    if (chains[1].name == NULL)
        chains[0].name = NULL; // No chainloadable files.
}

void chainload_menu() {
    if (chains == NULL) {
        chains = memalign(16, sizeof(struct options_s) * 100);
        list_chain_build(PATH_CHAINS);
    }

    show_menu(chains);
}

#endif
