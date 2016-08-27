#ifndef __MENU_BACKEND_H__
#define __MENU_BACKEND_H__

/* Menu entry type. Determines how the menu is displayed and which (if any) options
 * can be changed.
 */
enum type
{
    option       = 0,      ///< Option
    option_n3ds  = 1,      ///< Option (N3DS only)
    unselectable = 2,      ///< Skip over this.
    break_menu   = 3       ///< Exit the menu (same as B)
};

typedef void  (*func_call_t)(void* data);
typedef char* (*get_value_t)(void* data);

struct options_s
{
    char *name;        ///< Name of menu option
    char *desc;        ///< Description of option, shown when pressing select
    enum type handle;  ///< Type of menu entry. See enum type.
    void *param;       ///< Parameter to pass to func_call_t
    func_call_t func;  ///< Function to call on selection of option
    get_value_t value; ///< Function to get the value of the menu entry
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
 */
int show_menu(struct options_s *options);

#endif
