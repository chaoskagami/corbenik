#include "draw.h"

#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include "memory.h"
#include "font.h"
#include "../fatfs/ff.h"
#include "fs.h"
#include "unused.h"

static unsigned int top_cursor_x = 0, top_cursor_y = 0;
static unsigned int bottom_cursor_x = 0, bottom_cursor_y = 0;

static char text_buffer_top    [TEXT_TOP_SIZE];
static char text_buffer_bottom [TEXT_BOTTOM_SIZE];

static char color_buffer_top    [TEXT_TOP_SIZE];
static char color_buffer_bottom [TEXT_BOTTOM_SIZE];

static uint32_t colors[16] = {
	0x000000, // Black
	0xaa0000, // Blue
	0x00aa00, // Green
	0xaaaa00, // Cyan
	0x0000aa, // Red
	0xaa00aa, // Magenta
	0x0055aa, // Brown
	0xaaaaaa, // Gray
	0x555555, // Dark gray
	0xff5555, // Bright blue
	0x55ff55, // Bright green
	0xffff55, // Bright cyan
	0x5555ff, // Bright red
	0xff55ff, // Bright megenta
	0x55ffff, // Yellow
	0xffffff  // White
};

void clear_screen(uint8_t* screen) {
	uint32_t size = 0;
	char* buffer  = 0;
	uint32_t buffer_size = 0;

    if (screen == TOP_SCREEN)
        screen = framebuffers->top_left;
    else if (screen == BOTTOM_SCREEN)
        screen = framebuffers->bottom;

	if(screen == framebuffers->top_left ||
       screen == framebuffers->top_right) {
		size = SCREEN_TOP_SIZE;
		buffer = text_buffer_top;
		buffer_size = TEXT_TOP_SIZE;
		top_cursor_x = 0;
		top_cursor_y = 0;
	} else if(screen == framebuffers->bottom) {
		size = SCREEN_BOTTOM_SIZE;
		buffer = text_buffer_bottom;
		buffer_size = TEXT_BOTTOM_SIZE;
		bottom_cursor_x = 0;
		bottom_cursor_y = 0;
	} else {
		return; // Invalid buffer.
	}

    memset(screen, 0, size);
	memset(buffer, 0, buffer_size);
}

void set_cursor(void* channel, unsigned int x, unsigned int y) {
	if (channel == TOP_SCREEN) {
        top_cursor_x = x;
        top_cursor_y = y;
	} else if (channel == BOTTOM_SCREEN) {
        bottom_cursor_x = x;
        bottom_cursor_y = y;
	}
}

void clear_screens() {
    clear_screen(framebuffers->top_left);
    clear_screen(framebuffers->bottom);
}

void draw_character(uint8_t* screen, const char character, const unsigned int buf_x, const unsigned int buf_y, const uint32_t color_fg, const uint32_t color_bg) {
    if (!isprint(character))
        return; // Don't output non-printables.

	_UNUSED int width  = 0;
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

unsigned char color_top = 0xf0;
unsigned char color_bottom = 0xf0;

void putc(void* buf, const int c) {
    if (buf == stdout || buf == stderr) {
		unsigned int width  = 0;
		_UNUSED unsigned int height = 0;
		unsigned int size = 0;
    	unsigned int cursor_x;
    	unsigned int cursor_y;
    	char* colorbuf;
    	char* strbuf;

		unsigned char* color = NULL;

		if (buf == TOP_SCREEN) {
			width    = TEXT_TOP_WIDTH;
			height   = TEXT_TOP_HEIGHT;
			size     = TEXT_TOP_SIZE;
    	    colorbuf = color_buffer_top;
    	    strbuf   = text_buffer_top;
    	    cursor_x = top_cursor_x;
    	    cursor_y = top_cursor_y;
			color = &color_top;
		} else if (buf == BOTTOM_SCREEN) {
			width    = TEXT_BOTTOM_WIDTH;
			height   = TEXT_BOTTOM_HEIGHT;
			size     = TEXT_BOTTOM_SIZE;
	        colorbuf = color_buffer_bottom;
	        strbuf   = text_buffer_bottom;
	        cursor_x = bottom_cursor_x;
	        cursor_y = bottom_cursor_y;
			color = &color_bottom;
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
	            colorbuf[offset] = *color; // White on black.
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
	} else {
		// FILE*, not stdin or stdout.
		fwrite(&c, 1, 1, (FILE*)buf);
	}
}

void puts(void* buf, const char *string) {
    char *ref = (char*)string;

    while(*ref != '\0') {
        putc(buf, *ref);
        ref++;
    }
}

void put_int64(void* channel, int64_t n, int length) {
    char conv[32], out[32];
    memset(conv, 0, 32);
    memset(out, 0, 32);

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

    if (length > 0)
        out[length] = '\0';

    int len = strlen(conv);
    for(int i=0; i < len; i++)
        out[i] = conv[(len-1) - i];

    puts(channel, out);
}

void put_uint64(void* channel, uint64_t n, int length) {
    char conv[32], out[32];
    memset(conv, 0, 32);
    memset(out, 0, 32);

    int i = 0;
    do {
        conv[i++] = (n % 10) + '0';
    } while((n /= 10) != 0);

    if (length > 0)
        out[length] = '\0';

    int len = strlen(conv);
    for(int i=0; i < len; i++)
        out[i] = conv[(len-1) - i];

    puts(channel, out);
}

void put_uint(void* channel, unsigned int n, int length) {
    put_uint64(channel, n, length);
}

void put_int(void* channel, int n, int length) {
    put_int64(channel, n, length);
}

void fflush(void* channel) {
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
            	uint32_t color_bg = colors[(color_buffer_bottom[y*TEXT_BOTTOM_WIDTH+x] & 0x0f)];
				draw_character(framebuffers->bottom, c, x, y, color_fg, color_bg);
			}
		}
	} else {
		f_sync(&(((FILE*)channel)->handle)); // Sync to disk.
	}
}

void fprintf(void* channel, const char* format, ...) {
    va_list ap;
    va_start( ap, format );

    char *ref = (char*)format;

	unsigned char* color;
	if (channel == TOP_SCREEN)
		color = &color_top;
	else if (channel == TOP_SCREEN)
		color = &color_bottom;

    while(*ref != '\0') {
		if (*ref == 0x1B && *(++ref) == '[' && ( channel == stdout || channel == stderr) ) {
ansi_codes:
            // Ansi escape code.
            ++ref;
            // [30-37] Set text color
            if (*ref == '3') {
                ++ref;
                if(*ref >= '0' && *ref <= '7') {
                    // Valid FG color.
                    *color &= 0x0f; // Remove fg color.
                    *color |= (*ref - '0') << 4;
                }
            }
            // [40-47] Set bg color
            else if (*ref == '4') {
                ++ref;
                if(*ref >= '0' && *ref <= '7') {
                    // Valid BG color.
                    *color &= 0xf0; // Remove bg color.
                    *color |= *ref - '0';
                }
            } else if (*ref == '0') {
				// Reset.
				*color = 0xf0;
			}

            ++ref;

            if (*ref == ';') {
                goto ansi_codes; // Another code.
            }

            // Loop until the character is somewhere 0x40 - 0x7E, which terminates an ANSI sequence
            while(!(*ref >= 0x40 && *ref <= 0x7E)) ref++;
        } else if (*ref == '%') {
            int type_size = 0;
            int length = -1;
check_format:
            // Format string.
            ++ref;
            switch(*ref) {
                case 'd':
                    switch(type_size) {
                        case 2:
                            put_int64(channel, va_arg( ap, int64_t ), length);
                            break;
                        default:
                            put_int(channel, va_arg( ap, int ), length);
                            break;
                    }
                    break;
                case 'u':
                    switch(type_size) {
                        case 2:
                            put_uint64(channel, va_arg( ap, uint64_t ), length);
                            break;
                        default:
                            put_uint(channel, va_arg( ap, unsigned int ), length);
                            break;
                    }
                    break;
                case 's':
                    puts(channel, va_arg( ap, char* ));
                    break;
                case 'c':
                    putc(channel, va_arg( ap, int ));
                    break;
                case 'p':
                    puts(channel, va_arg( ap, char* ));
                    break;
                case '%':
                    putc(channel, '%');
                    break;
                case 'h':
                    goto check_format; // Integers get promoted. No point here.
                case 'l':
                    ++type_size;
                    goto check_format;
                default:
                    if (*ref >= '0' && *ref <= '9') {
                        length = *ref - '0';
                        goto check_format;
                    }
                    break;
            }
        } else {
            putc(channel, *ref);
        }
        ++ref;
    }

    va_end( ap );

    fflush(channel);
}

