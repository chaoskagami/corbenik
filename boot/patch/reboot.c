/* This code was all nicked from Luma (before the GPL headers were corrected by TuxSH)
   Someone please remind me to fix this code.*/

#include <common.h>
#include <ctr9/io.h>

uint8_t*
getProcess9(uint8_t *pos, uint32_t size, uint32_t *process9Size, uint32_t *process9MemAddr)
{
    uint8_t *off = memfind(pos, size, "ess9", 4);
    *process9Size = *(uint32_t *)(off - 0x60) * 0x200;
    *process9MemAddr = *(uint32_t *)(off + 0xC);
    // Process9 code offset (start of NCCH + ExeFS offset + ExeFS header size)
    return off - 0x204 + (*(uint32_t *)(off - 0x64) * 0x200) + 0x200;
}

int
patch_reboot(firm_h* firm_loc)
{
    // Look for firmlaunch code
    const uint8_t pattern[] = { 0xDE, 0x1F, 0x8D, 0xE2 };

    uint32_t process9Size, process9MemAddr;
    uint8_t *process9Offset =
        getProcess9((uint8_t *)firm_loc + firm_loc->section[2].offset + 0x15000, firm_loc->section[2].size - 0x15000, &process9Size, &process9MemAddr);

    fprintf(stderr, "reboot: proc9 mem @ %lx\n", (uint32_t)process9MemAddr);

    wait();

    uint8_t *off = memfind(process9Offset, process9Size, pattern, 4) - 0x10;

    fprintf(stderr, "reboot: firmlaunch @ %lx\n", (uint32_t)off);

    // Firmlaunch function offset - offset in BLX opcode (A4-16 - ARM DDI 0100E) + 1
    uint32_t fOpenOffset = (uint32_t)(off + 9 - (-((*(uint32_t *)off & 0x00FFFFFF) << 2) & (0xFFFFFF << 2)) - (uint32_t)process9Offset + process9MemAddr);

    fprintf(stderr, "reboot: cropen @ %lx\n", fOpenOffset);

    wait();

    // Copy firmlaunch code
    FILE *f = cropen(PATH_REBOOT_HOOK, "r");
    if (!f)
        panic("reboot: hook not found on SD\n");

    uint32_t size = crsize(f);
    crread(off, 1, size, f);
    crclose(f);

    // Put the fOpen offset in the right location
    uint32_t *pos_cropen = (uint32_t *)memfind(off, size, "open", 4);
    if (!pos_cropen)
        return 1;

    *pos_cropen = fOpenOffset;

    uint32_t *pos_native = (uint32_t *)memfind(off, size, "NATF", 4);
    uint32_t *pos_twl = (uint32_t *)memfind(off, size, "TWLF", 4);
    uint32_t *pos_agb = (uint32_t *)memfind(off, size, "AGBF", 4);

    if (!pos_native && !pos_twl && !pos_agb)
        return 1;

    fprintf(stderr, "reboot: NATF @ %lx\n", (uint32_t)pos_native);
    fprintf(stderr, "reboot: TWLF @ %lx\n", (uint32_t)pos_twl);
    fprintf(stderr, "reboot: AGBF @ %lx\n", (uint32_t)pos_agb);

    uint8_t *mem = (uint8_t *)0x01FF8000; // 0x8000 space that will be resident. This is AXI WRAM. We have about 0x3700 bytes here.
    // According to 3dbrew, this space's props from userland:
    //   [L2L] VA fff00000..fff20000 -> PA 1ff80000..1ffa0000 [  X ] [ Priv: R-, User: -- ]
    // It can be executed by the system but not written (and can only be executed with privs)
    // This seems to be the perfect place to stick some code to be resident in memory. Beyond this,
    // it might be needed to replace a svc call to access it. I'm rather sure that NTR does this.

    *pos_native = (uint32_t)mem;
    memcpy(mem, u"sdmc:", 10);
    mem += 10;
    for (size_t i = 0; i < sizeof(PATH_NATIVE_P); i++, mem += 2) {
        *mem = PATH_NATIVE_P[i];
        *(mem + 1) = 0;
    }

    *pos_twl = (uint32_t)mem;
    memcpy(mem, u"sdmc:", 10);
    mem += 10;
    for (size_t i = 0; i < sizeof(PATH_TWL_P); i++, mem += 2) {
        *mem = PATH_TWL_P[i];
        *(mem + 1) = 0;
    }

    *pos_agb = (uint32_t)mem;
    memcpy(mem, u"sdmc:", 10);
    mem += 10;
    for (size_t i = 0; i < sizeof(PATH_AGB_P); i++, mem += 2) {
        *mem = PATH_AGB_P[i];
        *(mem + 1) = 0;
    }

    uint32_t *pos_rebc = (uint32_t *)memfind(off, size, "rebc", 4);
    *pos_rebc = (uint32_t)mem;

    fprintf(stderr, "reboot: rebc @ %lx\n", (uint32_t)pos_rebc);

    f = cropen(PATH_REBOOT_CODE, "r");
    if (!f)
        return 1;

    crread(mem, 1, crsize(f), f);
    crclose(f);

    return 0;
}
