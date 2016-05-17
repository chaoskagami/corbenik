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

static u32 patchMemory(u8 *start, u32 size, const void *pattern, u32 patSize, int offset, const void *replace, u32 repSize, u32 count)
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

struct config_file config;
int failed_load_config = 1;

void clear_config() {
	// Basically; memset.
	for(int i=0;i<sizeof(struct config_file);i++)
		((char*)&config)[i]=0;
}

void load_config() {
	IFile file;
    u64 total;

	// Open file.
    if (!fileOpen(&file, ARCHIVE_SDMC, PATH_CONFIG, FS_OPEN_READ)) {
		// Failed to open.
		failed_load_config = 1;
        goto ret_fail;
    }

	// Read file.
    if(!IFile_Read(&file, &total, &config, sizeof(struct config_file))) {
		// Failed to read.
		goto ret_fail;
	}

	IFile_Close(&file); // Read to memory.

	if (config.magic[0] != 'O' || config.magic[1] != 'V' || config.magic[2] != 'A' || config.magic[3] != 'N') {
		// Incorrect magic.
		// Failed to read.
		goto ret_fail;
	}

	if (config.config_ver != config_version) {
		// Invalid version.
		goto ret_fail;
    }

	failed_load_config = 0;

	return;

ret_fail:
	clear_config();

	return;
}

static int loadTitleLocaleConfig(u64 progId, u8 *regionId, u8 *languageId)
{
	// FIXME - Rewrite this function to use a single line-based config
	// Directory seeks have severe issues on FAT and
	// dumping configs based on 3dsdb (more than 1000) causes things
	// to kind a choke.

    /* Here we look for "/corbenik/locale/[u64 titleID in hex, uppercase].txt"
       If it exists it should contain, for example, "EUR IT" */

    char path[] = "/corbenik/locales/0000000000000000.txt";

	u32 i = 0, j = 0;
	while(path[j] != 0) {
		if (path[j] == '/')
			i = j;
		j++;
	}
	i += 1;

    while(progId > 0)
    {
        static const char hexDigits[] = "0123456789ABCDEF";
        path[i--] = hexDigits[(u32)(progId & 0xF)];
        progId >>= 4;
    }

    IFile file;
    Result ret = fileOpen(&file, ARCHIVE_SDMC, path, FS_OPEN_READ);
    if(R_SUCCEEDED(ret))
    {
        char buf[6];
        u64 total;

        ret = IFile_Read(&file, &total, buf, 6);
        IFile_Close(&file);

        if(!R_SUCCEEDED(ret) || total < 6) return -1;

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

void region_patch(u64 progId, u8 *code, u32 size) {
	static const u8 regionFreePattern[] = {0x00, 0x00, 0x55, 0xE3, 0x01, 0x10, 0xA0, 0xE3};
	static const u8 regionFreePatch[]   = {0x01, 0x00, 0xA0, 0xE3, 0x1E, 0xFF, 0x2F, 0xE1};

	//Patch SMDH region checks
	patchMemory(code, size,
		regionFreePattern,
		sizeof(regionFreePattern), -16,
		regionFreePatch,
		sizeof(regionFreePatch), 1
    );
}

void disable_nim_updates(u64 progId, u8 *code, u32 size) {
	static const u8 blockAutoUpdatesPattern[] = {0x25, 0x79, 0x0B, 0x99};
	static const u8 blockAutoUpdatesPatch[]   = {0xE3, 0xA0};

	//Block silent auto-updates
	patchMemory(code, size,
		blockAutoUpdatesPattern,
		sizeof(blockAutoUpdatesPattern), 0,
		blockAutoUpdatesPatch,
		sizeof(blockAutoUpdatesPatch), 1
	);
}

void disable_eshop_updates(u64 progId, u8 *code, u32 size) {
	static const u8 skipEshopUpdateCheckPattern[] = {0x30, 0xB5, 0xF1, 0xB0};
	static const u8 skipEshopUpdateCheckPatch[]   = {0x00, 0x20, 0x08, 0x60, 0x70, 0x47};

	//Skip update checks to access the EShop
	patchMemory(code, size,
		skipEshopUpdateCheckPattern,
		sizeof(skipEshopUpdateCheckPattern), 0,
		skipEshopUpdateCheckPatch,
		sizeof(skipEshopUpdateCheckPatch), 1
	);
}

void fake_friends_version(u64 progId, u8 *code, u32 size) {
	static const u8 fpdVerPattern[] = {0xE0, 0x1E, 0xFF, 0x2F, 0xE1, 0x01, 0x01, 0x01};
	static const u8 fpdVerPatch = 0x06; // Latest version.

	//Allow online access to work with old friends modules
	patchMemory(code, size,
		fpdVerPattern,
		sizeof(fpdVerPattern), 9,
		&fpdVerPatch,
		sizeof(fpdVerPatch), 1
	);
}

void settings_string(u64 progId, u8 *code, u32 size) {
	static const u16 verPattern[] = u"Ver.";
	static const u16 verPatch[] = u".hax";

	//Patch Ver. string
	patchMemory(code, size,
		verPattern,
		sizeof(verPattern) - sizeof(u16), 0,
		verPatch,
		sizeof(verPatch) - sizeof(u16), 1
	);
}

void disable_cart_updates(u64 progId, u8 *code, u32 size) {
	static const u8 stopCartUpdatesPattern[] = {0x0C, 0x18, 0xE1, 0xD8};
	static const u8 stopCartUpdatesPatch[]   = {0x0B, 0x18, 0x21, 0xC8};

	//Disable updates from foreign carts (makes carts region-free)
	patchMemory(code, size,
		stopCartUpdatesPattern,
		sizeof(stopCartUpdatesPattern), 0,
		stopCartUpdatesPatch,
		sizeof(stopCartUpdatesPatch), 2
	);
}

void adjust_cpu_settings(u64 progId, u8 *code, u32 size) {
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

void secureinfo_sigpatch(u64 progId, u8 *code, u32 size) {
	static const u8 secureinfoSigCheckPattern[] = {0x06, 0x46, 0x10, 0x48, 0xFC};
	static const u8 secureinfoSigCheckPatch[]   = {0x00, 0x26};

	//Disable SecureInfo signature check
	patchMemory(code, size,
		secureinfoSigCheckPattern,
		sizeof(secureinfoSigCheckPattern), 0,
		secureinfoSigCheckPatch,
		sizeof(secureinfoSigCheckPatch), 1
	);
}

void patchCode(u64 progId, u8 *code, u32 size)
{
	// FIXME - Config loading breaks loader. WTF is this?
	// Maybe the memcpy?
	// load_config();

    switch(progId)
    {
        case 0x0004003000008F02LL: // USA Menu
        case 0x0004003000008202LL: // EUR Menu
        case 0x0004003000009802LL: // JPN Menu
        case 0x000400300000A102LL: // CHN Menu
        case 0x000400300000A902LL: // KOR Menu
        case 0x000400300000B102LL: // TWN Menu
        {
			region_patch(progId, code, size);
            break;
        }

        case 0x0004013000002C02LL: // NIM
        {
			disable_nim_updates(progId, code, size);
			disable_eshop_updates(progId, code, size);
            break;
        }

        case 0x0004013000003202LL: // FRIENDS
        {
			fake_friends_version(progId, code, size);
            break;
        }

        case 0x0004001000021000LL: // USA MSET
        case 0x0004001000020000LL: // JPN MSET
        case 0x0004001000022000LL: // EUR MSET
        case 0x0004001000026000LL: // CHN MSET
        case 0x0004001000027000LL: // KOR MSET
        case 0x0004001000028000LL: // TWN MSET
        {
			settings_string(progId, code, size);
            break;
        }
        case 0x0004013000008002LL: // NS
        {
			disable_cart_updates(progId, code, size);
			adjust_cpu_settings(progId, code, size);
            break;
        }

        case 0x0004013000001702LL: // CFG
        {
			secureinfo_sigpatch(progId, code, size);
            break;
        }
/*        default:
		{
            if(!failed_load_config && config.options[OPTION_LOADER_LANGEMU])
            {
                u32 tidHigh = (progId & 0xFFFFFFF000000000LL) >> 0x24;

                if(tidHigh == 0x0004000)
                {
                    //Language emulation
                    u8 regionId = 0xFF,
                       languageId = 0xFF;

                    if(R_SUCCEEDED(loadTitleLocaleConfig(progId, &regionId, &languageId)))
                    {
                        u32 CFGUHandleOffset;

                        u8 *CFGU_GetConfigInfoBlk2_endPos = getCfgOffsets(code, size, &CFGUHandleOffset);

                        if(CFGU_GetConfigInfoBlk2_endPos != NULL)
                        {
                            if(languageId != 0xFF)
                                patchCfgGetLanguage(code, size, languageId, CFGU_GetConfigInfoBlk2_endPos);
                            if(regionId != 0xFF)
                                patchCfgGetRegion(code, size, regionId, CFGUHandleOffset);
                        }
                    }
                }
            }
            break;
        } */
    }
}
