#ifndef __MENU_BACKEND_H__
#define __MENU_BACKEND_H__

/* Menu entry type. Determines how the menu is displayed and which (if any) options
 * can be changed.
 */
enum type
{
    boolean_val = 0,      ///< Toggleable boolean
    ranged_val = 1,       ///< N1 - N2, left and right to pick.
    mask_val = 2,         ///< Bitmask allowed values.
    not_option = 3,       ///< Skip over this.
    call_fun = 4,         ///< Call a function. Treat (a) as (void)(*)(void).
    boolean_val_n3ds = 5, ///< Toggle, but only show on n3DS
    break_menu = 6        ///< Exit the menu (same as B)
};

typedef void (*func_call_t)(uint32_t data);

struct range_str
{
    int a, b;
};

struct options_s
{
    int64_t index;     ///< Option index. Used for displaying values.
    char name[64];     ///< Name of patch to display in menus.
    char desc[256];    ///< Description of option, shown when pressing select
    enum type allowed; ///< Misnomer, FIXME. Type of menu entry. See enum type.
    uint32_t a, b;     ///< Should be union, FIXME. Data values used for menu entry.
    uint8_t indent;    ///< Indentation/ownership level of menu.
} __attribute__((packed));

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
