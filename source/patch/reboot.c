#include "emunand.h"
#include "../std/memory.h"
#include "../std/draw.h"
#include "../std/fs.h"
#include "../std/abort.h"
#include "../firm/firm.h"
#include "../firm/fcram.h"
#include "../fatfs/sdmmc.h"
#include "../firm/headers.h"
#include "../patch_format.h"

int wait();

uint8_t *
getProcess9(uint8_t *pos, uint32_t size, uint32_t *process9Size, uint32_t *process9MemAddr)
{
    uint8_t *off = memfind(pos, size, "ess9", 4);
    *process9Size = *(uint32_t *)(off - 0x60) * 0x200;
    *process9MemAddr = *(uint32_t *)(off + 0xC);
    // Process9 code offset (start of NCCH + ExeFS offset + ExeFS header size)
    return off - 0x204 + (*(uint32_t *)(off - 0x64) * 0x200) + 0x200;
}

void
patch_reboot()
{
    // Look for firmlaunch code
    const uint8_t pattern[] = { 0xDE, 0x1F, 0x8D, 0xE2 };

    uint32_t process9Size, process9MemAddr;
    uint8_t *process9Offset =
        getProcess9((uint8_t *)firm_loc + firm_loc->section[2].offset + 0x15000, firm_loc->section[2].size - 0x15000, &process9Size, &process9MemAddr);

    fprintf(stderr, "reboot: proc9 mem @ %x\n", process9MemAddr);

    wait();

    uint8_t *off = memfind(process9Offset, process9Size, pattern, 4) - 0x10;

    fprintf(stderr, "reboot: firmlaunch @ %x\n", off);

    // Firmlaunch function offset - offset in BLX opcode (A4-16 - ARM DDI 0100E) + 1
    uint32_t fOpenOffset = (uint32_t)(off + 9 - (-((*(uint32_t *)off & 0x00FFFFFF) << 2) & (0xFFFFFF << 2)) - process9Offset + process9MemAddr);

    fprintf(stderr, "reboot: fopen @ %x\n", fOpenOffset);

    wait();

    // Copy firmlaunch code
    FILE *f = fopen(PATH_BITS "/reboot_hook.bin", "r");
    if (!f)
        abort("reboot: hook not found on SD\n");

    uint32_t size = fsize(f);
    fread(off, 1, size, f);
    fclose(f);

    // Put the fOpen offset in the right location
    uint32_t *pos_fopen = (uint32_t *)memfind(off, size, "open", 4);
    if (!pos_fopen)
        abort("reboot: fopen location missing\n");

    *pos_fopen = fOpenOffset;

    uint32_t *pos_native = (uint32_t *)memfind(off, size, "NATF", 4);
    uint32_t *pos_twl = (uint32_t *)memfind(off, size, "TWLF", 4);
    uint32_t *pos_agb = (uint32_t *)memfind(off, size, "AGBF", 4);

    if (!pos_native && !pos_twl && !pos_agb)
        abort("reboot: missing string placeholder?\n");

    fprintf(stderr, "reboot: NATF @ %x\n", pos_native);
    fprintf(stderr, "reboot: TWLF @ %x\n", pos_twl);
    fprintf(stderr, "reboot: AGBF @ %x\n", pos_agb);

    uint8_t *mem = (uint8_t *)0x01FF8000; // 0x8000 space that will be resident.

    *pos_native = (uint32_t)mem;
    memcpy(mem, L"sdmc:", 10);
    mem += 10;
    for (size_t i = 0; i < sizeof(PATH_NATIVE_P); i++, mem += 2) {
        *mem = PATH_NATIVE_P[i];
        *(mem + 1) = 0;
    }

    *pos_twl = (uint32_t)mem;
    memcpy(mem, L"sdmc:", 10);
    mem += 10;
    for (size_t i = 0; i < sizeof(PATH_TWL_P); i++, mem += 2) {
        *mem = PATH_TWL_P[i];
        *(mem + 1) = 0;
    }

    *pos_agb = (uint32_t)mem;
    memcpy(mem, L"sdmc:", 10);
    mem += 10;
    for (size_t i = 0; i < sizeof(PATH_AGB_P); i++, mem += 2) {
        *mem = PATH_AGB_P[i];
        *(mem + 1) = 0;
    }

    uint32_t *pos_rebc = (uint32_t *)memfind(off, size, "rebc", 4);
    *pos_rebc = (uint32_t)mem;

    fprintf(stderr, "reboot: rebc @ %x\n", pos_rebc);

    f = fopen(PATH_BITS "/reboot_code.bin", "r");
    if (!f)
        abort("reboot: boot not found on SD\n");

    fread(mem, 1, fsize(f), f);
    fclose(f);

    write_file((void *)0x1FF8000, "/test", 0x8000);
}
