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

    if (fmount())
        poweroff(); // Failed to mount SD. Bomb out.

    set_font(PATH_TERMFONT); // Read the font before all else.

    load_config(); // Load configuration.

    screen_mode(RGBA8); // Use RGBA8 mode.

    clear_bg();

    load_bg_top   (PATH_TOP_BG);
    load_bg_bottom(PATH_BOTTOM_BG); // This is basically a menuhax splash (90deg rotated BGR8 pixel data)

    clear_disp(TOP_SCREEN);
    clear_disp(BOTTOM_SCREEN);

//    ctr_screen_enable_backlight(CTR_SCREEN_BOTH);

//    install_interrupts(); // Get some free debug info.

    if (CFG_BOOTENV == 7) {
        fprintf(stderr, "Rebooted from AGB, disabling EmuNAND.\n");
        config->options[OPTION_EMUNAND] = 0;
    }

    // Autoboot. Non-standard code path.
    if (config->options[OPTION_AUTOBOOT] && !(ctr_hid_get_buttons() & CTR_HID_RT)) {
        if (config->options[OPTION_SILENCE])
            shut_up(); // This does exactly what it sounds like.
        doing_autoboot = 1;
    } else {
        menu_handler();
    }

    boot_cfw();
    // Under ideal conditions, we never get here.
}
