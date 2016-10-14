#include <3ds.h>
#include "patcher.h"
#include "pxipm.h"
#include <string.h>
#include "logger.h"

extern char* fake_heap_start;
extern char* fake_heap_end;
extern u32 __ctru_heap;
extern u32 __ctru_heap_size;
extern u32 __ctru_linear_heap;
extern u32 __ctru_linear_heap_size;

// NOTE - stubs for non-needed pre-main functions
void __sync_init();
void __sync_fini();
void __system_initSyscalls(void);

// Pre-main initialization function.
void
__appInit()
{
    srvInit();
    fsregInit();
    fsInitFromService("fs:LDR");
    pxipmInit();
}

// Post-main cleanup function.
void
__appExit()
{
    pxipmExit();
    fsExit();
    fsregExit();
    srvExit();
}

void __system_allocateHeaps(void) {
    u32 dummy = 0;

    // Allocate a small-ish heap. Why, you ask? Half the system hasn't started up (we start it)
    // so we actually don't know how much memory is needed later.
    // If we do it ctrulib-ish like this:

    // u32 size = (osGetMemRegionFree(MEMREGION_BASE) / 2) & 0xFFFFF000;

    // Things just go terribly awry, because we'll eventually OOM when loading everything in the base region.
    // This results in HOME loading and half the system modules not loading.

    // According to 3dbrew, on 4.5.0 we have 0x001FE000 bytes in BASE once HOME is loaded. We play it safe and take a very tiny amount.
    u32 size = 0x20000;
    __ctru_heap_size = size;

    // Allocate the module's heap
    __ctru_heap        = 0x08000000;
    if(R_FAILED(svcControlMemory(&dummy, __ctru_heap, 0x0, __ctru_heap_size, MEMOP_ALLOC | MEMOP_REGION_BASE, MEMPERM_READ | MEMPERM_WRITE)))
        svcBreak(USERBREAK_ASSERT); // Memory allocation failed.

    // Set up newlib heap
    fake_heap_start = (char*)__ctru_heap;
    fake_heap_end = fake_heap_start + __ctru_heap_size;
}

void __libctru_init()
{
    // Initialize newlib support system calls
    __system_initSyscalls();

    // Allocate module heap
    __system_allocateHeaps();
}

void __attribute__((noreturn)) __libctru_exit()
{
    u32 dummy = 0;

    // Unmap the application heap
    svcControlMemory(&dummy, __ctru_heap, 0x0, __ctru_heap_size, MEMOP_FREE, 0x0);

    // Since above did not jump, end this process
    svcExitProcess();
}

// See: https://github.com/smealum/ctrulib/blob/master/libctru/source/system/stack_adjust.s
// This is overridden in all likelyhood to avoid the stack being fucked.
void
initSystem()
{
    __libctru_init();
    __appInit();
}


// See above initSystem for why this is overridden.
void
__ctru_exit()
{
    __appExit();
    __libctru_exit();
}

