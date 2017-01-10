#ifndef __CONFIG_H
#define __CONFIG_H

#define config_version 3      ///< Static version of the config. Updated when the config has changed in an
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

    char firm[3][256]; ///< FIRMS.
} __attribute__((packed));

#ifdef LOADER
extern struct config_file config;
#else
extern struct config_file *config;
#endif

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

#define OPTION_LOADER_DUMPCODE_MERGED 23 ///< Dump and load merged codebins (Luma-style) rather than split segment. Note that this is lossy and you will be unable to split these properly without the exheader information.

#define OPTION_SAVE_LOGS           253 ///< Save log files during boot and from loader. Slows down boot a bit.

#define OPTION_OVERLY_VERBOSE      254 ///< Output so much debugging info, it'd make your head spin.

#define OPTION_RECONFIGURED        255 ///< This is for internal use only. It's set when any changes
                                       ///< require caches to be regenerated.

#define OPTION_NFIRM_PATH 0xFFFD
#define OPTION_TFIRM_PATH 0xFFFE
#define OPTION_AFIRM_PATH 0xFFFF

//#define HEADER_COLOR        12 // Color of header text.
//#define BG_COLOR            13 // Color of background.
//#define TEXT_COLOR          14 // Color of most text.
//#define ARROW_COLOR         15 // Color of Arrow.

#ifndef LOADER

/// Bear in mind the following functions are only defined insofar
/// as that they exist. Do not make assumptions as to what is
/// backing these functions on disk.

/* Loads the config file off SD from the configured location.
 */
void load_config(void);

/* Saves the config file to SD at the configured location.
 */
void save_config(void);

/* Changes an option according to internal rules. Used in menus.
 */
void  toggle_opt(void* val);

/* Gets an option as a readable string.
 */
char* get_opt(void* val);

/* Gets an option in uint32_t form.
 */
uint32_t get_opt_u32(uint32_t val);

/* Sets an option in uint32_t form
 */
int set_opt_u32(uint32_t key, uint32_t val);

/* Sets an option in uint32_t form
 */
int set_opt_str(uint32_t key, const char* val);

#endif

#endif
