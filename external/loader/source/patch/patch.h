#ifndef __PATCH_PATCH_H
#define __PATCH_PATCH_H

#include <3ds.h>
#include "../patcher.h"
#include "../memory.h"
#include "../logger.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include "../config.h"
#include "../../../../source/patch_format.h"

void disable_cart_updates(u64 progId, u8* code, u32 size);
void disable_eshop_updates(u64 progId, u8* code, u32 size);
void disable_nim_updates(u64 progId, u8* code, u32 size);
void fake_friends_version(u64 progId, u8* code, u32 size);
void settings_string(u64 progId, u8* code, u32 size);
void region_patch(u64 progId, u8* code, u32 size);
void ro_sigpatch(u64 progId, u8* code, u32 size);
void secureinfo_sigpatch(u64 progId, u8* code, u32 size);
void errdisp_devpatch(u64 progId, u8* code, u32 size);

#endif
