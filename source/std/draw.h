#ifndef __STD_DRAW_H
#define __STD_DRAW_H

/* This is in a nutshell a partial implementation of stdio.
   It isn't perfect, but it does work. */

#include <stdint.h>

#define SCREEN_TOP_WIDTH 400
#define SCREEN_TOP_HEIGHT 240

#define SCREEN_BOTTOM_WIDTH 320
#define SCREEN_BOTTOM_HEIGHT 240

#define SCREEN_DEPTH 3

#define SCREEN_TOP_SIZE (SCREEN_TOP_WIDTH * SCREEN_TOP_HEIGHT * SCREEN_DEPTH)
#define SCREEN_BOTTOM_SIZE (SCREEN_BOTTOM_WIDTH * SCREEN_BOTTOM_HEIGHT * SCREEN_DEPTH)

#define CHARA_HEIGHT 8
#define CHARA_WIDTH 8

#define TEXT_TOP_WIDTH (SCREEN_TOP_WIDTH / CHARA_WIDTH)
#define TEXT_TOP_HEIGHT (SCREEN_TOP_HEIGHT / CHARA_HEIGHT)

#define TEXT_BOTTOM_WIDTH (SCREEN_BOTTOM_WIDTH / CHARA_WIDTH)
#define TEXT_BOTTOM_HEIGHT (SCREEN_BOTTOM_HEIGHT / CHARA_HEIGHT)

#define TEXT_TOP_SIZE (TEXT_TOP_WIDTH * TEXT_TOP_HEIGHT)
#define TEXT_BOTTOM_SIZE (TEXT_BOTTOM_WIDTH * TEXT_BOTTOM_HEIGHT)

enum screen
{
    screen_top_left,
    screen_top_right,
    screen_bottom
};

#include "unused.h"

_UNUSED static struct framebuffers
{
    uint8_t *top_left;
    uint8_t *top_right;
    uint8_t *bottom;
} *framebuffers = (struct framebuffers *)0x23FFFE00;
// This is marked unused since it occurs in all files.

#define TOP_FB framebuffers->top_left
#define BOTTOM_FB framebuffers->bottom

void clear_screen(uint8_t *screen);
void clear_screens();
void draw_character(uint8_t *screen, const char character, const unsigned int pos_x, const unsigned int pos_y, const uint32_t color_fg,
                    const uint32_t color_bg);

#define TOP_SCREEN ((void *)0)
#define BOTTOM_SCREEN ((void *)2)

#define stdout TOP_SCREEN
#define stderr BOTTOM_SCREEN

void putc(void *buf, const int c);
void puts(void *buf, const char *string);
void fflush(void *channel);

void set_cursor(void *channel, unsigned int x, unsigned int y);

// Like printf. Supports the following format specifiers:
//  %s - char*
//  %c - char
//  %d - int
//  %u - unsigned int
// The following non-standard
// The following prefixes are also supported:
//  %h  - word (stub)
//  %hh - byte (stub)
//  %[0-9][0-9]
// Formats are also supported (but are subject to replacement)
//  %p - unsigned char, changes color of text (will be replaced with ANSI codes
//  eventually)
void fprintf(void *channel, const char *format, ...);

#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGENTA 5
#define BROWN 6
#define GRAY 7
#define D_GRAY 8
#define B_BLUE 9
#define B_GREEN 10
#define B_CYAN 11
#define B_RED 12
#define B_MAGENTA 13
#define YELLOW 14
#define WHITE 15

#define COLOR(fg, bg) "\x1b[3" #fg ";4" #bg "m"

#endif
