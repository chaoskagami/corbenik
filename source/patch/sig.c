#include "patch_file.h"

// This patch is responsible for fixing signature checks for the firmware.

PATCH(signatures)
{
	execb(PATH_PATCHES "/sig.vco");

    return 0;
}
