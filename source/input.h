#ifndef __INPUT_H
#define __INPUT_H

#define BUTTON_A (1 << 0)
#define BUTTON_B (1 << 1)
#define BUTTON_SEL (1 << 2)
#define BUTTON_STA (1 << 3)
#define BUTTON_RIGHT (1 << 4)
#define BUTTON_LEFT (1 << 5)
#define BUTTON_UP (1 << 6)
#define BUTTON_DOWN (1 << 7)
#define BUTTON_R (1 << 8)
#define BUTTON_L (1 << 9)
#define BUTTON_X (1 << 10)
#define BUTTON_Y (1 << 11)
/* FIXME - I had ZR and ZL in, but they don't appear to
           behave rationally without some initialization. */

#define BUTTON_ANY 0xFFF

#define HID_PAD ((*(volatile uint32_t *)0x10146000) ^ BUTTON_ANY)

uint32_t wait_key(_UNUSED int sleep);

#endif
