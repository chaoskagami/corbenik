#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <common.h>
#include <ctr9/io.h>

static unsigned int top_cursor_x = 0, top_cursor_y = 0;
static unsigned int bottom_cursor_x = 0, bottom_cursor_y = 0;

#define LOG_BUFFER_SIZE 4096

static size_t  log_size = 0;
static char*   log_buffer; // Log buffer.

unsigned int font_w = 8;
unsigned int font_h = 8;
static unsigned int font_kern = 0;

static unsigned int text_top_width = 20;
static unsigned int text_top_height = 10;

static unsigned int text_bottom_width = 20;
static unsigned int text_bottom_height = 10;

uint8_t *top_bg;
uint8_t *bottom_bg;

unsigned char color_top = 0xf0;
unsigned char color_bottom = 0xf0;
int kill_output = 0;

uint8_t* font_data = NULL;

#define TRANSP_THRESH 178

static uint8_t alphamap[256] = {0};

void std_init() {
	for(uint16_t i=0; i < 0x100; i++) {
		alphamap[i] = 0;
		if (i > 0x7F)
			alphamap[i] = i - 0x7F;
	}

    top_bg     = malloc(TOP_SIZE);
    bottom_bg  = malloc(BOTTOM_SIZE);
    log_buffer = malloc(LOG_BUFFER_SIZE);
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

void rect(void* channel, unsigned int x, unsigned int y, unsigned int x2, unsigned int y2, uint8_t color) {
    uint8_t* screen = NULL;
    unsigned int height = 0;
    if (channel == stdout) {
        screen = framebuffers->top_left;
        height = TOP_HEIGHT;
    } else if (channel == stderr) {
        screen = framebuffers->bottom;
        height = BOTTOM_HEIGHT;
    } else {
        return; // Invalid on non-screen displays.
    }

    for(unsigned int y_a = y; y_a < y2; y_a++) {
        for(unsigned int x_a = x; x_a < x2; x_a++) {
            unsigned int xDisplacement = (x_a * SCREEN_DEPTH * height);
            unsigned int yDisplacement = ((height - y_a - 1) * SCREEN_DEPTH);
            unsigned int pos = xDisplacement + yDisplacement;

            screen[pos + 1] = colors[color & 0xF];
            screen[pos + 2] = colors[color & 0xF] >> 8;
            screen[pos + 3] = colors[color & 0xF] >> 16;
            screen[pos] = 0xFF;
        }
    }
}

void fill_line(void* channel, unsigned int y, uint8_t color) {
    unsigned int x2 = 0;
    if (channel == stdout)
        x2 = TOP_WIDTH;
    else if (channel == stderr)
        x2 = BOTTOM_WIDTH;

    rect(channel, 0, (y * font_h), x2, ((y+1) * font_h), color);
}

// This is (roughly) the screenshot specs as used by smeas scrtool.
void screenshot() {
    f_unlink(PATH_TEMP "/screenshot.ppm");

    // Open the screenshot blob used by hbmenu et al
    FILE* f = fopen(PATH_TEMP "/screenshot.ppm", "w");

    if (!f) return;

    fwrite("P6 400 480 255 ", 1, 15, f);

    for(int y = 0; y < 240; y++) {
        for(int x = 0; x < 400; x++) {
            int xDisplacement = (x * SCREEN_DEPTH * 240);
            int yDisplacement = ((240 - y - 1) * SCREEN_DEPTH);
            int pos = xDisplacement + yDisplacement;

            fwrite(& framebuffers->top_left[pos + 3], 1, 1, f);
            fwrite(& framebuffers->top_left[pos + 2], 1, 1, f);
            fwrite(& framebuffers->top_left[pos + 1], 1, 1, f);
        }
    }

    uint8_t zero = 0;

    for(int y = 0; y < 240; y++) {
        for (int i = 0; i < 40 * 3; i++)
            fwrite(& zero, 1, 1, f);

        for (int x = 0; x < 320; x++) {
            int xDisplacement = (x * SCREEN_DEPTH * 240);
            int yDisplacement = ((240 - y - 1) * SCREEN_DEPTH);
            int pos = xDisplacement + yDisplacement;

            fwrite(& framebuffers->bottom[pos + 3], 1, 1, f);
            fwrite(& framebuffers->bottom[pos + 2], 1, 1, f);
            fwrite(& framebuffers->bottom[pos + 1], 1, 1, f);
        }

        for (int i = 0; i < 40 * 3; i++)
            fwrite(& zero, 1, 1, f);
    }

    fclose(f);

    fprintf(stderr, "Screenshot: %s\n", PATH_TEMP "/screenshot.ppm");
}

void clear_bg() {
    memset(top_bg, 0, TOP_SIZE);
    memset(bottom_bg, 0, BOTTOM_SIZE);
}

void load_bg_top(char* fname_top) {
    FILE* f = fopen(fname_top, "r");
    if (!f) return;

    fread(top_bg, 1, TOP_SIZE, f);

    fclose(f);
}

void load_bg_bottom(char* fname_bottom) {
    FILE* f = fopen(fname_bottom, "r");
    if (!f)
        return;

    fread(bottom_bg, 1, BOTTOM_SIZE, f);
    fclose(f);
}

void set_font(const char* filename) {
    // TODO - Unicode support. Right now, we only load 32

    FILE* f = fopen(filename, "r");

    if (!f) abort("Failed to load font file!\n");

    unsigned int new_w, new_h;

    fread(&new_w, 1, 4, f);
    fread(&new_h, 1, 4, f);

    if (new_w == 0 || new_h == 0) {
        abort("Invalid font file: w/h is 0 - not loaded\n");
    }

    unsigned int c_font_w = (new_w / 8) + ((new_w % 8) ? 1 : 0);

	font_data = malloc(c_font_w * new_h * (256 - ' '));

    fread(font_data, 1, c_font_w * new_h * (256 - ' '), f); // Skip non-printing chars.

    fclose(f);

    font_w = new_w;
    font_h = new_h;

    text_top_width  = TOP_WIDTH  / (font_w + font_kern);
    text_top_height = TOP_HEIGHT / font_h;

    text_bottom_width  = BOTTOM_WIDTH / (font_w + font_kern);
    text_bottom_height = BOTTOM_HEIGHT / font_h;
}

void dump_log(unsigned int force) {
    if(!config->options[OPTION_SAVE_LOGS])
        return;

    if (force == 0 && log_size < LOG_BUFFER_SIZE-1)
        return;

    if (log_size == 0)
        return;

    FILE *f = fopen(PATH_BOOTLOG, "w");
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
        for(int i=0, j=0; j < TOP_SIZE; i += 3, j += 4) {
            screen[j] = 0xFF;
            screen[j + 1] = top_bg[i];
            screen[j + 2] = top_bg[i + 1];
            screen[j + 3] = top_bg[i + 2];
            if (!kill_output && config->options[OPTION_DIM_MODE]) {
                screen[j + 1] = alphamap[screen[j + 1]];
                screen[j + 2] = alphamap[screen[j + 2]];
                screen[j + 3] = alphamap[screen[j + 3]];
            }
        }

        top_cursor_x = 0;
        top_cursor_y = 0;
    } else if (screen == framebuffers->bottom) {
        for(int i=0, j=0; j < BOTTOM_SIZE; i += 3, j += 4) {
            screen[j] = 0x7F;
            screen[j + 1] = bottom_bg[i];
            screen[j + 2] = bottom_bg[i + 1];
            screen[j + 3] = bottom_bg[i + 2];
            if (!kill_output && config->options[OPTION_DIM_MODE]) {
                screen[j + 1] = alphamap[screen[j + 1]];
                screen[j + 2] = alphamap[screen[j + 2]];
                screen[j + 3] = alphamap[screen[j + 3]];
            }
        }

        bottom_cursor_x = 0;
        bottom_cursor_y = 0;
    }
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
draw_character(uint8_t *screen, const unsigned int character, unsigned int ch_x, unsigned int ch_y, const uint32_t color_fg, const uint32_t color_bg)
{
    if (!isprint(character))
        return; // Don't output non-printables.

    _UNUSED unsigned int width = 0;
    unsigned int height = 0;
    uint8_t* buffer_bg;
    if (screen == framebuffers->top_left || screen == framebuffers->top_right) {
        width = TOP_WIDTH;
        height = TOP_HEIGHT;
        buffer_bg = top_bg;
    } else if (screen == framebuffers->bottom) {
        width = BOTTOM_WIDTH;
        height = BOTTOM_HEIGHT;
        buffer_bg = bottom_bg;
    } else {
        return; // Invalid buffer.
    }

    unsigned int x = (font_w + font_kern) * ch_x;
    unsigned int y = font_h * ch_y;

    if (x >= width || y >= height)
        return; // OOB

    unsigned int c_font_w = (font_w / 8) + ((font_w % 8) ? 1 : 0);

    for (unsigned int yy = 0; yy < font_h; yy++) {
        unsigned int xDisplacement   = (x * SCREEN_DEPTH * height);
        unsigned int yDisplacement   = ((height - (y + yy) - 1) * SCREEN_DEPTH);
        unsigned int pos    = xDisplacement + yDisplacement;

        unsigned int xDisplacementBg = (x * 3 * height);
        unsigned int yDisplacementBg = ((height - (y + yy) - 1) * 3);
        unsigned int pos_b  = xDisplacementBg + yDisplacementBg;

        unsigned char char_dat = font_data[(character - ' ') * (c_font_w * font_h) + yy];

        for(unsigned int i=0; i < font_w + font_kern; i++) {
            screen[pos] = 0xFF;

            if (color_bg == 0) {
                screen[pos + 1] = buffer_bg[pos_b];
                screen[pos + 2] = buffer_bg[pos_b + 1];
                screen[pos + 3] = buffer_bg[pos_b + 2];
                if (config->options[OPTION_DIM_MODE]) {
                    screen[pos + 1] = alphamap[screen[pos + 1]];
                    screen[pos + 2] = alphamap[screen[pos + 2]];
                    screen[pos + 3] = alphamap[screen[pos + 3]];
                }
            } else {
                screen[pos + 1] = color_bg;
                screen[pos + 2] = color_bg >> 8;
                screen[pos + 3] = color_bg >> 16;
            }

            if (char_dat & 0x80) {
                if (color_fg == 0) {
                    screen[pos + 1] = buffer_bg[pos_b];
                    screen[pos + 2] = buffer_bg[pos_b + 1];
                    screen[pos + 3] = buffer_bg[pos_b + 2];
                    if (config->options[OPTION_DIM_MODE]) {
                        screen[pos + 1] = alphamap[screen[pos + 1]];
                        screen[pos + 2] = alphamap[screen[pos + 2]];
                        screen[pos + 3] = alphamap[screen[pos + 3]];
                    }
                } else {
                    screen[pos + 1] = color_fg;
                    screen[pos + 2] = color_fg >> 8;
                    screen[pos + 3] = color_fg >> 16;
                }
            }

            char_dat <<= 1;
            pos      += SCREEN_DEPTH * height;
            pos_b    += 3 * height;
        }
    }
}

void
shut_up()
{
    kill_output = 1; // Immediately cancel all output operations.
}

#define TEXT       0
#define ANSI_NEXT  1
#define ANSI_END   2
#define ANSI_PARSE 3

int stdout_state = TEXT, stderr_state = TEXT;
int stdout_val   = 0,    stderr_val   = 0;

// Returns 1 if state machine is parsing - and will not output.
// This is how colors are now handled.
int
ansi_statemach(void* buf, const int c)
{
	int* state = NULL, *val = NULL;
    uint8_t *color = NULL;

    uint8_t color_tmp = 0;

    if (buf == stdout) {
        color = &color_top;
	    state = &stdout_state;
	    val = &stdout_val;
    } else if (buf == stderr) {
        color = &color_bottom;
	    state = &stderr_state;
	    val = &stderr_val;
    } else {
        return 0;
    }

	switch(*state) {
		case TEXT:
			if (c == '\x1b') {
				*state = ANSI_NEXT;
				return 1;
			}
			return 0;
		case ANSI_NEXT:
			if (c == '[') {
				*state = ANSI_PARSE;
				*val = 0;
				return 1;
			}
			// INVALID; this is a bad ansi sequence. Term early.
			*state = TEXT;
			return 0;
		case ANSI_END:
			if (c == ';') {
				*state = ANSI_PARSE; // Another code coming up.
			} else if(c >= 0x40 && c <= 0x7E) {
				*state = TEXT;
			}

			return 1;
		case ANSI_PARSE:
			if (c >= '0' && c <= '9') {
				*val *= 10;
				*val += (c - '0');

				*state = ANSI_PARSE;

				if (*val == 0) {
					// Reset formatting.
					*color = 0xf0;
					*state = ANSI_END;
				}

				if (*val >= 10) {
					switch(*val / 10) {
						case 3: // Foreground color
		                    *color &= 0x0f; // Remove fg color.
		                    *color |= ((*val % 30) & 0xf) << 4;
                            break;
						case 4: // Background color
		                    *color &= 0xf0; // Remove bg color.
		                    *color |= ((*val % 40) & 0xf);
                            break;
                        default: // ???
                            break;
					}
					*state = ANSI_END;
				}
			}
			return 1;
        default:
            *state = TEXT;
            return 1;
	}

    return 0; // Should not be reached.
}

void
putc(void *buf, int c)
{
	if(ansi_statemach(buf, c) == 1) // Inside ANSI escape?
		return;

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
                draw_character(screen, (unsigned int)c, cursor_x[0], cursor_y[0], colors[(color[0] >> 4) & 0xF], colors[color[0] & 0xF]);

                cursor_x[0]++;

                break;
        }
    } else {
        // FILE*, not stdin or stdout.
        fwrite(&c, 1, 1, (FILE *)buf);
    }
}

void
puts(void *buf, char *string)
{
    if ((buf == stdout || buf == stderr) && kill_output)
        return;

    char *ref = string;

    while (ref[0] != 0) {
        putc(buf, ref[0]);
        ref++;
    }
}

void
put_int64(void *channel, int64_t n, int length)
{
    char conv[32], out[32];
    size_t i = 0, sign = 0;

    memset(conv, 0, 32);
    memset(out, 0, 32);

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

    size_t len = strlen(conv);
    for (i = 0; i < len; i++)
        out[i] = conv[(len - 1) - i];

    puts(channel, out);
}

void
put_uint64(void *channel, uint64_t n, int length)
{
    char conv[32], out[32];
    size_t i = 0;

    memset(conv, 0, 32);
    memset(out, 0, 32);

    do {
        conv[i++] = (n % 10) + '0';
    } while ((n /= 10) != 0);

    if (length > 0)
        out[length] = '\0';

    size_t len = strlen(conv);
    for (i = 0; i < len; i++)
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
vfprintf(void *channel, char *format, va_list ap)
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
        if (ref[0] == '%' && !disable_format) {
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
                case 'c':
                    putc(channel, va_arg(ap, int));
                    break;
                case 's':
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
fprintf(void *channel, char *format, ...)
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

void
printf(char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    vfprintf(stdout, format, ap);

    va_end(ap);
}
