/**
 * Cache, address translation and system globals (src/libultra/os).
 *
 * The host has no CPU cache that is incoherent with a device and no KSEG segments, so these are no-ops or identities.
 */
#include "ultra64.h"
#include "port/port.h"

// N64 system globals that the game reads (os_system.h). The port reports an expansion pak (8 MB) like the
// development hardware the debug ROM targets.
void* osRomBase = NULL;
s32 osTvType = OS_TV_NTSC;
s32 osResetType = 0; // cold reset
s32 osCicId = 0;
s32 osVersion = 0;
u32 osMemSize = 0x800000;
s32 osAppNMIBuffer[0x10];

void osWritebackDCache(void* vaddr, s32 nbytes) {
    (void)vaddr;
    (void)nbytes;
}

void osWritebackDCacheAll(void) {
}

void osInvalDCache(void* vaddr, s32 nbytes) {
    (void)vaddr;
    (void)nbytes;
}

void osInvalICache(void* vaddr, s32 nbytes) {
    (void)vaddr;
    (void)nbytes;
}

uintptr_t osVirtualToPhysical(void* vaddr) {
    return (uintptr_t)vaddr;
}

u32 osGetMemSize(void) {
    return osMemSize;
}
