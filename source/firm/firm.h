#ifndef __FIRM_H

#include <stdint.h>
#include "headers.h"

extern firm_h *firm_loc;
extern struct firm_signature *current_firm;

extern firm_h *twl_firm_loc;
extern struct firm_signature *current_twl_firm;

extern firm_h *agb_firm_loc;
extern struct firm_signature *current_agb_firm;

struct firm_signature *get_firm_info(firm_h *firm);
void slot0x11key96_init();

int  load_firms();
void boot_firm();
void boot_cfw();

#endif
