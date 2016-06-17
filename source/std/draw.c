#include "draw.h"

#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include "memory.h"
#include "../fatfs/ff.h"
#include "../firm/fcram.h"
#include "fs.h"
#include "unused.h"
#include "../config.h"
#include "../patch_format.h"

static unsigned int top_cursor_x = 0, top_cursor_y = 0;
static unsigned int bottom_cursor_x = 0, bottom_cursor_y = 0;

static size_t  log_size = 0;
static char    log_buffer[4096]; // Log buffer.

unsigned int font_w = 8;
unsigned int font_h = 8;
static unsigned int font_kern = 0;

static unsigned int text_top_width = 20;
static unsigned int text_top_height = 10;

static unsigned int text_bottom_width = 20;
static unsigned int text_bottom_height = 10;

void set_font(const char* filename) {
    // TODO - Unicode support. Right now, we only load 32

    FILE* f = fopen(filename, "r");

    if (!f) return;

    unsigned int new_w, new_h;

    fread(&new_w, 1, 4, f);
    fread(&new_h, 1, 4, f);

    if (new_w == 0 || new_h == 0) {
        fprintf(stderr, "Invalid font file: w/h is 0 - not loaded\n");
        return;
    }

    unsigned int c_font_w = (new_w / 8) + (new_w % 8 ? 1 : 0);

    fread((void*)FCRAM_FONT_LOC, 1, c_font_w * new_h * (256 - ' '), f); // Skip non-printing chars.

    fclose(f);

    font_w = new_w;
    font_h = new_h;

    text_top_width  = TOP_WIDTH  / (font_w + font_kern);
    text_top_height = TOP_HEIGHT / font_h;

    text_bottom_width  = BOTTOM_WIDTH / (font_w + font_kern);
    text_bottom_height = BOTTOM_HEIGHT / font_h;
}

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
}; // VGA color table.

void dump_log(unsigned int force) {
    if(!config.options[OPTION_SAVE_LOGS])
        return;

    if (force == 0 && log_size < sizeof(log_buffer)-1)
        return;

    if (log_size == 0)
        return;

    FILE *f = fopen(PATH_CFW "/boot.log", "w");
    fseek(f, 0, SEEK_END);

    fwrite(log_buffer, 1, log_size, f);

    fclose(f);
    log_size = 0;
}

void
clear_disp(uint8_t *screen)
{
    if (screen == TOP_SCREEN)
        screen = framebuffers->top_left;
    else if (screen == BOTTOM_SCREEN)
        screen = framebuffers->bottom;

    if (screen == framebuffers->top_left || screen == framebuffers->top_right) {
        memset(screen, 0, TOP_SIZE);
    } else if (screen == framebuffers->bottom) {
        memset(screen, 0, BOTTOM_SIZE);
    }
}

void
clear_screen(uint8_t *screen)
{
    clear_disp(screen);
}

void
set_cursor(void *channel, unsigned int x, unsigned int y)
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
draw_character(uint8_t *screen, const uint32_t character, int ch_x, int ch_y, const uint32_t color_fg, const uint32_t color_bg)
{
    if (!isprint(character))
        return; // Don't output non-printables.

    _UNUSED int width = 0;
    int height = 0;
    if (screen == framebuffers->top_left || screen == framebuffers->top_right) {
        width = TOP_WIDTH;
        height = TOP_HEIGHT;
    } else if (screen == framebuffers->bottom) {
        width = BOTTOM_WIDTH;
        height = BOTTOM_HEIGHT;
    } else {
        return; // Invalid buffer.
    }

    int x = (font_w + font_kern) * ch_x;
    int y = font_h * ch_y;

    if (x >= width || y >= height)
        return; // OOB

    unsigned int c_font_w = (font_w / 8) + (font_w % 8 ? 1 : 0);

	for (unsigned int yy = 0; yy < font_h; yy++) {
		int xDisplacement = (x * SCREEN_DEPTH * height);
		int yDisplacement = ((height - (y + yy) - 1) * SCREEN_DEPTH);
		unsigned int pos = xDisplacement + yDisplacement;
        unsigned char char_dat = ((char*)FCRAM_FONT_LOC)[(character - ' ') * (c_font_w * font_h) + yy];
        for(unsigned int i=0; i < font_w + font_kern; i++) {
            screen[pos]     = color_bg >> 16;
            screen[pos + 1] = color_bg >> 8;
            screen[pos + 2] = color_bg;

			if (char_dat & 0x80) {
                screen[pos]     = color_fg >> 16;
                screen[pos + 1] = color_fg >> 8;
                screen[pos + 2] = color_fg;
			}

            char_dat <<= 1;
			pos += SCREEN_DEPTH * height;
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
putc(void *buf, const int c)
{
    if (buf == stdout || buf == stderr) {
        if (kill_output)
            return;

        unsigned int width = 0;
        _UNUSED unsigned int height = 0;
        unsigned int *cursor_x = NULL;
        unsigned int *cursor_y = NULL;
        uint8_t *screen = NULL;
        unsigned char *color = NULL;

        if (buf == TOP_SCREEN) {
            width = text_top_width;
            height = text_top_height;
            screen = framebuffers->top_left;
            cursor_x = &top_cursor_x;
            cursor_y = &top_cursor_y;
            color = &color_top;
        } else if (buf == BOTTOM_SCREEN) {
            width = text_bottom_width;
            height = text_bottom_height;
            screen = framebuffers->bottom;
            cursor_x = &bottom_cursor_x;
            cursor_y = &bottom_cursor_y;
            color = &color_bottom;
        }

        if (cursor_x[0] >= width) {
            cursor_x[0] = 0;
            cursor_y[0]++;
        }

        while (cursor_y[0] >= height) {
            clear_disp(buf);
            cursor_x[0] = 0;
            cursor_y[0] = 0;

/*			uint32_t col = SCREEN_TOP_HEIGHT * SCREEN_DEPTH;
            uint32_t one_c = 8 * SCREEN_DEPTH;
            for (unsigned int x = 0; x < width * 8; x++) {
                memmove(&screen[x * col + one_c], &screen[x * col + one_c], col - one_c);
            } */
        }

        if ((isprint(c) || c == '\n') && buf == BOTTOM_SCREEN) {
            log_buffer[log_size] = c;
            log_size++;
            dump_log(0);
        }

        switch (c) {
            case '\n':
                cursor_y[0]++;
            // Fall through intentional.
            case '\r':
                cursor_x[0] = 0; // Reset to beginning of line.
                break;
            default:
                draw_character(screen, c, cursor_x[0], cursor_y[0], colors[(color[0] >> 4) & 0xF], colors[color[0] & 0xF]);

                cursor_x[0]++;

                break;
        }
    } else {
        // FILE*, not stdin or stdout.
        fwrite(&c, 1, 1, (FILE *)buf);
    }
}

void
puts(void *buf, const char *string)
{
    if ((buf == stdout || buf == stderr) && kill_output)
        return;

    char *ref = (char *)string;

    while (ref[0] != 0) {
        putc(buf, ref[0]);
        ref++;
    }
}

void
put_int64(void *channel, int64_t n, int length)
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
put_uint64(void *channel, uint64_t n, int length)
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
put_hexdump(void *channel, unsigned int num)
{
    uint8_t *num_8 = (uint8_t *)&num;
    for (int i = 3; i >= 0; i--) {
        uint8_t high = (num_8[i] >> 4) & 0xf;
        uint8_t low = num_8[i] & 0xf;

        putc(channel, ("0123456789abcdef")[high]);
        putc(channel, ("0123456789abcdef")[low]);
    }
}

void
put_uint(void *channel, unsigned int n, int length)
{
    put_uint64(channel, n, length);
}

void
put_int(void *channel, int n, int length)
{
    put_int64(channel, n, length);
}

void
fflush(void *channel)
{
    if (channel == BOTTOM_SCREEN) {
        dump_log(1);
    } if (channel != TOP_SCREEN && channel != BOTTOM_SCREEN) {
        f_sync(&(((FILE *)channel)->handle)); // Sync to disk.
    }
}

int disable_format = 0;

void
vfprintf(void *channel, const char *format, va_list ap)
{
    if ((channel == stdout || channel == stderr) && kill_output)
        return;

    char *ref = (char *)format;

    unsigned char *color = NULL;
    if (channel == TOP_SCREEN)
        color = &color_top;
    else if (channel == BOTTOM_SCREEN)
        color = &color_bottom;

    while (ref[0] != '\0') {
        if (ref[0] == 0x1B && (++ref)[0] == '[' && (channel == stdout || channel == stderr)) {
        ansi_codes:
            // Ansi escape code.
            ++ref;
            // [30-37] Set text color
            if (ref[0] == '3') {
                ++ref;
                if (ref[0] >= '0' && ref[0] <= '7') {
                    // Valid FG color.
                    color[0] &= 0x0f; // Remove fg color.
                    color[0] |= (ref[0] - '0') << 4;
                }
            }
            // [40-47] Set bg color
            else if (ref[0] == '4') {
                ++ref;
                if (ref[0] >= '0' && ref[0] <= '7') {
                    // Valid BG color.
                    color[0] &= 0xf0; // Remove bg color.
                    color[0] |= ref[0] - '0';
                }
            } else if (ref[0] == '0') {
                // Reset.
                color[0] = 0xf0;
            }

            ++ref;

            if (ref[0] == ';') {
                goto ansi_codes; // Another code.
            }

            // Loop until the character is somewhere 0x40 - 0x7E, which
            // terminates an ANSI sequence
            while (!(ref[0] >= 0x40 && ref[0] <= 0x7E))
                ref++;
        } else if (ref[0] == '%' && !disable_format) {
            int type_size = 0;
            int length = -1;
        check_format:
            // Format string.
            ++ref;
            switch (ref[0]) {
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
                    fprintf(channel, va_arg(ap, char *));
                    disable_format = 0; // Reenable.
                    break;
                case 'c':
                    putc(channel, va_arg(ap, int));
                    break;
                case 'p':
                    puts(channel, va_arg(ap, char *));
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
                    if (ref[0] >= '0' && ref[0] <= '9') {
                        length = ref[0] - '0';
                        goto check_format;
                    }
                    break;
            }
        } else {
            putc(channel, ref[0]);
        }
        ++ref;
    }
}

void
fprintf(void *channel, const char *format, ...)
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
