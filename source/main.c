#include <ctr9/ctr_hid.h>
#include <ctr9/io.h>
#include <ctr9/ctr_screen.h>
#include <ctr9/i2c.h>

#include "common.h"
#include "firm/firm.h"
#include "input.h"
#include "config.h"
#include "screeninit.h"
#include "std/abort.h"
#include "interrupt.h"

int is_n3ds = 0;
int doing_autoboot = 0;

int menu_handler();
void shut_up();

int
main(int argc, char** argv)
{
    if (PDN_MPCORE_CFG == 7)
        is_n3ds = 1; // Enable n3ds specific options.

    std_init();

    int c = fmount();

    screen_mode(0); // Use RGBA8 mode.

    clear_bg();

    load_bg_top   (PATH_BITS "/top.bin");
    load_bg_bottom(PATH_BITS "/bottom.bin"); // This is basically a menuhax splash (90deg rotated BGR8 pixel data)

    clear_disp(TOP_SCREEN);
    clear_disp(BOTTOM_SCREEN);

    ctr_screen_enable_backlight(CTR_SCREEN_BOTH);

    set_font(PATH_BITS "/termfont.bin");

    install_interrupts(); // Get some free debug info.

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
    if (config.options[OPTION_AUTOBOOT] && !(ctr_hid_get_buttons() & CTR_HID_RT)) {
        if (config.options[OPTION_SILENCE])
            shut_up(); // This does exactly what it sounds like.
        doing_autoboot = 1;
    } else {
        menu_handler();
    }

    boot_cfw();
    // Under ideal conditions, we never get here.
}
