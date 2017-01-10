#ifndef __PATCHER_H
#define __PATCHER_H

#include <3ds/types.h>
#include <3ds/exheader.h>

void code_handler(u64 progId, EXHEADER_prog_addrs* shared);
void patch_exe(u64 progId, u16 progver, EXHEADER_prog_addrs* shared, EXHEADER_prog_addrs* original);

void code_handler(u64 progId, EXHEADER_prog_addrs* shared);

u32 get_text_extend(u64 progId, u16 progver, u32 size_orig);
u32 get_ro_extend(u64 progId, u16 progver, u32 size_orig);
u32 get_data_extend(u64 progId, u16 progver, u32 size_orig);

void load_config();

int fileOpen(Handle *file, FS_ArchiveID id, const char *path, int flags);

u8 get_cpumode(u64 progId);

void hexdump_titleid(u64 progId, char *buf);

#endif
