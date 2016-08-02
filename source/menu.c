#include <common.h>
#include <ctr9/ctr_system.h>

static struct options_s options[] = {
    // Patches.
    { 0, "General Options", "", not_option, 1, 0 },

    { OPTION_AUTOBOOT, "Autoboot", "Boot the system automatically, unless the R key is held while booting.", boolean_val, 0, 0 },
    { OPTION_SILENCE, "  Silent mode", "Suppress all debug output during autoboot. You'll see the screen turn on and then off once.", boolean_val, 0, 0 },
    { OPTION_DIM_MODE, "Dim Background", "Experimental! Dims colors on lighter backgrounds to improve readability with text. You won't notice the change until scrolling or exiting the current menu due to the way rendering works.", boolean_val, 0, 0 },

    { OPTION_ACCENT_COLOR, "Accent color", "Changes the accent color in menus.", ranged_val, 1, 7},

    { OPTION_BRIGHTNESS, "Brightness", "Changes the screeninit brightness in menu. WIP, only takes effect on reboot (this will change.)", ranged_val, 0, 3},

    // Sentinel.
    { -1, "", "", 0, 0, 0 }, // cursor_min and cursor_max are stored in the last two.
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

int show_menu(struct options_s *options, uint8_t *toggles);

void
menu_options()
{
    show_menu(options, config.options);
}

#ifndef REL
#define REL "master"
#endif

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

void chainload_menu();

void
menu_handler()
{
    while(1) {
        chainload_menu();
        menu_options();
    }
}
