#include "patch_file.h"

// This patch makes the console think it is a developer unit.
// Note that this is generally invasive and not useful to users;
// usually the ErrDisp patch in loader should be good enough for
// debugging crashes.

PATCH(unitinfo)
{
	execb(PATH_PATCHES "/unitinfo.vco");

    return 0;
}
