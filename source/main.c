#include "common.h"
#include "firm/firm.h"
#include "input.h"
#include "config.h"
#include "screeninit.h"
#include "std/abort.h"

int menu_handler();

int is_n3ds = 0;

int doing_autoboot = 0;
void shut_up();

int
main()
{
    if (PDN_MPCORE_CFG == 7)
        is_n3ds = 1; // Enable n3ds specific options.

    int c = fmount();
    screen_init();

    set_font(PATH_BITS "/termfont.bin");

    if (c) {
        // Failed to mount SD. Bomb out.
        abort("Failed to mount SD card.\n");
    }

    load_config(); // Load configuration.

    if (CFG_BOOTENV == 7) {
        fprintf(stderr, "Rebooted from AGB, disabling EmuNAND...\n");
        config.options[OPTION_EMUNAND] = 0;
    }

    // Autoboot. Non-standard code path.
    if (config.options[OPTION_AUTOBOOT] && !(HID_PAD & BUTTON_R)) {
        if (config.options[OPTION_SILENCE])
            shut_up(); // This does exactly what it sounds like.
        doing_autoboot = 1;
    } else {
        menu_handler();
    }

    boot_cfw();
    // Under ideal conditions, we never get here.
}
