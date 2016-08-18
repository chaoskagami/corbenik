#ifndef __ABORT_H
#define __ABORT_H

/* Aborts the program and gives the user an opportunity to power off
 * via a button.
 *
 * Does NOT return.
 *
 * \param x Format string
 * \param ... Format operands, see printf manpage
 */

void abort(const char* x, ...) __attribute__ ((format (printf, 1, 2)));

#endif
