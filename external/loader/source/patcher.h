#pragma once

#include <3ds/types.h>

void patch_text(u64 progId, u8 *text, u32 size, u32 orig_size);
void patch_data(u64 progId, u8 *data, u32 size, u32 orig_size);
void patch_ro(u64 progId, u8 *ro, u32 size, u32 orig_size);

u32 get_text_extend(u64 progId, u32 size_orig);
u32 get_ro_extend(u64 progId, u32 size_orig);
u32 get_data_extend(u64 progId, u32 size_orig);

void load_config();

u32 patchMemory(u8 *start, u32 size, const void *pattern, u32 patSize, int offset, const void *replace, u32 repSize, u32 count);
