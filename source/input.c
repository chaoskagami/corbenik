#include <stdint.h>
#include <common.h>

extern void waitcycles(uint32_t cycles);

#define ARM9_APPROX_DELAY_MAX 134058675 / 95

uint32_t
wait_key(_UNUSED int sleep)
{
    uint32_t ret = 0, get = 0;
    while (ret == 0) {
        get = ctr_hid_get_buttons();

        if ((get & (CTR_HID_LT | CTR_HID_RT | CTR_HID_START)) == (CTR_HID_LT | CTR_HID_RT | CTR_HID_START)) {
            screenshot();
            waitcycles(ARM9_APPROX_DELAY_MAX); // Approximately what a human can input - fine tuning needed (sorry, TASers!)
        } else if (get & CTR_HID_UP)
            ret = CTR_HID_UP;
        else if (get & CTR_HID_DOWN)
            ret = CTR_HID_DOWN;
        else if (get & CTR_HID_RIGHT)
            ret = CTR_HID_RIGHT;
        else if (get & CTR_HID_LEFT)
            ret = CTR_HID_LEFT;
        else if (get & CTR_HID_A)
            ret = CTR_HID_A;
        else if (get & CTR_HID_B)
            ret = CTR_HID_B;
        else if (get & CTR_HID_X)
            ret = CTR_HID_X;
        else if (get & CTR_HID_SELECT)
            ret = CTR_HID_SELECT;

    }
    while (ctr_hid_get_buttons());

    return ret;
}

extern int doing_autoboot;

void
wait()
{
    if (config->options[OPTION_TRACE] && !doing_autoboot) {
        fprintf(stderr, "[Waiting...]");
        wait_key(0); // No delay on traces.
    }
    fprintf(stderr, "\r            \r");
}

