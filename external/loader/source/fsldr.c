/**
 * Random annoyance; All the command headers etc for fsLdr are EXACTLY the same as fsUser.
 * This is annpying mainly because - if I could open the file handle manually in ctrulib -
 * e.g. not static and inaccessible - I could simply use the FSUSER API for this. ALL. OF. THIS.
 */
#include <3ds.h>
#include "fsldr.h"
#include "fsreg.h"
#include "srvsys.h"

static Handle fsldrHandle;
static int fsldrRefCount;

// MAKE SURE fsreg has been init before calling this
static Result
fsldrPatchPermissions(void)
{
    u32 pid;
    Result res;
    FS_ProgramInfo info;
    u32 storage[8] = { 0 };

    storage[6] = 0x680;                    // SDMC access and NAND access flag
    info.programId = 0x0004013000001302LL; // loader PID
    info.mediaType = MEDIATYPE_NAND;
    res = svcGetProcessId(&pid, 0xFFFF8001);
    if (R_SUCCEEDED(res)) {
        res = FSREG_Register(pid, 0xFFFF000000000000LL, &info, (u8 *)storage);
    }
    return res;
}

Result
fsldrInit(void)
{
    Result ret = 0;

    if (AtomicPostIncrement(&fsldrRefCount))
        return 0;

    ret = srvSysGetServiceHandle(&fsldrHandle, "fs:LDR");
    if (R_SUCCEEDED(ret)) {
        fsldrPatchPermissions();
        ret = FSLDR_Initialize(fsldrHandle);
        if (R_FAILED(ret))
            svcBreak(USERBREAK_ASSERT); // Can't properly panic here; no logger
    } else {
        AtomicDecrement(&fsldrRefCount);
    }

    return ret;
}

void
fsldrExit(void)
{
    if (AtomicDecrement(&fsldrRefCount))
        return;
    svcCloseHandle(fsldrHandle);
}

Result
FSLDR_Initialize(Handle session)
{
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x801, 0, 2); // 0x8010002
    cmdbuf[1] = 32;

    Result ret = 0;
    if (R_FAILED(ret = svcSendSyncRequest(session)))
        return ret;

    return cmdbuf[1];
}

Result
FSLDR_OpenFileDirectly(Handle *out, FS_ArchiveID archiveId, FS_Path archivePath, FS_Path filePath, u32 openFlags, u32 attributes)
{
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x803, 8, 4); // 0x8030204
    cmdbuf[1] = 0;
    cmdbuf[2] = archiveId;
    cmdbuf[3] = archivePath.type;
    cmdbuf[4] = archivePath.size;
    cmdbuf[5] = filePath.type;
    cmdbuf[6] = filePath.size;
    cmdbuf[7] = openFlags;
    cmdbuf[8] = attributes;
    cmdbuf[9] = IPC_Desc_StaticBuffer(archivePath.size, 2);
    cmdbuf[10] = (u32)archivePath.data;
    cmdbuf[11] = IPC_Desc_StaticBuffer(filePath.size, 0);
    cmdbuf[12] = (u32)filePath.data;

    Result ret = 0;
    if (R_FAILED(ret = svcSendSyncRequest(fsldrHandle)))
        return ret;

    if (out)
        *out = cmdbuf[3];

    return cmdbuf[1];
}

Result
FSLDR_GetNandCid(u8* out, u32 length)
{
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x81A, 1, 2); // 0x81A0042
	cmdbuf[1] = length;
	cmdbuf[2] = IPC_Desc_Buffer(length, IPC_BUFFER_W);
	cmdbuf[3] = (u32) out;

	Result ret = 0;
	if(R_FAILED(ret = svcSendSyncRequest(fsldrHandle)))
		return ret;

	return cmdbuf[1];
}
