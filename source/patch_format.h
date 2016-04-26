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

// Structure of a patch file.
struct system_patch {
	char magic[4];            // "AIDA" for shits and giggles and because we like .hack.
    uint32_t patch_ver;       // Version of the patch itself.
    uint32_t load_ver;        // Version of the CFW the patch is intended for.

    char name[64];            // User-readable name for patch in menu.
    char desc[256];           // User-readable description for patch in menu.
    uint64_t patch_id;        // Unique ID for patch. Each unique patch should provide a unique ID.

	uint64_t tid;             // What title this patch is intended for. Certain values are specially handled.
    uint64_t depends[64];     // What patches need to be applied for this patch to be applied; as unique IDs

    uint32_t patch_size;      // Size of the following patch data.
    uint8_t patch_data[];     // The data for the patch. This is a sort of interpreted code...see below.
} __attribute__((packed));

#endif

