/**
 * Faithful native versions of the game's libm (see port/include/libc_compat/math.h for why they are renamed).
 *
 * - src/libc64/fp.s: MIPS floor/ceil/trunc/round.w.{s,d} convert to a 32-bit integer; an out-of-range or NaN input
 *   raises "invalid operation", which with exceptions disabled yields 0x7FFFFFFF. roundf is floor(x + 0.5).
 * - src/libultra/gu/sinf.c, cosf.c: ported here because their constants are `du` unions initialized as
 *   {hi word, lo word}, which is only correct on a big-endian host. The algorithm is unchanged.
 * - src/libc/fmodf.c: compiled from the original source (renamed through the macro).
 */
#include "ultra64.h"
#include "z_math.h"

#include <string.h>

// The macros in libc_compat/math.h rename these to ultra_*; the host functions are reached with __builtin_*.

static s32 CvtToWord(double x) {
    if (!(x >= -2147483648.0 && x < 2147483648.0)) {
        return 0x7FFFFFFF; // MIPS default result for an invalid conversion (also covers NaN)
    }
    return (s32)x;
}

float floorf(float x) {
    return (float)CvtToWord(__builtin_floorf(x));
}
double floor(double x) {
    return (double)CvtToWord(__builtin_floor(x));
}
long lfloorf(float x) {
    return CvtToWord(__builtin_floorf(x));
}
long lfloor(double x) {
    return CvtToWord(__builtin_floor(x));
}

float ceilf(float x) {
    return (float)CvtToWord(__builtin_ceilf(x));
}
double ceil(double x) {
    return (double)CvtToWord(__builtin_ceil(x));
}
long lceilf(float x) {
    return CvtToWord(__builtin_ceilf(x));
}
long lceil(double x) {
    return CvtToWord(__builtin_ceil(x));
}

float truncf(float x) {
    return (float)CvtToWord(__builtin_truncf(x));
}
double trunc(double x) {
    return (double)CvtToWord(__builtin_trunc(x));
}
long ltruncf(float x) {
    return CvtToWord(__builtin_truncf(x));
}
long ltrunc(double x) {
    return CvtToWord(__builtin_trunc(x));
}

// round.w uses the current rounding mode, round-to-nearest-even by default (same as the host's rint).
float nearbyintf(float x) {
    return (float)CvtToWord(__builtin_rintf(x));
}
double nearbyint(double x) {
    return (double)CvtToWord(__builtin_rint(x));
}
long lnearbyintf(float x) {
    return CvtToWord(__builtin_rintf(x));
}
long lnearbyint(double x) {
    return CvtToWord(__builtin_rint(x));
}

float roundf(float x) {
    return (float)CvtToWord(__builtin_floorf(x + 0.5f));
}
double round(double x) {
    return (double)CvtToWord(__builtin_floor(x + 0.5));
}
long lroundf(float x) {
    return CvtToWord(__builtin_floorf(x + 0.5f));
}
long lround(double x) {
    return CvtToWord(__builtin_floor(x + 0.5));
}

// Special values defined in src/libc64/fp.s (non-N64 versions) and src/libultra/gu/libm_vals.s
static float FloatFromBits(u32 bits) {
    float f;
    memcpy(&f, &bits, sizeof(f));
    return f;
}

float gPositiveInfinity = __builtin_inff();
float gNegativeInfinity = -__builtin_inff();
float gPositiveZero = 0.0f;
float gNegativeZero = -0.0f;
float qNaN0x3FFFFF;
float qNaN0x10000;
float sNaN0x3FFFFF;
f32 __libm_qnan_f;

__attribute__((constructor)) static void InitNaNs(void) {
    qNaN0x3FFFFF = FloatFromBits(0x7FBFFFFF);
    qNaN0x10000 = FloatFromBits(0x7F810000);
    sNaN0x3FFFFF = FloatFromBits(0x7FFFFFFF);
    __libm_qnan_f = FloatFromBits(0x7F810000);
}

static f64 DoubleFromWords(u32 hi, u32 lo) {
    u64 bits = ((u64)hi << 32) | lo;
    f64 d;
    memcpy(&d, &bits, sizeof(d));
    return d;
}

static s32 FloatBitsAsInt(f32 f) {
    s32 i;
    memcpy(&i, &f, sizeof(i));
    return i;
}

// Constants from src/libultra/gu/sinf.c and cosf.c, as (hi, lo) words.
#define P1 DoubleFromWords(0xBFC55554, 0xBC83656D)
#define P2 DoubleFromWords(0x3F8110ED, 0x3804C2A0)
#define P3 DoubleFromWords(0xBF29F6FF, 0xEEA56814)
#define P4 DoubleFromWords(0x3EC5DBDF, 0x0E314BFE)
#define RPI DoubleFromWords(0x3FD45F30, 0x6DC9C883)
#define PIHI DoubleFromWords(0x400921FB, 0x50000000)
#define PILO DoubleFromWords(0x3E6110B4, 0x611A6263)

f32 sinf(f32 angle) {
    f64 dx;
    f64 xSq;
    f64 polyApprox;
    f64 dn;
    s32 n;
    f64 result;
    s32 xpt = (FloatBitsAsInt(angle) >> 22) & 0x1FF;

    if (xpt < 0xFF) {
        dx = angle;
        if (xpt >= 0xE6) {
            xSq = SQ(dx);
            polyApprox = ((P4 * xSq + P3) * xSq + P2) * xSq + P1;
            result = dx + (dx * xSq) * polyApprox;
            return (f32)result;
        }
        return angle;
    }

    if (xpt < 0x136) {
        dx = angle;
        dn = dx * RPI;
        n = ROUND(dn);
        dn = n;

        dx -= dn * PIHI;
        dx -= dn * PILO;
        xSq = SQ(dx);

        polyApprox = ((P4 * xSq + P3) * xSq + P2) * xSq + P1;
        result = dx + (dx * xSq) * polyApprox;

        if (!(n & 1)) {
            return (f32)result;
        }
        return -(f32)result;
    }

    if (angle != angle) {
        return __libm_qnan_f;
    }
    return 0.0f;
}

f32 cosf(f32 angle) {
    f32 absx;
    f64 dx;
    f64 xSq;
    f64 polyApprox;
    f64 dn;
    s32 n;
    f64 result;
    s32 xpt = (FloatBitsAsInt(angle) >> 22) & 0x1FF;

    if (xpt < 0x136) {
        absx = (angle > 0) ? angle : -angle;
        dx = absx;

        dn = dx * RPI + 0.5;
        n = ROUND(dn);
        dn = n;

        dn -= 0.5;

        dx -= dn * PIHI;
        dx -= dn * PILO;

        xSq = SQ(dx);

        polyApprox = ((P4 * xSq + P3) * xSq + P2) * xSq + P1;
        result = dx + (dx * xSq) * polyApprox;

        if (!(n & 1)) {
            return (f32)result;
        }
        return -(f32)result;
    }
    if (angle != angle) {
        return __libm_qnan_f;
    }
    return 0.0f;
}
