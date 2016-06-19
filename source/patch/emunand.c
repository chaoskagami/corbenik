/*
*   emunand.c
*/

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
#include "../config.h"

uint8_t *emunand_temp = (uint8_t *)FCRAM_JUNK_LOC;

void
verify_loop_emunand(char *filename)
{
    // FIXME - This won't work unless the NAND file is completely contiguous on disk, sadly.
    // Technically speaking if I were to defrag my SD this would work, I suspect.
    // For now, this will remain as dead code.

    uint32_t offset = get_file_sector(filename); // Get the sector of the file

    // Check for RedNAND image on SD
    if (!sdmmc_sdcard_readsectors(offset, 1, emunand_temp) && *(uint32_t *)(emunand_temp + 0x100) == NCSD_MAGIC) {
        fprintf(stderr, "emunand: found NCSD magic\n");
    } else {
        abort("emunand: selected NAND image is not valid.\n");
    }
}

void
verify_emunand(uint32_t index, uint32_t *off, uint32_t *head)
{
    uint32_t nandSize = getMMCDevice(0)->total_size;

    uint32_t offset;
    if (nandSize > 0x200000)
        offset = 0x400000;
    else
        offset = 0x200000;

    if (config.options[OPTION_EMUNAND_REVERSE]) {
        // Subtract offset from back of disk.
        offset = (getMMCDevice(1)->total_size - 1) - (offset * (index + 1));
    } else {
        offset = offset * index;
    }

    // Check for RedNAND/Normal physical layout on SD
    if (!sdmmc_sdcard_readsectors(offset + 1, 1, emunand_temp) && *(uint32_t *)(emunand_temp + 0x100) == NCSD_MAGIC) {
        *off = offset + 1;
        *head = offset + 1;

        fprintf(stderr, "emunand: found NCSD magic for %u\n", index);
        fprintf(stderr, "emunand: layout is normal\n");
    }
    // Check for GW EmuNAND on SD
    else if (!sdmmc_sdcard_readsectors(offset + nandSize, 1, emunand_temp) && *(uint32_t *)(emunand_temp + 0x100) == NCSD_MAGIC) {
        *off = offset;
        *head = offset + nandSize;

        fprintf(stderr, "emunand: found NCSD magic for %u\n", index);
        fprintf(stderr, "emunand: layout is gateway\n");
    } else {
        abort("emunand: selected NAND image is not valid.\n");
    }
}

static void *
getEmuCode(uint8_t *pos, uint32_t size)
{
    const uint8_t pattern[] = { 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 };

    // Looking for the last free space before Process9
    uint8_t *ret = memfind(pos + 0x13500, size - 0x13500, pattern, 6) + 0x455;

    if (ret) {
        fprintf(stderr, "emunand: free space @ %x\n", ret);
        fprintf(stderr, "emunand: size is %u bytes\n", (uint8_t *)ret - pos);
    }

    return ret;
}

static uint32_t
getSDMMC(uint8_t *pos, uint32_t size)
{
    // Look for struct code
    const uint8_t pattern[] = { 0x21, 0x20, 0x18, 0x20 };
    const uint8_t *off = memfind(pos, size, pattern, 4);

    uint32_t ret = *(uint32_t *)(off + 9) + *(uint32_t *)(off + 0xD);

    fprintf(stderr, "emunand: SDMMC code @ %x\n", ret);

    return ret;
}

static void
patchNANDRW(uint8_t *pos, uint32_t size, uint32_t branchOffset)
{
    const uint16_t nandRedir[2] = { 0x4C00, 0x47A0 };

    // Look for read/write code
    const uint8_t pattern[] = { 0x1E, 0x00, 0xC8, 0x05 };

    uint16_t *readOffset = (uint16_t *)memfind(pos, size, pattern, 4) - 3;
    uint16_t *writeOffset = (uint16_t *)memfind((uint8_t*)(readOffset + 5), 0x100, pattern, 4) - 3;

    if (!readOffset || !writeOffset)
        abort("emunand: pattern for r/w missing!\n");

    readOffset[0] = nandRedir[0];
    readOffset[1] = nandRedir[1];
    ((uint32_t *)readOffset)[1] = branchOffset;

    writeOffset[0] = nandRedir[0];
    writeOffset[1] = nandRedir[1];
    ((uint32_t *)writeOffset)[1] = branchOffset;

    fprintf(stderr, "emunand: write @ %x\n", writeOffset);
    fprintf(stderr, "emunand: read @ %x\n", readOffset);
}

static void
patchMPU(uint8_t *pos, uint32_t size)
{
    const uint32_t mpuPatch[3] = { 0x00360003, 0x00200603, 0x001C0603 };

    // Look for MPU pattern
    const uint8_t pattern[] = { 0x03, 0x00, 0x24, 0x00 };

    uint32_t *off = (uint32_t *)memfind(pos, size, pattern, 4);

    off[0] = mpuPatch[0];
    off[6] = mpuPatch[1];
    off[9] = mpuPatch[2];

    fprintf(stderr, "emunand: mpu @ %x\n", off);
}

void
patch_emunand(uint32_t index)
{
    // ARM9 section.
    uint8_t *arm9Section = (uint8_t *)firm_loc + firm_loc->section[2].offset;
    uint32_t arm9SectionSize = firm_loc->section[2].size;

    uint8_t *process9Offset = (uint8_t *)firm_p9_exefs + sizeof(exefs_h) + firm_p9_exefs->fileHeaders[0].offset;
    uint32_t process9Size = firm_p9_exefs->fileHeaders[0].size;

    // Copy emuNAND code
    void *emuCodeOffset = getEmuCode(arm9Section, arm9SectionSize);
    if (!emuCodeOffset)
        abort("emunand: code missing from arm9?\n");

    FILE *f = fopen(PATH_EMUNAND_CODE, "r");
    if (!f)
        abort("emunand: code not found on SD.\n");

    uint32_t emunand_size = fsize(f);
    fread(emuCodeOffset, 1, emunand_size, f);
    fclose(f);

    uint32_t branchOffset = (uintptr_t)emuCodeOffset - ((uintptr_t)firm_loc + firm_loc->section[2].offset - firm_loc->section[2].address);

    fprintf(stderr, "emunand: read in emunand code\n");

    // Add the data of the found emuNAND
    uint32_t *pos_offset = (uint32_t *)memfind(emuCodeOffset, emunand_size, "NAND", 4),
             *pos_head   = (uint32_t *)memfind(emuCodeOffset, emunand_size, "NCSD", 4),
             *pos_sdmmc  = (uint32_t *)memfind(emuCodeOffset, emunand_size, "SDMC", 4);

    if (!pos_offset || !pos_head || !pos_sdmmc)
        abort("emunand: couldn't find pattern in hook?\n");

    verify_emunand(index, pos_offset, pos_head);

    fprintf(stderr, "emunand: nand is on sector %u\n", *pos_offset);
    fprintf(stderr, "emunand: head is on sector %u\n", *pos_head);

    // Add emuNAND hooks
    patchNANDRW(process9Offset, process9Size, branchOffset);

    fprintf(stderr, "emunand: patched read/write calls\n");

    // Find and add the SDMMC struct

    *pos_sdmmc = getSDMMC(process9Offset, process9Size);

    // Set MPU for emu code region
    patchMPU(arm9Section, arm9SectionSize);

    fprintf(stderr, "emunand: patched MPU settings\n");
}
