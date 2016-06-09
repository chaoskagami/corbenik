#ifndef __PATCH_HEADER__
#define __PATCH_HEADER__

#include <stdint.h>

// Build patch into CFW instead of as module.
#include "../std/unused.h"
#include "../std/memory.h"
#include "../firm/firm.h"
#include "../firm/fcram.h"
#include "../config.h"
#include "../common.h"
#include "../interp.h"

#define PATCH(name) int patch_##name()

#endif
