#ifndef __MENU_BACKEND_H__
#define __MENU_BACKEND_H__

/* Set the accent foreground color for a screen.
 *
 * \param screen Which screen it should be set for
 * \param fg Foreground color to use for accents.
 */
void accent_color(void* screen, int fg);

/* Display a menu structure.
 *
 * \param options Menu structure to display
 * \param toggles Array of bytes to have values changed according to options
 */
int show_menu(struct options_s *options, uint8_t *toggles);

#endif
