#include <3ds.h>
#include "memory.h"
#include "patcher.h"
#include "ifile.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include "config.h"
#include "../../../source/patch_format.h"

#include "patch/patch.h"

static int memcmp(const void *buf1, const void *buf2, u32 size)
{
    const u8 *buf1c = (const u8 *)buf1;
    const u8 *buf2c = (const u8 *)buf2;

    for(u32 i = 0; i < size; i++)
    {
        int cmp = buf1c[i] - buf2c[i];
        if(cmp) return cmp;
    }

    return 0;
}

//Quick Search algorithm, adapted from http://igm.univ-mlv.fr/~lecroq/string/node19.html#SECTION00190
static u8 *memsearch(u8 *startPos, const void *pattern, u32 size, u32 patternSize)
{
    const u8 *patternc = (const u8 *)pattern;

    //Preprocessing
    u32 table[256];

    for(u32 i = 0; i < 256; ++i)
        table[i] = patternSize + 1;
    for(u32 i = 0; i < patternSize; ++i)
        table[patternc[i]] = patternSize - i;

    //Searching
    u32 j = 0;

    while(j <= size - patternSize)
    {
        if(memcmp(patternc, startPos + j, patternSize) == 0)
            return startPos + j;
        j += table[startPos[j + patternSize]];
    }

    return NULL;
}

u32 patchMemory(u8 *start, u32 size, const void *pattern, u32 patSize, int offset, const void *replace, u32 repSize, u32 count)
{
    u32 i;

    for(i = 0; i < count; i++)
    {
        u8 *found = memsearch(start, pattern, size, patSize);

        if(found == NULL) break;

		// FIXME - This is throwing on Werror.
        memcpy(found + offset, replace, repSize);

        u32 at = (u32)(found - start);

        if(at + patSize > size) break;

        size -= at + patSize;
        start = found + patSize;
    }

    return i;
}

static inline size_t strnlen(const char *string, size_t maxlen)
{
    size_t size;

    for(size = 0; *string && size < maxlen; string++, size++);

    return size;
}

static int fileOpen(IFile *file, FS_ArchiveID id, const char *path, int flags)
{
    FS_Archive archive;
    FS_Path ppath;

    size_t len = strnlen(path, PATH_MAX);
    archive.id = id;
    archive.lowPath.type = PATH_EMPTY;
    archive.lowPath.size = 1;
    archive.lowPath.data = (u8 *)"";
    ppath.type = PATH_ASCII;
    ppath.data = path;
    ppath.size = len + 1;

    return IFile_Open(file, archive, ppath, flags);
}

static u32 secureInfoExists(void)
{
    static u32 secureInfoExists = 0;

    if(!secureInfoExists)
    {
        IFile file;
        if(!fileOpen(&file, ARCHIVE_NAND_RW, "/sys/SecureInfo_C", FS_OPEN_READ))
        {
            secureInfoExists = 1;
            IFile_Close(&file);
        }
    }

    return secureInfoExists;
}

static struct config_file config;
static int failed_load_config = 1;

void load_config() {
	static IFile file;
    static u64 total;

	// Open file.
    if (!R_SUCCEEDED(fileOpen(&file, ARCHIVE_SDMC, PATH_CONFIG, FS_OPEN_READ))) {
		// Failed to open.
        return;
    }

	// Read file.
    if (!R_SUCCEEDED(IFile_Read(&file, &total, &config, sizeof(struct config_file)))) {
		IFile_Close(&file); // Read to memory.

		// Failed to read.
		return;
	}

	IFile_Close(&file); // Read to memory.

	if (config.magic[0] != 'O' || config.magic[1] != 'V' || config.magic[2] != 'A' || config.magic[3] != 'N') {
		// Incorrect magic.
		// Failed to read.
		return;
	}

	if (config.config_ver != config_version) {
		// Invalid version.
		return;
    }

	failed_load_config = 0;

	return;
}

static int loadTitleLocaleConfig(u64 progId, u8 *regionId, u8 *languageId)
{
	// FIXME - Rewrite this function to use a single line-based config

	// Directory seeks have severe issues on FAT and
	// dumping configs based on 3dsdb (more than 1000) causes things
	// to kinda choke. FAT is not meant for large numbers of files per
	// directory due to linear seeks rather than tree or hash-based indexes

    char path[] = "/corbenik/etc/locale.conf"; // The locale config file.

    char progIdStr[17]; // Sizeof titleid as string + null terminator
	progIdStr[sizeof(progIdStr)-1] = 0; // Set null term

	// Hexdump progId
	for(int i=0; i < 8; i++) {
        static const char hexDigits[] = "0123456789ABCDEF";
        progIdStr[i*2]   = hexDigits[(((uint8_t*)progId)[i] >> 4) & 0xf];
        progIdStr[i*2+1] = hexDigits[ ((uint8_t*)progId)[i] & 0xf];
    }

    IFile file;
    Result ret = fileOpen(&file, ARCHIVE_SDMC, path, FS_OPEN_READ);
    if(R_SUCCEEDED(ret))
    {
        char buf[6];
        u64 total;

		// TODO - Open and seek file properly

        ret = IFile_Read(&file, &total, buf, 6);
        IFile_Close(&file);

        if(!R_SUCCEEDED(ret) || total < 6) return -1;

		// TODO - Split by nl/spaces strtok style. This is not acceptable code.
		// Entries should be of the format:
		//   <tid> <region> <lang>
		// No attempt will be made to fix dumbass use of CRLF or CR-only, but newline
		// characters should be treated as spaces.

        for(u32 i = 0; i < 7; ++i)
        {
			// TODO - Permit alternative names. They're using fixed strings for ease of use;
			// we need to permit names like 'Japan', 'Europe', etc
			// Maybe read locale data from the FS? http://cldr.unicode.org/
            static const char *regions[] = {"JPN", "USA", "EUR", "AUS", "CHN", "KOR", "TWN"};

            if(memcmp(buf, regions[i], 3) == 0)
            {
                *regionId = (u8)i;
                break;
            }
        }

        for(u32 i = 0; i < 12; ++i)
        {
			// TODO - Same as above.
            static const char *languages[] = {"JP", "EN", "FR", "DE", "IT", "ES", "ZH", "KO", "NL", "PT", "RU", "TW"};

            if(memcmp(buf + 4, languages[i], 2) == 0)
            {
                *languageId = (u8)i;
                break;
            }
        }
    }

    return ret;
}

static u8 *getCfgOffsets(u8 *code, u32 size, u32 *CFGUHandleOffset)
{
    /* HANS:
       Look for error code which is known to be stored near cfg:u handle
       this way we can find the right candidate
       (handle should also be stored right after end of candidate function) */

    u32 n = 0,
        possible[24];

    for(u8 *pos = code + 4; n < 24 && pos < code + size - 4; pos += 4)
    {
        if(*(u32 *)pos == 0xD8A103F9)
        {
            for(u32 *l = (u32 *)pos - 4; n < 24 && l < (u32 *)pos + 4; l++)
                if(*l <= 0x10000000) possible[n++] = *l;
        }
    }

    for(u8 *CFGU_GetConfigInfoBlk2_endPos = code; CFGU_GetConfigInfoBlk2_endPos < code + size - 8; CFGU_GetConfigInfoBlk2_endPos += 4)
    {
        static const u32 CFGU_GetConfigInfoBlk2_endPattern[] = {0xE8BD8010, 0x00010082};

        //There might be multiple implementations of GetConfigInfoBlk2 but let's search for the one we want
        u32 *cmp = (u32 *)CFGU_GetConfigInfoBlk2_endPos;

        if(cmp[0] == CFGU_GetConfigInfoBlk2_endPattern[0] && cmp[1] == CFGU_GetConfigInfoBlk2_endPattern[1])
        {
            *CFGUHandleOffset = *((u32 *)CFGU_GetConfigInfoBlk2_endPos + 2);

            for(u32 i = 0; i < n; i++)
                if(possible[i] == *CFGUHandleOffset) return CFGU_GetConfigInfoBlk2_endPos;

            CFGU_GetConfigInfoBlk2_endPos += 4;
        }
    }

    return NULL;
}

static void patchCfgGetLanguage(u8 *code, u32 size, u8 languageId, u8 *CFGU_GetConfigInfoBlk2_endPos)
{
    u8 *CFGU_GetConfigInfoBlk2_startPos; //Let's find STMFD SP (there might be a NOP before, but nevermind)

    for(CFGU_GetConfigInfoBlk2_startPos = CFGU_GetConfigInfoBlk2_endPos - 4;
        CFGU_GetConfigInfoBlk2_startPos >= code && *((u16 *)CFGU_GetConfigInfoBlk2_startPos + 1) != 0xE92D;
        CFGU_GetConfigInfoBlk2_startPos -= 2);

    for(u8 *languageBlkIdPos = code; languageBlkIdPos < code + size; languageBlkIdPos += 4)
    {
        if(*(u32 *)languageBlkIdPos == 0xA0002)
        {
            for(u8 *instr = languageBlkIdPos - 8; instr >= languageBlkIdPos - 0x1008 && instr >= code + 4; instr -= 4) //Should be enough
            {
                if(instr[3] == 0xEB) //We're looking for BL
                {
                    u8 *calledFunction = instr;
                    u32 i = 0,
                        found;

                    do
                    {
                        u32 low24 = (*(u32 *)calledFunction & 0x00FFFFFF) << 2;
                        u32 signMask = (u32)(-(low24 >> 25)) & 0xFC000000; //Sign extension
                        s32 offset = (s32)(low24 | signMask) + 8;          //Branch offset + 8 for prefetch

                        calledFunction += offset;

                        found = calledFunction >= CFGU_GetConfigInfoBlk2_startPos - 4 && calledFunction <= CFGU_GetConfigInfoBlk2_endPos;
                        i++;
                    }
                    while(i < 2 && !found && calledFunction[3] == 0xEA);

                    if(found)
                    {
                        *((u32 *)instr - 1)  = 0xE3A00000 | languageId; // mov    r0, sp                 => mov r0, =languageId
                        *(u32 *)instr        = 0xE5CD0000;              // bl     CFGU_GetConfigInfoBlk2 => strb r0, [sp]
                        *((u32 *)instr + 1)  = 0xE3B00000;              // (1 or 2 instructions)         => movs r0, 0             (result code)

                        //We're done
                        return;
                    }
                }
            }
        }
    }
}

static void patchCfgGetRegion(u8 *code, u32 size, u8 regionId, u32 CFGUHandleOffset)
{
    for(u8 *cmdPos = code; cmdPos < code + size - 28; cmdPos += 4)
    {
        static const u32 cfgSecureInfoGetRegionCmdPattern[] = {0xEE1D4F70, 0xE3A00802, 0xE5A40080};

        u32 *cmp = (u32 *)cmdPos;

        if(cmp[0] == cfgSecureInfoGetRegionCmdPattern[0] && cmp[1] == cfgSecureInfoGetRegionCmdPattern[1] &&
           cmp[2] == cfgSecureInfoGetRegionCmdPattern[2] && *((u16 *)cmdPos + 7) == 0xE59F &&
           *(u32 *)(cmdPos + 20 + *((u16 *)cmdPos + 6)) == CFGUHandleOffset)
        {
            *((u32 *)cmdPos + 4) = 0xE3A00000 | regionId; // mov    r0, =regionId
            *((u32 *)cmdPos + 5) = 0xE5C40008;            // strb   r0, [r4, 8]
            *((u32 *)cmdPos + 6) = 0xE3B00000;            // movs   r0, 0            (result code) ('s' not needed but nvm)
            *((u32 *)cmdPos + 7) = 0xE5840004;            // str    r0, [r4, 4]

            //The remaining, not patched, function code will do the rest for us
            break;
        }
    }
}

static void adjust_cpu_settings(u64 progId, u8 *code, u32 size) {
	if (!failed_load_config) {
		u32 cpuSetting = 0;
		// L2
		cpuSetting |= config.options[OPTION_LOADER_CPU_L2];
		// Speed
		cpuSetting |= config.options[OPTION_LOADER_CPU_800MHZ] << 1;

	    if(cpuSetting) {
			static const u8 cfgN3dsCpuPattern[] = {0x00, 0x40, 0xA0, 0xE1, 0x07, 0x00};

			u32 *cfgN3dsCpuLoc = (u32 *)memsearch(code, cfgN3dsCpuPattern, size, sizeof(cfgN3dsCpuPattern));

			//Patch N3DS CPU Clock and L2 cache setting
			if(cfgN3dsCpuLoc != NULL) {
				*(cfgN3dsCpuLoc + 1) = 0xE1A00000;
				*(cfgN3dsCpuLoc + 8) = 0xE3A00000 | cpuSetting;
			}
		}
	}
}

void language_emu(u64 progId, u8 *code, u32 size) {
	if(!failed_load_config && config.options[OPTION_LOADER_LANGEMU]) {
		u32 tidHigh = (progId & 0xFFFFFFF000000000LL) >> 0x24;

		if(tidHigh == 0x0004000) { // Normal Game
			//Language emulation
			u8 regionId = 0xFF,
			languageId = 0xFF;

			if(R_SUCCEEDED(loadTitleLocaleConfig(progId, &regionId, &languageId))) {
				u32 CFGUHandleOffset;

				u8 *CFGU_GetConfigInfoBlk2_endPos = getCfgOffsets(code, size, &CFGUHandleOffset);

				if(CFGU_GetConfigInfoBlk2_endPos != NULL) {
					if(languageId != 0xFF)
						patchCfgGetLanguage(code, size, languageId, CFGU_GetConfigInfoBlk2_endPos);
					if(regionId != 0xFF)
						patchCfgGetRegion(code, size, regionId, CFGUHandleOffset);
				}
			}
		}
	}
}

void overlay_patch(u64 progId, u8 *code, u32 size) {
	// TODO - Implement.
}

// This is only for the .data segment.
void patch_data(u64 progId, u8 *data, u32 size, u32 orig_size) {
}

// This is only for the .ro segment.
void patch_ro(u64 progId, u8 *ro, u32 size, u32 orig_size) {
}

// This is only for the .code segment.
void patch_text(u64 progId, u8 *text, u32 size, u32 orig_size) {
    switch(progId)
    {
        case 0x0004003000008F02LL: // USA Menu
        case 0x0004003000008202LL: // EUR Menu
        case 0x0004003000009802LL: // JPN Menu
        case 0x000400300000A102LL: // CHN Menu
        case 0x000400300000A902LL: // KOR Menu
        case 0x000400300000B102LL: // TWN Menu
        {
			region_patch(progId, text, orig_size);
            break;
        }

        case 0x0004013000002C02LL: // NIM
        {
			disable_nim_updates(progId, text, orig_size);
			disable_eshop_updates(progId, text, orig_size);
            break;
        }

        case 0x0004013000003202LL: // FRIENDS
        {
			fake_friends_version(progId, text, orig_size);
            break;
        }

        case 0x0004001000021000LL: // USA MSET
        case 0x0004001000020000LL: // JPN MSET
        case 0x0004001000022000LL: // EUR MSET
        case 0x0004001000026000LL: // CHN MSET
        case 0x0004001000027000LL: // KOR MSET
        case 0x0004001000028000LL: // TWN MSET
        {
			settings_string(progId, text, size);
            break;
        }
        case 0x0004013000008002LL: // NS
        {
			disable_cart_updates(progId, text, orig_size);
			adjust_cpu_settings(progId, text, orig_size);
            break;
        }

        case 0x0004013000001702LL: // CFG
        {
			secureinfo_sigpatch(progId, text, orig_size);
            break;
        }
		case 0x0004013000003702LL: // RO
		{
			ro_sigpatch(progId, text, orig_size);
			break;
		}
        default: // Anything else.
		{
			language_emu(progId, text, orig_size);
            break;
        }
    }
}

// Gets how many bytes .text must be extended by for patches to fit.
u32 get_text_extend(u64 progId, u32 size_orig) {
	return 0; // Stub - nothing needs this yet
}

// Gets how many bytes .ro must be extended.
u32 get_ro_extend(u64 progId, u32 size_orig) {
	return 0; // Stub - nothing needs this yet
}

// Again, same, but for .data.
u32 get_data_extend(u64 progId, u32 size_orig) {
	return 0; // Stub - nothing needs this yet
}
