/**
 * Thin C wrappers that let the C++ tests reach game-ABI code (the game headers are C-only).
 */
#include "ultra64.h"
#include "segmented_address.h"
#include "sys_math.h"
#include "ultra64/abi.h"

// Defined in src/code/main.c in the full game; the unit tests do not link it.
uintptr_t gSegments[NUM_SEGMENTS];

// Exposed from port/src/ultra (internal headers are not visible here).
void Port_TimeOverrideNs(int64_t ns);
void Port_TimersService(void);

uintptr_t* Helpers_Segments(void) {
    return gSegments;
}

void* Helpers_SegmentedToVirtual(uintptr_t addr) {
    return SEGMENTED_TO_VIRTUAL(addr);
}

uint32_t Helpers_SegmentNumber(uintptr_t addr) {
    return (uint32_t)SEGMENT_NUMBER(addr);
}

uintptr_t Helpers_GfxWord0(const void* gfx, int index) {
    return ((const Gfx*)gfx)[index].words.w0;
}

uintptr_t Helpers_GfxWord1(const void* gfx, int index) {
    return ((const Gfx*)gfx)[index].words.w1;
}

void Helpers_BuildRuntimeDL(void* buf, const void* dl, const void* vtx, const void* tex) {
    Gfx* gfx = buf;

    gSPDisplayList(gfx++, dl);
    gSPVertex(gfx++, vtx, 4, 0);
    gDPSetTextureImage(gfx++, G_IM_FMT_RGBA, G_IM_SIZ_16b, 32, tex);
    gSPSegment(gfx++, 0x08, tex);
    gSPEndDisplayList(gfx++);
}

uintptr_t Helpers_AcmdLoadADPCM(const void* book) {
    Acmd cmd;
    Acmd* ptr = &cmd;

    aLoadADPCM(ptr++, 32, book);
    return cmd.words.w1;
}

// Message queues and timers
size_t Helpers_SizeofMesgQueue(void) {
    return sizeof(OSMesgQueue);
}
size_t Helpers_SizeofTimer(void) {
    return sizeof(OSTimer);
}
void Helpers_CreateMesgQueue(void* mq, void** msgs, int count) {
    osCreateMesgQueue(mq, msgs, count);
}
int Helpers_SendMesg(void* mq, void* msg, int block) {
    return osSendMesg(mq, msg, block ? OS_MESG_BLOCK : OS_MESG_NOBLOCK);
}
int Helpers_JamMesg(void* mq, void* msg, int block) {
    return osJamMesg(mq, msg, block ? OS_MESG_BLOCK : OS_MESG_NOBLOCK);
}
int Helpers_RecvMesg(void* mq, void** msg, int block) {
    return osRecvMesg(mq, msg, block ? OS_MESG_BLOCK : OS_MESG_NOBLOCK);
}
int Helpers_MesgCount(void* mq) {
    return MQ_GET_COUNT((OSMesgQueue*)mq);
}
void Helpers_SetTimer(void* timer, uint64_t countdown, uint64_t interval, void* mq, void* msg) {
    osSetTimer(timer, countdown, interval, mq, msg);
}
int Helpers_StopTimer(void* timer) {
    return osStopTimer(timer);
}
uint64_t Helpers_GetTime(void) {
    return osGetTime();
}
uint32_t Helpers_GetCount(void) {
    return osGetCount();
}
void Helpers_SetTime(uint64_t t) {
    osSetTime(t);
}
void Helpers_TimeOverrideNs(int64_t ns) {
    Port_TimeOverrideNs(ns);
}
void Helpers_TimersService(void) {
    Port_TimersService();
}

// Matrices
void Helpers_MtxF2L(float mf[4][4], void* mtx) {
    guMtxF2L(mf, mtx);
}
void Helpers_MtxL2F(float mf[4][4], void* mtx) {
    guMtxL2F(mf, mtx);
}
uint16_t Helpers_MtxIntPart(const void* mtx, int i, int j) {
    return ((const Mtx*)mtx)->intPart[i][j];
}
uint16_t Helpers_MtxFracPart(const void* mtx, int i, int j) {
    return ((const Mtx*)mtx)->fracPart[i][j];
}
void Helpers_Translate(void* mtx, float x, float y, float z) {
    guTranslate(mtx, x, y, z);
}

// Game libm (renamed to ultra_* by port/include/libc_compat/math.h)
float Helpers_GameSinf(float x) {
    return sinf(x);
}
float Helpers_GameCosf(float x) {
    return cosf(x);
}
float Helpers_GameRoundf(float x) {
    return roundf(x);
}
float Helpers_GameFloorf(float x) {
    return floorf(x);
}
float Helpers_GameFmodf(float x, float y) {
    return fmodf(x, y);
}
float Helpers_GamePi(void) {
    return M_PI;
}
s16 Helpers_Sins(u16 angle) {
    return sins(angle);
}

// src/code/sys_math.c
float Helpers_MathFactorialF(float n) {
    return Math_FactorialF(n);
}
float Helpers_MathFactorial(int n) {
    return Math_Factorial(n);
}
float Helpers_MathPowF(float base, int exp) {
    return Math_PowF(base, exp);
}
float Helpers_MathSinF(float angle) {
    return Math_SinF(angle);
}
float Helpers_MathCosF(float angle) {
    return Math_CosF(angle);
}
