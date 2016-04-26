#ifndef __PATCH_FORMAT_H
#define __PATCH_FORMAT_H

// The following are titleids which are handled specially for one reason or another.
#define NATIVE_FIRM_TITLEID      0x0004013800000002llu // NATIVE_FIRM
#define NATIVE_FIRM_N3DS_TITLEID 0x0004013820000002llu // NATIVE_FIRM, n3ds

#define TWL_FIRM_TITLEID         0x0004013000000102llu // TWL_FIRM (DSi Firmware)
#define TWL_FIRM_N3DS_TITLEID    0x0004013020000102llu // TWL_FIRM, n3ds (DSi Firmware)

#define AGB_FIRM_TITLEID         0x0004013000000202llu // AGB_FIRM (GBA firmware)
#define AGB_FIRM_N3DS_TITLEID    0x0004013020000202llu // AGB_FIRM (GBA firmware)

#define LOADER_TITLEID           0x0004013000001302llu // Loader is handled specially.

#define PATCH_MANDATORY  (1 << 0) // Patch must be applied for successful boot.
#define PATCH_FAIL_ABORT (1 << 1) // If patch fails to apply, abort and show an error.
#define PATCH_DISABLED   (1 << 2) // Do not allow changing this patch's status. With PATCH_MANDATORY, this prevents disabling it.

#define PATH_CFW        "/corbenik"                // CFW root directory.
#define PATH_CONFIG     PATH_CFW "/main.conf"      // Config file.
#define PATH_LOCEMU     PATH_CFW "/locale.conf"    // Locale emulation config
#define PATH_CPU_CFG    PATH_CFW "/cpu.conf"       // CPU settings config

#define PATH_FIRMWARES  PATH_CFW "/firmware"       // Firmware folder.
#define PATH_PATCHES    PATH_CFW "/patches"        // Patches folder.
#define PATH_TEMP       PATH_CFW "/temp"           // Files that are transient (user can delete them and they will be regenerated)
#define PATH_KEYS       PATH_CFW "/keys"           // Keyfiles will be loaded from this dir, and additionally the root if not found.
#define PATH_EXEFS      PATH_CFW "/exefs"          // ExeFS overrides, named like '<titleid>.exefs'

#define PATH_NATIVE_F   PATH_FIRMWARES "/native"
#define PATH_AGB_F      PATH_FIRMWARES "/agb"
#define PATH_TWL_F      PATH_FIRMWARES "/twl"

#define PATH_NATIVE_CETK       PATH_KEYS "/native.cetk"
#define PATH_NATIVE_FIRMKEY    PATH_KEYS "/native.key"

#define PATH_TWL_CETK       PATH_KEYS "/twl.cetk"
#define PATH_TWL_FIRMKEY    PATH_KEYS "/twl.key"

#define PATH_AGB_CETK       PATH_KEYS "/agb.cetk"
#define PATH_AGB_FIRMKEY    PATH_KEYS "/agb.key"

#define PATH_SLOT0X11KEY96  PATH_KEYS "/0x11.key"

#define PATH_ALT_SLOT0X11KEY96 "/slot0x11key96.bin" // Hey, your perrogative, buddy. I like cleaned up paths.

#define PATH_MEMBIN     PATH_TEMP "/memory_p"        // Memory binary
#define PATH_NATIVE_P   PATH_TEMP "/native_p"   // Native FIRM patched
#define PATH_AGB_P      PATH_TEMP "/agb_firm_p"      // AGB FIRM patched
#define PATH_TWL_P      PATH_TEMP "/twl_firm_p"      // TWL FIRM patched

// Structure of a patch file.
struct system_patch {
	char magic[4];            // "AIDA" for shits and giggles and because we like .hack.
    uint32_t patch_ver;       // Version of the patch itself.
    uint32_t load_ver;        // Version of the CFW the patch is intended for.

    char name[64];            // User-readable name for patch in menu.
    char desc[256];           // User-readable description for patch in menu.
    uint64_t patch_id;        // Unique ID for patch. Each unique patch should provide a unique ID.

	uint64_t tid;             // What title this patch is intended for. Certain values are specially handled.

    uint8_t extra_flags;      // Extra flags for patch.

    uint64_t depends[16];     // What patches need to be applied for this patch to be applied; as unique IDs

    uint32_t patch_size;      // Size of the following patch data.
    uint8_t patch_data[];     // The data for the patch. This is a sort of interpreted code...see below.
} __attribute__((packed));

/*
[PATCH]
version=1
cfw_version=1

name=Signature Check
desc=Disables firmware signature checking.
uuid=1
title_id=0004013800000002
[DEPENDS]
none
[DATA]
<BYTECODE AS HEX>
*/

#endif

