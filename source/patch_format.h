#ifndef __PATCH_FORMAT_H
#define __PATCH_FORMAT_H

// The following are titleids which are handled specially for one reason or
// another.
// We use titleIDs to be generic; it ensures that patches can share the same
// format
// regardless of whether they're intended for loader or not. Simple logistics.
#define NATIVE_FIRM_TITLEID 0x0004013800000002llu      // NATIVE_FIRM
#define NATIVE_FIRM_N3DS_TITLEID 0x0004013820000002llu // NATIVE_FIRM, n3ds

#define TWL_FIRM_TITLEID 0x0004013000000102llu      // TWL_FIRM (DSi Firmware)
#define TWL_FIRM_N3DS_TITLEID 0x0004013020000102llu // TWL_FIRM, n3ds (DSi Firmware)

#define AGB_FIRM_TITLEID 0x0004013000000202llu      // AGB_FIRM (GBA firmware)
#define AGB_FIRM_N3DS_TITLEID 0x0004013020000202llu // AGB_FIRM (GBA firmware)

#define LOADER_TITLEID 0x0004013000001302llu // Loader is handled specially.

#define PATCH_MANDATORY (1 << 0)  // Patch must be applied for successful boot.
#define PATCH_FAIL_ABORT (1 << 1) // If patch fails to apply, abort and show an error.
#define PATCH_DISABLED (1 << 2)   // Do not allow changing this patch's status. With PATCH_MANDATORY,
                                  // this prevents disabling it.

// You can redefine this in the Makefile, if you'd like.
// Recommended names for being silly:
//   Windows
//   system
#ifndef PATH_CFW
#define PATH_CFW "/corbenik" // CFW root directory.
#endif

#define PATH_CONFIG_DIR PATH_CFW "/config"       // Config file directory.
#define PATH_CONFIG PATH_CONFIG_DIR "/main.conf" // Config file.
#define PATH_LOCEMU PATH_CONFIG_DIR "/locale"    // Locale emulation config
#define PATH_CPU_CFG PATH_CONFIG_DIR "/cpu.conf" // CPU settings config

#define PATH_PATCHES PATH_CFW "/patch"      // Patch binary folder.
#define PATH_FIRMWARES PATH_CFW "/firmware" // Firmware folder.
#define PATH_MODULES PATH_CFW "/module"     // Sysmodule location
#define PATH_SVC PATH_CFW "/svc"            // Svc code location.

#define PATH_TEMP PATH_CFW "/cache"           // Files that are transient and used to speed operation
#define PATH_LOADER_CACHE PATH_TEMP "/loader" // Cached patch bytecode for loader.

#define PATH_NATIVE_P PATH_TEMP "/p_native"
#define PATH_AGB_P PATH_TEMP "/p_agb"
#define PATH_TWL_P PATH_TEMP "/p_twl"

#define PATH_KEYS PATH_CFW "/keys" // Keyfiles will be loaded from this dir, and
                                   // additionally the root if not found.

#define PATH_EXEFS PATH_CFW "/exe" // ExeFS overrides, named by titleid

#define PATH_BITS PATH_CFW "/bits" // Path to misc bits we need (emunand code, reboot code, etc)

#define PATH_EMUNAND_CODE PATH_BITS "/emunand.bin"       // Emunand hook.
#define PATH_SCREENINIT_CODE PATH_BITS "/screeninit.bin" // Screeninit code (ARM11)

#define PATH_NATIVE_F PATH_FIRMWARES "/native"
#define PATH_AGB_F PATH_FIRMWARES "/agb"
#define PATH_TWL_F PATH_FIRMWARES "/twl"

#define PATH_NATIVE_CETK PATH_KEYS "/native.cetk"
#define PATH_TWL_CETK PATH_KEYS "/twl.cetk"
#define PATH_AGB_CETK PATH_KEYS "/agb.cetk"

#define PATH_NATIVE_FIRMKEY PATH_KEYS "/native.key"
#define PATH_TWL_FIRMKEY PATH_KEYS "/twl.key"
#define PATH_AGB_FIRMKEY PATH_KEYS "/agb.key"

#define PATH_SLOT0X11KEY96 PATH_KEYS "/11.key"

#define PATH_ALT_SLOT0X11KEY96 "/slot0x11key96.bin" // Hey, your perrogative, buddy. I like cleaned up
                                                    // paths.

#define PATCH_FLAG_REQUIRE (1 << 0) // Force enable patch unless 'Unsafe Options' is checked.
#define PATCH_FLAG_DEVMODE (1 << 1) // Require 'Developer Options' to be checked.
#define PATCH_FLAG_NOABORT (1 << 2) // Don't abort on error.

// Structure of a patch file.
struct system_patch
{
    char magic[4];   // "AIDA" for shits and giggles and because we like .hack.
    uint8_t version; // Version of the patch itself.

    char name[64];  // User-readable name for patch in menu.
    char desc[256]; // User-readable description for patch in menu.
    uint64_t uuid;  // Unique ID for patch. Each unique patch should provide
                    // a unique ID.

    uint32_t flags; // Extra flags for patch.

    uint32_t titles; // How many titles this patch should be applied to (listed later)

    uint32_t depends; // How many deps there are.

    uint32_t size; // Size of the patch bytecode in bytes.

    // This stuff needs to be read not as part of the struct, but is technically part of it.

    // uint64_t tids[titles]     // TitleIDs.
    // uint64_t deps[depends]    // Dependencies as uuid refs.
    // uint8_t  patch_data[size] // Patch data.
} __attribute__((packed));

#endif
