#ifndef __FIRM_H
#define __FIRM_H

#include <stdint.h>

/* Console type for FIRM image
 */
enum consoles
{
    console_o3ds, ///< O3DS
    console_n3ds  ///< N3DS
};

enum firm_type
{
    type_native, ///< NATIVE FIRM
    type_twl,    ///< TWL FIRM
    type_agb     ///< AGB FIRM
};

/* Storage struct for version information of FIRMs
 */
struct firm_signature
{
    unsigned int k9l;    ///< CDN/contents version of FIRM
    enum consoles console;   ///< Console type
    enum firm_type type;     ///< Type of FIRM.
};

/* Returns a struct describing the version of a decrypted FIRM
 */
struct firm_signature *get_firm_info(firm_h *firm);

/* Boots native FIRM - do not call directly.
 */
void boot_firm();

/* Boots the CFW, generating caches and applying patches as-needed
 */
void boot_cfw();

/* Loads a firmware off disk, returning it. The memory should be free()'d when done, unless you plan to boot.
 */
firm_h* load_firm(const char *path);

#endif
