#ifndef SEGMENTED_ADDRESS_H
#define SEGMENTED_ADDRESS_H

#include "ultra64.h"
#include "stdint.h"

extern uintptr_t gSegments[NUM_SEGMENTS];

#if TARGET_NATIVE
/**
 * port: assets are compiled into the executable, so most "segmented" addresses are already native pointers.
 * On 64-bit macOS the first 4 GiB of the address space are __PAGEZERO, so no valid pointer is below 2^32:
 * anything below that is an N64 segmented address and is resolved through gSegments, which stores full native
 * pointers (no K0BASE). See docs/port-macos/05-problemas-de-portabilidade.md §5 and adr/0004-assets-e-segmentos.md
 */
static inline void* SegmentedToVirtual_Native(uintptr_t addr) {
    if (addr >= 0x100000000ull) {
        return (void*)addr;
    }
    return (void*)(gSegments[SEGMENT_NUMBER(addr)] + SEGMENT_OFFSET(addr));
}
#define SEGMENTED_TO_VIRTUAL(addr) SegmentedToVirtual_Native((uintptr_t)(addr))
#else
#define SEGMENTED_TO_VIRTUAL(addr) (void*)(gSegments[SEGMENT_NUMBER(addr)] + SEGMENT_OFFSET(addr) + K0BASE)
#endif

#endif
