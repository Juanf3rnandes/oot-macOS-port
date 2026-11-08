#ifndef ULTRA64_ULTRATYPES_H
#define ULTRA64_ULTRATYPES_H

#include "target.h"

#ifdef _LANGUAGE_C

#if TARGET_NATIVE
// port: `long` is 64 bits on macOS (LP64), so s32/u32 must use fixed-width types.
// See docs/port-macos/05-problemas-de-portabilidade.md §1
#include <stdint.h>
typedef int8_t                 s8;
typedef uint8_t                u8;
typedef int16_t                s16;
typedef uint16_t               u16;
typedef int32_t                s32;
typedef uint32_t               u32;
typedef int64_t                s64;
typedef uint64_t               u64;
#else
typedef signed char            s8;
typedef unsigned char          u8;
typedef signed short int       s16;
typedef unsigned short int     u16;
typedef signed long            s32;
typedef unsigned long          u32;
typedef signed long long int   s64;
typedef unsigned long long int u64;
#endif

typedef volatile u8  vu8;
typedef volatile u16 vu16;
typedef volatile u32 vu32;
typedef volatile u64 vu64;
typedef volatile s8  vs8;
typedef volatile s16 vs16;
typedef volatile s32 vs32;
typedef volatile s64 vs64;

typedef float  f32;
typedef double f64;


typedef float MtxF_t[4][4];
typedef union MtxF {
    MtxF_t mf;
    struct {
        // Note: The order displayed here is the transpose of the order in which matrices are typically written.
        // For example, [xw, yw, zw] is the translation part of the matrix, not [wx, wy, wz].
        float xx, yx, zx, wx,
              xy, yy, zy, wy,
              xz, yz, zz, wz,
              xw, yw, zw, ww;
    };
} MtxF;

#endif

#endif
