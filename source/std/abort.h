#ifndef __ABORT_H
#define __ABORT_H

#include "draw.h"

int menu_poweroff();
uint32_t wait_key();

#define abort(x...)                                                                                                                                            \
    {                                                                                                                                                          \
        fprintf(stderr, x);                                                                                                                                    \
        wait_key();                                                                                                                                            \
        clear_disp(stderr);                                                                                                                                    \
        menu_poweroff();                                                                                                                                       \
    }

#endif
