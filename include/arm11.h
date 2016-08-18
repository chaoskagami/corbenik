#ifndef __SCREENINIT_H
#define __SCREENINIT_H

#define PDN_GPU_CNT (*(volatile uint32_t*)0x10141200)

#define LCD_REG(offset) (*((volatile uint32_t*)(0x10202000 + (offset))))
#define LCD_TOP_CONF_REG(offset) (*((volatile uint32_t*)(0x10202200 + (offset))))
#define LCD_BOT_CONF_REG(offset) (*((volatile uint32_t*)(0x10202A00 + (offset))))
#define LCD_TOP_CONF_BRIGHTNESS LCD_TOP_CONF_REG(0x40)
#define LCD_BOT_CONF_BRIGHTNESS LCD_BOT_CONF_REG(0x40)

#define PDC0_FRAMEBUFFER_SETUP_REG(offset) (*((volatile uint32_t*)(0x10400400 + (offset))))
#define PDC1_FRAMEBUFFER_SETUP_REG(offset) (*((volatile uint32_t*)(0x10400500 + (offset))))

#define PDC0_FRAMEBUFFER_SETUP_DIMS PDC0_FRAMEBUFFER_SETUP_REG(0x5C)
#define PDC0_FRAMEBUFFER_SETUP_FBA_ADDR_1 PDC0_FRAMEBUFFER_SETUP_REG(0x68)
#define PDC0_FRAMEBUFFER_SETUP_FBA_ADDR_2 PDC0_FRAMEBUFFER_SETUP_REG(0x6C)
#define PDC0_FRAMEBUFFER_SETUP_FB_FORMAT PDC0_FRAMEBUFFER_SETUP_REG(0x70)
#define PDC0_FRAMEBUFFER_SETUP_FB_SELECT PDC0_FRAMEBUFFER_SETUP_REG(0x78)
#define PDC0_FRAMEBUFFER_SETUP_DISCO PDC0_FRAMEBUFFER_SETUP_REG(0x84)
#define PDC0_FRAMEBUFFER_SETUP_FB_STRIDE PDC0_FRAMEBUFFER_SETUP_REG(0x90)
#define PDC0_FRAMEBUFFER_SETUP_FBB_ADDR_1 PDC0_FRAMEBUFFER_SETUP_REG(0x94)
#define PDC0_FRAMEBUFFER_SETUP_FBB_ADDR_2 PDC0_FRAMEBUFFER_SETUP_REG(0x98)

#define PDC1_FRAMEBUFFER_SETUP_DIMS PDC1_FRAMEBUFFER_SETUP_REG(0x5C)
#define PDC1_FRAMEBUFFER_SETUP_FBA_ADDR_1 PDC1_FRAMEBUFFER_SETUP_REG(0x68)
#define PDC1_FRAMEBUFFER_SETUP_FBA_ADDR_2 PDC1_FRAMEBUFFER_SETUP_REG(0x6C)
#define PDC1_FRAMEBUFFER_SETUP_FB_FORMAT PDC1_FRAMEBUFFER_SETUP_REG(0x70)
#define PDC1_FRAMEBUFFER_SETUP_FB_SELECT PDC1_FRAMEBUFFER_SETUP_REG(0x78)
#define PDC1_FRAMEBUFFER_SETUP_DISCO PDC1_FRAMEBUFFER_SETUP_REG(0x84)
#define PDC1_FRAMEBUFFER_SETUP_FB_STRIDE PDC1_FRAMEBUFFER_SETUP_REG(0x90)
#define PDC1_FRAMEBUFFER_SETUP_FBB_ADDR_1 PDC1_FRAMEBUFFER_SETUP_REG(0x94)
#define PDC1_FRAMEBUFFER_SETUP_FBB_ADDR_2 PDC1_FRAMEBUFFER_SETUP_REG(0x98)

#define MAKE_FRAMEBUFFER_PIXFMT(col, plx, screen) ((col & 0x7) | ((plx & 1) << 5) | ((screen & 1) << 6) | 0x80300)

#define RGBA8        0
#define BGR8         1
#define RGB565_OES   2
#define RGB5_A1_OES  3
#define RGBA4_OES    4

#define ARM11_STUB_ADDRESS (0x25000000 - 0x30) //It's currently only 0x28 bytes large. We're putting 0x30 just to be sure here
#define WAIT_FOR_ARM9() *arm11Entry = 0; while(!*arm11Entry); ((void (*)())*arm11Entry)();

/* Initializes the screen and sets the display mode.
 *
 * \param mode Screen mode to initialize in, one of RGBA8, BGR8, RGB565_OES, RGB5_A1_OES, or RGBA4_OES
 */
void screen_mode(uint32_t mode);

/* Invokes a bare ARM11 function. For usage, see arm11.c.
 *
 * \param func Function to call.
 */
void invokeArm11Function(void (*func)());

/* Deinitializes the screens.
 */
void deinitScreens(void);

/* Sets the brightness.
 *
 * \param brightnessIndex The brightness level, in the range 0-4.
 */
void updateBrightness(uint32_t brightnessIndex);

/* Clears screens to black via GPU fill.
 */
void clearScreens(void);

/* Copies the ARM11 stub. Called automatically by invokeArm11Function if needed.
 */
void installArm11Stub(void);

#endif
