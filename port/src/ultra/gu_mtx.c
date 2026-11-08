/**
 * Fixed-point matrix conversion (src/libultra/gu/mtxutil.c and the assembly in src/libultra/mgu).
 *
 * Mtx stores each element as s15.16, split in an integer half (intPart) and a fractional half (fracPart). The original
 * code writes it two ways: by u16 index (sys_matrix.c) and as s32 words that pack two halves (mtxutil.c). On a
 * big-endian host both agree; on a little-endian host the two u16 inside each s32 are swapped. The port's single
 * convention (docs/port-macos/05-problemas-de-portabilidade.md §4.3): element (i, j) is intPart[i][j] / fracPart[i][j].
 * mtxutil.c is therefore not compiled natively; these replace it.
 */
#include "ultra64.h"

void guMtxF2L(f32 mf[4][4], Mtx* m) {
    for (s32 i = 0; i < 4; i++) {
        for (s32 j = 0; j < 4; j++) {
            s32 e = FTOFIX32(mf[i][j]);

            m->intPart[i][j] = (u16)((u32)e >> 16);
            m->fracPart[i][j] = (u16)(e & 0xFFFF);
        }
    }
}

void guMtxL2F(f32 mf[4][4], Mtx* m) {
    for (s32 i = 0; i < 4; i++) {
        for (s32 j = 0; j < 4; j++) {
            s32 e = (s32)(((u32)m->intPart[i][j] << 16) | m->fracPart[i][j]);

            mf[i][j] = FIX32TOF(e);
        }
    }
}

void guMtxIdentF(f32 mf[4][4]) {
    for (s32 i = 0; i < 4; i++) {
        for (s32 j = 0; j < 4; j++) {
            mf[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

void guMtxIdent(Mtx* m) {
    f32 mf[4][4];

    guMtxIdentF(mf);
    guMtxF2L(mf, m);
}
