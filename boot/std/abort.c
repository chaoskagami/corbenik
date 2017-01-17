#ifndef __ABORT_H
#define __ABORT_H

#include <stdarg.h>    // for va_end, va_list, va_start
#include "input.h"     // for wait_key
#include "menu.h"      // for poweroff
#include "std/draw.h"  // for stderr, clear_disp, set_cursor

void panic(char* x, ...) {
    va_list ap;
    va_start(ap, x);

    vfprintf(stderr, x, ap);

    va_end(ap);

    wait_key(1);
    clear_disp(stderr);
    set_cursor(stderr, 0, 0);
    poweroff();
}

#endif
