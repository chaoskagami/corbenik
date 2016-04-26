#ifndef __STD_DRAW_H
#define __STD_DRAW_H

#include <stdint.h>

#define SCREEN_TOP_WIDTH 400
#define SCREEN_TOP_HEIGHT 240

#define SCREEN_BOTTOM_WIDTH 320
#define SCREEN_BOTTOM_HEIGHT 240

#define SCREEN_DEPTH 3

#define SCREEN_TOP_SIZE (SCREEN_TOP_WIDTH * SCREEN_TOP_HEIGHT * SCREEN_DEPTH)
#define SCREEN_BOTTOM_SIZE (SCREEN_BOTTOM_WIDTH * SCREEN_BOTTOM_HEIGHT * SCREEN_DEPTH)

#define CHARA_HEIGHT 8
#define CHARA_WIDTH  8

#define TEXT_TOP_WIDTH     (SCREEN_TOP_WIDTH / CHARA_WIDTH)
#define TEXT_TOP_HEIGHT    (SCREEN_TOP_HEIGHT / CHARA_HEIGHT)

#define TEXT_BOTTOM_WIDTH  (SCREEN_BOTTOM_WIDTH / CHARA_WIDTH)
#define TEXT_BOTTOM_HEIGHT (SCREEN_BOTTOM_HEIGHT / CHARA_HEIGHT)

#define TEXT_TOP_SIZE    (TEXT_TOP_WIDTH * TEXT_TOP_HEIGHT)
#define TEXT_BOTTOM_SIZE (TEXT_BOTTOM_WIDTH * TEXT_BOTTOM_HEIGHT)

enum screen {
    screen_top_left,
    screen_top_right,
    screen_bottom
};

static struct framebuffers {
    uint8_t *top_left;
    uint8_t *top_right;
    uint8_t *bottom;
} *framebuffers = (struct framebuffers *)0x23FFFE00;

#define TOP_FB    framebuffers->top_left
#define BOTTOM_FB framebuffers->bottom

void clear_screen(uint8_t* screen);
void clear_screens();
void draw_character(char* screen, const char character, const unsigned int pos_x, const unsigned int pos_y, const uint32_t color);

#define TOP_SCREEN    0
#define BOTTOM_SCREEN 0

void putc(int buf, const char c);
void puts(int buf, const char *string);
void render_textbufs();

void put_int(int channel, int n);
void put_uint(int channel, unsigned int n);

// Like printf. Supports the following format specifiers:
//   %s %c %d %u
int cprintf(int channel, const char* format, ...);

#endif
