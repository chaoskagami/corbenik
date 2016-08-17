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

    load_config(); // Load configuration.

    install_interrupts(); // Get some free debug info.

    installArm11Stub();

    if (CFG_BOOTENV == 7)
        config->options[OPTION_EMUNAND] = 0; // Disable EmuNAND on AGB reboot.

    if (config->options[OPTION_AUTOBOOT] && !(ctr_hid_get_buttons() & CTR_HID_RT)) {
        doing_autoboot = 1;

        if (config->options[OPTION_SILENCE] || config->options[OPTION_SILENT_NOSCREEN])
            shut_up(); // This does exactly what it sounds like.

        if (config->options[OPTION_SILENT_NOSCREEN])
            boot_cfw(); // Skip all other checks and just boot.
    }

    set_font(PATH_TERMFONT); // Read the font before all else.

    // Check key down for autoboot
    screen_mode(RGBA8); // Use RGBA8 mode.

    clear_bg();

    load_bg_top   (PATH_TOP_BG);
    load_bg_bottom(PATH_BOTTOM_BG); // This is a menuhax splash (90deg rotated BGR8 pixel data)

    clear_disp(TOP_SCREEN);
    clear_disp(BOTTOM_SCREEN);

    // Autoboot. Non-standard code path.
    if (!doing_autoboot)
        menu_handler();

    boot_cfw();
}
