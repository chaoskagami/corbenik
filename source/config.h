#ifndef __CONFIG_H
#define __CONFIG_H

static unsigned int config_version = 1;

#define CONFIG_MAGIC "OVAN"

// Structure of config file
struct config_file {
    char magic[4];              // "OVAN" for shits and giggles again.
    uint32_t config_ver;        // Config file version.

    uint8_t  options[256];      // Options in the menu - deliberately large to avoid config version bumps.

    uint64_t patch_ids[256];    // What patches are enabled by UUID. 256 is an arbitrary limit - contact me if you hit it.
}__attribute__((packed));

static struct config_file config;

#define OPTION_AUTOBOOT 0
#define OPTION_SILENCE  1
#define OPTION_TRACE    2

void load_config();

/*
[CORBENIK]
version=1
option_<name>=<int>
...
[PATCHES]
<uuid>=<1|0>
...
*/


#endif
