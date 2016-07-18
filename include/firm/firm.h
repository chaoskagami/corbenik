#ifndef __FIRM_H

#include <stdint.h>

enum consoles
{
    console_o3ds,
    console_n3ds
};

struct firm_signature
{
    uint8_t sig[0x10];
    unsigned int version;
    char version_string[16];
    enum consoles console;
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

struct firm_signature *get_firm_info(firm_h *firm);
void slot0x11key96_init();

void extract_firm1();
void extract_slot0x05keyY();
void extract_slot0x3DkeyY();

int load_firms();
void boot_firm();
void boot_cfw();

#endif
