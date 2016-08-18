#ifndef __PATCH_FORMAT_H
#define __PATCH_FORMAT_H

#include <corbconf.h>

// The following are titleids which are handled specially for one reason or another.
// We use titleIDs to be generic; it ensures that patches can share the same
// format regardless of whether they're intended for loader or not.

#define NATIVE_FIRM_TITLEID      0x0004013800000002llu  ///< NATIVE_FIRM
#define NATIVE_FIRM_N3DS_TITLEID 0x0004013820000002llu  ///< NATIVE_FIRM, n3ds

#define TWL_FIRM_TITLEID         0x0004013000000102llu  ///< TWL_FIRM (DSi Firmware)
#define TWL_FIRM_N3DS_TITLEID    0x0004013020000102llu  ///< TWL_FIRM, n3ds (DSi Firmware)

#define AGB_FIRM_TITLEID         0x0004013000000202llu  ///< AGB_FIRM (GBA firmware)
#define AGB_FIRM_N3DS_TITLEID    0x0004013020000202llu  ///< AGB_FIRM (GBA firmware)

#define LOADER_TITLEID           0x0004013000001302llu  // Loader is handled specially.

// TODO - We also need to handle patches for internal system modules here, performing lzss decompression (and either recompression, or getting a patch to skip that if needed

#define PATH_MODULES         LIBDIR "/module"        ///< Sysmodule location
#define PATH_PATCHES         SBINDIR                 ///< Patch binary folder.

#define PATH_BITS            LIBEXECDIR              ///< Path to misc bits we need (emunand code, reboot code, etc)

#define PATH_EMUNAND_CODE    PATH_BITS "/emunand.bin"       ///< Emunand hook.
#define PATH_SCREENINIT_CODE PATH_BITS "/screeninit.bin"    ///< Screeninit code (ARM11)
#define PATH_BACKDOOR        PATH_BITS "/backdoor.bin"      ///< svcBackdoor
#define PATH_REBOOT_HOOK     PATH_BITS "/reboot_hook.bin"   ///< Reboot hook
#define PATH_REBOOT_CODE     PATH_BITS "/reboot_code.bin"   ///< Reboot entry code

#define PATH_TOP_BG          SHAREDIR "/top.bin"            ///< Top screen background
#define PATH_BOTTOM_BG       SHAREDIR "/bottom.bin"         ///< Bottom screen background
#define PATH_TERMFONT        SHAREDIR "/termfont.bin"       ///< Font data

#define PATH_CHAINS          PREFIX "/boot"                 ///< Chainloader payloads folder.

#define PATH_TEMP            LOCALSTATEDIR "/cache"         ///< Files that are transient and generated to improve speed
#define PATH_LOADER_CACHE    PATH_TEMP "/loader"            ///< Cached patch bytecode for loader.

#define PATH_LOCEMU          LOCALEDIR "/emu"               ///< Locale emulation config
#define PATH_FIRMWARES       LIBDIR   "/firmware"           ///< Firmware folder.

#define PATH_CONFIG_DIR      SYSCONFDIR                     ///< Config file directory.
#define PATH_CONFIG          PATH_CONFIG_DIR "/main.conf"   ///< Config file.

#define PATH_NATIVE_P        PATH_TEMP "/p_native"          ///< Patched native_firm.
#define PATH_AGB_P           PATH_TEMP "/p_agb"             ///< Patched agb_firm
#define PATH_TWL_P           PATH_TEMP "/p_twl"             ///< Patched twl_firm

#define PATH_KEYS            SHAREDIR "/keys"      ///< Keyfiles will be loaded from this dir, and
                                                   ///< additionally the root if not found.

#define PATH_EXEFS           LIBDIR "/exefs"       ///< ExeFS overrides/dumps, named by titleid
#define PATH_EXEFS_TEXT      PATH_EXEFS "/text"    ///< Text segment overrides/dumps, named by titleid
#define PATH_EXEFS_RO        PATH_EXEFS "/ro"      ///< RO segment overrides/dumps, named by titleid
#define PATH_EXEFS_DATA      PATH_EXEFS "/data"    ///< Data segment overrides/dumps, named by titleid

#define PATH_NATIVE_F        PATH_FIRMWARES "/native" ///< Native FIRM location
#define PATH_AGB_F           PATH_FIRMWARES "/agb"    ///< AGB FIRM location
#define PATH_TWL_F           PATH_FIRMWARES "/twl"    ///< TWL FIRM location

#define PATH_NATIVE_CETK     PATH_KEYS "/native.cetk" ///< Native FIRM cetk, used for decryption
#define PATH_TWL_CETK        PATH_KEYS "/twl.cetk"    ///< TWL FIRM cetk
#define PATH_AGB_CETK        PATH_KEYS "/agb.cetk"    ///< AGB FIRM cetk

#define PATH_NATIVE_FIRMKEY  PATH_KEYS "/native.key"  ///< Native FIRM decrypted titlekey
#define PATH_TWL_FIRMKEY     PATH_KEYS "/twl.key"     ///< TWL FIRM decrypted titlekey
#define PATH_AGB_FIRMKEY     PATH_KEYS "/agb.key"     ///< AGB FIRM decrypted titlekey

#define PATH_SLOT0X11KEY96   PATH_KEYS "/11Key96.key" ///< 0x11 KeyY (for 9.6 FIRM arm9loader)

#define PATH_ALT_SLOT0X11KEY96 "/slot0x11key96.bin"   ///< Alternate path for 0x11 KeyY

#define PATH_LOG             LOCALSTATEDIR "/log"     ///< Log directory

#define PATH_BOOTLOG         PATH_LOG "/boot.log"     ///< Boot time log
#define PATH_LOADERLOG       PATH_LOG "/loader.log"   ///< Loader log

#define PATCH_FLAG_REQUIRE (1 << 0) ///< Force enable patch unless 'Unsafe Options' is checked.
#define PATCH_FLAG_DEVMODE (1 << 1) ///< Require 'Developer Options' to be checked.
#define PATCH_FLAG_NOABORT (1 << 2) ///< Don't abort on error.

/* Patch file header.
 */
struct system_patch
{
    char magic[4];   ///< Magic to represent a patch. Should be "AIDA" for shits and giggles and because we like .hack.
    uint8_t version; ///< Version of the patch itself.

    /// NOTE - This metadata stuff is temporary, I eventually plan to move it down
    /// to the same 'variable' width section as tids.

    char name[64];   ///< User-readable name for patch in menu.
    char desc[256];  ///< User-readable description for patch in menu.

    uint64_t uuid;  ///< Unique ID for patch. Each unique patch should provide
                    ///< a unique ID.

    uint32_t flags; ///< Extra flags for patch.

    uint32_t titles; ///< How many titles this patch should be applied to (listed later)

    uint32_t depends; ///< How many deps there are.

    uint32_t size; ///< Size of the patch bytecode in bytes.

    // This stuff needs to be read not as part of the struct, but is technically part of it.

    // uint64_t tids[titles]     // TitleIDs.
    // uint64_t deps[depends]    // Dependencies as uuid refs.
    // uint8_t  patch_data[size] // Patch data.
} __attribute__((packed));

#endif
