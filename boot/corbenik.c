#include <stdint.h>
#include <stdlib.h>
#include <arm11.h>         // for screen_mode, RGBA8, installArm11Stub, set_...
#include <ctr9/ctr_hid.h>  // for ctr_hid_get_buttons, CTR_HID_RT
#include <ctr9/ctr_system.h>  // for ctr_system_poweroff
#include <firm/headers.h>     // for prepatch_firm, boot_firm
#include <firm/firm.h>     // for prepatch_firm, boot_firm
#include <input.h>         // for wait
#include <interrupt.h>     // for install_interrupts
#include <malloc.h>        // for mallinfo
#include <menu.h>          // for poweroff
#include <option.h>        // for get_opt_u32, config, config_file, OPTION_S...
#include <patcher.h>       // for generate_patch_cache
#include <std/draw.h>      // for clear_disp, stderr, BOTTOM_SCREEN, TOP_SCREEN
#include <std/fs.h>        // for crmount
#include <std/types.h>     // for CFG_BOOTENV, PDN_MPCORE_CFG
#include <structures.h>    // for PATH_AGB_P, PATH_BOTTOM_BG, PATH_MODULE_AGB

int is_n3ds = 0;
int doing_autoboot = 0;

int menu_handler(void);
void shut_up(void);

extern int changed_consoles;

extern uint16_t titleid_passthru[8];

int get_firmtype() {
    if (titleid_passthru[5] >= u'0' && titleid_passthru[5] <= u'2')
        return titleid_passthru[5] - u'0';

    return 0;
}

int
main(int argc, char** argv)
{
    if (get_firmtype() != 0)
        ctr_system_poweroff();

    int have_bg = 0;
    int si = 0;

    int r_held = (ctr_hid_get_buttons() & CTR_HID_RT);

    if (PDN_MPCORE_CFG == 7)
        is_n3ds = 1; // Enable n3ds specific options.

    std_init();

    if (crmount())
        ctr_system_poweroff(); // We can't use poweroff here, since that does n/a cleanup that will cause a hang.

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
                screen_mode(RGBA8, get_opt_u32(OPTION_BRIGHTNESS)); // Use RGBA8 mode.
                si = 1;

                clear_disp(TOP_SCREEN);
                clear_disp(BOTTOM_SCREEN);
            }
        } else {
            if (!si) {
                screen_mode(RGBA8, get_opt_u32(OPTION_BRIGHTNESS)); // Use RGBA8 mode.
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
