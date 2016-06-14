#ifndef __ABORT_H
#define __ABORT_H

#include "draw.h"

void poweroff();
uint32_t wait_key(int sleep);

#define abort(x...)                                                                                                                                            \
    {                                                                                                                                                          \
        fprintf(stderr, x);                                                                                                                                    \
        wait_key(1);                                                                                                                                            \
        clear_disp(stderr);                                                                                                                                    \
        set_cursor(stderr, 0, 0);                                                                                                                              \
        poweroff();                                                                                                                                            \
    }

#endif
