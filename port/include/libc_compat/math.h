/**
 * port: native replacement for include/libc/math.h.
 *
 * Uses the system <math.h> and adds the game-specific declarations. M_PI and friends are redefined as float
 * literals like in the original header: the system defines them as double, which would silently promote game math
 * to double precision and change results. See docs/port-macos/05-problemas-de-portabilidade.md §6
 */
#ifndef PORT_LIBC_COMPAT_MATH_H
#define PORT_LIBC_COMPAT_MATH_H

#include_next <math.h>

#include "ultra64/ultratypes.h"

#undef M_PI
#undef M_SQRT2
#undef MAXFLOAT
#define M_PI 3.14159265358979323846f
#define M_SQRT2 1.41421356237309504880f
#define MAXFLOAT 3.40282347e+38f
#define SHT_MAX 32767.0f
#define SHT_MINV (1.0f / SHT_MAX)

/*
 * The game's libm differs from the host's: src/libc64/fp.s converts through a 32-bit integer (roundf is
 * floor(x + 0.5)), src/libc/fmodf.c truncates through s32 and src/libultra/gu/{sinf,cosf}.c use their own polynomials.
 * Physics depends on these exact results, so game code is redirected to faithful native versions
 * (port/src/ultra/libm_native.c). Renaming instead of defining `floorf` etc. keeps the host libm intact for
 * everything else in the process (SDL, the port's C++ code).
 */
#define floorf      ultra_floorf
#define floor       ultra_floor
#define lfloorf     ultra_lfloorf
#define lfloor      ultra_lfloor
#define ceilf       ultra_ceilf
#define ceil        ultra_ceil
#define lceilf      ultra_lceilf
#define lceil       ultra_lceil
#define truncf      ultra_truncf
#define trunc       ultra_trunc
#define ltruncf     ultra_ltruncf
#define ltrunc      ultra_ltrunc
#define nearbyintf  ultra_nearbyintf
#define nearbyint   ultra_nearbyint
#define lnearbyintf ultra_lnearbyintf
#define lnearbyint  ultra_lnearbyint
#define roundf      ultra_roundf
#define round       ultra_round
#define lroundf     ultra_lroundf
#define lround      ultra_lround
#define fmodf       ultra_fmodf
#define sinf        ultra_sinf
#define cosf        ultra_cosf

float floorf(float);
double floor(double);
long lfloorf(float);
long lfloor(double);
float ceilf(float);
double ceil(double);
long lceilf(float);
long lceil(double);
float truncf(float);
double trunc(double);
long ltruncf(float);
long ltrunc(double);
float nearbyintf(float);
double nearbyint(double);
long lnearbyintf(float);
long lnearbyint(double);
float roundf(float);
double round(double);
long lroundf(float);
long lround(double);
float fmodf(float, float);
float sinf(float);
float cosf(float);

#define fabsf(f) __builtin_fabsf((f32)(f))
#define fabs(f) __builtin_fabs((f64)(f))

#if !PLATFORM_N64
extern float gPositiveInfinity;
extern float gNegativeInfinity;
extern float gPositiveZero;
extern float gNegativeZero;
extern float qNaN0x3FFFFF;
extern float qNaN0x10000;
extern float sNaN0x3FFFFF;
#endif

#endif
