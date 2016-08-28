#include <corbconf.h>
#if defined(CHAINLOADER) && CHAINLOADER == 1

#include <common.h>

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
    char chain_file[256];
    strncpy(chain_file, chain_file_data, 255);

    char code_file[] = PATH_BITS "/chain.bin";

    uint8_t* bootstrap = (uint8_t*)0x24F00000;
    uint32_t size = 0, b_size = 0;
    uint8_t* chain_data;

    FILE* f = fopen(code_file, "r");
    if (!f) {
        // File missing.
        abort("Missing chainloader.\n");
    }

    b_size = fsize(f);
    fread(bootstrap, 1, b_size, f);
    fclose(f);

    chain_data = bootstrap + b_size;

    f = fopen(chain_file, "r");
    if (!f) {
        // File missing.
        abort("Missing program to chainload?\n");
    }

    size = fsize(f);
    fread(chain_data, 1, size, f);
    fclose(f);

    fprintf(stderr, "Setting argc, argv...\n");

    size = size - (size % 4) + 4;

    uint32_t* off = (uint32_t*) &chain_data[size];

    off[0] = (uint32_t)off + 4; // char**
    off[1] = (uint32_t)off + 8; // char*

    char* arg0 = (char*)&off[1];
    memcpy(arg0, chain_file, strlen(chain_file) + 1);

    uint32_t* argc_off = (uint32_t*)memfind(bootstrap, b_size, "ARGC", 4);
    uint32_t* argv_off = (uint32_t*)memfind(bootstrap, b_size, "ARGV", 4);

    argc_off[0] = 1;
    argv_off[0] = (uint32_t)off;

    fprintf(stderr, "Changing display mode and chainloading...\n");

    screen_mode(1); // TODO - Because RGBA8 screeninit is non-standard...ugh

    // Copy CakeHax struct where it is expected (at 0x23FFFE00)
    // It's very very likely we'll corrupt memory with this, but we aren't coming back anyways as of the
    // next call, so not my problem
    memcpy((void*)0x23FFFE00, framebuffers, sizeof(struct framebuffers));

    ((void(*)(void*, uint32_t))0x24F00000)(chain_data, size + 256 + 8); // Size of payload + argv.

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

    current_chain_index += 1;

    recurse_call(name, chain_file_hdl);

    chains[current_chain_index].name = NULL;

    if (chains[1].name == NULL)
        chains[0].name = NULL; // No chainloadable files.
}

void chainload_menu() {
    if (chains == NULL) {
        chains = malloc(sizeof(struct options_s) * 100);
        list_chain_build(PATH_CHAINS);
    }

    show_menu(chains);
}

#endif
