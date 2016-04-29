#include "common.h"

void init_system() {}

#define MENU_BOOTME  -1
#define MENU_MAIN     1

#define MENU_OPTIONS  2
#define MENU_PATCHES  3
#define MENU_INFO     4
#define MENU_HELP     5
#define MENU_RESET    6
#define MENU_POWER    7

static int cursor_y = 0;
static int which_menu = 1;

uint32_t wait_key() {
    uint32_t get = 0;
    while(get == 0) {
        if(HID_PAD & BUTTON_UP)
            get = BUTTON_UP;
        else if (HID_PAD & BUTTON_DOWN)
            get = BUTTON_DOWN;
        else if (HID_PAD & BUTTON_A)
            get = BUTTON_A;
        else if (HID_PAD & BUTTON_B)
            get = BUTTON_B;
    }
    while(HID_PAD&get);
    return get;
}

void header() {
    fprintf(stdout, "\x1b[33;40m[.corbenik//%s]\n", VERSION);
}

int menu_options() { return MENU_MAIN; }

int menu_patches() { return MENU_MAIN; }

int menu_info() { return MENU_MAIN; }

int menu_help() {
    set_cursor(TOP_SCREEN, 0, 0);

    header();

    fprintf(stdout,     "Corbenik is a 3DS firmware patcher\n"
                        "  commonly known as a CFW. It seeks to address\n"
                        "  some faults in other CFWs and is generally\n"
                        "  just another choice for users - but primarily\n"
                        "  the kind of person who runs Gentoo or LFS. ;P\n"
                        "\n"
                        "Credits to people who've helped me put this\n"
                        "  together either by having written GPL code,\n"
                        "  or being just generally helpful/cool people:\n"
                        "  @mid-kid, @Wolfvak, @Reisyukaku, @AuroraWright\n"
                        "  @d0k3, and others\n"
                        "\n"
                        "The name of this comes from the .hack//series.\n"
                        "  Look it up, if you don't already know it.\n"
                        "\n"
                        "Any bugs filed including the letters S, A\n"
                        "  and O will be closed with no discussion.\n"
                        "\n"
                        " <https://github.com/chaoskagami/corbenik>\n"
                        "\n"
                        "Press B to return.\n");
    while (1) {
        if (wait_key() & BUTTON_B)
            break;
    }

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
        "Options   ",
        "Patches   ",
        "Info   ",
        "Help/Readme   ",
        "Reset   ",
        "Power off   ",
        "Boot firmware   "
    };

    header();

    int menu_max = 6;

    for(int i=0; i < menu_max; i++) {
        if (cursor_y == i)
            fprintf(TOP_SCREEN, "\x1b[32m>>   ");
        else
            fprintf(TOP_SCREEN, "   ");
        fprintf(TOP_SCREEN, "%s\n", list[i]);
    }

    uint32_t key = wait_key();
    int entry = cursor_y + 2;
    if (cursor_y > MENU_POWER)
        entry = MENU_BOOTME;

    switch(key) {
        case BUTTON_UP:
            cursor_y -= 1;
            break;
        case BUTTON_DOWN:
            cursor_y += 1;
            break;
        case BUTTON_A:
            return entry;
            break;
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
    }

    if (to_menu != 0)
        which_menu = to_menu;

    return 1;
}

void main() {
    if (fmount()) {
        // Failed to mount SD. Bomb out.
        fprintf(BOTTOM_SCREEN, "%pFailed to mount SD card.\n", COLOR(RED, BLACK));
    } else {
        fprintf(BOTTOM_SCREEN, "Mounted SD card.\n");
    }

    load_config(); // Load configuration.

    load_firms();

    int in_menu = 1;

    while(in_menu) {
        in_menu = menu_handler();
    }

    init_system();
    // Under ideal conditions, we never get here.
}
