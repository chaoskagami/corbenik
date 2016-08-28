#ifndef __STD_DRAW_H
#define __STD_DRAW_H

/* This is in a nutshell a partial implementation of stdio.
   It isn't perfect, but it does work. */

#include <stdint.h>
#include <stdarg.h>

#define TOP_WIDTH     400 ///< Top screen width
#define TOP_HEIGHT    240 ///< Top screen height

#define BOTTOM_WIDTH  320 ///< Bottom screen width
#define BOTTOM_HEIGHT 240 ///< Bottom screen height

#define SCREEN_DEPTH  4   ///< Pixel depth of the screen

#define TOP_SIZE      (TOP_WIDTH * TOP_HEIGHT * SCREEN_DEPTH)       ///< Buffer size of top screen
#define BOTTOM_SIZE   (BOTTOM_WIDTH * BOTTOM_HEIGHT * SCREEN_DEPTH) ///< Buffer size of bottom screen

enum screen
{
    screen_top_left,
    screen_top_right,
    screen_bottom
};

struct framebuffers {
    uint8_t *top_left;
    uint8_t *top_right;
    uint8_t *bottom;
};

extern struct framebuffers *framebuffers;
// This is marked unused since it occurs in all files.

#define TOP_FB    framebuffers->top_left ///< Compact way to specify top
#define BOTTOM_FB framebuffers->bottom   ///< Compact way to specify bottom

/* Initialize stdlib functionality. Must be called before ANY other functions here can be used.
 */
void std_init();

/* Take a screenshot and save to path.
 */
void screenshot();

/* Fill an area on the screen with a color.
 *
 * \param channel Buffer to draw rectangle to. Should be zero or two.
 * \param x X1 coordinate to start at.
 * \param y Y1 coordinate to start at.
 * \param x2 X2 coordinate
 * \param y2 Y2 coordinate
 * \param color Color to fill with
 */
void rect(void* channel, unsigned int x, unsigned int y, unsigned int x2, unsigned int y2, uint8_t color);

/* Fill a line behind characters with a color.
 *
 * \param channel Which buffer to fill line on, zero or two
 * \param y Which line to fill
 * \param color Color to fill with
 */
void fill_line(void* channel, unsigned int y, uint8_t color);

/* Clears background image bitmaps.
 */
void clear_bg();

/* Loads top background image from a path.
 *
 * \param fname_top filename to load from.
 */
void load_bg_top(const char* fname_top);

/* Loads bottom background image from a path.
 *
 * \param fname_bottom filename to load from.
 */
void load_bg_bottom(const char* fname_bottom);

/* Clears the displays either to black or the background image.
 */
void clear_screens();

/* Draws a character to the screen. Internal use.
 *
 * \param screen Buffer of pixels to draw to
 * \param character Character to draw
 * \param ch_x X coordinate to draw to
 * \param ch_y Y coordinate to draw to
 * \param color_fg RGB color to draw character as (as uint32_t)
 * \param color_bg RGB color to draw behind character (as uint32_t)
 */
void draw_character(uint8_t *screen, const unsigned int character, unsigned int ch_x, unsigned int ch_y, const uint32_t color_fg, const uint32_t color_bg);

/* Sets the font.
 *
 * \param filename Font file to load.
 */
void set_font(const char* filename);

#define TOP_SCREEN    ((void *)0) ///< Another name for stdout
#define BOTTOM_SCREEN ((void *)2) ///< Another name for stderr

#define stdout TOP_SCREEN         ///< Top screen/stdout
#define stderr BOTTOM_SCREEN      ///< Bottom screen/stderr

/* Outputs a character to a handle
 *
 * \param buf Handle to output to.
 * \param c Character (as int) to output
 */
void putc(void *buf, int c);

/* Outputs a string to a handle
 *
 * \param buf Handle to output to.
 * \param string String to output
 */
void puts(void *buf, char *string);

/* Flushes a handle
 *
 * \param channel Handle to flush output on
 */
void fflush(void *channel);

/* Moves the cursor/output location on a display device
 *
 * \param channel Display to move on (stderr/stdout)
 * \param x X coordinate
 * \param y Y coordinate
 */
void set_cursor(void *channel, unsigned int x, unsigned int y);

/* Clear the display specified to black or a background.
 *
 * \param screen Which screen to clear.
 */
void clear_disp(uint8_t *screen);

/* Minimal fprintf implementation.
 *
 * Supports the following format specifiers:
 *   %s - char*
 *   %c - char
 *   %d - int
 *   %u - unsigned int
 * The following prefixes are also supported:
 *   %h  - word (stub)
 *   %hh - byte (stub)
 *   %[0-9][0-9]
 * Formats are also supported (but are subject to replacement)
 *  %p - unsigned char, changes color of text (deprecated, use ANSI codes please)
 *
 * \param channel Handle to output to.
 * \param Format string.
 * \param ... Format arguments
 */
void fprintf(void *channel, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

/* See fprintf. Takes a va_list instead of variable arguments.
 */
void vfprintf(void *channel, const char *format, va_list ap);

#define BLACK     0
#define BLUE      1
#define GREEN     2
#define CYAN      3
#define RED       4
#define MAGENTA   5
#define BROWN     6
#define GRAY      7
#define D_GRAY    8
#define B_BLUE    9
#define B_GREEN   10
#define B_CYAN    11
#define B_RED     12
#define B_MAGENTA 13
#define YELLOW    14
#define WHITE     15

#define COLOR(fg, bg) "\x1b[3" #fg ";4" #bg "m"

#endif
