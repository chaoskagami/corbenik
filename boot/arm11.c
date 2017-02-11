//   Copyright (C) 2016 Aurora Wright, TuxSH

//   This program is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.

//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.

//   You should have received a copy of the GNU General Public License
//   along with this program.  If not, see <http://www.gnu.org/licenses/>.

//   Additional Terms 7.b of GPLv3 applies to this file: Requiring preservation of specified
//   reasonable legal notices or author attributions in that material or in the Appropriate Legal
//   Notices displayed by works containing it.

// ============================================

//   Screen init code by dark_samus, bil1s, Normmatt, delebile and others.
//   Screen deinit code by tiniVi.
//   More readable screeninit by Gelex.

// ============================================

//   This is less a permanent solution and more a quickfix since I'd rather use libctr11
//   whenever Gelex finishes it. Don't rely on it staying here.

//   Also, this contains quite a few changes from luma's screen.c, which is where it originates from.

#include <string.h>
#include <stdbool.h>         // for false, true
#include <stdint.h>          // for uint32_t, uint8_t
#include <arm11.h>           // for PDC0_FRAMEBUFFER_SETUP_REG, PDC1_FRAMEBU...
#include <ctr9/ctr_cache.h>  // for ctr_cache_clean_and_flush_all
#include <ctr9/i2c.h>        // for i2cWriteRegister, I2C_DEV_MCU
#include <malloc.h>          // for memalign
#include <std/draw.h>        // for framebuffers
#include <util.h>

struct framebuffers *framebuffers;

volatile uint32_t *arm11Entry = (volatile uint32_t *)0x1FFFFFF8;
static const uint32_t brightness[4] = {0x26, 0x39, 0x4C, 0x5F};

void  __attribute__((naked)) arm11Stub(void)
{
    //Disable interrupts
    __asm(".word 0xF10C01C0");

    //Wait for the entry to be set
    while(*arm11Entry == ARM11_STUB_ADDRESS);

    //Jump to it
    ((void (*)())*arm11Entry)();
}

void installArm11Stub(void) {
    static int hasCopiedStub = false;
    if(!hasCopiedStub)
    {
        memcpy((void *)ARM11_STUB_ADDRESS, arm11Stub, 0x30);
        ctr_cache_clean_and_flush_all();
        hasCopiedStub = true;
    }

    if (is_firmlaunch())
        arm11Entry = (volatile uint32_t*)0x1FFFFFFC;
}

void invokeArm11Function(void (*func)())
{
    installArm11Stub();

    *arm11Entry = (uint32_t)func;
    while(*arm11Entry);
    *arm11Entry = ARM11_STUB_ADDRESS;
}

void deinitScreens(void)
{
    void __attribute__((naked)) ARM11(void)
    {
        //Disable interrupts
        __asm(".word 0xF10C01C0");

        //Shutdown LCDs
        *(volatile uint32_t *)0x10202A44 = 0;
        *(volatile uint32_t *)0x10202244 = 0;
        *(volatile uint32_t *)0x1020200C = 0;
        *(volatile uint32_t *)0x10202014 = 0;

        WAIT_FOR_ARM9();
    }

    // If screen is initialized, invoke.
    if(PDN_GPU_CNT != 1) invokeArm11Function(ARM11);
}

void updateBrightness(uint32_t brightnessIndex)
{
    static uint32_t brightnessLevel;
    brightnessLevel = brightness[brightnessIndex];

    void __attribute__((naked)) ARM11(void)
    {
        //Disable interrupts
        __asm(".word 0xF10C01C0");

        //Change brightness
        *(volatile uint32_t *)0x10202240 = brightnessLevel;
        *(volatile uint32_t *)0x10202A40 = brightnessLevel;

        WAIT_FOR_ARM9();
    }

    ctr_cache_clean_and_flush_all();
    invokeArm11Function(ARM11);
}

void clearScreens(void) {
    void __attribute__((naked)) ARM11(void)
    {
        //Disable interrupts
        __asm(".word 0xF10C01C0");

        //Setting up two simultaneous memory fills using the GPU
        volatile uint32_t *REGs_PSC0 = (volatile uint32_t *)0x10400010;

        REGs_PSC0[0] = (uint32_t)framebuffers->top_left >> 3; //Start address
        REGs_PSC0[1] = (uint32_t)(framebuffers->top_left + (400 * 240 * 4)) >> 3; //End address
        REGs_PSC0[2] = 0; //Fill value
        REGs_PSC0[3] = (2 << 8) | 1; //32-bit pattern; start

        volatile uint32_t *REGs_PSC1 = (volatile uint32_t *)0x10400020;

        REGs_PSC1[0] = (uint32_t)framebuffers->bottom >> 3; //Start address
        REGs_PSC1[1] = (uint32_t)(framebuffers->bottom + (320 * 240 * 4)) >> 3; //End address
        REGs_PSC1[2] = 0; //Fill value
        REGs_PSC1[3] = (2 << 8) | 1; //32-bit pattern; start

        while(!((REGs_PSC0[3] & 2) && (REGs_PSC1[3] & 2)));

        WAIT_FOR_ARM9();
    }

    ctr_cache_clean_and_flush_all();
    invokeArm11Function(ARM11);
}

void set_fb_struct() {
    if (!framebuffers) {
        // Look ma, dynamically allocating the CakeHax struct! (joking)
        // We literally just discard the previous state - for sanity's sake.
        // On chainload, it is needed to copy the framebuffer struct.
        framebuffers = memalign(16, sizeof(struct framebuffers));

        // Set not-actually cakebrah framebuffers. Meh.
        framebuffers->top_left  = (uint8_t *)0x18300000;
        framebuffers->top_right = (uint8_t *)0x18300000;
        framebuffers->bottom    = (uint8_t *)0x1835dc00;
    }
}

void screen_mode(uint32_t mode, uint32_t bright_level) {
    static uint32_t stride, init_top, init_bottom, bright;

    bright = brightness[bright_level];

    stride = 240;
    switch(mode) {
        case RGB8:
            stride *= 3;
            break;
        case RGBA8:
            stride *= 4;
            break;
        default:
            stride *= 2;
            break;
    }

    init_top    = MAKE_FRAMEBUFFER_PIXFMT(mode, 0, 1);
    init_bottom = MAKE_FRAMEBUFFER_PIXFMT(mode, 0, 0);

    void __attribute__((naked)) ARM11(void) {
        //Disable interrupts
        __asm(".word 0xF10C01C0");

        PDN_GPU_CNT = 0x1007F; //bit0: Enable GPU regs 0x10400000+, bit16 turn on LCD backlight?
        LCD_REG(0x14) = 0x00000001; //UNKNOWN register, maybe LCD related? 0x10202000
        LCD_REG(0xC) &= 0xFFFEFFFE; //UNKNOWN register, maybe LCD related?

        LCD_TOP_CONF_BRIGHTNESS = bright;
        LCD_BOT_CONF_BRIGHTNESS = bright;
        LCD_TOP_CONF_REG(0x44) = 0x1023E; //unknown
        LCD_BOT_CONF_REG(0x44) = 0x1023E; //unknown

        // Top screen
        PDC0_FRAMEBUFFER_SETUP_REG(0x00) = 0x000001c2; //unknown
        PDC0_FRAMEBUFFER_SETUP_REG(0x04) = 0x000000d1; //unknown
        PDC0_FRAMEBUFFER_SETUP_REG(0x08) = 0x000001c1;
        PDC0_FRAMEBUFFER_SETUP_REG(0x0c) = 0x000001c1;
        PDC0_FRAMEBUFFER_SETUP_REG(0x10) = 0x00000000;
        PDC0_FRAMEBUFFER_SETUP_REG(0x14) = 0x000000cf;
        PDC0_FRAMEBUFFER_SETUP_REG(0x18) = 0x000000d1;
        PDC0_FRAMEBUFFER_SETUP_REG(0x1c) = 0x01c501c1;
        PDC0_FRAMEBUFFER_SETUP_REG(0x20) = 0x00010000;
        PDC0_FRAMEBUFFER_SETUP_REG(0x24) = 0x0000019d;
        PDC0_FRAMEBUFFER_SETUP_REG(0x28) = 0x00000002;
        PDC0_FRAMEBUFFER_SETUP_REG(0x2c) = 0x00000192;
        PDC0_FRAMEBUFFER_SETUP_REG(0x30) = 0x00000192;
        PDC0_FRAMEBUFFER_SETUP_REG(0x34) = 0x00000192;
        PDC0_FRAMEBUFFER_SETUP_REG(0x38) = 0x00000001;
        PDC0_FRAMEBUFFER_SETUP_REG(0x3c) = 0x00000002;
        PDC0_FRAMEBUFFER_SETUP_REG(0x40) = 0x01960192;
        PDC0_FRAMEBUFFER_SETUP_REG(0x44) = 0x00000000;
        PDC0_FRAMEBUFFER_SETUP_REG(0x48) = 0x00000000;
        PDC0_FRAMEBUFFER_SETUP_DIMS = (240u << 16) | (400u);
        PDC0_FRAMEBUFFER_SETUP_REG(0x60) = 0x01c100d1;
        PDC0_FRAMEBUFFER_SETUP_REG(0x64) = 0x01920002;
        PDC0_FRAMEBUFFER_SETUP_FBA_ADDR_1 = 0x18300000;
        PDC0_FRAMEBUFFER_SETUP_FB_FORMAT = init_top;
        PDC0_FRAMEBUFFER_SETUP_REG(0x74) = 0x00010501;
        PDC0_FRAMEBUFFER_SETUP_FB_SELECT = 0;
        PDC0_FRAMEBUFFER_SETUP_FB_STRIDE = stride;
        PDC0_FRAMEBUFFER_SETUP_REG(0x9C) = 0x00000000;

        // Disco register
        for(volatile uint32_t i = 0; i < 256; i++)
            PDC0_FRAMEBUFFER_SETUP_DISCO = 0x10101 * i;

        // Bottom screen
        PDC1_FRAMEBUFFER_SETUP_REG(0x00) = 0x000001c2;
        PDC1_FRAMEBUFFER_SETUP_REG(0x04) = 0x000000d1;
        PDC1_FRAMEBUFFER_SETUP_REG(0x08) = 0x000001c1;
        PDC1_FRAMEBUFFER_SETUP_REG(0x0c) = 0x000001c1;
        PDC1_FRAMEBUFFER_SETUP_REG(0x10) = 0x000000cd;
        PDC1_FRAMEBUFFER_SETUP_REG(0x14) = 0x000000cf;
        PDC1_FRAMEBUFFER_SETUP_REG(0x18) = 0x000000d1;
        PDC1_FRAMEBUFFER_SETUP_REG(0x1c) = 0x01c501c1;
        PDC1_FRAMEBUFFER_SETUP_REG(0x20) = 0x00010000;
        PDC1_FRAMEBUFFER_SETUP_REG(0x24) = 0x0000019d;
        PDC1_FRAMEBUFFER_SETUP_REG(0x28) = 0x00000052;
        PDC1_FRAMEBUFFER_SETUP_REG(0x2c) = 0x00000192;
        PDC1_FRAMEBUFFER_SETUP_REG(0x30) = 0x00000192;
        PDC1_FRAMEBUFFER_SETUP_REG(0x34) = 0x0000004f;
        PDC1_FRAMEBUFFER_SETUP_REG(0x38) = 0x00000050;
        PDC1_FRAMEBUFFER_SETUP_REG(0x3c) = 0x00000052;
        PDC1_FRAMEBUFFER_SETUP_REG(0x40) = 0x01980194;
        PDC1_FRAMEBUFFER_SETUP_REG(0x44) = 0x00000000;
        PDC1_FRAMEBUFFER_SETUP_REG(0x48) = 0x00000011;
        PDC1_FRAMEBUFFER_SETUP_DIMS = (240u << 16) | 320u;
        PDC1_FRAMEBUFFER_SETUP_REG(0x60) = 0x01c100d1;
        PDC1_FRAMEBUFFER_SETUP_REG(0x64) = 0x01920052;
        PDC1_FRAMEBUFFER_SETUP_FBA_ADDR_1 = 0x1835dc00;
        PDC1_FRAMEBUFFER_SETUP_FB_FORMAT = init_bottom;
        PDC1_FRAMEBUFFER_SETUP_REG(0x74) = 0x00010501;
        PDC1_FRAMEBUFFER_SETUP_FB_SELECT = 0;
        PDC1_FRAMEBUFFER_SETUP_FB_STRIDE = stride;
        PDC1_FRAMEBUFFER_SETUP_REG(0x9C) = 0x00000000;

        // Disco register
        for(volatile uint32_t i = 0; i < 256; i++)
            PDC1_FRAMEBUFFER_SETUP_DISCO = 0x10101 * i;

        PDC0_FRAMEBUFFER_SETUP_FBA_ADDR_1 = 0x18300000;
        PDC0_FRAMEBUFFER_SETUP_FBA_ADDR_2 = 0x18300000;
        PDC0_FRAMEBUFFER_SETUP_FBB_ADDR_1 = 0x18300000;
        PDC0_FRAMEBUFFER_SETUP_FBB_ADDR_2 = 0x18300000;

        PDC1_FRAMEBUFFER_SETUP_FBA_ADDR_1 = 0x1835dc00;
        PDC1_FRAMEBUFFER_SETUP_FBA_ADDR_2 = 0x1835dc00;

        WAIT_FOR_ARM9();
    }

    ctr_cache_clean_and_flush_all();
    invokeArm11Function(ARM11);

    clearScreens();

    // Turn on backlight
//    i2cWriteRegister(I2C_DEV_MCU, 0x22, 1 << 1);
    //Turn on backlight
    i2cWriteRegister(I2C_DEV_MCU, 0x22, 0x2A);
}

