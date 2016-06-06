#ifndef __CONFIG_H
#define __CONFIG_H

#define config_version 1

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

struct patch_state
{
    char filename[256];

    uint8_t state;
} __attribute__((packed));

extern struct config_file config;

enum type
{
    boolean_val = 0, // Toggle
    ranged_val = 1,  // N1 - N2, left and right to pick.
    mask_val = 2,    // Bitmask allowed values.
    not_option = 3,  // Skip over this.
    call_fun = 4     // Call a function. Treat (a) as (void)(*)(void).
};

struct range_str
{
    int a, b;
};

struct options_s
{
    int64_t index;
    char name[64];
    char desc[256];
    enum type allowed;
    uint32_t a, b;
} __attribute__((packed));

// Use builtin loader module replacer.
#define OPTION_LOADER 2

// Inject svc calls (including backdoor for 11)
#define OPTION_SVCS 3

// Use builtin ARM9 thread injector.
#define OPTION_ARM9THREAD 4

// Skip menu unless L is held.
#define OPTION_AUTOBOOT 5

// Don't print debug information.
#define OPTION_SILENCE 6

// Pause for A key on each step.
#define OPTION_TRACE 7

// Background color is not drawn under text.
#define OPTION_TRANSP_BG 8

// Framebuffer is preserved from whatever ran before us.
#define OPTION_NO_CLEAR_BG 9

// Remove Help/Readme from menu.
#define OPTION_READ_ME 10

// Enable L2 cache.
#define OPTION_LOADER_CPU_L2 11

// Enable 800Mhz mode.
#define OPTION_LOADER_CPU_800MHZ 12

// Enable language emulation.
#define OPTION_LOADER_LANGEMU 13

// Force replacement of non-null svcs. Normally you don't want this.
#define OPTION_REPLACE_ALLOCATED_SVC 14

// Ignore patch UUID dependencies. Not recommended.
#define IGNORE_PATCH_DEPS 14

// Allow enabling patches which are marked as 'incompatible'. Chances are there's a reason.
#define IGNORE_BROKEN_SHIT 15

// Whether to use an EmuNAND
#define OPTION_EMUNAND 16

// Which EmuNAND to use (currently only allows 10 total, but eh, I can change that if anyone truly needs it)
#define OPTION_EMUNAND_INDEX 17

// Save log files during boot and from loader.
// This will slow things down a bit.
#define OPTION_SAVE_LOGS 253

// Output so much debugging info, it'd make your head spin.
#define OPTION_OVERLY_VERBOSE 254

// This is for internal use only. It's set when any patches
// change and causes caches to be regenerated.
#define OPTION_RECONFIGURED 255

//#define HEADER_COLOR        12 // Color of header text.
//#define BG_COLOR            13 // Color of background.
//#define TEXT_COLOR          14 // Color of most text.
//#define ARROW_COLOR         15 // Color of Arrow.

#ifndef LOADER
void load_config();
void save_config();
#endif
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
