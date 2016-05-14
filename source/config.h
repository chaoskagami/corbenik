#ifndef __CONFIG_H
#define __CONFIG_H

_UNUSED static unsigned int config_version = 1;

#define CONFIG_MAGIC "OVAN"

// Structure of config file
struct config_file {
    char magic[4];              // "OVAN" for shits and giggles again.
    uint32_t config_ver;        // Config file version.

    uint8_t  options[256];      // Options in the menu - deliberately large to avoid config version bumps.

    uint64_t patch_ids[256];    // What patches are enabled by UUID. 256 is an arbitrary limit - contact me if you hit it.
}__attribute__((packed));

extern struct config_file config;

#define OPTION_SIGPATCH     0  // Use builtin signature patch.
#define OPTION_LOADER       1  // Use builtin loader module replacer.
#define OPTION_ARM9THREAD   2  // Use builtin ARM9 thread injector.

#define OPTION_AUTOBOOT     3  // Skip menu unless L is held.
#define OPTION_SILENCE      4  // Don't print debug information.
#define OPTION_TRACE        5  // Pause for A key on each step.

#define OPTION_TRANSP_BG    6  // Background color is not drawn under text.
#define OPTION_NO_CLEAR_BG  7  // Framebuffer is preserved from whatever ran before us.
#define OPTION_READ_ME      8  // Remove Help/Readme from menu.

#define IGNORE_PATCH_DEPS   9  // Ignore patch UUID dependencies. Not recommended.
#define IGNORE_BROKEN_SHIT  10 // Allow enabling patches which are marked as 'incompatible'. Chances are there's a reason.

#define HEADER_COLOR        11 // Color of header text.
#define BG_COLOR            12 // Color of background.
#define TEXT_COLOR          13 // Color of most text.
#define ARROW_COLOR         14 // Color of Arrow.

void load_config();
void save_config();

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
