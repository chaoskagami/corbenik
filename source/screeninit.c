#include <common.h>
#include <ctr9/io.h>
#include <ctr9/ctr_screen.h>
#include <ctr9/i2c.h>

#define PDN_GPU_CNT (*(volatile uint8_t *)0x10141200)

static volatile uint32_t *const a11_entry = (volatile uint32_t *)0x1FFFFFF8;

struct framebuffers* framebuffers = NULL;

#define FB_FMT(col, plx, screen) ((col & 0b111) | ((plx & 1) << 5) | ((screen & 1) << 6) | 0b10000000001100000000)

#define RGBA8        0
#define BGR8         1
#define RGB565_OES   2
#define RGB5_A1_OES  3
#define RGBA4_OES    4

#define INIT_FULL    0
#define INIT_PARAMS  1
#define INIT_DEINIT  2

uint32_t do_init = 0;
uint32_t brightnessLevel = 0;
uint32_t init_mode = 0;

void __attribute__((naked)) screen11(void) {
   // FIXME - We could use some serious macros here...

   uint32_t yaw, init_top, init_bottom;
   switch(init_mode) {
       case RGBA8:
           yaw = 240 * 4;
           break;
       case BGR8:
       case RGB565_OES:
       case RGB5_A1_OES:
       case RGBA4_OES:
           yaw = 240 * 3;
           break;
   }

   init_top    = FB_FMT(init_mode, 0, 1);
   init_bottom = FB_FMT(init_mode, 0, 0);

   volatile uint32_t *const arm11 = (uint32_t *)0x1FFFFFF8;

   if (do_init == INIT_FULL) {
   *(volatile uint32_t *)0x10141200 = 0x1007F;
   *(volatile uint32_t *)0x10202014 = 0x00000001;
   *(volatile uint32_t *)0x1020200C &= 0xFFFEFFFE;
   }

   *(volatile uint32_t *)0x10202240 = brightnessLevel; // Alteration; directly read brightness.
   *(volatile uint32_t *)0x10202A40 = brightnessLevel;

   if (do_init == INIT_FULL) {
   *(volatile uint32_t *)0x10202244 = 0x1023E;
   *(volatile uint32_t *)0x10202A44 = 0x1023E;
   }

   // Top screen
   *(volatile uint32_t *)0x10400400 = 0x000001c2;
   *(volatile uint32_t *)0x10400404 = 0x000000d1;
   *(volatile uint32_t *)0x10400408 = 0x000001c1;
   *(volatile uint32_t *)0x1040040c = 0x000001c1;
   *(volatile uint32_t *)0x10400410 = 0x00000000;
   *(volatile uint32_t *)0x10400414 = 0x000000cf;
   *(volatile uint32_t *)0x10400418 = 0x000000d1;
   *(volatile uint32_t *)0x1040041c = 0x01c501c1;
   *(volatile uint32_t *)0x10400420 = 0x00010000;
   *(volatile uint32_t *)0x10400424 = 0x0000019d;
   *(volatile uint32_t *)0x10400428 = 0x00000002;
   *(volatile uint32_t *)0x1040042c = 0x00000192;
   *(volatile uint32_t *)0x10400430 = 0x00000192;
   *(volatile uint32_t *)0x10400434 = 0x00000192;
   *(volatile uint32_t *)0x10400438 = 0x00000001;
   *(volatile uint32_t *)0x1040043c = 0x00000002;
   *(volatile uint32_t *)0x10400440 = 0x01960192;
   *(volatile uint32_t *)0x10400444 = 0x00000000;
   *(volatile uint32_t *)0x10400448 = 0x00000000;

   *(volatile uint32_t *)0x1040045C = 0x00f00190;
   *(volatile uint32_t *)0x10400460 = 0x01c100d1;
   *(volatile uint32_t *)0x10400464 = 0x01920002;
   *(volatile uint32_t *)0x10400468 = 0x18300000;
   *(volatile uint32_t *)0x10400470 = init_top; // Format
   *(volatile uint32_t *)0x10400474 = 0x00010501;
   *(volatile uint32_t *)0x10400478 = 0;
   *(volatile uint32_t *)0x10400490 = yaw;
   *(volatile uint32_t *)0x1040049C = 0x00000000;

   // Disco register
   for(uint32_t i = 0; i < 256; i++)
       *(volatile uint32_t *)0x10400484 = 0x10101 * i;

   // Bottom screen
   *(volatile uint32_t *)0x10400500 = 0x000001c2;
   *(volatile uint32_t *)0x10400504 = 0x000000d1;
   *(volatile uint32_t *)0x10400508 = 0x000001c1;
   *(volatile uint32_t *)0x1040050c = 0x000001c1;
   *(volatile uint32_t *)0x10400510 = 0x000000cd;
   *(volatile uint32_t *)0x10400514 = 0x000000cf;
   *(volatile uint32_t *)0x10400518 = 0x000000d1;
   *(volatile uint32_t *)0x1040051c = 0x01c501c1;
   *(volatile uint32_t *)0x10400520 = 0x00010000;
   *(volatile uint32_t *)0x10400524 = 0x0000019d;
   *(volatile uint32_t *)0x10400528 = 0x00000052;
   *(volatile uint32_t *)0x1040052c = 0x00000192;
   *(volatile uint32_t *)0x10400530 = 0x00000192;
   *(volatile uint32_t *)0x10400534 = 0x0000004f;
   *(volatile uint32_t *)0x10400538 = 0x00000050;
   *(volatile uint32_t *)0x1040053c = 0x00000052;
   *(volatile uint32_t *)0x10400540 = 0x01980194;
   *(volatile uint32_t *)0x10400544 = 0x00000000;
   *(volatile uint32_t *)0x10400548 = 0x00000011;
   *(volatile uint32_t *)0x1040055C = 0x00f00140;
   *(volatile uint32_t *)0x10400560 = 0x01c100d1;
   *(volatile uint32_t *)0x10400564 = 0x01920052;
   *(volatile uint32_t *)0x10400568 = 0x18300000 + (yaw * 400);
   *(volatile uint32_t *)0x10400570 = init_bottom; // Format
   *(volatile uint32_t *)0x10400574 = 0x00010501;
   *(volatile uint32_t *)0x10400578 = 0;
   *(volatile uint32_t *)0x10400590 = yaw;
   *(volatile uint32_t *)0x1040059C = 0x00000000;

   // Disco register
   for(uint32_t i = 0; i < 256; i++)
       *(volatile uint32_t *)0x10400584 = 0x10101 * i;

   *(volatile uint32_t *)0x10400468 = 0x18300000;
   *(volatile uint32_t *)0x1040046c = 0x18300000;
   *(volatile uint32_t *)0x10400494 = 0x18300000;
   *(volatile uint32_t *)0x10400498 = 0x18300000;
   *(volatile uint32_t *)0x10400568 = 0x18300000 + (yaw * 400);
   *(volatile uint32_t *)0x1040056c = 0x18300000 + (yaw * 400);

   //Set CakeBrah framebuffers
   *((volatile uint32_t *)0x23FFFE00) = 0x18300000;
   *((volatile uint32_t *)0x23FFFE04) = 0x18300000;
   *((volatile uint32_t *)0x23FFFE08) = 0x18300000 + (yaw * 400);

   //Clear ARM11 entry offset
   *arm11 = 0;

   //Wait for the entry to be set
   while(!*arm11);

   //Jump to it
   ((void (*)())*arm11)();
}

void
screen_mode(uint32_t mode)
{
    // FIXME - At the moment, this seems mandatory to do full screeninit.

    // I get very fucked up results from just changing the framebuffer offsets
    // and display color mode. Until I figure out WHY a full screeninit has to
    // be performed, I have to do a full screeninit.

    // And no, 3dbrew didn't help. Partial init seems to be a superset of what
    // I was attempting.

    do_init = 1; // Do a partial init.

    if (PDN_GPU_CNT == 1)
        do_init = 0; // Do a full init.

	// FIXME - God awful syntactical hack.
    brightnessLevel = ("\x40\x8F\xC0\xFF")[config->options[OPTION_BRIGHTNESS]];

    init_mode = mode; // Mode

    *a11_entry = (uint32_t)screen11;

    while (*a11_entry);

    if (!framebuffers) {
        framebuffers = malloc(sizeof(struct framebuffers));
        memcpy(framebuffers, framebuffers_cakehax, sizeof(struct framebuffers));
    }

    // Turn on backlight
    i2cWriteRegister(I2C_DEV_MCU, 0x22, 1 << 1);
}
