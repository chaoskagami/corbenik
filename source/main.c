
#include <ctr9/ctr_hid.h>
#include <ctr9/io.h>
#include <ctr9/ctr_screen.h>
#include <ctr9/i2c.h>

#include <common.h>

int is_n3ds = 0;
int doing_autoboot = 0;

int menu_handler(void);
void shut_up(void);

extern int changed_consoles;

int
main(int argc, char** argv)
{
    int have_bg = 0;
    int si = 0;

    int r_held = (ctr_hid_get_buttons() & CTR_HID_RT);

    if (PDN_MPCORE_CFG == 7)
        is_n3ds = 1; // Enable n3ds specific options.

    std_init();

    if (crmount())
        poweroff(); // Failed to mount SD. Bomb out.

    load_config(); // Load configuration.

    install_interrupts(); // Get some free debug info.

    installArm11Stub();

    if (CFG_BOOTENV == 7)
        set_opt_u32(OPTION_EMUNAND, 0); // Disable EmuNAND on AGB reboot.

    set_font(PATH_TERMFONT); // Read the font before all else.

    // Check key down for autoboot
    set_fb_struct();

    clear_bg();

    // This is a menuhax splash (90deg rotated BGR8 pixel data)
    int bg_top    = load_bg_top(PATH_TOP_BG);
    int bg_bottom = load_bg_bottom(PATH_BOTTOM_BG);

    have_bg = (bg_top || bg_bottom);

    while (1) {
        if (get_opt_u32(OPTION_AUTOBOOT) && !r_held && !doing_autoboot) {
            doing_autoboot = 1;

            if (get_opt_u32(OPTION_SILENCE))
                shut_up(); // This does exactly what it sounds like.

            if (have_bg && !si) {
                screen_mode(RGBA8); // Use RGBA8 mode.
                si = 1;

                clear_disp(TOP_SCREEN);
                clear_disp(BOTTOM_SCREEN);
            }
        } else {
            if (!si) {
                screen_mode(RGBA8); // Use RGBA8 mode.
                si = 1;

                clear_disp(TOP_SCREEN);
                clear_disp(BOTTOM_SCREEN);
            }
            menu_handler();
        }

        if (changed_consoles) {
            fprintf(stderr, "Console changed, regenerating caches\n");
            save_config();
            generate_patch_cache();
        }

        if (prepatch_firm(config->firm[1], PATH_TWL_P, PATH_MODULE_TWL)) {
            fprintf(stderr, "WARNING: Failed to load/patch TWL.\n");
            wait();
        }

        if(prepatch_firm(config->firm[2], PATH_AGB_P, PATH_MODULE_AGB)) {
            fprintf(stderr, "WARNING: Failed to load/patch AGB.\n");
            wait();
        }

        if (get_opt_u32(OPTION_OVERLY_VERBOSE) && !get_opt_u32(OPTION_SILENCE)) {
            struct mallinfo mal = mallinfo();
            fprintf(stderr, "arena: %u\n"
                            "ordblks: %u\n"
                            "uordblks: %u\n"
                            "fordblks: %u\n",
                            mal.arena, mal.ordblks, mal.uordblks, mal.fordblks);
            wait();
        }

        boot_firm(config->firm[0], PATH_NATIVE_P, PATH_MODULE_NATIVE);

        fprintf(stderr, "Firmlaunch failed, returning to menu\n");
    }
}
