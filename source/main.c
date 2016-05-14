#include "common.h"
#include "firm/firm.h"

void init_system() {
}

int menu_handler();

int main() {
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

	fprintf(BOTTOM_SCREEN, "Booting CFW\n");

	save_config(); // Save config file.

	boot_cfw();
    // Under ideal conditions, we never get here.
}
