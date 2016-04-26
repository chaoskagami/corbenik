#include "common.h"

void init_system() {}

#define MENU_BOOTME  -1
#define MENU_MAIN     1
#define MENU_OPTIONS  2
#define MENU_PATCHES  3
#define MENU_INFO     4
#define MENU_RESET    5
#define MENU_POWER    6

static int cursor_y = 0;
static int which_menu = 1;

int menu_options() { return MENU_MAIN; }
int menu_patches() { return MENU_MAIN; }
int menu_info()    { return MENU_MAIN; }

int menu_main() {
    set_cursor(TOP_SCREEN, 0, 0);

    const char *list[] = {
        "Options",
        "Patches",
        "Info",
        "Boot firmware",
        "Reset",
        "Power off"
    };

    cprintf(TOP_SCREEN, "%p[Corbenik - The Rebirth]\n", COLOR(CYAN, BLACK));

    for(int i=0; i < 6; i++) {
        if (cursor_y == i)
            cprintf(TOP_SCREEN, "%p-> ", COLOR(GREEN, BLACK));
        else
            cprintf(TOP_SCREEN, "   ");
        cprintf(TOP_SCREEN, "%s\n", list[i]);
    }

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
        case MENU_BOOTME:
        default:
            return 0;
    }

    if (to_menu != 0)
        which_menu = to_menu;

    return 1;
}

void main() {
    if (fmount()) {
        // Failed to mount SD. Bomb out.
        cprintf(BOTTOM_SCREEN, "%pFailed to mount SD card.\n", COLOR(RED, BLACK));
    } else {
        cprintf(BOTTOM_SCREEN, "Mounted SD card.\n");
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
