#include "draw.h"

#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include "memory.h"
#include "font.h"

static unsigned int top_cursor_x = 0, top_cursor_y = 0;
static unsigned int bottom_cursor_x = 0, bottom_cursor_y = 0;

static char text_buffer_top    [TEXT_TOP_SIZE];
static char text_buffer_bottom [TEXT_BOTTOM_SIZE];

static char color_buffer_top    [TEXT_TOP_SIZE];
static char color_buffer_bottom [TEXT_BOTTOM_SIZE];

static uint32_t colors[16] = {
	0x000000, // Black
	0x0000aa, // Blue
	0x00aa00, // Green
	0x00aaaa, // Cyan
	0xaa0000, // Red
	0xaa00aa, // Magenta
	0xaa5500, // Brown
	0xaaaaaa, // Gray
	0x555555, // Dark gray
	0x5555ff, // Bright blue
	0x55ff55, // Bright green
	0x55ffff, // Bright cyan
	0xff5555, // Bright red
	0xff55ff, // Bright megenta
	0xffff55, // Yellow
	0xffffff  // White
};

void clear_screen(uint8_t* screen) {
	uint32_t size = 0;
	char* buffer  = 0;
	uint32_t buffer_size = 0;
	if(screen == framebuffers->top_left ||
       screen == framebuffers->top_right) {
		size = SCREEN_TOP_SIZE;
		buffer = text_buffer_top;
		buffer_size = TEXT_TOP_SIZE;
	} else if(screen == framebuffers->bottom) {
		size = SCREEN_BOTTOM_SIZE;
		buffer = text_buffer_bottom;
		buffer_size = TEXT_BOTTOM_SIZE;
	} else {
		return; // Invalid buffer.
	}

    memset(screen, 0, size);
	memset(buffer, 0, buffer_size);
}

void clear_screens() {
    clear_screen(framebuffers->top_left);
    clear_screen(framebuffers->bottom);
	top_cursor_x = 0;
	top_cursor_y = 0;
	bottom_cursor_x = 0;
	bottom_cursor_y = 0;
}

void draw_character(uint8_t* screen, const char character, const unsigned int buf_x, const unsigned int buf_y, const uint32_t color_fg, const uint32_t color_bg) {
    if (!isprint(character))
        return; // Don't output non-printables.

	int width  = 0;
	int height = 0;
	if(screen == framebuffers->top_left ||
       screen == framebuffers->top_right) {
		width  = SCREEN_TOP_WIDTH;
		height = SCREEN_TOP_HEIGHT;
	} else if(screen == framebuffers->bottom) {
		width  = SCREEN_BOTTOM_WIDTH;
		height = SCREEN_BOTTOM_HEIGHT;
	} else {
		return; // Invalid buffer.
	}

    unsigned int pos_x = buf_x * 8;
    unsigned int pos_y = buf_y * 8;

    for (int y = 0; y < 8; y++) {
        unsigned char char_pos = font[character * 8 + y];

        for (int x = 7; x >= 0; x--) {
            int screen_pos = (pos_x * height * 3 + (height - y - pos_y - 1) * 3) + (7 - x) * 3 * height;

            screen[screen_pos]     = color_bg >> 16;
            screen[screen_pos + 1] = color_bg >> 8;
            screen[screen_pos + 2] = color_bg;

            if ((char_pos >> x) & 1) {
                screen[screen_pos] = color_fg >> 16;
                screen[screen_pos + 1] = color_fg >> 8;
                screen[screen_pos + 2] = color_fg;
            }
        }
    }
}

void putc(int buf, unsigned char color, const char c) {
	unsigned int width  = 0;
	unsigned int height = 0;
	unsigned int size = 0;
    unsigned int cursor_x;
    unsigned int cursor_y;
    char* colorbuf;
    char* strbuf;

	if (buf == TOP_SCREEN) {
		width    = TEXT_TOP_WIDTH;
		height   = TEXT_TOP_HEIGHT;
		size     = TEXT_TOP_SIZE;
        colorbuf = color_buffer_top;
        strbuf   = text_buffer_top;
        cursor_x = top_cursor_x;
        cursor_y = top_cursor_y;
	} else if (buf == BOTTOM_SCREEN) {
		width    = TEXT_BOTTOM_WIDTH;
		height   = TEXT_BOTTOM_HEIGHT;
		size     = TEXT_BOTTOM_SIZE;
        colorbuf = color_buffer_bottom;
        strbuf   = text_buffer_bottom;
        cursor_x = bottom_cursor_x;
        cursor_y = bottom_cursor_y;
	} else {
		return; // Invalid buffer.
	}

	unsigned int offset = width * cursor_y + cursor_x;

	if (offset >= size) {
        // Scroll a line back. This involves memcpy.
        // Yes, memcpy overwrites part of the buffer it is reading.
        memcpy(strbuf, strbuf+width, size-width);
        memcpy(colorbuf, colorbuf+width, size-width);
        cursor_y -= 1;
		offset = width * cursor_y + cursor_x;
    }

	if (offset >= size) {
        // So if we're being real, this won't ever happen.
        return;
    }

    switch(c) {
        case '\n':
            cursor_y++;   // Increment line.
            cursor_x = 0;
            break;
        case '\r':
            cursor_x = 0; // Reset to beginning of line.
            break;
        default:
            strbuf[offset] = c;
            colorbuf[offset] = color; // White on black.
            cursor_x++;
            if (cursor_x >= width) {
                cursor_y++;
                cursor_x = 0;
            }
            break;
    }

    if (buf == TOP_SCREEN) {
        top_cursor_x = cursor_x;
        top_cursor_y = cursor_y;
    } else if (buf == BOTTOM_SCREEN) {
        bottom_cursor_x = cursor_x;
        bottom_cursor_y = cursor_y;
    }
}

void puts(int buf, unsigned char color, const char *string) {
    char *ref = (char*)string;

    while(*ref != '\0') {
        putc(buf, color, *ref);
        ref++;
    }
}

void put_int(int channel, unsigned char color, int n) {
    char conv[16], out[16];
    memset(conv, 0, 16);
    memset(out, 0, 16);

    int i = 0, sign = 0;
    if (n < 0) {
        n = -n;
        sign = 1;
    }
    do {
        conv[i] = n % 10;
        conv[i] += '0';
        ++i;
    } while((n /= 10) != 0);

    if (sign)
        conv[i++] = '-';

    int len = strlen(conv);
    for(int i=0; i < len; i++)
        out[i] = conv[(len-1) - i];

    puts(channel, color, out);
}

void put_uint(int channel, unsigned char color, unsigned int n) {
    char conv[16], out[16];
    memset(conv, 0, 16);
    memset(out, 0, 16);

    int i = 0;
    do {
        conv[i++] = (n % 10) + '0';
    } while((n /= 10) != 0);

    int len = strlen(conv);
    for(int i=0; i < len; i++)
        out[i] = conv[(len-1) - i];

    puts(channel, color, out);
}

void cflush(int channel) {
    if (channel == TOP_SCREEN) {
		for(int x=0; x < TEXT_TOP_WIDTH; x++) {
			for(int y=0; y < TEXT_TOP_HEIGHT; y++) {
            	char c = text_buffer_top[y*TEXT_TOP_WIDTH+x];
            	uint32_t color_fg = colors[((color_buffer_top[y*TEXT_TOP_WIDTH+x] >> 4) & 0x0f)];
            	uint32_t color_bg = colors[(color_buffer_top[y*TEXT_TOP_WIDTH+x] & 0x0f)];
				draw_character(framebuffers->top_left, c, x, y, color_fg, color_bg);
			}
		}
    } else if (channel == BOTTOM_SCREEN) {
		for(int x=0; x < TEXT_BOTTOM_WIDTH; x++) {
			for(int y=0; y < TEXT_BOTTOM_HEIGHT; y++) {
            	char c = text_buffer_bottom[y*TEXT_BOTTOM_WIDTH+x];
            	uint32_t color_fg = colors[((color_buffer_bottom[y*TEXT_BOTTOM_WIDTH+x] >> 4) & 0x0f)];
            	uint32_t color_bg = colors[(color_buffer_top[y*TEXT_TOP_WIDTH+x] & 0x0f)];
				draw_character(framebuffers->bottom, c, x, y, color_fg, color_bg);
			}
		}
	}
}

void cprintf(int channel, const char* format, ...) {
    va_list ap;
    va_start( ap, format );

    char *ref = (char*)format;
    unsigned char color = 0xf0;

    while(*ref != '\0') {
        if(*ref == '%') {
            // Format string.
            ref++;
            switch(*ref) {
                case 'd':
                    put_int(channel, color, va_arg( ap, int ));
                    break;
                case 'u':
                    put_uint(channel, color, va_arg( ap, unsigned int ));
                    break;
                case 's':
                    puts(channel, color, va_arg( ap, char* ));
                    break;
                case 'c':
                    putc(channel, color, va_arg( ap, int ));
                    break;
                case 'p':
                    color = va_arg( ap, unsigned int );
                    break;
                case '%':
                    putc(channel, color, '%');
                    break;
                default:
                    break;
            }
        } else {
            putc(channel, color, *ref);
        }
        ref++;
    }

    va_end( ap );

    cflush(channel);
}

