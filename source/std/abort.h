#ifndef __ABORT_H
#define __ABORT_H

void abort(const char* x, ...) __attribute__ ((format (printf, 1, 2)));

#endif
