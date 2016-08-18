#ifndef __CONFIG_H
#define __CONFIG_H

#define config_version 1      ///< Static version of the config. Updated when the config has changed in an
                              ///< incompatible way.

#define CONFIG_MAGIC   "OVAN" ///< Magic to identify config files.

/* Structure of config file
 */
struct config_file
{
    char magic[4]; // "OVAN" for shits and giggles again.

    uint32_t config_ver; ///< Config file version.

    uint8_t options[256]; ///< Options in the menu - deliberately large to avoid
                          ///< config version bumps.

    uint64_t patch_ids[256]; ///< What patches are enabled by UUID. 256 is an
                             ///< arbitrary limit - contact me if you hit it.
} __attribute__((packed));

/* State of a patch file
 */
struct patch_state
{
    char filename[256]; ///< Patch filename.

    uint8_t state;      ///< Status of patch.
} __attribute__((packed));

#ifdef LOADER
extern struct config_file config;
#else
extern struct config_file *config;
#endif

/* Menu entry type. Determines how the menu is displayed and which (if any) options
 * can be changed.
 */
enum type
{
    boolean_val = 0,      ///< Toggleable boolean
    ranged_val = 1,       ///< N1 - N2, left and right to pick.
    mask_val = 2,         ///< Bitmask allowed values.
    not_option = 3,       ///< Skip over this.
    call_fun = 4,         ///< Call a function. Treat (a) as (void)(*)(void).
    boolean_val_n3ds = 5, ///< Toggle, but only show on n3DS
    break_menu = 6        ///< Exit the menu (same as B)
};

typedef void (*func_call_t)(uint32_t data);

struct range_str
{
    int a, b;
};

struct options_s
{
    int64_t index;     ///< Option index. Used for displaying values.
    char name[64];     ///< Name of patch to display in menus.
    char desc[256];    ///< Description of option, shown when pressing select
    enum type allowed; ///< Misnomer, FIXME. Type of menu entry. See enum type.
    uint32_t a, b;     ///< Should be union, FIXME. Data values used for menu entry.
    uint8_t indent;    ///< Indentation/ownership level of menu.
} __attribute__((packed));

#define OPTION_LOADER              2   ///< Use builtin loader module replacer.

#define OPTION_SVCS                3   ///< Inject svcBackdoor (FIXME, misnomer)

#define OPTION_ARM9THREAD          4   ///< Use builtin ARM9 thread injector. (NYI, in the future perhaps)

#define OPTION_AUTOBOOT            5   ///< Skip menu unless R is held.

#define OPTION_SILENCE             6   ///< Don't print debug information.

#define OPTION_TRACE               7   ///< Pause for A key on each step.

#define OPTION_DIM_MODE            8   ///< Dim background for readability.

#define OPTION_ACCENT_COLOR        9   ///< Accent color in menus.

#define OPTION_BRIGHTNESS          10  ///< Screeninit brightness

#define OPTION_LOADER_CPU_L2       11  ///< Enable L2 cache.

#define OPTION_LOADER_CPU_800MHZ   12  ///< Enable 800Mhz mode.

#define OPTION_LOADER_LANGEMU      13  ///< Enable language emulation.

#define IGNORE_PATCH_DEPS          14  ///< Ignore patch UUID dependencies. Not recommended. (NYI)

#define IGNORE_BROKEN_SHIT         15  ///< Allow enabling patches which are marked as 'incompatible'.
                                       ///< Chances are there's a reason. (NYI)

#define OPTION_EMUNAND             16  ///< Whether to use an EmuNAND

#define OPTION_EMUNAND_INDEX       17  ///< Which EmuNAND to use (currently only allows 10 total due to arbitary limit)

#define OPTION_LOADER_DUMPCODE     18  ///< Dump titles' code sections as they're loaded by the loader module.

#define OPTION_REBOOT              19  ///< Hook firmlaunches.

#define OPTION_LOADER_DUMPCODE_ALL 20  ///< Dump *all* code, from system applications, modules, etc. You'll
                                       ///< be sitting around for about five minutes.

#define OPTION_LOADER_LOADCODE     21  ///< Load *all* code sections. This is intended for big patches that
                                       ///< are currently not implementable and quick testing (e.g. SaltySD)

#define OPTION_SILENT_NOSCREEN     22  ///< Silenced autoboot will not init the screen.

#define OPTION_SAVE_LOGS           253 ///< Save log files during boot and from loader. Slows down boot a bit.

#define OPTION_OVERLY_VERBOSE      254 ///< Output so much debugging info, it'd make your head spin.

#define OPTION_RECONFIGURED        255 ///< This is for internal use only. It's set when any changes
                                       ///< require caches to be regenerated.

//#define HEADER_COLOR        12 // Color of header text.
//#define BG_COLOR            13 // Color of background.
//#define TEXT_COLOR          14 // Color of most text.
//#define ARROW_COLOR         15 // Color of Arrow.

#ifndef LOADER

/* Loads the config file off SD from the configured location.
 */
void load_config();

/* Saves the config file to SD at the configured location.
 */
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
