#ifndef __ABORT_H
#define __ABORT_H

#include <stdarg.h>
#include <common.h>

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
