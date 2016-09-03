#include <stdint.h>
#include <stddef.h>

#include <ctr9/io.h>
#include <ctr9/aes.h>
#include <ctr9/sha.h>
#include <common.h>

__attribute__ ((noreturn))
void
boot_firm(firm_h* firm)
{
    abort("Temporarily not implemented\n");
    while(1);
}

int firm_loaded = 0;

firm_h*
load_firm(const char *path)
{
    return NULL;
}


void
boot_cfw(char* firm_path)
{
    firm_h* firm = load_firm(firm_path);

    fprintf(stderr, "Patching firmware...\n");
    if (patch_firm_all(firm) != 0)
        return;

    boot_firm(firm);
}
