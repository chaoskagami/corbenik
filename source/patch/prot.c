#include "patch_file.h"

// This patch applies the FIRM protection code needed for safe a9lh usage.

PATCH(firmprot)
{
	execb(PATH_PATCHES "/prot.vco");

    return 0;
}
