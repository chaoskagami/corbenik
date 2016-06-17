#include "common.h"
#include "firm/firm.h"
#include "firm/headers.h"

#define MAX_PATCHES ((FCRAM_SPACING / 2) / sizeof(struct options_s))
struct options_s *patches = (struct options_s *)FCRAM_MENU_LOC;
uint8_t *enable_list = (uint8_t *)FCRAM_PATCHLIST_LOC;

static struct options_s options[] = {
    // Patches.
    { 0, "\x1b[32;40mGeneral Options\x1b[0m", "", not_option, 0, 0 },

    { OPTION_SVCS, "svcBackdoor Fixup", "Reinserts svcBackdoor on 11.0 NATIVE_FIRM.", boolean_val, 0, 0 },

    { OPTION_REBOOT, "Reboot Hook", "Hooks firmlaunch to allow largemem games on o3DS (and allow patching TWL/AGB on all consoles)", boolean_val, 0, 0 },

    { OPTION_EMUNAND, "Use EmuNAND", "Redirects NAND write/read to the SD.", boolean_val, 0, 0 },
    { OPTION_EMUNAND_INDEX, "  Index", "Which EmuNAND to use. Currently, 10 maximum (but this is arbitrary)", ranged_val, 0, 0x9 },
    { OPTION_EMUNAND_REVERSE, "  Reverse layout", "EmuNAND is at the end of the disk, not the start.", boolean_val, 0, 0x9 },

    { OPTION_AUTOBOOT, "Autoboot", "Boot the system automatically, unless the R key is held.", boolean_val, 0, 0 },
    { OPTION_SILENCE, "  Silent mode", "Suppress all debug output during autoboot. You'll see the screen turn on, then off.", boolean_val, 0, 0 },

    // space
    { 0, "", "", not_option, 0, 0 },
    // Patches.
    { 0, "\x1b[32;40mLoader Options\x1b[0m", "", not_option, 0, 0 },

    { OPTION_LOADER, "Use Loader Replacement", "Replaces loader with one capable of the below and also applying bytecode patches.", boolean_val, 0, 0 },
    { OPTION_LOADER_CPU_L2, "  CPU - L2 cache (n3ds)", "Forces the system to use the L2 cache. Ignored if not a N3DS.", boolean_val_n3ds, 0, 0 },
    { OPTION_LOADER_CPU_800MHZ, "  CPU - 800Mhz (n3ds)", "Forces the system to run in 800Mhz mode. Ignored if not a N3DS.", boolean_val_n3ds, 0, 0 },
    { OPTION_LOADER_LANGEMU, "  Language Emulation", "Reads language emulation configuration and imitates the region/language.", boolean_val, 0, 0 },
    { OPTION_LOADER_LOADCODE, "  Load Code Sections", "Loads code sections from SD card and patches afterwards.", boolean_val, 0, 0 },

    { OPTION_LOADER_DUMPCODE, "  Dump Title Code Sections",
      "Dumps code sections for titles to SD card the first time they're loaded. Slows things down considerably.", boolean_val, 0, 0 },

    { OPTION_LOADER_DUMPCODE_ALL, "    + System Titles",
      "Dumps code sections for system titles, too. Expect to sit at a black screen for >3mins on the first time.", boolean_val, 0, 0 },

    // space
    { 0, "", "", not_option, 0, 0 },
    // Patches.
    { 0, "\x1b[32;40mDeveloper Options\x1b[0m", "", not_option, 0, 0 },

    { OPTION_TRACE, "Step Through", "After each important step, [WAIT] will be shown and you'll need to press a key. Debug.", boolean_val, 0, 0 },
    { OPTION_OVERLY_VERBOSE, "Verbose", "Output more debug information than the average user needs.", boolean_val, 0, 0 },
    { OPTION_SAVE_LOGS, "Logging", "Save logs to the corbenik folder. Slows operation a bit.", boolean_val, 0, 0 },

    //    { OPTION_ARM9THREAD,        "ARM9 Thread", boolean_val, 0, 0 },
    //    { IGNORE_PATCH_DEPS,   "Ignore dependencies", boolean_val, 0, 0 },
    //    { IGNORE_BROKEN_SHIT,  "Allow unsafe options", boolean_val, 0, 0 },

    // Sentinel.
    { -1, "", "", 0, -1, -1 }, // cursor_min and cursor_max are stored in the last two.
};

static int need_redraw = 1;

extern void waitcycles(uint32_t cycles);

uint32_t
wait_key(int sleep)
{
    if (sleep) {
        #define ARM9_APPROX_DELAY_MAX 134058675 / 85
        waitcycles(ARM9_APPROX_DELAY_MAX); // Approximately what a human can input - fine tuning needed (sorry, TASers!)
    }

    uint32_t ret = 0, get = 0;
    while (ret == 0) {
        get = HID_PAD;

        if (get & BUTTON_UP)
            ret = BUTTON_UP;
        else if (get & BUTTON_DOWN)
            ret = BUTTON_DOWN;
        else if (get & BUTTON_RIGHT)
            ret = BUTTON_RIGHT;
        else if (get & BUTTON_LEFT)
            ret = BUTTON_LEFT;
        else if (get & BUTTON_A)
            ret = BUTTON_A;
        else if (get & BUTTON_B)
            ret = BUTTON_B;
        else if (get & BUTTON_X)
            ret = BUTTON_X;

    }
    while (HID_PAD & ret);

    return ret;
}

extern unsigned int font_w;

void
header(char *append)
{
    for (unsigned int i = 0; i < TOP_WIDTH / font_w; i++) {
        fprintf(stdout, "\x1b[30;42m ");
    }
    set_cursor(TOP_SCREEN, 0, 0);
    fprintf(stdout, "\x1b[30;42m Corbenik//%s %s\x1b[0m\n\n", VERSION, append);
}

static int current_menu_index_patches = 0;

// This function is based on PathDeleteWorker from GodMode9.
// It was easier to just import it.
int
list_patches_build_back(char *fpath, int desc_is_path)
{
    FILINFO fno = {.lfname = NULL };

    // this code handles directory content deletion
    if (f_stat(fpath, &fno) != FR_OK)
        return 1; // fpath does not exist

    if (fno.fattrib & AM_DIR) { // process folder contents
        DIR pdir;
        char *fname = fpath + strnlen(fpath, 255);
        if (f_opendir(&pdir, fpath) != FR_OK)
            return 1;

        *(fname++) = '/';
        fno.lfname = fname;
        fno.lfsize = fpath + 255 - fname;

        while (f_readdir(&pdir, &fno) == FR_OK) {
            if ((strncmp(fno.fname, ".", 2) == 0) || (strncmp(fno.fname, "..", 3) == 0))
                continue; // filter out virtual entries
            if (fname[0] == 0)
                strncpy(fname, fno.fname, fpath + 255 - fname);
            if (fno.fname[0] == 0)
                break;
            else // return value won't matter
                list_patches_build_back(fpath, desc_is_path);
        }

        f_closedir(&pdir);
        *(--fname) = '\0';
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
        strncpy(patches[0].name, "\x1b[40;32mPatches\x1b[0m", 64);
        strncpy(patches[0].desc, "", 255);
        patches[0].index = 0;
        patches[0].allowed = not_option;
        patches[0].a = 0;
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

void
menu_info()
{
    // This menu requres firm to be loaded. Unfortunately.
    load_firms(); // Lazy load!

    clear_screen(TOP_SCREEN);

    set_cursor(TOP_SCREEN, 0, 0);

    header("Any:Back");
    struct firm_signature *native = get_firm_info(firm_loc);
    struct firm_signature *agb = get_firm_info(agb_firm_loc);
    struct firm_signature *twl = get_firm_info(twl_firm_loc);

    fprintf(stdout, "NATIVE_FIRM / Firmware:\n"
                    "  Version: %s (%x)\n"
                    "AGB_FIRM / GBA Firmware:\n"
                    "  Version: %s (%x)\n"
                    "TWL_FIRM / DSi Firmware:\n"
                    "  Version: %s (%x)\n",
            native->version_string, native->version, agb->version_string, agb->version, twl->version_string, twl->version);

    wait_key(1);

    need_redraw = 1;
    clear_screen(TOP_SCREEN);
}

void
menu_help()
{
    clear_screen(TOP_SCREEN);

    set_cursor(TOP_SCREEN, 0, 0);

    header("Any:Back");

    fprintf(stdout, "Corbenik is another 3DS CFW for power users.\n"
                    "  It seeks to address some faults in other\n"
                    "  CFWs and is generally just another choice\n"
                    "  for users - but primarily is intended for\n"
                    "  developers.\n"
                    "\n"
                    "Credits to people who've helped me put this\n"
                    "  together either by code or documentation:\n"
                    "  @mid-kid, @Wolfvak, @Reisyukaku, @AuroraWright\n"
                    "  @d0k3, @TuxSH, @Steveice10, @delebile,\n"
                    "  @Normmatt, @b1l1s, @dark-samus, @TiniVi, etc\n"
                    "\n"
                    " <https://github.com/chaoskagami/corbenik>\n"
                    "\n");

    wait_key(1);

    need_redraw = 1;
    clear_screen(TOP_SCREEN);
}

void
reset()
{
    fflush(stderr);

    fumount(); // Unmount SD.

    // Reboot.
    fprintf(BOTTOM_SCREEN, "Rebooting system.\n");
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while (1)
        ;
}

void
poweroff()
{
    fflush(stderr);

    fumount(); // Unmount SD.

    // Reboot.
    fprintf(BOTTOM_SCREEN, "Powering off system.\n");
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while (1)
        ;
}

static struct options_s main_s[] = {
    { 0, "Options",            "", call_fun, (uint32_t)menu_options, 0 },
    { 0, "Patches",            "", call_fun, (uint32_t)menu_patches, 0 },
    { 0, "Info",               "", call_fun, (uint32_t)menu_info,    0 },
    { 0, "Help/Readme",        "", call_fun, (uint32_t)menu_help,    0 },
    { 0, "Reboot",             "", call_fun, (uint32_t)reset,        0 },
    { 0, "Power off",          "", call_fun, (uint32_t)poweroff,     0 },
    { 0, "Save Configuration", "", call_fun, (uint32_t)save_config,  0 },
    { 0, "Boot Firmware",      "", break_menu, 0, 0 },

    // Sentinel.
    { -1, "", "", 0, -1, -1 }, // cursor_min and cursor_max are stored in the last two.
};

void
menu_handler()
{
    show_menu(main_s, NULL);
}
