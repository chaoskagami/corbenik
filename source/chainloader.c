#include <corbconf.h>
#if defined(CHAINLOADER) && CHAINLOADER == 1

#include <common.h>

uint32_t current_chain_index = 0;

struct options_s *chains = NULL;

int show_menu(struct options_s *options, uint8_t *toggles);

// TODO - The near same function is called in different places. It would
//        be better to have a recursive listing that calls a function for
//        each entry (it would cut code density)

__attribute__ ((noreturn))
void chainload_file(char* chain_file_data)
{
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
}

// This function is based on PathDeleteWorker from GodMode9.
// It was easier to just import it.
int
list_chain_build_back(char *fpath)
{
    FILINFO fno;
    DIR pdir;
    char *fname = &fpath[strnlen(fpath, 255)];
    if (f_opendir(&pdir, fpath) != FR_OK)
        return 1;

    fname[0] = '/';
    fname++;

    while (f_readdir(&pdir, &fno) == FR_OK) {
        strncpy(fname, fno.fname, strlen(fno.fname));

        if (fno.fname[0] == 0)
            break;

        FILINFO f2;
        if (f_stat(fpath, &f2) != FR_OK)
            break;

        if (f2.fattrib & AM_DIR) {
            // return value won't matter
            list_chain_build_back(fpath);
        } else {
            char* basename = &fpath[strlen(fpath) - 1];
            while(basename[0] != '/') basename--;
            basename++;

            strncpy(chains[current_chain_index].name, basename, 64);
            strncpy(chains[current_chain_index].desc, fpath, 255);

            chains[current_chain_index].index = 0;
            chains[current_chain_index].allowed = call_fun;
            chains[current_chain_index].a = (uint32_t) chainload_file;
            chains[current_chain_index].b = (uint32_t) chains[current_chain_index].desc;

            current_chain_index++;
        }
    }

    f_closedir(&pdir);
    --fname;
    fname[0] = 0;

    return 0;
}

// This is dual purpose. When we actually list
// patches to build the cache - desc_is_fname
// will be set to 1.

void
list_chain_build(char *name)
{
    current_chain_index = 0;

    strncpy(chains[0].name, "Chainloader Payloads", 64);
    strncpy(chains[0].desc, "", 255);
    chains[0].index = 0;
    chains[0].allowed = not_option;
    chains[0].a = 1;
    chains[0].b = 0;

    current_chain_index += 1;

    char fpath[256];
    strncpy(fpath, name, 256);
    list_chain_build_back(fpath);
    chains[current_chain_index].index = -1;

    if (chains[1].index == -1)
        chains[0].index = -1; // No chainloadable files.
}

void chainload_menu() {
	if (chains == NULL) {
		chains = malloc(sizeof(struct options_s) * 100);
	    list_chain_build(PATH_CHAINS);
	}

    show_menu(chains, NULL);
}

#endif
