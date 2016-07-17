#include <ctr9/io.h>
#include <ctr9/ctr_screen.h>
#include <ctr9/i2c.h>
#include "common.h"
#include "std/fs.h"
#include "patch_format.h"

#define PDN_GPU_CNT (*(volatile uint8_t *)0x10141200)

static volatile uint32_t *const a11_entry = (volatile uint32_t *)0x1FFFFFF8;

void
screen_mode(uint32_t mode)
{
    uint32_t *screenInitAddress = (uint32_t *)0x24FFFC00;

    FILE *f = fopen(PATH_SCREENINIT_CODE, "r");
    fread(screenInitAddress, 1, fsize(f), f); // Read in the screeninit payload.
    fclose(f);

    // FIXME - At the moment, this seems mandatory to do full screeninit.

    // I get very fucked up results from just changing the framebuffer offsets
    // and display color mode. Until I figure out WHY a full screeninit has to
    // be performed, I have to do a full screeninit.

    // And no, 3dbrew didn't help. Partial init seems to be a superset of what
    // I was attempting.

//    if (PDN_GPU_CNT == 1) {
        screenInitAddress[2] = 0; // Do a full init.
        screenInitAddress[3] = 0xFF; // Brightness
        screenInitAddress[4] = mode; // Mode

        *a11_entry = (uint32_t)screenInitAddress;

        while (*a11_entry);

        // Turn on backlight
        i2cWriteRegister(I2C_DEV_MCU, 0x22, 1 << 1);

        // FIXME - Time to yell at Gelex now. :|
        ctr_screen_enable_backlight(CTR_SCREEN_BOTH);
//    }
}
