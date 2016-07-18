// 'Tis not ready for the world at large yet.
// I don't want to delete it since I'm working on it though,
// so it's temporarliy #if'd 0.
#include <3ds.h>
#include "patcher.h"
#include "exheader.h"
#include "fsldr.h"
#include "internal.h"
#include "memory.h"
#include "logger.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include <option.h>
#include <patch_format.h>

// Patches must consist of fewer bytes than this.
// 16K is reasonable, IMO, and doesn't cause issues.

// I'm too lazy to implement file buffering here,
// and 16K is large enough for anything.

#define MAX_PATCHSIZE 16384

// Yes, we're including a C file. Problem?
#include "../../../source/interp.c"

#if 0

#endif
