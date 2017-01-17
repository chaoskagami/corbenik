#include <3ds.h>
#include <string.h>
#include "patcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memory.h"
#include "logger.h"
#include <structures.h>
#include "interp.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include <option.h>

#define MERGE_PATH PATH_EXEFS      "/0000000000000000"
#define TEXT_PATH  PATH_EXEFS_TEXT "/0000000000000000"
#define DATA_PATH  PATH_EXEFS_DATA "/0000000000000000"
#define RO_PATH    PATH_EXEFS_RO   "/0000000000000000"
#define LANG_PATH  PATH_LOCEMU     "/0000000000000000"

const char hexDigits[] = "0123456789ABCDEF";

int
fileOpen(Handle *file, FS_ArchiveID id, const char *path, int flags)
{
    FS_Path apath;
    FS_Path ppath;

    apath.type = PATH_EMPTY;
    apath.size = 1;
    apath.data = (uint8_t *)"";

    ppath.type = PATH_ASCII;
    ppath.data = path;
    ppath.size = strnlen(path, PATH_MAX) + 1;

    return FSUSER_OpenFileDirectly(file, id, apath, ppath, flags, 0);
}

struct config_file config;
static int failed_load_config = 1;

void
load_config()
{
    static Handle file;
    static uint32_t total;
    static uint32_t cid[4];
    static char config_file_path[] = SYSCONFDIR "/config-00000000";

    if (!R_SUCCEEDED(FSUSER_GetNandCid((uint8_t*)cid, 0x10))) {
        return;
    }

    char* cfg = config_file_path;
    while(cfg[1] != 0) cfg++;

    // Get path of actual config.
    while (cid[0]) {
        cfg[0] = hexDigits[(uint32_t)(cid[0] & 0xF)];
        cfg--;
        cid[0] >>= 4;
    }

    // Open file.
    if (!R_SUCCEEDED(fileOpen(&file, ARCHIVE_SDMC, config_file_path, FS_OPEN_READ))) {
        // Failed to open.
        return;
    }

    // Read file.
    if (!R_SUCCEEDED(FSFILE_Read(file, &total, 0, &config, sizeof(struct config_file)))) {
        FSFILE_Close(file); // Read to memory.

        // Failed to read.
        return;
    }

    FSFILE_Close(file); // Read to memory.

    if (memcmp(config.magic, "OVAN", 4)) {
        // Incorrect magic.
        // Failed to read.
        return;
    }

    if (config.config_ver != config_version) {
        // Invalid version.
        return;
    }

    failed_load_config = 0;

    logstr("  loaded config file\n");

    return;
}

void
hexdump_titleid(uint64_t progId, char *buf)
{
    uint32_t i = strlen(buf) - 1;
    uint32_t j = 16;
    while (j--) {
        buf[i--] = hexDigits[(uint32_t)(progId & 0xF)];
        progId >>= 4;
    }
}

static int
loadTitleLocaleConfig(uint64_t progId, uint8_t *regionId, uint8_t *languageId)
{
    // FIXME - Rewrite this function to use a single line-based config of
    // the grammar:

    // lang := "ja" | "en" | "fr" | "de" | "it" | "es" | "zh" | "ko" | "nl" | "pt" | "ru" | "tw"
    // region := "JP" | "US" | "EU" | "AU" | "CN" | "KO" | "TW"
    // title := (0123456789abcdef)16
    // langcode := lang . "_" . country
    // line := title langcode

    // So this would be valid as an example file:
    // 0040000000012300 en_US
    // 0040000000032100 ja_JP

    // Directory seeks have severe issues on FAT and
    // dumping configs based on 3dsdb (more than 1000) causes things
    // to kinda choke. FAT is not meant for large numbers of files per
    // directory due to linear seeks rather than tree or hash-based indexes.

    // This really does need a rewrite.

    static char path[] = LANG_PATH;
    hexdump_titleid(progId, path);

    static const char *regions[] = { "JPN", "USA", "EUR", "AUS", "CHN", "KOR", "TWN" };
    static const char *languages[] = { "JA", "EN", "FR", "DE", "IT", "ES", "ZH", "KO", "NL", "PT", "RU", "TW" };
    Handle file;
    Result ret = fileOpen(&file, ARCHIVE_SDMC, path, FS_OPEN_READ);
    if (R_SUCCEEDED(ret)) {
        char buf[6];
        uint32_t total;
        ret = FSFILE_Read(file, &total, 0, buf, 6);
        FSFILE_Close(file);

        if (!R_SUCCEEDED(ret) || total < 6)
            return -1;

        for (uint32_t i = 0; i < 7; i++) {
            if (memcmp(buf, regions[i], 3) == 0) {
                *regionId = (uint8_t)i;
                logstr("  localeemu region - ");
                logstr(regions[i]);
                logstr("\n");
                break;
            }
        }

        for (uint32_t i = 0; i < 12; i++) {
            if (memcmp(buf + 4, languages[i], 2) == 0) {
                *languageId = (uint8_t)i;
                logstr("  localeemu lang - ");
                logstr(languages[i]);
                logstr("\n");
                break;
            }
        }

        logstr("  localeemu read for ");
        logstr(path);
        logstr("\n");
    }
    return ret;
}

static uint8_t *
getCfgOffsets(uint8_t *code, uint32_t size, uint32_t *CFGUHandleOffset)
{
    // HANS:
    // Look for error code which is known to be stored near cfg:u handle
    // this way we can find the right candidate
    // (handle should also be stored right after end of candidate function)

    uint32_t n = 0, possible[24];

    for (uint8_t *pos = code + 4; n < 24 && pos < code + size - 4; pos += 4) {
        if (*(uint32_t *)pos == 0xD8A103F9) {
            for (uint32_t *l = (uint32_t *)pos - 4; n < 24 && l < (uint32_t *)pos + 4; l++)
                if (*l <= 0x10000000)
                    possible[n++] = *l;
        }
    }

    if(!n)
        return NULL;

    for (uint8_t *CFGU_GetConfigInfoBlk2_endPos = code; CFGU_GetConfigInfoBlk2_endPos < code + size - 8; CFGU_GetConfigInfoBlk2_endPos += 4) {
        static const uint32_t CFGU_GetConfigInfoBlk2_endPattern[] = { 0xE8BD8010, 0x00010082 };

        // There might be multiple implementations of GetConfigInfoBlk2 but
        // let's search for the one we want
        uint32_t *cmp = (uint32_t *)CFGU_GetConfigInfoBlk2_endPos;

        if (cmp[0] == CFGU_GetConfigInfoBlk2_endPattern[0] && cmp[1] == CFGU_GetConfigInfoBlk2_endPattern[1]) {
            *CFGUHandleOffset = *((uint32_t *)CFGU_GetConfigInfoBlk2_endPos + 2);

            for (uint32_t i = 0; i < n; i++)
                if (possible[i] == cmp[2])
                    return CFGU_GetConfigInfoBlk2_endPos;

            CFGU_GetConfigInfoBlk2_endPos += 4;
        }
    }

    return NULL;
}

static void
patchCfgGetLanguage(uint8_t *code, uint32_t size, uint8_t languageId, uint8_t *CFGU_GetConfigInfoBlk2_endPos)
{
    uint8_t *CFGU_GetConfigInfoBlk2_startPos; // Let's find STMFD SP (there might be
                                         // a NOP before, but nevermind)

    for (CFGU_GetConfigInfoBlk2_startPos = CFGU_GetConfigInfoBlk2_endPos - 4;
         CFGU_GetConfigInfoBlk2_startPos >= code && *((uint16_t *)CFGU_GetConfigInfoBlk2_startPos + 1) != 0xE92D; CFGU_GetConfigInfoBlk2_startPos -= 2)
        ;

    for (uint8_t *languageBlkIdPos = code; languageBlkIdPos < code + size; languageBlkIdPos += 4) {
        if (*(uint32_t *)languageBlkIdPos == 0xA0002) {
            for (uint8_t *instr = languageBlkIdPos - 8; instr >= languageBlkIdPos - 0x1008 && instr >= code + 4; instr -= 4) // Should be enough
            {
                if (instr[3] == 0xEB) // We're looking for BL
                {
                    uint8_t *calledFunction = instr;
                    uint32_t i = 0, found;

                    do {
                        uint32_t low24 = (*(uint32_t *)calledFunction & 0x00FFFFFF) << 2;
                        uint32_t signMask = (uint32_t)(-(low24 >> 25)) & 0xFC000000; // Sign extension
                        s32 offset = (s32)(low24 | signMask) + 8;          // Branch offset + 8 for prefetch

                        calledFunction += offset;

                        if(calledFunction >= CFGU_GetConfigInfoBlk2_startPos - 4 && calledFunction <= CFGU_GetConfigInfoBlk2_endPos) {
                            *((uint32_t *)instr - 1) = 0xE3A00000 | languageId; // mov    r0, sp => mov r0, =languageId
                            *(uint32_t *)instr = 0xE5CD0000; // bl CFGU_GetConfigInfoBlk2 => strb r0, [sp]
                            *((uint32_t *)instr + 1) = 0xE3B00000; // (1 or 2 instructions)         => movs r0, 0             (result code)

                            logstr("  patched cfggetlanguage\n");

                            // We're done
                            return;
                        }
                        i++;
                    } while (i < 2 && !found && calledFunction[3] == 0xEA);
                }
            }
        }
    }
}

static void
patchCfgGetRegion(uint8_t *code, uint32_t size, uint8_t regionId, uint32_t CFGUHandleOffset)
{
    for (uint8_t *cmdPos = code; cmdPos < code + size - 28; cmdPos += 4) {
        static const uint32_t cfgSecureInfoGetRegionCmdPattern[] = { 0xEE1D4F70, 0xE3A00802 };

        uint32_t *cmp = (uint32_t *)cmdPos;

        if (cmp[0] == cfgSecureInfoGetRegionCmdPattern[0] && cmp[1] == cfgSecureInfoGetRegionCmdPattern[1] &&
            *((uint16_t *)cmdPos + 7) == 0xE59F && *(uint32_t *)(cmdPos + 20 + *((uint16_t *)cmdPos + 6)) == CFGUHandleOffset) {

            *((uint32_t *)cmdPos + 4) = 0xE3A00000 | regionId; // mov    r0, =regionId
            *((uint32_t *)cmdPos + 5) = 0xE5C40008;            // strb   r0, [r4, 8]
            *((uint32_t *)cmdPos + 6) = 0xE3A00000;            // mov    r0, 0            (result code)
            *((uint32_t *)cmdPos + 7) = 0xE5840004;            // str    r0, [r4, 4]

            // The remaining, not patched, function code will do the rest for us
            break;
        }
    }

    logstr("  patched cfggetregion\n");
}

static void
adjust_cpu_settings(__attribute__((unused)) uint64_t progId, EXHEADER_prog_addrs *shared)
{
    uint8_t* code = (uint8_t*)shared->text_addr;
    uint32_t size = shared->text_size << 12;

    if (!failed_load_config) {
        uint32_t cpuSetting = 0;
        // L2
        cpuSetting |= config.options[OPTION_LOADER_CPU_L2];
        // Speed
        cpuSetting |= config.options[OPTION_LOADER_CPU_800MHZ] << 1;

        if (cpuSetting) {
            static const uint8_t cfgN3dsCpuPattern[] = { 0x0C, 0x00, 0x94, 0x15 };

            uint32_t *cfgN3dsCpuLoc = (uint32_t *)memfind(code, size, cfgN3dsCpuPattern, sizeof(cfgN3dsCpuPattern));

            // Patch N3DS CPU Clock and L2 cache setting
            if (cfgN3dsCpuLoc != NULL) {
                *(cfgN3dsCpuLoc - 4) = *(cfgN3dsCpuLoc - 3);
                *(cfgN3dsCpuLoc - 3) = *(cfgN3dsCpuLoc - 1);
                memcpy(cfgN3dsCpuLoc - 1, cfgN3dsCpuLoc, 16);
                *(cfgN3dsCpuLoc + 3) = 0xE3800000 | cpuSetting;
            }
        }
    }

    logstr("  patched cpu\n");
}

void
language_emu(uint64_t progId, EXHEADER_prog_addrs *shared)
{
    uint8_t* code = (uint8_t*)shared->text_addr;
    uint32_t size = shared->text_size << 12;

    if (!failed_load_config && config.options[OPTION_LOADER_LANGEMU]) {
        uint32_t tidHigh = (progId & 0xFFFFFFF000000000LL) >> 0x24;

        if (tidHigh == 0x0004000) { // Normal Game
            // Language emulation
            uint8_t regionId = 0xFF, languageId = 0xFF;

            if (R_SUCCEEDED(loadTitleLocaleConfig(progId, &regionId, &languageId))) {
                uint32_t CFGUHandleOffset;

                uint8_t *CFGU_GetConfigInfoBlk2_endPos = getCfgOffsets(code, size, &CFGUHandleOffset);

                if (CFGU_GetConfigInfoBlk2_endPos != NULL) {
                    if (languageId != 0xFF)
                        patchCfgGetLanguage(code, size, languageId, CFGU_GetConfigInfoBlk2_endPos);
                    if (regionId != 0xFF)
                        patchCfgGetRegion(code, size, regionId, CFGUHandleOffset);
                }
            }
        }
    }
}

void
code_handler(uint64_t progId, EXHEADER_prog_addrs *shared)
{
    // If configuration was not loaded, or both options (load / dump) are disabled
    if (failed_load_config || (!config.options[OPTION_LOADER_DUMPCODE] && !config.options[OPTION_LOADER_LOADCODE]))
        return;

    uint32_t highTid = progId >> 0x20;

    if (!(highTid == 0x00040000 || highTid == 0x00040002) && !config.options[OPTION_LOADER_DUMPCODE_ALL])
        return;

    if (config.options[OPTION_LOADER_DUMPCODE_MERGED]) {
        static char merge_path[] = MERGE_PATH;
        Handle code_f;

        hexdump_titleid(progId, merge_path);

        uint32_t len;

        uint32_t size = shared->total_size << 12;

        // Attempts to load code section from SD card, including system titles/modules/etc.
        if (R_SUCCEEDED(fileOpen(&code_f, ARCHIVE_SDMC, merge_path, FS_OPEN_READ)) && config.options[OPTION_LOADER_LOADCODE]) {
            FSFILE_Read(code_f, &len, 0, (void*)shared->text_addr, size);
            logstr("  loaded codebin from ");
            logstr(merge_path);
            logstr("\n");
        }
        // Either system title with OPTION_LOADER_DUMPCODE_ALL enabled, or regular title
        else if (config.options[OPTION_LOADER_DUMPCODE]) {
            if (R_SUCCEEDED(fileOpen(&code_f, ARCHIVE_SDMC, merge_path, FS_OPEN_WRITE | FS_OPEN_CREATE))) {
                FSFILE_Write(code_f, &len, 0, (void*)shared->text_addr, size, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME);
                logstr("  dumped codebin to ");
                logstr(merge_path);
                logstr("\n");
            }
        }
        FSFILE_Close(code_f);
    } else {
        static char text_path[] = TEXT_PATH;
        static char data_path[] = DATA_PATH;
        static char ro_path[]   = RO_PATH;
        Handle code_f;

        hexdump_titleid(progId, text_path);
        hexdump_titleid(progId, data_path);
        hexdump_titleid(progId, ro_path);

        uint32_t len;

        // Text section.

        // Attempts to load code section from SD card, including system titles/modules/etc.
        if (R_SUCCEEDED(fileOpen(&code_f, ARCHIVE_SDMC, text_path, FS_OPEN_READ)) && config.options[OPTION_LOADER_LOADCODE]) {
            FSFILE_Read(code_f, &len, 0, (void*)shared->text_addr, shared->text_size << 12);
            logstr("  loaded text from ");
            logstr(text_path);
            logstr("\n");
        }
        // Either system title with OPTION_LOADER_DUMPCODE_ALL enabled, or regular title
        else if (config.options[OPTION_LOADER_DUMPCODE]) {
            if (R_SUCCEEDED(fileOpen(&code_f, ARCHIVE_SDMC, text_path, FS_OPEN_WRITE | FS_OPEN_CREATE))) {
                FSFILE_Write(code_f, &len, 0, (void*)shared->text_addr, shared->text_size << 12, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME);
                logstr("  dumped text to ");
                logstr(text_path);
                logstr("\n");
            }
        }
        FSFILE_Close(code_f);

        // Data section.

        // Attempts to load code section from SD card, including system titles/modules/etc.
        if (R_SUCCEEDED(fileOpen(&code_f, ARCHIVE_SDMC, data_path, FS_OPEN_READ)) && config.options[OPTION_LOADER_LOADCODE]) {
            FSFILE_Read(code_f, &len, 0, (void*)shared->data_addr, shared->data_size << 12);
            logstr("  loaded data from ");
            logstr(data_path);
            logstr("\n");
        }
        // Either system title with OPTION_LOADER_DUMPCODE_ALL enabled, or regular title
        else if (config.options[OPTION_LOADER_DUMPCODE]) {
            if (R_SUCCEEDED(fileOpen(&code_f, ARCHIVE_SDMC, data_path, FS_OPEN_WRITE | FS_OPEN_CREATE))) {
                FSFILE_Write(code_f, &len, 0, (void*)shared->data_addr, shared->data_size << 12, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME);
                logstr("  dumped data to ");
                logstr(data_path);
                logstr("\n");
            }
        }
        FSFILE_Close(code_f);

        // RO Section.

        // Attempts to load code section from SD card, including system titles/modules/etc.
        if (R_SUCCEEDED(fileOpen(&code_f, ARCHIVE_SDMC, ro_path, FS_OPEN_READ)) && config.options[OPTION_LOADER_LOADCODE]) {
            FSFILE_Read(code_f, &len, 0, (void*)shared->ro_addr, shared->ro_size << 12);
            logstr("  loaded ro from ");
            logstr(ro_path);
            logstr("\n");
        }
        // Either system title with OPTION_LOADER_DUMPCODE_ALL enabled, or regular title
        else if (config.options[OPTION_LOADER_DUMPCODE]) {
            if (R_SUCCEEDED(fileOpen(&code_f, ARCHIVE_SDMC, ro_path, FS_OPEN_WRITE | FS_OPEN_CREATE))) {
                FSFILE_Write(code_f, &len, 0, (void*)shared->ro_addr, shared->ro_size << 12, FS_WRITE_FLUSH | FS_WRITE_UPDATE_TIME);
                logstr("  dumped ro to ");
                logstr(ro_path);
                logstr("\n");
            }
        }
        FSFILE_Close(code_f);
    }
}

// This is only for the .code segment.
void
patch_exe(uint64_t progId, uint16_t progver, EXHEADER_prog_addrs* shared, __attribute__((unused)) EXHEADER_prog_addrs* original)
{
    if (progId == 0x0004013000008002LL)
        adjust_cpu_settings(progId, shared);

    execb(progId, progver, shared);

    language_emu(progId, shared);
}

// Gets how many bytes .text must be extended by for patches to fit.
uint32_t
get_text_extend(__attribute__((unused)) uint64_t progId, __attribute__((unused)) uint16_t progver, __attribute__((unused)) uint32_t size_orig)
{
    return 0; // Stub - nothing needs this yet
}

// Gets how many bytes .ro must be extended.
uint32_t
get_ro_extend(__attribute__((unused)) uint64_t progId, __attribute__((unused)) uint16_t progver, __attribute__((unused)) uint32_t size_orig)
{
    return 0; // Stub - nothing needs this yet
}

// Again, same, but for .data.
uint32_t
get_data_extend(__attribute__((unused)) uint64_t progId, __attribute__((unused)) uint16_t progver, __attribute__((unused)) uint32_t size_orig)
{
    return 0; // Stub - nothing needs this yet
}

// Get CPU speed for progId.
uint8_t
get_cpumode(__attribute__((unused)) uint64_t progId)
{
    return 0xff; // Skip.
}
