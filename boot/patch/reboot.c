/* This code was all nicked from Luma (before the GPL headers were corrected by TuxSH)
   Someone please remind me to fix this code.*/

#include <stdint.h>        // for uint32_t, uint8_t
#include <stddef.h>         // for fprintf, size_t, FILE
#include <string.h>        // for memcpy
#include "firm/headers.h"  // for firm_section_h, firm_h
#include "input.h"         // for wait
#include "std/abort.h"     // for panic
#include "std/draw.h"      // for stderr
#include "std/fs.h"        // for crclose, cropen, crread, crsize
#include "std/memory.h"    // for memfind
#include "structures.h"    // for PATH_AGB_P, PATH_NATIVE_P, PATH_TWL_P, PAT...

uint8_t*
getProcess9(uint8_t *pos, uint32_t size, uint32_t *process9Size, uint32_t *process9MemAddr)
{
    uint8_t *magic = memfind(pos, size, "NCCH", 4);
    cxi_h* cxi = (cxi_h*)(magic - 0x100);

    *process9Size = (cxi->ncch.exeFSSize - 1) * 0x200;
    *process9MemAddr = cxi->exheader.sci.textCodeSet.address;

    // Process9 code offset (start of NCCH + ExeFS offset + ExeFS header size)
    return (uint8_t*)cxi + (cxi->ncch.exeFSOffset + 1) * 0x200;
}

int
patch_reboot(firm_h* firm_loc)
{
    // Look for firmlaunch code
    const uint8_t pattern[] = { 0xE2, 0x20, 0x20, 0x90 };

    uint32_t process9Size, process9MemAddr;
    uint8_t *process9Offset =
        getProcess9((uint8_t *)firm_loc + firm_loc->section[2].offset, firm_loc->section[2].size, &process9Size, &process9MemAddr);

    fprintf(stderr, "reboot: proc9 mem @ %lx\n", (uint32_t)process9MemAddr);
    fprintf(stderr, "reboot: proc9 off @ %lx\n", (uint32_t)process9Offset);

    wait();

    uint8_t *off = memfind(process9Offset, process9Size, pattern, 4);

    if (!off)
        return 1;

    off -= 0x13;

    fprintf(stderr, "reboot: firmlaunch @ %lx\n", (uint32_t)off);

    // Firmlaunch function offset - offset in BLX opcode (A4-16 - ARM DDI 0100E) + 1
    uint32_t fOpenOffset = (uint32_t)(off + 9 - (-((*(uint32_t *)off & 0x00FFFFFF) << 2) & (0xFFFFFF << 2)) - (uint32_t)process9Offset + process9MemAddr);

    fprintf(stderr, "reboot: fopen @ %lx\n", fOpenOffset);

    wait();

    // Copy firmlaunch code
    FILE *f = cropen(PATH_REBOOT_HOOK, "r");
    if (!f)
        panic("reboot: hook not found on SD\n");

    uint32_t size = crsize(f);
    crread(off, 1, size, f);
    crclose(f);

    // Put the fOpen offset in the right location
    uint32_t *pos_fopen = (uint32_t *)memfind(off, size, "OPEN", 4);
    if (!pos_fopen)
        return 1;

    *pos_fopen = fOpenOffset;

    // The path is positioned at the end of our assembly to avoid reservation of space.
    // We append it here.
    uint8_t path[] = "sdmc:" PATH_BITS "/corbenik.bin";

    uint16_t *pos = (uint16_t*)(off + size);

    for(int i=0; path[i] != 0; i++) {
        pos[0] = path[i];
        pos++;
    }
    pos[0] = 0;

    return 0;
}
