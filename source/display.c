#include "common.h"
#include "firm/firm.h"
#include "firm/headers.h"

void header(char *append);

extern int is_n3ds;

extern unsigned int font_h;

void show_help(char* help) {
    clear_screen(TOP_SCREEN);
    set_cursor(TOP_SCREEN, 0, 0);
    header("Any:Back");
    fprintf(stdout, "%s", help);
    wait_key(1);
}

int
show_menu(struct options_s *options, uint8_t *toggles)
{
    int cursor_y = 0;
    int cursor_min = -1;
    int cursor_max = -1;
    int exit = 0;
    int window_size = (TOP_HEIGHT / font_h) - 3;
    int window_top = 0, window_bottom = window_size;
    int less_mode = 0;

    clear_screen(TOP_SCREEN);

    if (options[0].index == -1) {
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
            while (options[cursor_max].index != -1)
                ++cursor_max;

            while (options[cursor_max].allowed == not_option && cursor_max > 0)
                --cursor_max;
        }

        if (cursor_max == 0)
            less_mode = 1; // Behave as a pager

        // Figure out the min if unset.
        if (cursor_min == -1) {
            if (less_mode == 1) {
                cursor_max = 0;
                while (options[cursor_max].index != -1)
                    ++cursor_max;

                cursor_min = 0;
            } else {
                cursor_min = 0;
                while (options[cursor_min].allowed == not_option)
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


        int i = window_top;
        while (options[i].index != -1) { // -1 Sentinel.
            if (i > window_bottom)
                break;

            set_cursor(TOP_SCREEN, 0, i-window_top+2);

            if (options[i].allowed == boolean_val || (is_n3ds && options[i].allowed == boolean_val_n3ds)) {
                if (cursor_y == i)
                    fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
                else
                    fprintf(TOP_SCREEN, "   ");

                fprintf(TOP_SCREEN, "[%c]  %s", (toggles[options[i].index] ? '*' : ' '), options[i].name);
            } else if (options[i].allowed == call_fun || options[i].allowed == break_menu) {
                if (cursor_y == i)
                    fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
                else
                    fprintf(TOP_SCREEN, "   ");

                fprintf(TOP_SCREEN, "%s", options[i].name);
            } else if (options[i].allowed == ranged_val) {
                if (cursor_y == i)
                    fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
                else
                    fprintf(TOP_SCREEN, "   ");

                fprintf(TOP_SCREEN, "[%u]  %s  ", toggles[options[i].index], options[i].name);
            } else if (options[i].allowed == not_option) {
                fprintf(TOP_SCREEN, "%s", options[i].name);
            }
            ++i;
        }

        uint32_t key = wait_key(1);

        switch (key) {
            case BUTTON_UP:
                if (cursor_min == cursor_max)
                    break;
                cursor_y -= 1;
                while ((options[cursor_y].allowed == not_option || (options[cursor_y].allowed == boolean_val_n3ds && !is_n3ds)) && cursor_y >= cursor_min && !less_mode)
                    cursor_y--;
                break;
            case BUTTON_DOWN:
                if (cursor_min == cursor_max)
                    break;
                cursor_y += 1;
                while ((options[cursor_y].allowed == not_option || (options[cursor_y].allowed == boolean_val_n3ds && !is_n3ds)) && cursor_y < cursor_max && !less_mode)
                    cursor_y++;
                break;
            case BUTTON_LEFT:
                if (cursor_min == cursor_max)
                    break;
                cursor_y -= 5;
                while ((options[cursor_y].allowed == not_option || (options[cursor_y].allowed == boolean_val_n3ds && !is_n3ds)) && cursor_y >= cursor_min && !less_mode)
                    cursor_y--;
                break;
            case BUTTON_RIGHT:
                if (cursor_min == cursor_max)
                    break;
                cursor_y += 5;
                while ((options[cursor_y].allowed == not_option || (options[cursor_y].allowed == boolean_val_n3ds && !is_n3ds)) && cursor_y < cursor_max && !less_mode)
                    cursor_y++;
                break;
            case BUTTON_A:
                if (less_mode)
                    break;

                if (options[cursor_y].allowed == boolean_val || options[cursor_y].allowed == boolean_val_n3ds) {
                    toggles[options[cursor_y].index] = !toggles[options[cursor_y].index];
                } else if (options[cursor_y].allowed == ranged_val) {
                    if (toggles[options[cursor_y].index] == options[cursor_y].b)
                        toggles[options[cursor_y].index] = options[cursor_y].a;
                    else
                        toggles[options[cursor_y].index]++;
                } else if (options[cursor_y].allowed == call_fun) {
                    ((func_call_t)(options[cursor_y].a))(options[cursor_y].b); // Call 'a' as a function.
                } else if (options[cursor_y].allowed == break_menu) {
                    exit = 1;
                    clear_screen(TOP_SCREEN);
                    cursor_y = cursor_min;
                }
                break;
            case BUTTON_X:
                if (options[cursor_y].allowed == ranged_val) {
                    if (toggles[options[cursor_y].index] == options[cursor_y].a)
                        toggles[options[cursor_y].index] = options[cursor_y].b;
                    else
                        toggles[options[cursor_y].index]--;
                }
                break;
            case BUTTON_B:
                exit = 1;
                clear_screen(TOP_SCREEN);
                cursor_y = cursor_min;
                break;
            case BUTTON_SEL:
                if (options[cursor_y].desc[0] != 0) {
                    show_help(options[cursor_y].desc);
                    clear_screen(TOP_SCREEN);
                }
                break;
        }

        if (cursor_y < cursor_min)
            cursor_y = cursor_max - 1;
        else if (cursor_y > cursor_max - 1)
            cursor_y = cursor_min;

        if (cursor_y < window_top + cursor_min) {
            window_top = cursor_y - cursor_min;
            window_bottom = window_top + window_size;
            clear_screen(TOP_SCREEN);

        } else if (cursor_y > window_bottom - cursor_min) {
            window_bottom = cursor_y + cursor_min;
            window_top = window_bottom - window_size;
            clear_screen(TOP_SCREEN);
        }
    }

    return 0;
}
