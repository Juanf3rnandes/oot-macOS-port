// C++ declarations of the game-ABI helpers in game_helpers.c and types_check.c.
#pragma once

#include <cstddef>
#include <cstdint>

extern "C" {
// types_check.c
const void* TypesCheck_GetTestDL(void);
const void* TypesCheck_GetTestSubDL(void);
const void* TypesCheck_GetTestVtx(void);

// segments / gbi
uintptr_t* Helpers_Segments(void);
void* Helpers_SegmentedToVirtual(uintptr_t addr);
uint32_t Helpers_SegmentNumber(uintptr_t addr);
uintptr_t Helpers_GfxWord0(const void* gfx, int index);
uintptr_t Helpers_GfxWord1(const void* gfx, int index);
void Helpers_BuildRuntimeDL(void* buf, const void* dl, const void* vtx, const void* tex);
uintptr_t Helpers_AcmdLoadADPCM(const void* book);

// message queues and timers
size_t Helpers_SizeofMesgQueue(void);
size_t Helpers_SizeofTimer(void);
void Helpers_CreateMesgQueue(void* mq, void** msgs, int count);
int Helpers_SendMesg(void* mq, void* msg, int block);
int Helpers_JamMesg(void* mq, void* msg, int block);
int Helpers_RecvMesg(void* mq, void** msg, int block);
int Helpers_MesgCount(void* mq);
void Helpers_SetTimer(void* timer, uint64_t countdown, uint64_t interval, void* mq, void* msg);
int Helpers_StopTimer(void* timer);
uint64_t Helpers_GetTime(void);
uint32_t Helpers_GetCount(void);
void Helpers_SetTime(uint64_t t);
void Helpers_TimeOverrideNs(int64_t ns);
void Helpers_TimersService(void);

// matrices
void Helpers_MtxF2L(float mf[4][4], void* mtx);
void Helpers_MtxL2F(float mf[4][4], void* mtx);
uint16_t Helpers_MtxIntPart(const void* mtx, int i, int j);
uint16_t Helpers_MtxFracPart(const void* mtx, int i, int j);
void Helpers_Translate(void* mtx, float x, float y, float z);

// game libm
float Helpers_GameSinf(float x);
float Helpers_GameCosf(float x);
float Helpers_GameRoundf(float x);
float Helpers_GameFloorf(float x);
float Helpers_GameFmodf(float x, float y);
float Helpers_GamePi(void);
int16_t Helpers_Sins(uint16_t angle);

// sys_math.c
float Helpers_MathFactorialF(float n);
float Helpers_MathFactorial(int n);
float Helpers_MathPowF(float base, int exp);
float Helpers_MathSinF(float angle);
float Helpers_MathCosF(float angle);
}

// Opaque storage big enough for the game structs used through the helpers.
struct alignas(16) MesgQueueStorage {
    unsigned char bytes[64];
};
struct alignas(16) TimerStorage {
    unsigned char bytes[64];
};
struct alignas(16) MtxStorage {
    unsigned char bytes[64];
};
