#include <ctr9/ctr_hid.h>
#include <ctr9/io.h>
#include <ctr9/ctr_screen.h>
#include <ctr9/i2c.h>

#include <common.h>

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

    if (c) {
        // Failed to mount SD. Bomb out.
        // TODO - What the hell does one even do in this situation?
        //        Spin until the card is available to mount, maybe?
        abort("Failed to mount SD card.\n");
    }

    if (argc >= 1 && argc < 2) {
        // Valid argc passed.
        fprintf(stderr, "Chainloaded. Path: %s\n", argv[0]);
    }

    set_font(PATH_TERMFONT); // Read the font before all else.

    load_config(); // Load configuration.

    screen_mode(0); // Use RGBA8 mode.

    clear_bg();

    load_bg_top   (PATH_TOP_BG);
    load_bg_bottom(PATH_BOTTOM_BG); // This is basically a menuhax splash (90deg rotated BGR8 pixel data)

    clear_disp(TOP_SCREEN);
    clear_disp(BOTTOM_SCREEN);

    ctr_screen_enable_backlight(CTR_SCREEN_BOTH);

    install_interrupts(); // Get some free debug info.

    menu_handler();
}
