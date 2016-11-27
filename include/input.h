#ifndef __INPUT_H
#define __INPUT_H

#include <ctr9/ctr_hid.h>

/* Waits for a single key to be pressed and released, returning the key when done.
 *
 * \param sleep Unused. Previously would add some delay to each read for dpads with issues.
 * \return The key pressed and released.
 */
uint32_t wait_key(_UNUSED int sleep);

/* Displays a prompt on the bottom screen if the relevant option is enabled and waits on input
 * to continue.
 */
void wait(void);

#endif
