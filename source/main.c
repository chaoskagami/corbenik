#include "common.h"
#include "firm/firm.h"
#include "input.h"
#include "config.h"

void init_system() {
}

int menu_handler();

int doing_autoboot = 0;

int main() {
    if (fmount()) {
        // Failed to mount SD. Bomb out.
        fprintf(BOTTOM_SCREEN, "%pFailed to mount SD card.\n", COLOR(RED, BLACK));
    } else {
        fprintf(BOTTOM_SCREEN, "Mounted SD card.\n");
    }

    load_config(); // Load configuration.

    load_firms();

	// Autoboot, and not R?
	if (config.options[OPTION_AUTOBOOT] && !(HID_PAD & BUTTON_R)) {
		doing_autoboot = 1;
		boot_cfw(); // Just boot shit.
	}

    int in_menu = 1;

    while(in_menu) {
        in_menu = menu_handler();
    }

	fprintf(BOTTOM_SCREEN, "Booting CFW\n");

	save_config(); // Save config file.

	boot_cfw();
    // Under ideal conditions, we never get here.
}
