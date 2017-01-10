#include <common.h>

extern int is_n3ds;
extern unsigned int font_w;
extern unsigned int font_h;

void
header(const char *append)
{
    set_cursor(TOP_SCREEN, 0, 0);
    fill_line(stdout, 0, get_opt_u32(OPTION_ACCENT_COLOR));
    accent_color(TOP_SCREEN, 0);
    fprintf(stdout, "\x1b[30m ." FW_NAME " // %s\x1b[0m\n\n", append);
}

void show_help(const char* help) {
    clear_disp(TOP_SCREEN);
    set_cursor(TOP_SCREEN, 0, 0);
    header("Any:Back");
    fprintf(stdout, "%s", help);
    wait_key(1);
}

void accent_color(void* screen, int fg) {
    char color[] = "\x1b[30m";
    if (!fg) color[2] = '4';
    color[3] = ("01234567")[get_opt_u32(OPTION_ACCENT_COLOR)];
    fprintf(screen, "%s", color);
}

int
show_menu(struct options_s *options)
{
    int cursor_y = 0;
    int cursor_min = -1;
    int cursor_max = -1;
    int exit = 0;

    // Font height is user-controlled. Realistically, if it's higher than a signed int, that's not my fault.
    int window_size = (TOP_HEIGHT / (int)font_h) - 3;
    int window_top = 0, window_bottom = window_size;
    int less_mode = 0;

    clear_disp(TOP_SCREEN);

    if (options[0].name == NULL) {
        set_cursor(TOP_SCREEN, 0, 0);
        header("Any:Back");
        fprintf(stdout, "No entries.\n");
        wait_key(1);
        return 0;
    }

    while (!exit) {
        set_cursor(TOP_SCREEN, 0, 0);

        // Figure out the max if unset.
        if (cursor_max == -1) {
            cursor_max = 0;
            while (options[cursor_max].name != NULL)
                ++cursor_max;

            while (options[cursor_max].handle == unselectable && cursor_max > 0)
                --cursor_max;
        }

        if (cursor_max == 0)
            less_mode = 1; // Behave as a pager

        // Figure out the min if unset.
        if (cursor_min == -1) {
            if (less_mode == 1) {
                cursor_max = 0;
                while (options[cursor_max].name != NULL)
                    ++cursor_max;

                cursor_min = 0;
            } else {
                cursor_min = 0;
                while (options[cursor_min].handle == unselectable)
                    ++cursor_min;
                cursor_y = cursor_min;
            }
        }

        if (less_mode)
            header("B:Back DPAD:Scroll");
        else if (cursor_max == cursor_min)
            header("A:Enter B:Back");
        else
            header("A:Enter B:Back DPAD:Nav Select:Info");

        for (int i = window_top, skip = 0; options[i].name != NULL; ++i) { // -1 Sentinel.
            if (i > window_bottom + skip)
                break;

            if (options[i].handle == option_n3ds && !is_n3ds) {
                ++skip;
                continue;
            }

            // NOTE - Signed to unsigned conversion here. Again, not an issue.
            set_cursor(TOP_SCREEN, 0, (unsigned int)(i - window_top - skip + 2) );

            // Print option
            if (options[i].handle != unselectable) {
                int indent = options[i].indent;
                for(int j=0; j < indent; j++)
                    fprintf(TOP_SCREEN, "  ");

                if (cursor_y == i) {
                    accent_color(TOP_SCREEN, 1);
                    fprintf(TOP_SCREEN, ">>\x1b[0m ");
                } else {
                    fprintf(TOP_SCREEN, "   ");
                }

                if (options[i].handle != break_menu && options[i].value != NULL) {
                    fprintf(TOP_SCREEN, "[%s]  ", options[i].value(options[i].param));
                }
            }

            if (options[i].highlight == 1)
                accent_color(TOP_SCREEN, 1);
            fprintf(TOP_SCREEN, "%s\x1b[0m", options[i].name);
        }

        uint32_t key = wait_key(1);

        switch (key) {
            case CTR_HID_UP:
                if (cursor_min == cursor_max)
                    break;
                cursor_y -= 1;
                while ((options[cursor_y].handle == unselectable || (options[cursor_y].handle == option_n3ds && !is_n3ds)) && cursor_y >= cursor_min && !less_mode)
                    cursor_y--;
                break;
            case CTR_HID_DOWN:
                if (cursor_min == cursor_max)
                    break;
                cursor_y += 1;
                while ((options[cursor_y].handle == unselectable || (options[cursor_y].handle == option_n3ds && !is_n3ds)) && cursor_y < cursor_max && !less_mode)
                    cursor_y++;
                break;
            case CTR_HID_LEFT:
                if (cursor_min == cursor_max)
                    break;
                cursor_y -= 5;
                while ((options[cursor_y].handle == unselectable || (options[cursor_y].handle == option_n3ds && !is_n3ds)) && cursor_y >= cursor_min && !less_mode)
                    cursor_y--;
                break;
            case CTR_HID_RIGHT:
                if (cursor_min == cursor_max)
                    break;
                cursor_y += 5;
                while ((options[cursor_y].handle == unselectable || (options[cursor_y].handle == option_n3ds && !is_n3ds)) && cursor_y < cursor_max && !less_mode)
                    cursor_y++;
                break;
            case CTR_HID_A:
                if (less_mode)
                    break;

                if (options[cursor_y].handle == option || options[cursor_y].handle == option_n3ds) {
                    options[cursor_y].func(options[cursor_y].param);
                } else if (options[cursor_y].handle == break_menu) {
                    exit = 1;
                    clear_disp(TOP_SCREEN);
                    cursor_y = cursor_min;
                }
                break;
            case CTR_HID_B:
                exit = 1;
                clear_disp(TOP_SCREEN);
                cursor_y = cursor_min;
                break;
            case CTR_HID_SELECT:
                if (options[cursor_y].desc[0] != 0) {
                    show_help(options[cursor_y].desc);
                    clear_disp(TOP_SCREEN);
                }
                break;
            default:
                break;
        }

        if (cursor_y < cursor_min)
            cursor_y = cursor_max - 1;
        else if (cursor_y > cursor_max - 1)
            cursor_y = cursor_min;

        if (less_mode) {
            window_top = cursor_y;
            window_bottom = window_top + window_size;
            clear_disp(TOP_SCREEN);
        }

        if (cursor_y < window_top + cursor_min) {
            window_top = cursor_y - cursor_min;
            window_bottom = window_top + window_size;
            clear_disp(TOP_SCREEN);
        } else if (cursor_y > window_bottom - cursor_min) {
            window_bottom = cursor_y + cursor_min;
            window_top = window_bottom - window_size;
            clear_disp(TOP_SCREEN);
        }
    }

    return 0;
}
