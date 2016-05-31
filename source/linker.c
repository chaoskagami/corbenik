#include "common.h"
#include "firm/fcram.h"
#include "firm/firm.h"

// Yes, this is EXACTLY what it looks like. We dynamically link and
// load patches as binaries; they use functions from corbenik to do
// the work, and therefore must have a function table in them.

// See vco/template for how this magic works.

// This ensures relatively small patches while also having incredible
// flexibility unlike a 'patch format'.

extern exefs_h* firm_p9_exefs;
exefs_h*
get_firm_proc9_exefs()
{
    return firm_p9_exefs;
}

extern exefs_h* twl_firm_p9_exefs;
exefs_h*
get_twl_proc9_exefs()
{
    return twl_firm_p9_exefs;
}

extern exefs_h* agb_firm_p9_exefs;
exefs_h*
get_agb_proc9_exefs()
{
    return agb_firm_p9_exefs;
}

int
execp(char* path)
{
    int basename = 0;
    for (basename = strlen(path); basename > 0; basename--)
        if (path[basename] == '/')
            break;
    basename++;

    fprintf(stderr, "Exec: %s\n", &path[basename]);

    struct system_patch patch;

    // Load patch from path.
    FILE* f = fopen(path, "r");
    fread(&patch, 1, sizeof(patch), f);

    fprintf(stderr, "  [h]");

    fread((uint8_t*)FCRAM_PATCHBIN_EXEC_LOC, 1, patch.patch_size, f);

    fprintf(stderr, "[x]");

    fclose(f);

    fprintf(stderr, "[b]\n");

    int (*patch_loc)() = (void*)FCRAM_PATCHBIN_EXEC_LOC;

    int ret = (*patch_loc)();

    fprintf(stderr, "  Exit: %d\n", ret);

    return ret;
}
