#ifndef __PATCHER_H
#define __PATCHER_H

#include <3ds/types.h>

void patch_text(u64 progId, u8* text, u32 size, u32 orig_size);
void patch_data(u64 progId, u8* data, u32 size, u32 orig_size);
void patch_ro(u64 progId, u8* ro, u32 size, u32 orig_size);

u32 get_text_extend(u64 progId, u32 size_orig);
u32 get_ro_extend(u64 progId, u32 size_orig);
u32 get_data_extend(u64 progId, u32 size_orig);

void load_config();

int fileOpen(Handle* file, FS_ArchiveID id, const char* path, int flags);

u8 get_cpumode(u64 progId);

#endif
