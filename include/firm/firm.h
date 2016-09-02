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

extern firm_h *firm_loc;
extern uint32_t firm_size;
extern struct firm_signature *current_firm;
extern firm_section_h firm_proc9;
extern exefs_h *firm_p9_exefs;

extern firm_h *twl_firm_loc;
extern uint32_t twl_firm_size;
extern struct firm_signature *current_twl_firm;
extern firm_section_h twl_firm_proc9;
extern exefs_h *twl_firm_p9_exefs;

extern firm_h *agb_firm_loc;
extern uint32_t agb_firm_size;
extern struct firm_signature *current_agb_firm;
extern firm_section_h agb_firm_proc9;
extern exefs_h *agb_firm_p9_exefs;

/* Returns a struct describing the version of a decrypted FIRM
 */
struct firm_signature *get_firm_info(firm_h *firm);

/* Initializes 0x11 KeyY
 */
void slot0x11key96_init();

/* Extracts 0x05 KeyY from FIRM0 on NAND
 */
void extract_slot0x05keyY();

/* Extracts 0x3D KeyY from FIRM0 on NAND
 */
void extract_slot0x3DkeyY();

/* Loads FIRM files
 */
int load_firms();

/* Boots native FIRM - do not call directly.
 */
void boot_firm();

/* Boots the CFW, generating caches and applying patches as-needed
 */
void boot_cfw();

/* Loads a firmware off disk, returning it. The memory should be free()'d when done, unless you plan to boot.
 */
firm_h* load_firm(const char *path, const char *path_firmkey, const char *path_cetk, uint32_t *size);

#endif
