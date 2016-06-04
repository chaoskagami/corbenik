#include "common.h"
#include "firm/firm.h"
#include "firm/headers.h"
#define MENU_MAIN 1

#define MENU_OPTIONS 2
#define MENU_PATCHES 3
#define MENU_INFO 4
#define MENU_HELP 5
#define MENU_RESET 6
#define MENU_POWER 7
#define MENU_BOOTME 8

#define MAX_PATCHES ( ( FCRAM_SPACING / 2) / sizeof(struct options_s) )
struct options_s *patches = (struct options_s*)FCRAM_MENU_LOC;
uint8_t* enable_list = (uint8_t*)FCRAM_PATCHLIST_LOC;

static struct options_s options[] = {
    // space
    { 0, "", "", not_option, 0, 0 },
    // Patches.
    { 0, "\x1b[32;40mOptions\x1b[0m", "", not_option, 0, 0 },

    { OPTION_LOADER, "System Modules", "Replaces system modules (including loader)", boolean_val, 0, 0 },
    { OPTION_LOADER_CPU_L2, "  CPU - L2 cache", "Forces the system to use the L2 cache. Ignored if not a N3DS.", boolean_val, 0, 0 },
    { OPTION_LOADER_CPU_800MHZ, "  CPU - 800Mhz", "Forces the system to run in 800Mhz mode. Ignored if not a N3DS.", boolean_val, 0, 0 },
    { OPTION_LOADER_LANGEMU, "  Language Emulation", "Reads language emulation configuration and imitates the region/language.", boolean_val, 0, 0 },

    { 0, "", "", not_option, 0, 0 },

    { OPTION_SERVICES, "Service Replacement", "Replaces ARM11 services, including svcBackdoor. With 11.0 NATIVE_FIRM, you need this.", boolean_val, 0, 0 },

    { 0, "", "", not_option, 0, 0 },

    { OPTION_AUTOBOOT, "Autoboot", "Boot the system automatically, unless the R key is held.", boolean_val, 0, 0 },
    { OPTION_SILENCE, "  Silent mode", "Suppress all debug output during autoboot. You'll see the screen turn on, then off.", boolean_val, 0, 0 },

    { 0, "", "", not_option, 0, 0 },

    { OPTION_READ_ME, "Hide `Help`", "Hides the help option from the main menu.", boolean_val, 0, 0 },

    // space
    { 0, "", "", not_option, 0, 0 },
    // Patches.
    { 0, "\x1b[32;40mDeveloper Options\x1b[0m", "", not_option, 0, 0 },

    { OPTION_REPLACE_ALLOCATED_SVC, "Force service replace", "Replace ARM11 services even if they exist. Don't use unless you know what you're doing.", boolean_val, 0, 0 },
    { OPTION_TRACE, "Debug Pauses", "After each important step, [WAIT] will be shown and you'll need to press a key. Debug.", boolean_val, 0, 0 },
    { OPTION_OVERLY_VERBOSE, "Verbose", "Output more debug information than the average user needs.", boolean_val, 0, 0 },

    //    { OPTION_ARM9THREAD,        "ARM9 Thread", boolean_val, 0, 0 },
    //    { IGNORE_PATCH_DEPS,   "Ignore dependencies", boolean_val, 0, 0 },
    //    { IGNORE_BROKEN_SHIT,  "Allow unsafe options", boolean_val, 0, 0 },

    // Sentinel.
    { -1, "", "", 0, -1, -1 }, // cursor_min and cursor_max are stored in the last two.
};

static int cursor_y = 0;
static int which_menu = 1;
static int need_redraw = 1;

uint32_t
wait_key()
{
    uint32_t get = 0;
    while (get == 0) {
        if (HID_PAD & BUTTON_UP)
            get = BUTTON_UP;
        else if (HID_PAD & BUTTON_DOWN)
            get = BUTTON_DOWN;
        else if (HID_PAD & BUTTON_RIGHT)
            get = BUTTON_RIGHT;
        else if (HID_PAD & BUTTON_LEFT)
            get = BUTTON_LEFT;
        else if (HID_PAD & BUTTON_A)
            get = BUTTON_A;
        else if (HID_PAD & BUTTON_B)
            get = BUTTON_B;
    }
    while (HID_PAD & get)
        ;
    return get;
}

void
header(char *append)
{
    fprintf(stdout, "\x1b[30;42m.corbenik//%s %s\x1b[0m\n", VERSION, append);
}

static int current_menu_index_patches = 0;

// This function is based on PathDeleteWorker from GodMode9.
// It was easier to just import it.
int list_patches_build_back(char* fpath, int desc_is_path) {
	FILINFO fno = {.lfname = NULL};

	// this code handles directory content deletion
	if (f_stat(fpath, &fno) != FR_OK)
		return 1; // fpath does not exist

	if (fno.fattrib & AM_DIR) { // process folder contents
		DIR pdir;
		char* fname = fpath + strnlen(fpath, 255);
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
			strncpy(patches[current_menu_index_patches].desc, fpath,  255);
		else
			strncpy(patches[current_menu_index_patches].desc, p.desc, 255);
		patches[current_menu_index_patches].index   = p.uuid;
		patches[current_menu_index_patches].allowed = boolean_val;
		patches[current_menu_index_patches].a = 0;
		patches[current_menu_index_patches].b = 0;
		if (desc_is_path)
			enable_list[p.uuid] = 0;

		current_menu_index_patches++;
	}

	return 0;
}

void list_patches_build(char* name, int desc_is_fname) {
	current_menu_index_patches = 0;

	memset(enable_list, 0, FCRAM_SPACING / 2);

	char fpath[256];
	strncpy(fpath, name, 256);
	list_patches_build_back(fpath, desc_is_fname);
	patches[current_menu_index_patches].index   = -1;

	FILE* f;
	if ((f = fopen(PATH_TEMP "/PATCHENABLE", "r"))) {
		fread(enable_list, 1, FCRAM_SPACING / 2, f);
		fclose(f);
	}
}

int show_menu(struct options_s *options, uint8_t* toggles);

int
menu_patches()
{
	list_patches_build(PATH_PATCHES, 0);

	show_menu(patches, enable_list);

	// Remove old settings, save new
	f_unlink(PATH_TEMP "/PATCHENABLE");
	write_file(enable_list, PATH_TEMP "/PATCHENABLE", FCRAM_SPACING / 2);

	// TODO - Determine whether it actually changed.
	config.options[OPTION_RECONFIGURED] = 1;

    return MENU_MAIN;
}

int
menu_options()
{
	show_menu(options, config.options);

    return MENU_MAIN;
}

int
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

    fprintf(stdout, "\nNATIVE_FIRM / Firmware:\n"
                    "  Version: %s (%x)\n"
                    "AGB_FIRM / GBA Firmware:\n"
                    "  Version: %s (%x)\n"
                    "TWL_FIRM / DSi Firmware:\n"
                    "  Version: %s (%x)\n",
            		native->version_string, native->version, agb->version_string, agb->version, twl->version_string, twl->version);

    wait_key();

    need_redraw = 1;
    clear_screen(TOP_SCREEN);

    return MENU_MAIN;
}

int
menu_help()
{
    clear_screen(TOP_SCREEN);

    set_cursor(TOP_SCREEN, 0, 0);

    header("Any:Back");

    fprintf(stdout, "\nCorbenik is a 3DS firmware patching tool;\n"
                    "  commonly known as a CFW. It seeks to address\n"
                    "  some faults in other CFWs and is generally\n"
                    "  just another choice for users - but primarily\n"
                    "  is intended for developers.\n"
                    "\n"
                    "Credits to people who've helped me put this\n"
                    "  together either by code or helpfulness:\n"
                    "  @mid-kid, @Wolfvak, @Reisyukaku, @AuroraWright\n"
                    "  @d0k3, @TuxSH, and others\n"
                    "\n"
                    "[PROTECT BREAK] DATA DRAIN: OK\n"
                    "\n"
                    " <https://github.com/chaoskagami/corbenik>\n"
                    "\n");

	wait_key();

    need_redraw = 1;
    clear_screen(TOP_SCREEN);

    return MENU_MAIN;
}

int
menu_reset()
{
    fumount(); // Unmount SD.

    // Reboot.
    fprintf(BOTTOM_SCREEN, "Rebooting system.\n");
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while (1)
        ;
}

int
menu_poweroff()
{
    fumount(); // Unmount SD.

    // Reboot.
    fprintf(BOTTOM_SCREEN, "Powering off system.\n");
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while (1)
        ;
}

int
menu_main()
{
    set_cursor(TOP_SCREEN, 0, 0);

    const char *list[] = { "Options", "Patches", "Info", "Help/Readme", "Reset", "Power off", "Boot firmware" };
    int menu_max = 7;

    header("A:Enter DPAD:Nav");

    for (int i = 0; i < menu_max; i++) {
        if (!(i + 2 == MENU_HELP && config.options[OPTION_READ_ME])) {
            if (cursor_y == i)
                fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
            else
                fprintf(TOP_SCREEN, "   ");

            if (need_redraw)
                fprintf(TOP_SCREEN, "%s\n", list[i]);
            else
                putc(TOP_SCREEN, '\n');
        }
    }

    need_redraw = 0;

    uint32_t key = wait_key();

    int ret = cursor_y + 2;

    switch (key) {
        case BUTTON_UP:
            cursor_y -= 1;
            if (config.options[OPTION_READ_ME] && cursor_y + 2 == MENU_HELP)
                cursor_y -= 1; // Disable help.
            break;
        case BUTTON_DOWN:
            cursor_y += 1;
            if (config.options[OPTION_READ_ME] && cursor_y + 2 == MENU_HELP)
                cursor_y += 1; // Disable help.
            break;
        case BUTTON_A:
            need_redraw = 1;
            cursor_y = 0;
            if (ret == MENU_BOOTME)
                return MENU_BOOTME; // Boot meh, damnit!
            clear_screen(TOP_SCREEN);
            if (ret == MENU_OPTIONS)
                cursor_y = 0; // Fixup positions
            return ret;
    }

    // Loop around the cursor.
    if (cursor_y < 0)
        cursor_y = menu_max - 1;
    if (cursor_y > menu_max - 1)
        cursor_y = 0;

    return 0;
}

int
menu_handler()
{
    int to_menu = 0;
    switch (which_menu) {
        case MENU_MAIN:
            to_menu = menu_main();
            break;
        case MENU_OPTIONS:
            to_menu = menu_options();
            break;
        case MENU_PATCHES:
            to_menu = menu_patches();
            break;
        case MENU_INFO:
            to_menu = menu_info();
            break;
        case MENU_HELP:
            to_menu = menu_help();
            break;
        case MENU_BOOTME:
            return 0;
        case MENU_RESET:
            menu_reset();
        case MENU_POWER:
            menu_poweroff();
        default:
            fprintf(stderr, "Attempt to enter wrong menu!\n");
            to_menu = MENU_MAIN;
    }

    if (to_menu != 0)
        which_menu = to_menu;

    return 1;
}
