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

#ifdef BUFFER
static char text_buffer_top[TEXT_TOP_HEIGHT * TEXT_TOP_WIDTH + 1];
static char text_buffer_bottom[TEXT_BOTTOM_HEIGHT * TEXT_BOTTOM_WIDTH + 1];

static char color_buffer_top[TEXT_TOP_HEIGHT * TEXT_TOP_WIDTH + 1];
static char color_buffer_bottom[TEXT_BOTTOM_HEIGHT * TEXT_BOTTOM_WIDTH + 1];
#endif

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

void
clear_disp(uint8_t* screen)
{
    if (screen == TOP_SCREEN)
        screen = framebuffers->top_left;
    else if (screen == BOTTOM_SCREEN)
        screen = framebuffers->bottom;

    if (screen == framebuffers->top_left || screen == framebuffers->top_right) {
        memset(screen, 0, SCREEN_TOP_SIZE);
    } else if (screen == framebuffers->bottom) {
        memset(screen, 0, SCREEN_BOTTOM_SIZE);
    }
}

#ifdef BUFFER
void
clear_text(uint8_t* screen)
{
    if (screen == TOP_SCREEN)
        screen = framebuffers->top_left;
    else if (screen == BOTTOM_SCREEN)
        screen = framebuffers->bottom;

    if (screen == framebuffers->top_left || screen == framebuffers->top_right) {
        for (int i = 0; i < TEXT_TOP_HEIGHT; i++) {
            text_buffer_top[i * TEXT_TOP_WIDTH] = 0;
            color_buffer_top[i * TEXT_TOP_WIDTH] = 0;
        }
    } else if (screen == framebuffers->bottom) {
        for (int i = 0; i < TEXT_BOTTOM_HEIGHT; i++) {
            text_buffer_bottom[i * TEXT_BOTTOM_WIDTH] = 0;
            color_buffer_bottom[i * TEXT_BOTTOM_WIDTH] = 0;
        }
    }
}
#endif

void
clear_screen(uint8_t* screen)
{
    clear_disp(screen);
#ifdef BUFFER
    clear_text(screen);
#endif
}

void
set_cursor(void* channel, unsigned int x, unsigned int y)
{
    if (channel == TOP_SCREEN) {
        top_cursor_x = x;
        top_cursor_y = y;
    } else if (channel == BOTTOM_SCREEN) {
        bottom_cursor_x = x;
        bottom_cursor_y = y;
    }
}

void
clear_screens()
{
    clear_screen(framebuffers->top_left);
    clear_screen(framebuffers->bottom);
}

void
draw_character(uint8_t* screen, const char character, const unsigned int buf_x,
               const unsigned int buf_y, const uint32_t color_fg,
               const uint32_t color_bg)
{
    if (!isprint(character))
        return; // Don't output non-printables.

    _UNUSED int width = 0;
    int height = 0;
    if (screen == framebuffers->top_left || screen == framebuffers->top_right) {
        width = SCREEN_TOP_WIDTH;
        height = SCREEN_TOP_HEIGHT;
    } else if (screen == framebuffers->bottom) {
        width = SCREEN_BOTTOM_WIDTH;
        height = SCREEN_BOTTOM_HEIGHT;
    } else {
        return; // Invalid buffer.
    }

    unsigned int pos_x = buf_x * 8;
    unsigned int pos_y = buf_y * 8;

    for (int y = 0; y < 8; y++) {
        unsigned char char_pos = font[character * 8 + y];

        for (int x = 7; x >= 0; x--) {
            int screen_pos =
                (pos_x * height * 3 + (height - y - pos_y - 1) * 3) +
                (7 - x) * 3 * height;

            screen[screen_pos] = color_bg >> 16;
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
int kill_output = 0;

void
shut_up()
{
    kill_output = !kill_output;
}

void
putc(void* buf, const int c)
{
    if (buf == stdout || buf == stderr) {
        if (kill_output)
            return;

        unsigned int width = 0;
        _UNUSED unsigned int height = 0;
        unsigned int* cursor_x = NULL;
        unsigned int* cursor_y = NULL;
#ifdef BUFFER
        char* colorbuf = NULL;
        char* strbuf = NULL;
#else
        uint8_t* screen = NULL;
#endif
        unsigned char* color = NULL;

        if (buf == TOP_SCREEN) {
            width = TEXT_TOP_WIDTH;
            height = TEXT_TOP_HEIGHT;
#ifdef BUFFER
            colorbuf = color_buffer_top;
            strbuf = text_buffer_top;
#else
            screen = framebuffers->top_left;
#endif
            cursor_x = &top_cursor_x;
            cursor_y = &top_cursor_y;
            color = &color_top;
        } else if (buf == BOTTOM_SCREEN) {
            width = TEXT_BOTTOM_WIDTH;
            height = TEXT_BOTTOM_HEIGHT;
#ifdef BUFFER
            colorbuf = color_buffer_bottom;
            strbuf = text_buffer_bottom;
#else
            screen = framebuffers->bottom;
#endif
            cursor_x = &bottom_cursor_x;
            cursor_y = &bottom_cursor_y;
            color = &color_bottom;
        }

        if (cursor_x[0] >= width) {
            cursor_x[0] = 0;
            cursor_y[0]++;
        }

        while (cursor_y[0] >= height) {
#ifdef BUFFER
            // Scroll.
            for (unsigned int y = 0; y < height - 1; y++) {
                memset(&strbuf[y * width], 0, width);
                memset(&colorbuf[y * width], 0, width);
                strncpy(&strbuf[y * width], &strbuf[(y + 1) * width], width);
                strncpy(&colorbuf[y * width], &colorbuf[(y + 1) * width],
                        width);
            }
            memset(&strbuf[(height - 1) * width], 0, width);
            memset(&colorbuf[(height - 1) * width], 0, width);

            clear_disp(buf); // Clear screen.

            cursor_y[0]--;
#else
			clear_disp(buf);
			cursor_x[0] = 0;
			cursor_y[0] = 0;
/*			uint32_t col = SCREEN_TOP_HEIGHT * SCREEN_DEPTH;
			uint32_t one_c = 8 * SCREEN_DEPTH;
			for (unsigned int x = 0; x < width * 8; x++) {
				memmove(&screen[x * col + one_c], &screen[x * col + one_c], col - one_c);
			} */
#endif
        }

        switch (c) {
            case '\n':
#ifdef BUFFER
                strbuf[cursor_y[0] * width + cursor_x[0]] = 0;
                colorbuf[cursor_y[0] * width + cursor_x[0]] = 0;
#endif
                cursor_y[0]++;
            // Fall through intentional.
            case '\r':
                cursor_x[0] = 0; // Reset to beginning of line.
                break;
            default:
#ifdef BUFFER
                strbuf[cursor_y[0] * width + cursor_x[0]] = c;
                colorbuf[cursor_y[0] * width + cursor_x[0]] = *color;

                if (cursor_x[0] + 1 < width) {
                    strbuf[cursor_y[0] * width + cursor_x[0] + 1] =
                        0; // Terminate.
                    colorbuf[cursor_y[0] * width + cursor_x[0] + 1] = 0;
                }

#else
				draw_character(screen, c, cursor_x[0], cursor_y[0], colors[(*color >> 4) & 0xF], colors[*color & 0xF]);
#endif

                cursor_x[0]++;

                break;
        }
    } else {
        // FILE*, not stdin or stdout.
        fwrite(&c, 1, 1, (FILE*)buf);
    }
}

void
puts(void* buf, const char* string)
{
    if ((buf == stdout || buf == stderr) && kill_output)
        return;

    char* ref = (char*)string;

    while (*ref != '\0') {
        putc(buf, *ref);
        ref++;
    }
}

void
put_int64(void* channel, int64_t n, int length)
{
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
    } while ((n /= 10) != 0);

    if (sign)
        conv[i++] = '-';

    if (length > 0)
        out[length] = '\0';

    int len = strlen(conv);
    for (int i = 0; i < len; i++)
        out[i] = conv[(len - 1) - i];

    puts(channel, out);
}

void
put_uint64(void* channel, uint64_t n, int length)
{
    char conv[32], out[32];
    memset(conv, 0, 32);
    memset(out, 0, 32);

    int i = 0;
    do {
        conv[i++] = (n % 10) + '0';
    } while ((n /= 10) != 0);

    if (length > 0)
        out[length] = '\0';

    int len = strlen(conv);
    for (int i = 0; i < len; i++)
        out[i] = conv[(len - 1) - i];

    puts(channel, out);
}

void
put_hexdump(void* channel, unsigned int num)
{
    uint8_t* num_8 = (uint8_t*)&num;
    for (int i = 3; i >= 0; i--) {
        uint8_t high = (num_8[i] >> 4) & 0xf;
        uint8_t low = num_8[i] & 0xf;

        putc(channel, ("0123456789abcdef")[high]);
        putc(channel, ("0123456789abcdef")[low]);
    }
}

void
put_uint(void* channel, unsigned int n, int length)
{
    put_uint64(channel, n, length);
}

void
put_int(void* channel, int n, int length)
{
    put_int64(channel, n, length);
}

void
fflush(void* channel)
{
    if (channel == TOP_SCREEN) {
#ifdef BUFFER
        if (kill_output)
            return;

        for (int y = 0; y < TEXT_TOP_HEIGHT; y++) {
            for (int x = 0; x < TEXT_TOP_WIDTH; x++) {
                char c = text_buffer_top[y * TEXT_TOP_WIDTH + x];
                if (c == 0)
                    break;
                uint32_t color_fg = colors[(
                    (color_buffer_top[y * TEXT_TOP_WIDTH + x] >> 4) & 0x0f)];
                uint32_t color_bg =
                    colors[(color_buffer_top[y * TEXT_TOP_WIDTH + x] & 0x0f)];
                draw_character(framebuffers->top_left, c, x, y, color_fg,
                               color_bg);
            }
        }
#endif
    } else if (channel == BOTTOM_SCREEN) {
#ifdef BUFFER
        if (kill_output)
            return;

        for (int y = 0; y < TEXT_BOTTOM_HEIGHT; y++) {
            for (int x = 0; x < TEXT_BOTTOM_WIDTH; x++) {
                char c = text_buffer_bottom[y * TEXT_BOTTOM_WIDTH + x];
                if (c == 0)
                    break;
                uint32_t color_fg = colors[(
                    (color_buffer_bottom[y * TEXT_BOTTOM_WIDTH + x] >> 4) &
                    0x0f)];
                uint32_t color_bg = colors[(
                    color_buffer_bottom[y * TEXT_BOTTOM_WIDTH + x] & 0x0f)];
                draw_character(framebuffers->bottom, c, x, y, color_fg,
                               color_bg);
            }
        }
#endif
    } else {
        f_sync(&(((FILE*)channel)->handle)); // Sync to disk.
    }
}

int disable_format = 0;

void
vfprintf(void* channel, const char* format, va_list ap)
{
    if ((channel == stdout || channel == stderr) && kill_output)
        return;

    char* ref = (char*)format;

    unsigned char* color;
    if (channel == TOP_SCREEN)
        color = &color_top;
    else if (channel == TOP_SCREEN)
        color = &color_bottom;

    while (*ref != '\0') {
        if (*ref == 0x1B && *(++ref) == '[' &&
            (channel == stdout || channel == stderr)) {
        ansi_codes:
            // Ansi escape code.
            ++ref;
            // [30-37] Set text color
            if (*ref == '3') {
                ++ref;
                if (*ref >= '0' && *ref <= '7') {
                    // Valid FG color.
                    *color &= 0x0f; // Remove fg color.
                    *color |= (*ref - '0') << 4;
                }
            }
            // [40-47] Set bg color
            else if (*ref == '4') {
                ++ref;
                if (*ref >= '0' && *ref <= '7') {
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

            // Loop until the character is somewhere 0x40 - 0x7E, which
            // terminates an ANSI sequence
            while (!(*ref >= 0x40 && *ref <= 0x7E))
                ref++;
        } else if (*ref == '%' && !disable_format) {
            int type_size = 0;
            int length = -1;
        check_format:
            // Format string.
            ++ref;
            switch (*ref) {
                case 'd':
                    switch (type_size) {
                        case 2:
                            put_int64(channel, va_arg(ap, int64_t), length);
                            break;
                        default:
                            put_int(channel, va_arg(ap, int), length);
                            break;
                    }
                    break;
                case 'u':
                    switch (type_size) {
                        case 2:
                            put_uint64(channel, va_arg(ap, uint64_t), length);
                            break;
                        default:
                            put_uint(channel, va_arg(ap, unsigned int), length);
                            break;
                    }
                    break;
                case 's':
                    // Using puts isn't correct here...
					disable_format = 1; // Disable format strings.
                    fprintf(channel, va_arg(ap, char*));
					disable_format = 0; // Reenable.
                    break;
                case 'c':
                    putc(channel, va_arg(ap, int));
                    break;
                case 'p':
                    puts(channel, va_arg(ap, char*));
                    break;
                case '%':
                    putc(channel, '%');
                    break;
                case 'h':
                    goto check_format; // Integers get promoted. No point here.
                case 'l':
                    ++type_size;
                    goto check_format;
                case 'x':
                    put_hexdump(channel, va_arg(ap, unsigned int));
                    break;
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

    fflush(channel);
}

void
fprintf(void* channel, const char* format, ...)
{
    // The output suppression is in all the functions to minimize overhead.
    // Function calls and format conversions take time and we don't just want
    // to stop at putc
    if ((channel == stdout || channel == stderr) && kill_output)
        return;

    va_list ap;
    va_start(ap, format);

    vfprintf(channel, format, ap);

    va_end(ap);
}
