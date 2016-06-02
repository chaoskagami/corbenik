#include "patch_file.h"

// This patch clears MPU settings which lock down memory
// execution from userland. You should NOT enable this
// unless you know you need it, because it makes an obvious
// behavioral change that can be used maliciously and/or to
// detect CFW use rather easily.

PATCH(memexec)
{
	execb(PATH_PATCHES "/memexec.vco");

    return 0;
}
