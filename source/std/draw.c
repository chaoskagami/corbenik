#include "draw.h"

#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include "memory.h"
#include "font.h"

static uint8_t cursor_x = 0, cursor_y = 0;
static uint8_t dirty_text = 0; // We need to redraw.

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
	} else if(screen == framebuffers->top_left) {
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
    clear_screen(screen_top_left);
    clear_screen(screen_bottom);
	cursor_x = 0;
	cursor_y = 0;
}

void draw_character(char* screen, const char character, const unsigned int buf_x, const unsigned int buf_y, const uint32_t color) {
	int width  = 0;
	int height = 0;
	if(screen == framebuffers->top_left ||
       screen == framebuffers->top_right) {
		width  = SCREEN_TOP_WIDTH;
		height = SCREEN_TOP_HEIGHT;
	} else if(screen == framebuffers->top_left) {
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

            if ((char_pos >> x) & 1) {
                screen[screen_pos] = color >> 16;
                screen[screen_pos + 1] = color >> 8;
                screen[screen_pos + 2] = color;
            }
        }
    }
}

void putc(int buf, const char c) {
	int width  = 0;
	int height = 0;
	int size   = 0;
    char* colorbuf;
    char* strbuf;
	if(buf == TOP_SCREEN) {
		width    = TEXT_TOP_WIDTH;
		height   = TEXT_TOP_HEIGHT;
		size     = TEXT_TOP_SIZE;
        colorbuf = color_buffer_top;
        strbuf   = text_buffer_top;
	} else if(buf == BOTTOM_SCREEN) {
		width    = TEXT_BOTTOM_WIDTH;
		height   = TEXT_BOTTOM_HEIGHT;
		size     = TEXT_BOTTOM_SIZE;
        colorbuf = color_buffer_bottom;
        strbuf   = text_buffer_bottom;
	} else {
		return; // Invalid buffer.
	}

	unsigned int offset = width * cursor_y + cursor_x;

	if (offset > size)
		return; // TODO - Scrollback.

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
            colorbuf[offset] = 0xf0; // White on black.
            cursor_x++;
            if (cursor_x >= width) {
                cursor_y++;
                cursor_x = 0;
            }
            break;
    }
}

void puts(int buf, const char *string) {
    char *ref = string;

    while(*ref != '\0') {
        putc(buf, *ref);
        *ref++;
    }

    dirty_text = 1; // Framebuffer needs update.
}

void render_textbufs() {
    if(!dirty_text)
        return;

    dirty_text = 0; // We're updating it.

	for(int x=0; x < TEXT_TOP_WIDTH; x++) {
		for(int y=0; y < TEXT_TOP_HEIGHT; y++) {
            char c = text_buffer_top[y*TEXT_TOP_WIDTH+x];
            uint32_t color_fg = colors[((color_buffer_top[y*TEXT_TOP_WIDTH+x] >> 4) & 0x0f)];
            uint32_t color_bg = colors[(color_buffer_top[y*TEXT_TOP_WIDTH+x] & 0x0f)];
			draw_character(framebuffers->top_left, c, x, y, color_fg);
		}
	}

	for(int x=0; x < TEXT_BOTTOM_WIDTH; x++) {
		for(int y=0; y < TEXT_BOTTOM_HEIGHT; y++) {
            char c = text_buffer_top[y*TEXT_TOP_WIDTH+x];
            uint32_t color_fg = colors[((color_buffer_top[y*TEXT_TOP_WIDTH+x] >> 4) & 0x0f)];
            uint32_t color_bg = colors[(color_buffer_top[y*TEXT_TOP_WIDTH+x] & 0x0f)];
			draw_character(framebuffers->top_left, c, x, y, color_fg);
		}
	}
}

void put_int(int channel, int n) {
    char conv[16], out[16];
    memset(conv, 0, 16);
    memset(out, 0, 16);

    int i = 0, sign = n;
    int inter;
    do {
        conv[i] = n % 10;
        if (conv[i] < 0)
            conv[i] = -conv[i];
        conv[i] += '0';
        ++i;
    } while((n /= 10) != 0);

    if (sign < 0)
        conv[i++] = '-';

    int len = strlen(conv);
    for(int i=0; i < len; i++)
        out[i] = conv[(len-1) - i];

    puts(channel, out);
}

void put_uint(int channel, unsigned int n) {
    char conv[16], out[16];
    memset(conv, 0, 16);
    memset(out, 0, 16);

    int i = 0;
    unsigned int inter;
    do {
        conv[i++] = (n % 10) + '0';
    } while((n /= 10) != 0);

    int len = strlen(conv);
    for(int i=0; i < len; i++)
        out[i] = conv[(len-1) - i];

    puts(channel, out);
}

int cprintf(int channel, const char* format, ...) {
    int rc;
    va_list ap;
    va_start( ap, format );

    int i;
    unsigned int u;
    char* s;
    char c;

    char *ref = format;

    while(*ref != '\0') {
        if(*ref == '%') {
            // Format string.
            ref++;
            switch(*ref) {
                case 'd':
                    i = va_arg( ap, int );
                    put_int(channel, i);
                    break;
                case 'u':
                    u = va_arg( ap, unsigned int );
                    put_uint(channel, u);
                    break;
                case 's':
                    s = va_arg( ap, char* );
                    puts(channel, s);
                    break;
                case 'c':
                    c = va_arg( ap, char );
                    putc(channel, c);
                    break;
                case '%':
                    putc(channel, '%');
                    break;
                default:
                    break;
            }
            ref++;
        } else {
            putc(channel, *ref);
            *ref++;
        }
    }

    va_end( ap );
    return rc;
}
