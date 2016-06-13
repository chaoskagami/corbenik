#include "common.h"
#include "firm/firm.h"
#include "firm/headers.h"
#define MENU_MAIN 1

#define MENU_OPTIONS 2
#define MENU_PATCHES 3
#define MENU_INFO 4
#define MENU_HELP 5
#define MENU_RESET 6
#define MENU_POWER 7
#define MENU_BOOTME 8

void header(char *append);

extern int is_n3ds;

int
show_menu(struct options_s *options, uint8_t *toggles)
{
    int cursor_y = 0;
    int need_redraw = 1;
    int cursor_min = -1;
    int cursor_max = -1;
    int exit = 0;

    clear_screen(TOP_SCREEN);

    if (options[0].index == -1) {
        set_cursor(TOP_SCREEN, 0, 0);
        header("Any:Back");
        fprintf(stdout, "No entries.\n");
        wait_key();
        return 0;
    }

    while (!exit) {
        set_cursor(TOP_SCREEN, 0, 0);

        // Figure out the max if unset.
        if (cursor_max == -1) {
            cursor_max = 0;
            while (options[cursor_max].index != -1)
                ++cursor_max;

            while (options[cursor_max].allowed == not_option)
                --cursor_max;
        }

        // Figure out the max if unset.
        if (cursor_min == -1) {
            cursor_min = 0;
            while (options[cursor_min].allowed == not_option)
                ++cursor_min;
            cursor_y = cursor_min;
        }

        header("A:Enter B:Back DPAD:Nav");

        int i = 0;
        while (options[i].index != -1) { // -1 Sentinel.
            if (options[i].allowed == boolean_val || (is_n3ds && options[i].allowed == boolean_val_n3ds)) {
                if (cursor_y == i)
                    fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
                else
                    fprintf(TOP_SCREEN, "   ");

                if (need_redraw)
                    fprintf(TOP_SCREEN, "[%c]  %s\n", (toggles[options[i].index] ? 'X' : ' '), options[i].name);
                else {
                    // Yes, this is weird. printf does a large number of extra things we
                    // don't
                    // want computed at the moment; this is faster.
                    putc(TOP_SCREEN, '[');
                    putc(TOP_SCREEN, (toggles[options[i].index] ? 'X' : ' '));
                    putc(TOP_SCREEN, ']');
                    putc(TOP_SCREEN, '\n');
                }
            } else if (options[i].allowed == call_fun || options[i].allowed == break_menu) {
                if (cursor_y == i)
                    fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
                else
                    fprintf(TOP_SCREEN, "   ");

                if (need_redraw)
                    fprintf(TOP_SCREEN, "%s\n", options[i].name);
                else
                    putc(TOP_SCREEN, '\n');
            } else if (options[i].allowed == ranged_val) {
                if (cursor_y == i)
                    fprintf(TOP_SCREEN, "\x1b[32m>>\x1b[0m ");
                else
                    fprintf(TOP_SCREEN, "   ");

                fprintf(TOP_SCREEN, "[%u]  %s  \n", toggles[options[i].index], options[i].name);
            } else if (options[i].allowed == not_option) {
                fprintf(TOP_SCREEN, "%s\n", options[i].name);
            }
            ++i;
        }

        need_redraw = 0;

        uint32_t key = wait_key();

        switch (key) {
            case BUTTON_UP:
                cursor_y -= 1;
                while ((options[cursor_y].allowed == not_option || (options[cursor_y].allowed == boolean_val_n3ds && !is_n3ds)) && cursor_y >= cursor_min)
                    cursor_y--;
                break;
            case BUTTON_DOWN:
                cursor_y += 1;
                while ((options[cursor_y].allowed == not_option || (options[cursor_y].allowed == boolean_val_n3ds && !is_n3ds)) && cursor_y < cursor_max)
                    cursor_y++;
                break;
            case BUTTON_LEFT:
                cursor_y -= 5;
                while ((options[cursor_y].allowed == not_option || (options[cursor_y].allowed == boolean_val_n3ds && !is_n3ds)) && cursor_y >= cursor_min)
                    cursor_y--;
                break;
            case BUTTON_RIGHT:
                cursor_y += 5;
                while ((options[cursor_y].allowed == not_option || (options[cursor_y].allowed == boolean_val_n3ds && !is_n3ds)) && cursor_y < cursor_max)
                    cursor_y++;
                break;
            case BUTTON_A:
                if (options[cursor_y].allowed == boolean_val || options[cursor_y].allowed == boolean_val_n3ds) {
                    toggles[options[cursor_y].index] = !toggles[options[cursor_y].index];
                } else if (options[cursor_y].allowed == ranged_val) {
                    if (toggles[options[cursor_y].index] == options[cursor_y].b)
                        toggles[options[cursor_y].index] = options[cursor_y].a;
                    else
                        toggles[options[cursor_y].index]++;
                } else if (options[cursor_y].allowed == call_fun) {
                    ((func_call_t)(options[cursor_y].a))(); // Call 'a' as a function.
                    need_redraw = 1;
                } else if (options[cursor_y].allowed == break_menu) {
                    exit = 1;
                    need_redraw = 1;
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
                need_redraw = 1;
                clear_screen(TOP_SCREEN);
                cursor_y = cursor_min;
                break;
        }

        if (cursor_y < cursor_min)
            cursor_y = cursor_max - 1;
        else if (cursor_y > cursor_max - 1)
            cursor_y = cursor_min;
    }

    clear_screen(TOP_SCREEN);

    return 0;
}
