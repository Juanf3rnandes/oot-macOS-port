/**
 * Compile-time checks of the types the port depends on (docs/port-macos/05-problemas-de-portabilidade.md §1,
 * adr/0006-tipos-e-ponteiros-64bit.md). Compiled with the game's ABI: if this file builds, the checks passed.
 */
#include "ultra64.h"
#include "ultra64/abi.h"
#include "segmented_address.h"

#include <stddef.h>

_Static_assert(sizeof(s8) == 1, "s8");
_Static_assert(sizeof(s16) == 2, "s16");
_Static_assert(sizeof(s32) == 4, "s32 must be 4 bytes (long is 8 bytes on macOS)");
_Static_assert(sizeof(u32) == 4, "u32 must be 4 bytes (long is 8 bytes on macOS)");
_Static_assert(sizeof(s64) == 8, "s64");
_Static_assert(sizeof(f32) == 4, "f32");
_Static_assert(sizeof(f64) == 8, "f64");
_Static_assert((char)-1 > 0, "char must be unsigned like on MIPS (-funsigned-char)");

_Static_assert(sizeof(Mtx) == 64, "Mtx must be 64 bytes");
_Static_assert(offsetof(Mtx, fracPart) == 32, "Mtx fractional half starts after the 16 integer halves");
_Static_assert(sizeof(Vtx) == 16, "Vtx must be 16 bytes");
_Static_assert(sizeof(OSContPad) == 6, "OSContPad must be 6 bytes");
_Static_assert(sizeof(OSMesgQueue) == 0x18 + 0x10, "OSMesgQueue: 3 pointers widened to 8 bytes");

_Static_assert(sizeof(Gfx) == 16, "Gfx is widened to hold a native pointer in w1");
_Static_assert(sizeof(((Gfx*)0)->words.w1) == sizeof(void*), "Gfx.w1 must hold a pointer");
_Static_assert(sizeof(((Acmd*)0)->words.w1) == sizeof(void*), "Acmd.w1 must hold a pointer");

// Static display lists with pointers must be valid constant initializers (they are everywhere in the assets).
static Gfx sTestSubDL[] = {
    gsSPEndDisplayList(),
};
static Vtx sTestVtx[3];
static Gfx sTestDL[] = {
    gsSPVertex(sTestVtx, 3, 0),
    gsSPDisplayList(sTestSubDL),
    gsSPEndDisplayList(),
};

const Gfx* TypesCheck_GetTestDL(void) {
    return sTestDL;
}

const void* TypesCheck_GetTestSubDL(void) {
    return sTestSubDL;
}

const void* TypesCheck_GetTestVtx(void) {
    return sTestVtx;
}
