#include "std/fs.h"
#include "i2c.h"
#include "patch_format.h"

#define PDN_GPU_CNT (*(volatile uint8_t *)0x10141200)

static volatile uint32_t *const a11_entry = (volatile uint32_t *)0x1FFFFFF8;

void screen_init() {
	if (PDN_GPU_CNT == 1) {
		uint32_t* screenInitAddress = (uint32_t*)0x24FFFC00;

		FILE* f = fopen(PATH_SCREENINIT_CODE, "r");
		fread(screenInitAddress, 1, fsize(f), f); // Read in the screeninit payload.
		fclose(f);

		//Write brightness level for the stub to pick up
		screenInitAddress[2] = 0x5F;
		*a11_entry = (uint32_t)screenInitAddress;

		while(*a11_entry);

		//Turn on backlight
		i2cWriteRegister(I2C_DEV_MCU, 0x22, 0x2A);
	}
}
