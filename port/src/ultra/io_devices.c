/**
 * Device APIs of libultra (src/libultra/io and parts of src/libultra/os) for the single-threaded port.
 *
 * Behaviour follows docs/port-macos/04-arquitetura-do-port.md §6. Functions the port should never reach (the RSP/RDP
 * task path, which the scheduler no longer uses) abort with PORT_UNIMPLEMENTED so a wrong integration is caught
 * immediately. Devices that later milestones implement for real are marked with the milestone:
 *   M2: PI (the "virtual ROM")   M4: SI/controllers, rumble   M5: AI (audio output)
 */
#include "ultra64.h"
#include "port/port.h"

#include <stdbool.h>
#include <string.h>

bool Port_SendEventMesg(OSEvent e); // os_mesg.c

// ---- Interrupts, FPU, scheduler internals ---------------------------------------------------------------------------
// One host thread, no interrupts: masking is meaningless.

s32 __osDisableInt(void) {
    return 0;
}

void __osRestoreInt(s32 im) {
    (void)im;
}

OSIntMask osSetIntMask(OSIntMask im) {
    (void)im;
    return OS_IM_ALL;
}

u32 __osGetFpcCsr(void) {
    return 0;
}

void __osSetFpcCsr(u32 csr) {
    (void)csr;
}

void __osSetHWIntrRoutine(OSHWIntr intr, s32 (*callback)(void), void* sp) {
    (void)intr;
    (void)callback;
    (void)sp;
}

void __osGetHWIntrRoutine(OSHWIntr intr, s32 (**callbackOut)(void), void** spOut) {
    (void)intr;
    *callbackOut = NULL;
    *spOut = NULL;
}

void __osInitialize_common(void) {
}

void __osInitialize_autodetect(void) {
}

void __osCleanupThread(void) {
}

OSThread* __osGetActiveQueue(void) {
    return NULL;
}

OSThread* __osGetCurrFaultedThread(void) {
    return NULL; // faults are host signals in the port, handled by the crash handler
}

s32 osAfterPreNMI(void) {
    return 0;
}

// ---- PI: cartridge ROM (M2: virtual ROM, docs/port-macos/adr/0004-assets-e-segmentos.md) ----------------------------

static OSPiHandle sCartHandle;
OSPiHandle* __osPiTable = NULL;
OSPiHandle* __osCurrentHandle[2] = { &sCartHandle, &sCartHandle };

OSPiHandle* osCartRomInit(void) {
    sCartHandle.type = DEVICE_TYPE_CART;
    sCartHandle.baseAddress = 0; // native: "ROM addresses" are offsets into the virtual ROM, not KSEG1 addresses
    __osPiTable = &sCartHandle;
    return &sCartHandle;
}

void osCreatePiManager(OSPri pri, OSMesgQueue* cmdQueue, OSMesg* cmdBuf, s32 cmdMsgCnt) {
    (void)pri;
    osCreateMesgQueue(cmdQueue, cmdBuf, cmdMsgCnt);
}

void __osPiGetAccess(void) {
}

void __osPiRelAccess(void) {
}

s32 osEPiStartDma(OSPiHandle* handle, OSIoMesg* mb, s32 direction) {
    (void)handle;
    (void)mb;
    (void)direction;
    PORT_UNIMPLEMENTED("osEPiStartDma (M2: virtual ROM)");
}

s32 osEPiReadIo(OSPiHandle* handle, u32 devAddr, u32* data) {
    (void)handle;
    (void)devAddr;
    (void)data;
    PORT_UNIMPLEMENTED("osEPiReadIo (M2: virtual ROM)");
}

// ---- VI: video interface. Presentation belongs to the renderer; this only keeps the state the game reads back. ------

static void* sViCurrentFramebuffer;
static void* sViNextFramebuffer;
static OSMesgQueue* sViEventQueue;
static OSMesg sViEventMsg;

void osCreateViManager(OSPri pri) {
    (void)pri;
}

void osViSetMode(OSViMode* mode) {
    (void)mode;
}

void osViSetEvent(OSMesgQueue* mq, OSMesg msg, u32 retraceCount) {
    (void)retraceCount; // the port main loop sends one retrace message per virtual 60 Hz retrace
    sViEventQueue = mq;
    sViEventMsg = msg;
}

/** Sends the VI retrace message registered with osViSetEvent. Called by the port main loop. */
void Port_ViRetrace(void) {
    sViCurrentFramebuffer = sViNextFramebuffer;
    if (sViEventQueue != NULL) {
        osSendMesg(sViEventQueue, sViEventMsg, OS_MESG_NOBLOCK);
    }
}

void osViSwapBuffer(void* frameBufPtr) {
    sViNextFramebuffer = frameBufPtr;
}

void* osViGetCurrentFramebuffer(void) {
    return sViCurrentFramebuffer;
}

void* osViGetNextFramebuffer(void) {
    return sViNextFramebuffer;
}

void osViBlack(u8 active) {
    (void)active;
}

void osViSetSpecialFeatures(u32 func) {
    (void)func;
}

void osViSetXScale(f32 value) {
    (void)value;
}

void osViSetYScale(f32 scale) {
    (void)scale;
}

void osViExtendVStart(u32 value) {
    (void)value;
}

// ---- AI: audio output (M5) ------------------------------------------------------------------------------------------

static u32 sAiFrequency = 32000;

s32 osAiSetFrequency(u32 frequency) {
    sAiFrequency = frequency;
    return (s32)frequency;
}

s32 osAiSetNextBuffer(void* buf, u32 size) {
    (void)buf;
    (void)size;
    return 0; // M5: push to the output ring buffer (Port_AudioPush)
}

u32 osAiGetLength(void) {
    return 0; // M5: bytes still queued in the output ring buffer
}

// ---- SI: controllers (M4: real input). One standard controller in port 1 with nothing pressed. ----------------------

#define PORT_NUM_CONTROLLERS 1

s32 osContInit(OSMesgQueue* mq, u8* ctlBitfield, OSContStatus* status) {
    (void)mq;
    *ctlBitfield = 0;
    for (s32 i = 0; i < MAXCONTROLLERS; i++) {
        if (i < PORT_NUM_CONTROLLERS) {
            status[i].type = CONT_TYPE_NORMAL;
            status[i].status = 0;
            status[i].errno = 0;
            *ctlBitfield |= 1 << i;
        } else {
            status[i].type = 0;
            status[i].status = 0;
            status[i].errno = CONT_NO_RESPONSE_ERROR;
        }
    }
    return 0;
}

s32 osContSetCh(u8 ch) {
    (void)ch;
    return 0;
}

/** On hardware the SI interrupt arrives later; here the transfer is instant, so the event is sent right away. */
static s32 CompleteSiTransfer(OSMesgQueue* mq) {
    if (!Port_SendEventMesg(OS_EVENT_SI)) {
        osSendMesg(mq, NULL, OS_MESG_NOBLOCK);
    }
    return 0;
}

s32 osContStartQuery(OSMesgQueue* mq) {
    return CompleteSiTransfer(mq);
}

void osContGetQuery(OSContStatus* data) {
    u8 bitfield;

    osContInit(NULL, &bitfield, data);
}

s32 osContStartReadData(OSMesgQueue* mq) {
    return CompleteSiTransfer(mq);
}

void osContGetReadData(OSContPad* contData) {
    for (s32 i = 0; i < MAXCONTROLLERS; i++) {
        memset(&contData[i], 0, sizeof(OSContPad));
        contData[i].errno = (i < PORT_NUM_CONTROLLERS) ? 0 : CONT_NO_RESPONSE_ERROR;
    }
}

// ---- Controller Pak and Rumble Pak: not present (M4: rumble through the host gamepad) -------------------------------

s32 osMotorInit(OSMesgQueue* ctrlrqueue, OSPfs* pfs, s32 channel) {
    (void)ctrlrqueue;
    (void)pfs;
    (void)channel;
    return PFS_ERR_NOPACK;
}

s32 __osMotorAccess(OSPfs* pfs, s32 vibrate) {
    (void)pfs;
    (void)vibrate;
    return PFS_ERR_NOPACK;
}

s32 osPfsInitPak(OSMesgQueue* queue, OSPfs* pfs, s32 channel) {
    (void)queue;
    (void)pfs;
    (void)channel;
    return PFS_ERR_NOPACK;
}

s32 osPfsFreeBlocks(OSPfs* pfs, s32* leftoverBytes) {
    (void)pfs;
    *leftoverBytes = 0;
    return PFS_ERR_NOPACK;
}

s32 osPfsAllocateFile(OSPfs* pfs, u16 companyCode, u32 gameCode, u8* gameName, u8* extName, s32 fileSize,
                      s32* fileNo) {
    (void)pfs, (void)companyCode, (void)gameCode, (void)gameName, (void)extName, (void)fileSize, (void)fileNo;
    return PFS_ERR_NOPACK;
}

s32 osPfsDeleteFile(OSPfs* pfs, u16 companyCode, u32 gameCode, u8* gameName, u8* extName) {
    (void)pfs, (void)companyCode, (void)gameCode, (void)gameName, (void)extName;
    return PFS_ERR_NOPACK;
}

s32 osPfsFindFile(OSPfs* pfs, u16 companyCode, u32 gameCode, u8* gameName, u8* extName, s32* fileNo) {
    (void)pfs, (void)companyCode, (void)gameCode, (void)gameName, (void)extName, (void)fileNo;
    return PFS_ERR_NOPACK;
}

s32 osPfsFileState(OSPfs* pfs, s32 fileNo, OSPfsState* state) {
    (void)pfs, (void)fileNo, (void)state;
    return PFS_ERR_NOPACK;
}

s32 osPfsReadWriteFile(OSPfs* pfs, s32 fileNo, u8 flag, s32 offset, s32 size, u8* data) {
    (void)pfs, (void)fileNo, (void)flag, (void)offset, (void)size, (void)data;
    return PFS_ERR_NOPACK;
}

// ---- SP/DP: the RSP/RDP task path. The scheduler is removed in the port (adr/0003); reaching these is a bug. --------

void osSpTaskLoad(OSTask* intp) {
    (void)intp;
    PORT_UNIMPLEMENTED("osSpTaskLoad (graphics/audio tasks run through Port_SubmitGfxTask)");
}

void osSpTaskStartGo(OSTask* tp) {
    (void)tp;
    PORT_UNIMPLEMENTED("osSpTaskStartGo (graphics/audio tasks run through Port_SubmitGfxTask)");
}

void osSpTaskYield(void) {
    PORT_UNIMPLEMENTED("osSpTaskYield");
}

OSYieldResult osSpTaskYielded(OSTask* task) {
    (void)task;
    PORT_UNIMPLEMENTED("osSpTaskYielded");
}

u32 __osSpGetStatus(void) {
    PORT_UNIMPLEMENTED("__osSpGetStatus");
}

void __osSpSetStatus(u32 status) {
    (void)status;
    PORT_UNIMPLEMENTED("__osSpSetStatus");
}

u32 osDpGetStatus(void) {
    PORT_UNIMPLEMENTED("osDpGetStatus");
}

void osDpSetStatus(u32 status) {
    (void)status;
    PORT_UNIMPLEMENTED("osDpSetStatus");
}
