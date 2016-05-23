#ifndef __CONFIG_H
#define __CONFIG_H

__attribute__((unused)) static unsigned int config_version = 1;

#define CONFIG_MAGIC "OVAN"

// Structure of config file
struct config_file {
    char magic[4];              // "OVAN" for shits and giggles again.

    uint32_t config_ver;        // Config file version.

    uint8_t  options[256];      // Options in the menu - deliberately large to avoid config version bumps.

    uint64_t patch_ids[256];    // What patches are enabled by UUID. 256 is an arbitrary limit - contact me if you hit it.
}__attribute__((packed));

#define OPTION_LOADER_CPU_L2   11   // Enable L2 cache.
#define OPTION_LOADER_CPU_800MHZ 12 // Enable 800Mhz mode.
#define OPTION_LOADER_LANGEMU  13 // Enable 800Mhz mode.

#endif
