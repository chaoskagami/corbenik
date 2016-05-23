#include <3ds.h>
#include "patcher.h"
#include "exheader.h"
#include "ifile.h"
#include "fsldr.h"
#include "fsreg.h"
#include "pxipm.h"
#include "srvsys.h"
#include "lzss.h"
#include "internal.h"

// TODO - a lot of this is unecessarily verbose and shitty. Clean it up to be tidy.

#define MAX_SESSIONS 1

const char CODE_PATH[] = {0x01, 0x00, 0x00, 0x00, 0x2E, 0x63, 0x6F, 0x64, 0x65, 0x00, 0x00, 0x00};

typedef struct
{
  u32 text_addr;
  u32 text_size;
  u32 ro_addr;
  u32 ro_size;
  u32 data_addr;
  u32 data_size;
  u32 total_size;
} prog_addrs_t;

static Handle g_handles[MAX_SESSIONS+2];
static int g_active_handles;
static u64 g_cached_prog_handle;
static exheader_header g_exheader;
static char g_ret_buf[1024];

static Result allocate_shared_mem(prog_addrs_t *shared, prog_addrs_t *vaddr, int flags)
{
  // Somehow, we need to allow reallocating.

  u32 dummy;

  memcpy(shared, vaddr, sizeof(prog_addrs_t));
  shared->text_addr = 0x10000000; // Code is forcibly relocated to this address to kill ASLR (I believe.)
  shared->ro_addr = shared->text_addr + (shared->text_size << 12);
  shared->data_addr = shared->ro_addr + (shared->ro_size << 12);
  return svcControlMemory(&dummy, shared->text_addr, 0, shared->total_size << 12, (flags & 0xF00) | MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE);
}

static Result load_code(u64 progid, prog_addrs_t *shared, prog_addrs_t *original, u64 prog_handle, int is_compressed)
{
  IFile file;
  FS_Path archivePath;
  FS_Path path;
  Result res;
  u64 size;
  u64 total;

  archivePath.type = PATH_BINARY;
  archivePath.data = &prog_handle;
  archivePath.size = 8;

  path.type = PATH_BINARY;
  path.data = CODE_PATH;
  path.size = sizeof(CODE_PATH);

  if (R_FAILED(IFile_Open(&file, ARCHIVE_SAVEDATA_AND_CONTENT2, archivePath, path, FS_OPEN_READ)))
  {
    svcBreak(USERBREAK_ASSERT);
  }

  // get file size
  if (R_FAILED(IFile_GetSize(&file, &size)))
  {
    IFile_Close(&file);
    svcBreak(USERBREAK_ASSERT);
  }

  // check size
  if (size > (u64)shared->total_size << 12)
  {
    IFile_Close(&file);
    return 0xC900464F;
  }

  // read code
  res = IFile_Read(&file, &total, (void *)shared->text_addr, size);
  IFile_Close(&file); // done reading
  if (R_FAILED(res))
  {
    svcBreak(USERBREAK_ASSERT);
  }

  // decompress in place
  if (is_compressed)
  {
    lzss_decompress((u8 *)shared->text_addr + size);
  }

  // Patch segments
  patch_text(progid, (u8 *)shared->text_addr, shared->text_size << 12, original->text_size << 12);
  patch_data(progid, (u8 *)shared->data_addr, shared->data_size << 12, original->data_size << 12);
  patch_ro  (progid, (u8 *)shared->ro_addr,   shared->ro_size << 12,   original->ro_size << 12);

  return 0;
}

static Result loader_GetProgramInfo(exheader_header *exheader, u64 prog_handle)
{
  Result res;

  if (prog_handle >> 32 == 0xFFFF0000)
  {
    return FSREG_GetProgramInfo(exheader, 1, prog_handle);
  }
  else
  {
    res = FSREG_CheckHostLoadId(prog_handle);
    //if ((res >= 0 && (unsigned)res >> 27) || (res < 0 && ((unsigned)res >> 27)-32))
    //so use PXIPM if FSREG fails OR returns "info", is the second condition a bug?
    if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS))
    {
      return PXIPM_GetProgramInfo(exheader, prog_handle);
    }
    else
    {
      return FSREG_GetProgramInfo(exheader, 1, prog_handle);
    }
  }
}

extern void KernelSetState(unsigned int, unsigned int, unsigned int, unsigned int);

static void ConfigureNew3DSCPU(u8 mode) {
  // 10 -> r0 (Type)
  // mode -> r1 (Mode)
  // svc 0x7C

  KernelSetState(10, mode, 0, 0); // Yes, we syscall directly. Problem?
}

static Result loader_LoadProcess(Handle *process, u64 prog_handle)
{
  Result res;
  int count;
  u32 flags;
  u32 desc;
  u32 dummy;
  prog_addrs_t shared_addr;
  prog_addrs_t vaddr;
  prog_addrs_t original_vaddr;
  Handle codeset;
  CodeSetInfo codesetinfo;
  u32 data_mem_size;
  u64 progid;
  u32 text_grow, data_grow, ro_grow;

  // make sure the cached info corrosponds to the current prog_handle
  if (g_cached_prog_handle != prog_handle)
  {
    res = loader_GetProgramInfo(&g_exheader, prog_handle);
    g_cached_prog_handle = prog_handle;
    if (res < 0)
    {
      g_cached_prog_handle = 0;
      return res;
    }
  }

  // get kernel flags
  flags = 0;
  for (count = 0; count < 28; count++)
  {
    desc = g_exheader.arm11kernelcaps.descriptors[count];
    if (0x1FE == desc >> 23)
    {
      flags = desc & 0xF00;
    }
  }
  if (flags == 0)
  {
    return MAKERESULT(RL_PERMANENT, RS_INVALIDARG, 1, 2);
  }

  load_config(); // First order of business - we need the config file.

  // Check and set the CPU mode. Possible values: 0 - Keep o3ds speed, 1 - n3ds speed, -1 force o3ds
  // This is A-OK because the CPU speed parameter isn't passed through to any kernel function; meaning it has already been set.
  u8 n3ds_mode = g_exheader.arm11systemlocalcaps.flags[1] & 0x3;
  u8 cpu_mode  = get_cpumode(progid);
  if (cpu_mode != 0xFF) { // Skip?
    u8 mode = n3ds_mode | cpu_mode; // Keep flags set by exheader.
    ConfigureNew3DSCPU(mode); // We do not use PXIPM because we are a sysmodule. It doesn't make sense.
  }

  // What the addressing info would be if not for expansion. This is passed to patchCode.
  original_vaddr.text_size = (g_exheader.codesetinfo.text.codesize + 4095) >> 12; // (Text size + one page) >> page size
  original_vaddr.ro_size = (g_exheader.codesetinfo.ro.codesize + 4095) >> 12;
  original_vaddr.data_size = (g_exheader.codesetinfo.data.codesize + 4095) >> 12;
  original_vaddr.total_size = original_vaddr.text_size + original_vaddr.ro_size + original_vaddr.data_size;

  // Allow changing code, ro, data sizes to allow adding code
  text_grow = get_text_extend(progid, g_exheader.codesetinfo.text.codesize);
  ro_grow   = get_ro_extend(progid, g_exheader.codesetinfo.ro.codesize);
  data_grow = get_data_extend(progid, g_exheader.codesetinfo.data.codesize);

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

  if ((res = allocate_shared_mem(&shared_addr, &vaddr, flags)) < 0)
  {
    return res;
  }

  // load code
  progid = g_exheader.arm11systemlocalcaps.programid;
  if ((res = load_code(progid, &shared_addr, &original_vaddr, prog_handle, g_exheader.codesetinfo.flags.flag & 1)) >= 0)
  {
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
    if (res >= 0)
    {
      res = svcCreateProcess(process, codeset, g_exheader.arm11kernelcaps.descriptors, count);
      svcCloseHandle(codeset);
      if (res >= 0)
      {
        return 0;
      }
    }
  }

  svcControlMemory(&dummy, shared_addr.text_addr, 0, shared_addr.total_size << 12, MEMOP_FREE, 0);
  return res;
}

static Result loader_RegisterProgram(u64 *prog_handle, FS_ProgramInfo *title, FS_ProgramInfo *update)
{
  Result res;
  u64 prog_id;

  prog_id = title->programId;
  if (prog_id >> 32 != 0xFFFF0000)
  {
    res = FSREG_CheckHostLoadId(prog_id);
    //if ((res >= 0 && (unsigned)res >> 27) || (res < 0 && ((unsigned)res >> 27)-32))
    if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS))
    {
      res = PXIPM_RegisterProgram(prog_handle, title, update);
      if (res < 0)
      {
        return res;
      }
      if (*prog_handle >> 32 != 0xFFFF0000)
      {
        res = FSREG_CheckHostLoadId(*prog_handle);
        //if ((res >= 0 && (unsigned)res >> 27) || (res < 0 && ((unsigned)res >> 27)-32))
        if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS))
        {
          return 0;
        }
      }
      svcBreak(USERBREAK_ASSERT);
    }
  }

  if ((title->mediaType != update->mediaType) || (prog_id != update->programId))
  {
    svcBreak(USERBREAK_ASSERT);
  }
  res = FSREG_LoadProgram(prog_handle, title);
  if (R_SUCCEEDED(res))
  {
    if (*prog_handle >> 32 == 0xFFFF0000)
    {
      return 0;
    }
    res = FSREG_CheckHostLoadId(*prog_handle);
    //if ((res >= 0 && (unsigned)res >> 27) || (res < 0 && ((unsigned)res >> 27)-32))
    if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS))
    {
      svcBreak(USERBREAK_ASSERT);
    }
  }
  return res;
}

static Result loader_UnregisterProgram(u64 prog_handle)
{
  Result res;

  if (prog_handle >> 32 == 0xFFFF0000)
  {
    return FSREG_UnloadProgram(prog_handle);
  }
  else
  {
    res = FSREG_CheckHostLoadId(prog_handle);
    //if ((res >= 0 && (unsigned)res >> 27) || (res < 0 && ((unsigned)res >> 27)-32))
    if (R_FAILED(res) || (R_SUCCEEDED(res) && R_LEVEL(res) != RL_SUCCESS))
    {
      return PXIPM_UnregisterProgram(prog_handle);
    }
    else
    {
      return FSREG_UnloadProgram(prog_handle);
    }
  }
}

static void handle_commands(void)
{
  FS_ProgramInfo title;
  FS_ProgramInfo update;
  u32* cmdbuf;
  u16 cmdid;
  int res;
  Handle handle;
  u64 prog_handle;

  cmdbuf = getThreadCommandBuffer();
  cmdid = cmdbuf[0] >> 16;
  res = 0;
  switch (cmdid)
  {
    case 1: // LoadProcess
    {
      res = loader_LoadProcess(&handle, *(u64 *)&cmdbuf[1]);
      cmdbuf[0] = 0x10042;
      cmdbuf[1] = res;
      cmdbuf[2] = 16;
      cmdbuf[3] = handle;
      break;
    }
    case 2: // RegisterProgram
    {
      memcpy(&title, &cmdbuf[1], sizeof(FS_ProgramInfo));
      memcpy(&update, &cmdbuf[5], sizeof(FS_ProgramInfo));
      res = loader_RegisterProgram(&prog_handle, &title, &update);
      cmdbuf[0] = 0x200C0;
      cmdbuf[1] = res;
      *(u64 *)&cmdbuf[2] = prog_handle;
      break;
    }
    case 3: // UnregisterProgram
    {
      if (g_cached_prog_handle == prog_handle)
      {
        g_cached_prog_handle = 0;
      }
      cmdbuf[0] = 0x30040;
      cmdbuf[1] = loader_UnregisterProgram(*(u64 *)&cmdbuf[1]);
      break;
    }
    case 4: // GetProgramInfo
    {
      prog_handle = *(u64 *)&cmdbuf[1];
      if (prog_handle != g_cached_prog_handle)
      {
        res = loader_GetProgramInfo(&g_exheader, prog_handle);
        if (res >= 0)
        {
          g_cached_prog_handle = prog_handle;
        }
        else
        {
          g_cached_prog_handle = 0;
        }
      }
      memcpy(&g_ret_buf, &g_exheader, 1024);
      cmdbuf[0] = 0x40042;
      cmdbuf[1] = res;
      cmdbuf[2] = 0x1000002;
      cmdbuf[3] = (u32) &g_ret_buf;
      break;
    }
    default: // error
    {
      cmdbuf[0] = 0x40;
      cmdbuf[1] = 0xD900182F;
      break;
    }
  }
}

static Result should_terminate(int *term_request)
{
  u32 notid;
  Result ret;

  ret = srvSysReceiveNotification(&notid);
  if (R_FAILED(ret))
  {
    return ret;
  }
  if (notid == 0x100) // term request
  {
    *term_request = 1;
  }
  return 0;
}

// this is called before main
void __appInit()
{
  srvSysInit();
  fsregInit();
  fsldrInit();
  pxipmInit();
}

// this is called after main exits
void __appExit()
{
  pxipmExit();
  fsldrExit();
  fsregExit();
  srvSysExit();
}

// stubs for non-needed pre-main functions
void __sync_init();
void __sync_fini();
void __system_initSyscalls();

void __ctru_exit()
{
  __appExit();
  __sync_fini();
  svcExitProcess();
}

void initSystem()
{
  __sync_init();
  __system_initSyscalls();
  __appInit();
}

int main()
{
  Result ret;
  Handle handle;
  Handle reply_target;
  Handle *srv_handle;
  Handle *notification_handle;
  s32 index;
  int i;
  int term_request;
  u32* cmdbuf;

  ret = 0;

  srv_handle = &g_handles[1];
  notification_handle = &g_handles[0];

  if (R_FAILED(srvSysRegisterService(srv_handle, "Loader", MAX_SESSIONS)))
  {
    svcBreak(USERBREAK_ASSERT);
  }

  if (R_FAILED(srvSysEnableNotification(notification_handle)))
  {
    svcBreak(USERBREAK_ASSERT);
  }

  g_active_handles = 2;
  g_cached_prog_handle = 0;
  index = 1;

  reply_target = 0;
  term_request = 0;
  do
  {
    if (reply_target == 0)
    {
      cmdbuf = getThreadCommandBuffer();
      cmdbuf[0] = 0xFFFF0000;
    }
    ret = svcReplyAndReceive(&index, g_handles, g_active_handles, reply_target);

    if (R_FAILED(ret))
    {
      // check if any handle has been closed
      if (ret == (int)0xC920181A)
      {
        if (index == -1)
        {
          for (i = 2; i < MAX_SESSIONS+2; i++)
          {
            if (g_handles[i] == reply_target)
            {
              index = i;
              break;
            }
          }
        }
        svcCloseHandle(g_handles[index]);
        g_handles[index] = g_handles[g_active_handles-1];
        g_active_handles--;
        reply_target = 0;
      }
      else
      {
        svcBreak(USERBREAK_ASSERT);
      }
    }
    else
    {
      // process responses
      reply_target = 0;
      switch (index)
      {
        case 0: // notification
        {
          if (R_FAILED(should_terminate(&term_request)))
          {
            svcBreak(USERBREAK_ASSERT);
          }
          break;
        }
        case 1: // new session
        {
          if (R_FAILED(svcAcceptSession(&handle, *srv_handle)))
          {
            svcBreak(USERBREAK_ASSERT);
          }
          if (g_active_handles < MAX_SESSIONS+2)
          {
            g_handles[g_active_handles] = handle;
            g_active_handles++;
          }
          else
          {
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

  srvSysUnregisterService("Loader");
  svcCloseHandle(*srv_handle);
  svcCloseHandle(*notification_handle);

  return 0;
}
