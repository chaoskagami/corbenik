#include "common.h"
#include "firm/firm.h"
#include "firm/headers.h"
#define MENU_MAIN     1

#define MENU_OPTIONS  2
#define MENU_PATCHES  3
#define MENU_INFO     4
#define MENU_HELP     5
#define MENU_RESET    6
#define MENU_POWER    7
#define MENU_BOOTME   8

static struct options_s options[] = {
	{ 0, "Signature Patch", boolean_val, 0, 0 },
	{ 1, "FIRM Protection", boolean_val, 0, 0 },
	{ 2, "SysModule Replacement", boolean_val, 0, 0 },
	{ 3, "Service Replacement", boolean_val, 0, 0 },
	{ 4, "ARM9 Thread", boolean_val, 0, 0 },

	{ 5, "Autoboot", boolean_val, 0, 0 },
	{ 6, "Silence w/ Autoboot", boolean_val, 0, 0 },
	{ 7, "Step through with button", boolean_val, 0, 0 },

	{ 8, "Don't draw background color", boolean_val, 0, 0 },
	{ 9, "Preserve framebuffer data", boolean_val, 0, 0 },

	{ 10, "Hide Help from menu", boolean_val, 0, 0 },

	{ 11, "Loader: CPU L2 enable", boolean_val, 0, 0 },
	{ 12, "Loader: CPU 800Mhz mode", boolean_val, 0, 0 },
	{ 13, "Loader: Language Emulation", boolean_val, 0, 0 },

	{ 14, "No dependency tracking", boolean_val, 0, 0 },
	{ 15, "Allow unsafe options", boolean_val, 0, 0 },
	{ -1, "", 0, 0, 0},
};

static int cursor_y = 0;
static int which_menu = 1;
static int need_redraw = 1;

uint32_t wait_key() {
    uint32_t get = 0;
    while(get == 0) {
        if(HID_PAD & BUTTON_UP)
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
    while(HID_PAD&get);
    return get;
}

void header(char* append) {
    fprintf(stdout, "\x1b[33;40m[.corbenik//%s] %s\x1b[0m\n", VERSION, append);
}

int menu_patches() { return MENU_MAIN; }

int menu_options() {
    set_cursor(TOP_SCREEN, 0, 0);

    header("A:Enter B:Back DPAD:Nav");

	int i = 0;
    while(options[i].index != -1) { // -1 Sentinel.
        if (cursor_y == i)
            fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
        else
            fprintf(TOP_SCREEN, "   ");

        if (need_redraw)
			fprintf(TOP_SCREEN, "[%c]  %s\n", (config.options[options[i].index] ? 'X' : ' '), options[i].name);
		else {
			// Yes, this is weird. printf does a large number of extra things we don't
			// want computed at the moment; this is faster.
			putc(TOP_SCREEN, '[');
			putc(TOP_SCREEN, (config.options[options[i].index] ? 'X' : ' '));
			putc(TOP_SCREEN, ']');
			putc(TOP_SCREEN, '\n');
		}
		++i;
    }

	need_redraw = 0;

    uint32_t key = wait_key();

    switch(key) {
        case BUTTON_UP:
            cursor_y -= 1;
			if (cursor_y < 0)
				cursor_y = 0;
            break;
        case BUTTON_DOWN:
            cursor_y += 1;
			if (options[cursor_y].index == -1)
				cursor_y -= 1;
            break;
        case BUTTON_A:
            // TODO - Value options
            config.options[options[cursor_y].index] = !config.options[options[cursor_y].index];
            break;
        case BUTTON_B:
			need_redraw = 1;
		    clear_screen(TOP_SCREEN);
			cursor_y = 0;
            return MENU_MAIN;
            break;
    }

    return 0;
}

int menu_info() {
    clear_screen(TOP_SCREEN);

    set_cursor(TOP_SCREEN, 0, 0);

    header("Any:Back");
	struct firm_signature *native = get_firm_info(firm_loc);
	struct firm_signature *agb = get_firm_info(agb_firm_loc);
	struct firm_signature *twl = get_firm_info(twl_firm_loc);

    fprintf(stdout,     "\nNATIVE_FIRM / Firmware:\n"
                        "  Version: %s (%x)\n"
                        "AGB_FIRM / GBA Firmware:\n"
                        "  Version: %s (%x)\n"
                        "TWL_FIRM / DSi Firmware:\n"
                        "  Version: %s (%x)\n",
						native->version_string, native->version,
						agb->version_string, agb->version,
						twl->version_string, twl->version);
    while (1) {
        if (wait_key() & BUTTON_ANY)
            break;
    }

	need_redraw = 1;
    clear_screen(TOP_SCREEN);

    return MENU_MAIN;
}

int menu_help() {
    clear_screen(TOP_SCREEN);

    set_cursor(TOP_SCREEN, 0, 0);

    header("Any:Back");

    fprintf(stdout,     "\nCorbenik is a 3DS firmware patching tool;\n"
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

    while (1) {
        if (wait_key() & BUTTON_ANY)
            break;
    }

	need_redraw = 1;
    clear_screen(TOP_SCREEN);

    return MENU_MAIN;
}

int menu_reset() {
    fumount(); // Unmount SD.

    // Reboot.
    fprintf(BOTTOM_SCREEN, "Resetting system.\n");
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 2);
    while(1);
}

int menu_poweroff() {
    fumount(); // Unmount SD.

    // Reboot.
    fprintf(BOTTOM_SCREEN, "Powering off system.\n");
    i2cWriteRegister(I2C_DEV_MCU, 0x20, 1 << 0);
    while(1);
}

int menu_main() {
    set_cursor(TOP_SCREEN, 0, 0);

    const char *list[] = {
        "Options",
        "Patches",
        "Info",
        "Help/Readme",
        "Reset",
        "Power off",
        "Boot firmware"
    };
    int menu_max = 7;

    header("A:Enter DPAD:Nav");

    for(int i=0; i < menu_max; i++) {
        if (!(i+2 == MENU_HELP && config.options[OPTION_READ_ME])) {
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

	int ret = cursor_y+2;

    switch(key) {
        case BUTTON_UP:
            cursor_y -= 1;
			if (config.options[OPTION_READ_ME] && cursor_y+2 == MENU_HELP)
				cursor_y -= 1; // Disable help.
            break;
        case BUTTON_DOWN:
            cursor_y += 1;
			if (config.options[OPTION_READ_ME] && cursor_y+2 == MENU_HELP)
				cursor_y += 1; // Disable help.
            break;
        case BUTTON_A:
			need_redraw = 1;
			cursor_y = 0;
            if (ret == MENU_BOOTME)
				return MENU_BOOTME; // Boot meh, damnit!
            return ret;
    }

    // Loop around the cursor.
    if (cursor_y < 0)
        cursor_y = menu_max -1;
    if (cursor_y > menu_max - 1)
        cursor_y = 0;

    return 0;
}

int menu_handler() {
    int to_menu = 0;
    switch(which_menu) {
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

