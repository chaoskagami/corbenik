#ifndef __PATCH_HEADER__
#define __PATCH_HEADER__

#include <stdint.h>

// Build patch into CFW instead of as module.
#define PATCH(name) int patch_##name()

#endif
