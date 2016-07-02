#include <stdint.h>
#include "std/unused.h"
#include "std/draw.h"
#include "input.h"

extern void waitcycles(uint32_t cycles);

uint32_t
wait_key(_UNUSED int sleep)
{
    // If your dpad has issues, please add this to the makefile.
    if (sleep) {
        #define ARM9_APPROX_DELAY_MAX 134058675 / 95
        waitcycles(ARM9_APPROX_DELAY_MAX); // Approximately what a human can input - fine tuning needed (sorry, TASers!)
    }

    uint32_t ret = 0, get = 0;
    while (ret == 0) {
        get = HID_PAD;

        if ((get & (BUTTON_L | BUTTON_R | BUTTON_STA)) == (BUTTON_L | BUTTON_R | BUTTON_STA)) {
            screenshot();
            waitcycles(ARM9_APPROX_DELAY_MAX); // Approximately what a human can input - fine tuning needed (sorry, TASers!)
        } else if (get & BUTTON_UP)
            ret = BUTTON_UP;
        else if (get & BUTTON_DOWN)
            ret = BUTTON_DOWN;
        else if (get & BUTTON_RIGHT)
            ret = BUTTON_RIGHT;
        else if (get & BUTTON_LEFT)
            ret = BUTTON_LEFT;
        else if (get & BUTTON_A)
            ret = BUTTON_A;
        else if (get & BUTTON_B)
            ret = BUTTON_B;
        else if (get & BUTTON_X)
            ret = BUTTON_X;
        else if (get & BUTTON_SEL)
            ret = BUTTON_SEL;

    }
    while (HID_PAD & ret);

    return ret;
}

