#pragma once

void disable_cart_updates(u64 progId, u8 *code, u32 size);
void disable_eshop_updates(u64 progId, u8 *code, u32 size);
void disable_nim_updates(u64 progId, u8 *code, u32 size);
void fake_friends_version(u64 progId, u8 *code, u32 size);
void settings_string(u64 progId, u8 *code, u32 size);
void region_patch(u64 progId, u8 *code, u32 size);
void ro_sigpatch(u64 progId, u8 *code, u32 size);
void secureinfo_sigpatch(u64 progId, u8 *code, u32 size);
