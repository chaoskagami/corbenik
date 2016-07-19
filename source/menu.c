#include <common.h>
#include <ctr9/ctr_system.h>

#define MAX_PATCHES ((FCRAM_SPACING / 2) / sizeof(struct options_s))
struct options_s *patches = (struct options_s *)FCRAM_MENU_LOC;
uint8_t *enable_list = (uint8_t *)FCRAM_PATCHLIST_LOC;

static struct options_s options[] = {
    // Patches.
    { 0, "General Options", "", not_option, 1, 0 },

    { OPTION_SVCS, "svcBackdoor Fixup", "Reinserts svcBackdoor on 11.0 NATIVE_FIRM. svcBackdoor allows executing arbitrary functions with ARM11 kernel permissions, and is required by some (poorly coded) applications.", boolean_val, 0, 0 },

    { OPTION_REBOOT, "Reboot Hook", "Hooks firmlaunch to allow largemem games on o3DS. Also allows patching TWL/AGB on all consoles.", boolean_val, 0, 0 },

    { OPTION_EMUNAND, "Use EmuNAND", "Redirects NAND write/read to the SD. This supports both Gateway and redirected layouts.", boolean_val, 0, 0 },
    { OPTION_EMUNAND_INDEX, "  Index", "Which EmuNAND to use. If you only have one, you want 0. Currently the maximum supported is 10 (0-9), but this is arbitrary.", ranged_val, 0, 0x9 },
//    { OPTION_EMUNAND_REVERSE, "  Reverse layout", "(Warning - Experimental!) Calculate EmuNAND sector from the end of the disk, not the start. This isn't supported by tools like Decrypt9, but has some advantages.", boolean_val, 0, 0x9 },

    { OPTION_AUTOBOOT, "Autoboot", "Boot the system automatically, unless the R key is held while booting.", boolean_val, 0, 0 },
    { OPTION_SILENCE, "  Silent mode", "Suppress all debug output during autoboot. You'll see the screen turn on and then off once.", boolean_val, 0, 0 },
    { OPTION_DIM_MODE, "Dim Background", "Experimental! Dims colors on lighter backgrounds to improve readability with text. You won't notice the change until scrolling or exiting the current menu due to the way rendering works.", boolean_val, 0, 0 },

    { OPTION_ACCENT_COLOR, "Accent color", "Changes the accent color in menus.", ranged_val, 1, 7},

    // space
    { 0, "", "", not_option, 0, 0 },
    // Patches.
    { 0, "Loader Options", "", not_option, 1, 0 },

    { OPTION_LOADER, "Use Loader Replacement", "Replaces loader with one capable of extra features. You should enable this even if you don't plan to use loader-based patches to kill ASLR and the Ninjhax/OOThax checks.", boolean_val, 0, 0 },
    { OPTION_LOADER_CPU_L2, "  CPU - L2 cache", "Forces the system to use the L2 cache on all applications. If you have issues with crashes, try turning this off.", boolean_val_n3ds, 0, 0 },
    { OPTION_LOADER_CPU_800MHZ, "  CPU - 804Mhz", "Forces the system to run in 804Mhz mode on all applications.", boolean_val_n3ds, 0, 0 },
    { OPTION_LOADER_LANGEMU, "  Language Emulation", "Reads language emulation configuration from `" PATH_LOCEMU "` and imitates the region/language.", boolean_val, 0, 0 },
    { OPTION_LOADER_LOADCODE, "  Load Code Sections", "Loads code sections (text/ro/data) from SD card and patches afterwards.", boolean_val, 0, 0 },

    { OPTION_LOADER_DUMPCODE, "  Dump Code Sections",
      "Dumps code sections for titles to SD card the first time they're loaded. Slows things down on first launch.", boolean_val, 0, 0 },

    { OPTION_LOADER_DUMPCODE_ALL, "    + System Titles",
      "Dumps code sections for system titles, too. Expect to sit at a blank screen for >3mins on the first time you do this, because it dumps everything.", boolean_val, 0, 0 },

    // space
    { 0, "", "", not_option, 0, 0 },
    // Patches.
    { 0, "Developer Options", "", not_option, 1, 0 },

    { OPTION_TRACE, "Step Through", "After each important step, [WAIT] will be shown and you'll need to press a key. Debug feature.", boolean_val, 0, 0 },
    { OPTION_OVERLY_VERBOSE, "Verbose", "Output more debug information than the average user needs.", boolean_val, 0, 0 },
    { OPTION_SAVE_LOGS, "Logging", "Save logs to `" LOCALSTATEDIR "` as `boot.log` and `loader.log`. Slows operation a bit.", boolean_val, 0, 0 },

    //    { OPTION_ARM9THREAD,        "ARM9 Thread", boolean_val, 0, 0 },
    //    { IGNORE_PATCH_DEPS,   "Ignore dependencies", boolean_val, 0, 0 },
    //    { IGNORE_BROKEN_SHIT,  "Allow unsafe options", boolean_val, 0, 0 },

    // Sentinel.
    { -1, "", "", 0, -1, -1 }, // cursor_min and cursor_max are stored in the last two.
};

extern unsigned int font_w;

void accent_color(void* screen, int fg);

void
header(char *append)
{
    set_cursor(TOP_SCREEN, 0, 0);
    fill_line(stdout, 0, config.options[OPTION_ACCENT_COLOR]);
    accent_color(TOP_SCREEN, 0);
    fprintf(stdout, "\x1b[30m ." FW_NAME " // %s\x1b[0m\n\n", append);
}

static int current_menu_index_patches = 0;

// This function is based on PathDeleteWorker from GodMode9.
// It was easier to just import it.
int
list_patches_build_back(char *fpath, int desc_is_path)
{
    FILINFO fno;
    DIR pdir;
    char *fname = &fpath[strnlen(fpath, 255)];
    if (f_opendir(&pdir, fpath) != FR_OK)
        return 1;

    fname[0] = '/';
    fname++;

    while (f_readdir(&pdir, &fno) == FR_OK) {
        strncpy(fname, fno.fname, strlen(fno.fname));

        if (fno.fname[0] == 0)
            break;

        FILINFO f2;
        if (f_stat(fpath, &f2) != FR_OK)
            break;

        if (f2.fattrib & AM_DIR) {
            // return value won't matter
            list_patches_build_back(fpath, desc_is_path);
        } else {
            struct system_patch p;
            read_file(&p, fpath, sizeof(struct system_patch));

            if (memcmp(p.magic, "AIDA", 4))
                return 0;

            strncpy(patches[current_menu_index_patches].name, p.name, 64);
            if (desc_is_path)
                strncpy(patches[current_menu_index_patches].desc, fpath, 255);
            else
                strncpy(patches[current_menu_index_patches].desc, p.desc, 255);
            patches[current_menu_index_patches].index = p.uuid;
            patches[current_menu_index_patches].allowed = boolean_val;
            patches[current_menu_index_patches].a = 0;
            patches[current_menu_index_patches].b = 0;
            if (desc_is_path)
                enable_list[p.uuid] = 0;

            current_menu_index_patches++;
        }
    }

    f_closedir(&pdir);
    --fname;
    fname[0] = 0;

    return 0;
}

// This is dual purpose. When we actually list
// patches to build the cache - desc_is_fname
// will be set to 1.

void
list_patches_build(char *name, int desc_is_fname)
{
    current_menu_index_patches = 0;

    memset(enable_list, 0, FCRAM_SPACING / 2);

    if (!desc_is_fname) {
        strncpy(patches[0].name, "Patches", 64);
        strncpy(patches[0].desc, "", 255);
        patches[0].index = 0;
        patches[0].allowed = not_option;
        patches[0].a = 1;
        patches[0].b = 0;

        current_menu_index_patches += 1;
    }

    char fpath[256];
    strncpy(fpath, name, 256);
    list_patches_build_back(fpath, desc_is_fname);
    patches[current_menu_index_patches].index = -1;

    FILE *f;
    if ((f = fopen(PATH_TEMP "/PATCHENABLE", "r"))) {
        fread(enable_list, 1, FCRAM_SPACING / 2, f);
        fclose(f);
    }
}

int show_menu(struct options_s *options, uint8_t *toggles);

void
menu_patches()
{
    show_menu(patches, enable_list);
}

void
menu_options()
{
    show_menu(options, config.options);
}

#ifndef REL
#define REL "master"
#endif

static struct options_s info_d[] = {
    { 0, "  Native FIRM: ", "The version of NATIVE_FIRM in use.", not_option, 0, 0},
    { 0, "  AGB FIRM:    ", "The version of AGB_FIRM in use. This is used to run GBA games.", not_option, 0, 0},
    { 0, "  TWL FIRM:    ", "The version of TWL_FIRM in use. This is used to run DS games and DSiWare.", not_option, 0, 0},
    { 0, "  " FW_NAME ":    " VERSION " (" REL ")", FW_NAME "'s version.", not_option, 0, 0},
    { -1, "", "", not_option, 0, 0 }
};
static int is_setup_info = 0;

void
menu_info()
{
    if (!is_setup_info) {
        // This menu requres firm to be loaded. Unfortunately.
        load_firms(); // Lazy load!

        struct firm_signature *native = get_firm_info(firm_loc);
        struct firm_signature *agb = get_firm_info(agb_firm_loc);
        struct firm_signature *twl = get_firm_info(twl_firm_loc);

        memcpy(&info_d[0].name[strlen(info_d[0].name)], native->version_string, strlen(native->version_string));
        memcpy(&info_d[1].name[strlen(info_d[1].name)], agb->version_string, strlen(agb->version_string));
        memcpy(&info_d[2].name[strlen(info_d[2].name)], twl->version_string, strlen(twl->version_string));

        is_setup_info = 1;
    }

    show_menu(info_d, NULL);
}

#define ln(s) { 0, s, "", not_option, 0, 0 }
#define lnh(s) { 0, s, "", not_option, 1, 0 }

static struct options_s help_d[] = {
    lnh("About"),
    ln("  This is another 3DS CFW for power users."),
    ln("  It seeks to address some faults in other"),
    ln("  CFWs and is generally just another choice"),
    ln("  for users - but primarily is intended for"),
    ln("  developers. It is not for the faint of heart."),
    ln(""),
    lnh("Usage"),
    ln("  A         -> Select/Toggle/Increment"),
    ln("  B         -> Back/Boot"),
    ln("  X         -> Decrement"),
    ln("  Select    -> Help/Information"),
    ln("  Down      -> Down"),
    ln("  Right     -> Down five"),
    ln("  Up        -> Up"),
    ln("  Left      -> Up five"),
    ln("  L+R+Start -> Menu Screenshot"),
    ln(""),
    lnh("Credits"),
    ln("  @mid-kid, @Wolfvak, @Reisyukaku, @AuroraWright"),
    ln("  @d0k3, @TuxSH, @Steveice10, @delebile,"),
    ln("  @Normmatt, @b1l1s, @dark-samus, @TiniVi,"),
    ln("  @gemarcano, and anyone else I may have"),
    ln("  forgotten (yell at me, please!)"),
    ln(""),
    ln("  <https://github.com/chaoskagami/corbenik>"),
    { -1, "", "", not_option, 0, 0 }
};

void
menu_help()
{
    show_menu(help_d, NULL);
}

void
reset()
{
    fflush(stderr);

    fumount(); // Unmount SD.

    // Reboot.
    fprintf(BOTTOM_SCREEN, "Rebooting system...\n");

    ctr_system_reset();
}

void
poweroff()
{
    fflush(stderr);

    fumount(); // Unmount SD.

    // Power off
    fprintf(BOTTOM_SCREEN, "Powering off system...\n");

    ctr_system_poweroff();
}

#if defined(CHAINLOADER) && CHAINLOADER == 1
void chainload_menu();
#endif

static struct options_s main_s[] = {
    { 0, "Options",            "Internal options for the CFW.\nThese are part of " FW_NAME " itself.", call_fun, (uint32_t)menu_options, 0 },
    { 0, "Patches",            "External bytecode patches found in `" PATH_PATCHES "`.\nYou can choose which to enable.", call_fun, (uint32_t)menu_patches, 0 },
    { 0, "Info",               "Shows the current FIRM versions (and loads them, if needed)", call_fun, (uint32_t)menu_info,    0 },
    { 0, "Readme",             "Mini-readme.\nWhy are you opening help on this, though?\nThat's kind of silly.", call_fun, (uint32_t)menu_help,    0 },
    { 0, "Reboot",             "Reboots the console.", call_fun, (uint32_t)reset,        0 },
    { 0, "Power off",          "Powers off the console.", call_fun, (uint32_t)poweroff,     0 },
    { 0, "Save Configuration", "Save the configuration.\nYou must do this prior to booting,\notherwise the cache will not be (re)generated..", call_fun, (uint32_t)save_config,  0 },
#if defined(CHAINLOADER) && CHAINLOADER == 1
    { 0, "Chainload",          "Boot another ARM9 payload file.", call_fun, (uint32_t)chainload_menu, 0 },
#endif
    { 0, "Boot Firmware",      "Generates caches, patches the firmware, and boots it.\nMake sure to 'Save Configuration' first if any options changed.", break_menu, 0, 0 },

    // Sentinel.
    { -1, "", "", 0, -1, -1 }, // cursor_min and cursor_max are stored in the last two.
};

void
menu_handler()
{
    show_menu(main_s, NULL);
}
