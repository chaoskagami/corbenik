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
main(int argc, char** argv)
{
    if (PDN_MPCORE_CFG == 7)
        is_n3ds = 1; // Enable n3ds specific options.

    int c = fmount();
    screen_init();
    clear_bg();
    load_bg_top(PATH_BITS "/top.bin");
    load_bg_bottom(PATH_BITS "/bottom.bin"); // This is basically a menuhax splash (90deg rotated RGB8 pixel data)
    clear_screens();

    set_font(PATH_BITS "/termfont.bin");

    if (c) {
        // Failed to mount SD. Bomb out.
        abort("Failed to mount SD card.\n");
    }

    if (argc >= 1 && argc < 2) {
        // Valid argc passed.
        fprintf(stderr, "Chainloaded. Path: %s\n", argv[0]);
    }

    load_config(); // Load configuration.

    if (CFG_BOOTENV == 7) {
        fprintf(stderr, "Rebooted from AGB, disabling EmuNAND.\n");
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
