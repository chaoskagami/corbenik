#include <common.h>

#define FIRM_INTERNAL_CODE
#include <firm/internal.h>

struct firm_signature *
get_firm_info(firm_h *firm)
{
    // What follows is a heuristic to detect the firmware's properties. Checks are as follows:

    struct firm_signature *signature = (struct firm_signature*)memalign(16, sizeof(struct firm_signature));

    signature->type    = type_native;
    signature->k9l     = 0;
    signature->console = console_o3ds;

    // Test: Is section #4 a stub
    //   True: If true, must be NFIRM.
    //   False: Continue checking. No conclusions drawn.
    //   Why: All current existing versions on NFIRM and ONLY NFIRM have a
    //        non-existent section #4. TWL and AGB both do, so this automatically rules them out.

    if (firm->section[3].size != 0) {
        // Test: Check for a specific sysModule name
        //   True: If true, must be the corresponding FIRM
        //   False: No conclusions unless both fail, which means this is NFIRM (see caveat)
        //   Why: TWL and AGB both contain a system module that handles various tasks.
        //        Neither of these can be misdetected and they only occur in the correct firmtype.
        //   Caveat: NFIRM. If we hit this here, unfortunate fact; either TWL and AGB are no longer
        //           sane or NFIRM has a 4th segment; this would need investigation immediately.

        // Check for the presence of a TWL/AGB only sysmodule
        if( memfind((uint8_t*)firm + firm->section[0].offset, firm->section[0].size, "TwlBg", 5)) {
            signature->type = type_twl;
        } else if( memfind((uint8_t*)firm + firm->section[0].offset, firm->section[0].size, "AgbBg", 5)) {
            signature->type = type_agb;
        }
    }

    for (firm_section_h *section = firm->section; section < firm->section + 4; section++) {
        if (section->type == FIRM_TYPE_ARM9) {
            if (signature->type == type_native) {
                // Test: Can we find the string K9L in the arm9bin?
                //   True: This is a N3DS FIRM
                //   False: This is an O3DS FIRM
                //   Why: New3ds arm9bins all use this fancy thing called K9L or Kernel9Loader.
                //        You probably know this better by the same Arm9Loader (which isn't correct)
                //        Only N3DS FIRMs have this property.
                uint8_t* k9l = (uint8_t*)memfind((uint8_t*)firm + section->offset, section->size, "K9L", 3);
                if (k9l == NULL) { // O3DS.
                    signature->console = console_o3ds;
                } else { // N3DS.
                    signature->console = console_n3ds;
                    signature->k9l = (unsigned int)(k9l[3] - '0'); // String is "K9LN" where N is the version
                }
            } else {
                // Test: Address of segment
                //   0x8006800: O3DS
                //   0x8006000: N3DS
                //   Why: O3DS and N3DS have slightly different load addresses on AGB and TWL
                //        which allows determining which console it is intended for.
                if (section->address == 0x08006800) { // O3DS entry
                    signature->console = console_o3ds;
                } else if (section->address == 0x08006000) { // N3DS entry
                    signature->console = console_n3ds;
                }
            }

            break;
        }
    }

    return signature;
}
