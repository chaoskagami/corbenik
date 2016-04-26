#include "common.h"

void load_options() {
	FILE config;
}

void init_system() {}

int menu_handler() {
    return 1;
}

void main() {
    if (fmount()) {
        // Failed to mount SD. Bomb out.
        cprintf(BOTTOM_SCREEN, "%pFailed to mount SD card.\n", COLOR(RED, BLACK));
    } else {
        cprintf(BOTTOM_SCREEN, "Mounted SD card.\n");
    }

    for (int i = 0; i < 200; i++) {
        cprintf(TOP_SCREEN, "%d\n", i);
    }

    cprintf(TOP_SCREEN, "Done printing.");

    load_options(); // Load configuration.

    int in_menu = 1;

    while(in_menu) {
        in_menu = menu_handler();
    }

    init_system();
    // Under ideal conditions, we never get here.
}
