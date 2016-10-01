// 'Tis not ready for the world at large yet.
// I don't want to delete it since I'm working on it though,
// so it's temporarliy #if'd 0.
#include <3ds.h>
#include <stdlib.h>
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

// Yes, we're including a C file. Problem?
#include "../../../source/interpreter.c"

#if 0

#endif
