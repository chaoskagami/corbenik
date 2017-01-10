#include <3ds.h>
#include "patcher.h"
#include <lzss.c>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "logger.h"

// TODO - a lot of this is unecessarily verbose and shitty. Clean it up to be
// tidy.

#define MAX_SESSIONS 1

const char CODE_PATH[] = { 0x01, 0x00, 0x00, 0x00, 0x2E, 0x63, 0x6F, 0x64, 0x65, 0x00, 0x00, 0x00 };

static Handle g_handles[MAX_SESSIONS + 2];
static int g_active_handles;
static u64 g_cached_prog_handle;
static EXHEADER_header g_exheader;
static char g_ret_buf[1024];

static Result
allocate_shared_mem(EXHEADER_prog_addrs *shared, EXHEADER_prog_addrs *vaddr, int flags)
{
    // Somehow, we need to allow reallocating.
    u32 dummy;

    memcpy(shared, vaddr, sizeof(EXHEADER_prog_addrs));
    shared->text_addr = 0x10000000; // Base virtual address for code.
    shared->ro_addr = shared->text_addr + (shared->text_size << 12);
    shared->data_addr = shared->ro_addr + (shared->ro_size << 12);
    return svcControlMemory(&dummy, shared->text_addr, 0, shared->total_size << 12, (flags & 0xF00) | MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE);
}

static Result
load_code(u64 progid, u16 progver, EXHEADER_prog_addrs *shared, EXHEADER_prog_addrs *original, u64 prog_handle, int is_compressed)
{
    Handle handle;
    FS_Path archivePath;
    FS_Path path;
    Result res;
    u64 size;
    u32 total;

    archivePath.type = PATH_BINARY;
    archivePath.data = &prog_handle;
    archivePath.size = 8;

    path.type = PATH_BINARY;
    path.data = CODE_PATH;
    path.size = sizeof(CODE_PATH);

    if (R_FAILED(FSUSER_OpenFileDirectly(&handle, ARCHIVE_SAVEDATA_AND_CONTENT2, archivePath, path, FS_OPEN_READ, 0))) {
        panicstr("Failed to open program code path.\n");
    }

    // get file size
    if (R_FAILED(FSFILE_GetSize(handle, &size))) {
        FSFILE_Close(handle);
        panicstr("Failed to get code size.\n");
    }

    // check size
    if (size > (u64)shared->total_size << 12) {
        panicstr("codebin (file) size is larger than code size?\n");
        FSFILE_Close(handle);
        return 0xC900464F;
    }

    // read code
    res = FSFILE_Read(handle, &total, 0, (void *)shared->text_addr, size);
    FSFILE_Close(handle); // done reading
    if (R_FAILED(res)) {
        panicstr("Failed to read program code.\n");
    }

    // decompress in place
    if (is_compressed) {
        lzss_decompress((u8 *)shared->text_addr + size);
    }

    // Load/Dump code sections
    code_handler(progid, shared);

    // Patch segments
    patch_exe(progid, progver, shared, original);

    return 0;
}

static Result
loader_GetProgramInfo(EXHEADER_header *exheader, u64 prog_handle)
{
    Result res;

    if (prog_handle >> 32 == 0xFFFF0000) {
        return FSREG_GetProgramInfo(exheader, 1, prog_handle);
    } else {
        res = FSREG_CheckHostLoadId(prog_handle);
        // if ((res >= 0 && (unsigned)res >> 27) || (res < 0 && ((unsigned)res
        // >> 27)-32))
        // so use PXIPM if FSREG fails OR returns "info", is the second
        // condition a bug?
        if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS)) {
            return PXIPM_GetProgramInfo(exheader, prog_handle);
        } else {
            return FSREG_GetProgramInfo(exheader, 1, prog_handle);
        }
    }
}

static Result
loader_LoadProcess(Handle *process, u64 prog_handle)
{
    Result res;
    int count;
    u32 flags;
    u32 desc;
    u32 dummy;
    EXHEADER_prog_addrs shared_addr;
    EXHEADER_prog_addrs vaddr;
    EXHEADER_prog_addrs original_vaddr;
    Handle codeset;
    CodeSetInfo codesetinfo;
    u32 data_mem_size;
    u64 progid;
    u32 text_grow, data_grow, ro_grow;
    u16 progver;

    load_config(); // First order of business - we need the config file.
    openLogger();  // Open logs if enabled in config.

    // make sure the cached info corrosponds to the current prog_handle
    if (g_cached_prog_handle != prog_handle) {
        res = loader_GetProgramInfo(&g_exheader, prog_handle);
        g_cached_prog_handle = prog_handle;
        if (res < 0) {
            g_cached_prog_handle = 0;
            return res;
        }
    }

    // get kernel flags
    flags = 0;
    for (count = 0; count < 28; count++) {
        desc = g_exheader.arm11kernelcaps.descriptors[count];
        if (0x1FE == desc >> 23) {
            flags = desc & 0xF00;
        }
    }
    if (flags == 0) {
        return MAKERESULT(RL_PERMANENT, RS_INVALIDARG, 1, 2);
    }

    // load code
    progid = g_exheader.arm11systemlocalcaps.programid;
    progver = g_exheader.codesetinfo.flags.remasterversion[0] + g_exheader.codesetinfo.flags.remasterversion[1] * 0x100;

    logu64(progid);
    logstr("  validated params\n");

    // TODO - clean up this shit below. Not only is it unoptimized but it reads like garbage.

    // What the addressing info would be if not for expansion. This is passed to
    // patchCode.
    original_vaddr.text_size = (g_exheader.codesetinfo.text.codesize + 4095) >> 12; // (Text size + one page) >> page size
    original_vaddr.ro_size = (g_exheader.codesetinfo.ro.codesize + 4095) >> 12;
    original_vaddr.data_size = (g_exheader.codesetinfo.data.codesize + 4095) >> 12;
    original_vaddr.total_size = original_vaddr.text_size + original_vaddr.ro_size + original_vaddr.data_size;

    // Allow changing code, ro, data sizes to allow adding code
    text_grow = get_text_extend(progid, progver, g_exheader.codesetinfo.text.codesize);
    ro_grow = get_ro_extend(progid, progver, g_exheader.codesetinfo.ro.codesize);
    data_grow = get_data_extend(progid, progver, g_exheader.codesetinfo.data.codesize);

    // One page is 4096 bytes, thus all the 4095 constants.

    // Allocate process memory, growing as needed for extra patches
    vaddr.text_addr = g_exheader.codesetinfo.text.address;
    vaddr.text_size = (g_exheader.codesetinfo.text.codesize + text_grow + 4095) >> 12; // (Text size + one page) >> page size
    vaddr.ro_addr = g_exheader.codesetinfo.ro.address;
    vaddr.ro_size = (g_exheader.codesetinfo.ro.codesize + ro_grow + 4095) >> 12;
    vaddr.data_addr = g_exheader.codesetinfo.data.address;
    vaddr.data_size = (g_exheader.codesetinfo.data.codesize + data_grow + 4095) >> 12;
    data_mem_size = (g_exheader.codesetinfo.data.codesize + text_grow + g_exheader.codesetinfo.bsssize + 4095) >> 12;
    vaddr.total_size = vaddr.text_size + vaddr.ro_size + vaddr.data_size + text_grow + ro_grow + data_grow;

    if ((res = allocate_shared_mem(&shared_addr, &vaddr, flags)) < 0) {
        return res;
    }

    if ((res = load_code(progid, progver, &shared_addr, &original_vaddr, prog_handle, g_exheader.codesetinfo.flags.flag & 1)) >= 0) {
        memcpy(&codesetinfo.name, g_exheader.codesetinfo.name, 8);
        codesetinfo.program_id = progid;
        codesetinfo.text_addr = vaddr.text_addr;
        codesetinfo.text_size = vaddr.text_size;
        codesetinfo.text_size_total = vaddr.text_size;
        codesetinfo.ro_addr = vaddr.ro_addr;
        codesetinfo.ro_size = vaddr.ro_size;
        codesetinfo.ro_size_total = vaddr.ro_size;
        codesetinfo.rw_addr = vaddr.data_addr;
        codesetinfo.rw_size = vaddr.data_size;
        codesetinfo.rw_size_total = data_mem_size;
        res = svcCreateCodeSet(&codeset, &codesetinfo, (void *)shared_addr.text_addr, (void *)shared_addr.ro_addr, (void *)shared_addr.data_addr);
        if (res >= 0) {
            res = svcCreateProcess(process, codeset, g_exheader.arm11kernelcaps.descriptors, count);

            logstr("Created process\n");

            closeLogger();

            svcCloseHandle(codeset);
            if (res >= 0) {
                return 0; // Succeeded in loading process.
            }
        }
    }

    // Failed to load process, unmap shared memory and return error.
    svcControlMemory(&dummy, shared_addr.text_addr, 0, shared_addr.total_size << 12, MEMOP_FREE, 0);
    return res;
}

static Result
loader_RegisterProgram(u64 *prog_handle, FS_ProgramInfo *title, FS_ProgramInfo *update)
{
    Result res;
    u64 prog_id;

    prog_id = title->programId;
    if (prog_id >> 32 != 0xFFFF0000) {
        res = FSREG_CheckHostLoadId(prog_id);
        if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS)) {
            res = PXIPM_RegisterProgram(prog_handle, title, update);
            if (res < 0) {
                return res;
            }
            if (*prog_handle >> 32 != 0xFFFF0000) {
                res = FSREG_CheckHostLoadId(*prog_handle);
                if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS)) {
                    return 0;
                }
            }
            panicstr("Registration of program failed.\n");
        }
    }

    if (title->mediaType != update->mediaType)
        panicstr("Program and update are different mediaTypes, abort.\n");

    if (prog_id != update->programId)
        panicstr("Program and update have different titleIDs, abort.\n");

    res = FSREG_LoadProgram(prog_handle, title);
    if (R_SUCCEEDED(res)) {
        if (*prog_handle >> 32 == 0xFFFF0000) {
            return 0;
        }

        res = FSREG_CheckHostLoadId(*prog_handle);
        if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS)) {
            panicstr("CheckHostLoadId failed.\n");
        }
    }
    return res;
}

static Result
loader_UnregisterProgram(u64 prog_handle)
{
    Result res;

    if (prog_handle >> 32 == 0xFFFF0000) {
        return FSREG_UnloadProgram(prog_handle);
    } else {
        res = FSREG_CheckHostLoadId(prog_handle);
        if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS)) {
            return PXIPM_UnregisterProgram(prog_handle);
        } else {
            return FSREG_UnloadProgram(prog_handle);
        }
    }
}

#define LoadProcess       1
#define RegisterProgram   2
#define UnregisterProgram 3
#define GetProgramInfo    4

static void
handle_commands(void)
{
    FS_ProgramInfo title;
    FS_ProgramInfo update;
    u32 *cmdbuf;
    u16 cmdid;
    int res;
    Handle handle;
    u64 prog_handle;

    cmdbuf = getThreadCommandBuffer();
    cmdid = cmdbuf[0] >> 16;
    res = 0;
    switch (cmdid) {
        case LoadProcess:
        {
            res = loader_LoadProcess(&handle, *(u64 *)&cmdbuf[1]);
            cmdbuf[0] = 0x10042;
            cmdbuf[1] = res;
            cmdbuf[2] = 16;
            cmdbuf[3] = handle;
            break;
        }
        case RegisterProgram:
        {
            memcpy(&title, &cmdbuf[1], sizeof(FS_ProgramInfo));
            memcpy(&update, &cmdbuf[5], sizeof(FS_ProgramInfo));
            res = loader_RegisterProgram(&prog_handle, &title, &update);
            cmdbuf[0] = 0x200C0;
            cmdbuf[1] = res;
            *(u64 *)&cmdbuf[2] = prog_handle;
            break;
        }
        case UnregisterProgram:
        {
            if (g_cached_prog_handle == prog_handle) {
                g_cached_prog_handle = 0;
            }
            cmdbuf[0] = 0x30040;
            cmdbuf[1] = loader_UnregisterProgram(*(u64 *)&cmdbuf[1]);
            break;
        }
        case GetProgramInfo:
        {
            prog_handle = *(u64 *)&cmdbuf[1];
            if (prog_handle != g_cached_prog_handle) {
                res = loader_GetProgramInfo(&g_exheader, prog_handle);
                if (res >= 0) {
                    g_cached_prog_handle = prog_handle;
                } else {
                    g_cached_prog_handle = 0;
                }
            }
            memcpy(&g_ret_buf, &g_exheader, 1024);
            cmdbuf[0] = 0x40042;
            cmdbuf[1] = res;
            cmdbuf[2] = 0x1000002;
            cmdbuf[3] = (u32)&g_ret_buf;
            break;
        }
        default: // error
        {
            cmdbuf[0] = 0x40;
            cmdbuf[1] = 0xD900182F;
            break;
        }
        // I don't see why it shouldn't be possible to add extra
        // functions callable by threadbuf. This can be used. ;)
    }
}

static Result
should_terminate(int *term_request)
{
    u32 notid;
    Result ret;

    ret = srvReceiveNotification(&notid);
    if (R_FAILED(ret)) {
        return ret;
    }
    if (notid == 0x100) // term request
    {
        *term_request = 1;
    }
    return 0;
}

int
main()
{
    Result ret;
    Handle handle;
    Handle reply_target;
    Handle *srv_handle;
    Handle *notification_handle;
    s32 index;
    int i;
    int term_request;
    u32 *cmdbuf;

    ret = 0;

    srv_handle = &g_handles[1];
    notification_handle = &g_handles[0];

    if (R_FAILED(srvRegisterService(srv_handle, "Loader", MAX_SESSIONS))) {
        panicstr("Failed to register loader service.\n");
    }

    if (R_FAILED(srvEnableNotification(notification_handle))) {
        panicstr("Failed to enable notifcations to loader service.\n");
    }

    g_active_handles = 2;
    g_cached_prog_handle = 0;
    index = 1;

    reply_target = 0;
    term_request = 0;
    do {
        if (reply_target == 0) {
            cmdbuf = getThreadCommandBuffer();
            cmdbuf[0] = 0xFFFF0000;
        }
        ret = svcReplyAndReceive(&index, g_handles, g_active_handles, reply_target);

        if (R_FAILED(ret)) {
            // check if any handle has been closed
            if (ret == (int)0xC920181A) {
                if (index == -1) {
                    for (i = 2; i < MAX_SESSIONS + 2; i++) {
                        if (g_handles[i] == reply_target) {
                            index = i;
                            break;
                        }
                    }
                }
                svcCloseHandle(g_handles[index]);
                g_handles[index] = g_handles[g_active_handles - 1];
                g_active_handles--;
                reply_target = 0;
            } else {
                panicstr("Unhandled response from svcReplyAndRecieve, abort.\n");
            }
        } else {
            // process responses
            reply_target = 0;
            switch (index) {
                case 0: // notification
                {
                    if (R_FAILED(should_terminate(&term_request))) {
                        panicstr("Failed to check termination.\n");
                    }
                    break;
                }
                case 1: // new session
                {
                    if (R_FAILED(svcAcceptSession(&handle, *srv_handle))) {
                        panicstr("Failed to accept and open session.\n");
                    }
                    if (g_active_handles < MAX_SESSIONS + 2) {
                        g_handles[g_active_handles] = handle;
                        g_active_handles++;
                    } else {
                        svcCloseHandle(handle);
                    }
                    break;
                }
                default: // session
                {
                    handle_commands();
                    reply_target = g_handles[index];
                    break;
                }
            }
        }
    } while (!term_request || g_active_handles != 2);

    srvUnregisterService("Loader");
    svcCloseHandle(*srv_handle);
    svcCloseHandle(*notification_handle);

    return 0;
}
