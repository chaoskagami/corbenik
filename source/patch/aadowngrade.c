#include "patch_file.h"

PATCH(aadowngrade)
{
	execb(PATH_PATCHES "/aadowngrade.vco");

    return 0;
}
