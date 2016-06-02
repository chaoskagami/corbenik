#ifndef __CONFIG_H
#define __CONFIG_H

_UNUSED static unsigned int config_version = 1;

#define CONFIG_MAGIC "OVAN"

// Structure of config file
struct config_file
{
    char magic[4]; // "OVAN" for shits and giggles again.

    uint32_t config_ver; // Config file version.

    uint8_t options[256]; // Options in the menu - deliberately large to avoid
                          // config version bumps.

    uint64_t patch_ids[256]; // What patches are enabled by UUID. 256 is an
                             // arbitrary limit - contact me if you hit it.
} __attribute__((packed));

extern struct config_file config;

enum type
{
    boolean_val, // Toggle
    ranged_val,  // N1 - N2, left and right to pick.
    mask_val,    // Bitmask allowed values.
    not_option   // Skip over this.
};

struct range_str
{
    int a, b;
};

struct options_s
{
    int index;
    char name[64];
    enum type allowed;
    uint32_t a, b;
} __attribute__((packed));

#define OPTION_SIGPATCH 0   // Use builtin signature patch.
#define OPTION_FIRMPROT 1   // Protect firmware from writes.
#define OPTION_LOADER 2     // Use builtin loader module replacer.
#define OPTION_SERVICES 3   // Inject services (including backdoor for 11)
#define OPTION_ARM9THREAD 4 // Use builtin ARM9 thread injector.

#define OPTION_AUTOBOOT 5 // Skip menu unless L is held.
#define OPTION_SILENCE 6  // Don't print debug information.
#define OPTION_TRACE 7    // Pause for A key on each step.

#define OPTION_TRANSP_BG 8 // Background color is not drawn under text.
#define OPTION_NO_CLEAR_BG 9
// Framebuffer is preserved from whatever ran before us.
#define OPTION_READ_ME 10 // Remove Help/Readme from menu.

#define OPTION_LOADER_CPU_L2 11     // Enable L2 cache.
#define OPTION_LOADER_CPU_800MHZ 12 // Enable 800Mhz mode.
#define OPTION_LOADER_LANGEMU 13    // Enable 800Mhz mode.

#define OPTION_REPLACE_ALLOCATED_SVC 14
// Replace allocated services. Normally you don't want this.

#define IGNORE_PATCH_DEPS 14 // Ignore patch UUID dependencies. Not recommended.
#define IGNORE_BROKEN_SHIT 15
// Allow enabling patches which are marked as 'incompatible'. Chances are
// there's a reason.

#define OPTION_AADOWNGRADE 16 // Anti-anti-downgrade.
#define OPTION_MEMEXEC 17     // Prevent MPU from disabling execute permissions.
#define OPTION_UNITINFO 18    // Dev UNITINFO. Note that this is overkill.

//#define HEADER_COLOR        12 // Color of header text.
//#define BG_COLOR            13 // Color of background.
//#define TEXT_COLOR          14 // Color of most text.
//#define ARROW_COLOR         15 // Color of Arrow.

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
