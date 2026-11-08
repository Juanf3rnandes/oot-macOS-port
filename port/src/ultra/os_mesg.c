/**
 * OSMesgQueue with the original libultra semantics (src/libultra/os/{createmesgqueue,sendmesg,recvmesg,jammesg}.c).
 *
 * The port runs the game on a single thread (docs/port-macos/adr/0003-modelo-de-threads.md), so there is no other
 * thread that could ever fill an empty queue or drain a full one. A blocking call first gives pending timers a chance
 * to deliver (this is how Sleep_Cycles works); if nothing can unblock it, that is an integration bug and we abort
 * with a diagnostic instead of hanging forever.
 */
#include "ultra64.h"
#include "port/port.h"
#include "ultra_internal.h"

void osCreateMesgQueue(OSMesgQueue* mq, OSMesg* msg, s32 count) {
    mq->mtqueue = NULL;
    mq->fullqueue = NULL;
    mq->validCount = 0;
    mq->first = 0;
    mq->msgCount = count;
    mq->msg = msg;
}

static bool WaitWhile(OSMesgQueue* mq, bool full, const char* func) {
    while (full ? MQ_IS_FULL(mq) : MQ_IS_EMPTY(mq)) {
        if (!Port_TimersWaitForQueue(mq)) {
            Port_Fatal(__FILE__, __LINE__,
                       "%s(OS_MESG_BLOCK) on %s queue %p would block forever: the port runs the game on a single "
                       "thread and nothing can wake it up (docs/port-macos/adr/0003-modelo-de-threads.md)",
                       func, full ? "a full" : "an empty", (void*)mq);
        }
    }
    return true;
}

s32 osSendMesg(OSMesgQueue* mq, OSMesg msg, s32 flag) {
    s32 index;

    if (MQ_IS_FULL(mq)) {
        if (flag != OS_MESG_BLOCK) {
            return -1;
        }
        WaitWhile(mq, true, "osSendMesg");
    }

    index = (mq->first + mq->validCount) % mq->msgCount;
    mq->msg[index] = msg;
    mq->validCount++;
    return 0;
}

s32 osJamMesg(OSMesgQueue* mq, OSMesg msg, s32 flag) {
    if (MQ_IS_FULL(mq)) {
        if (flag != OS_MESG_BLOCK) {
            return -1;
        }
        WaitWhile(mq, true, "osJamMesg");
    }

    mq->first = (mq->first + mq->msgCount - 1) % mq->msgCount;
    mq->msg[mq->first] = msg;
    mq->validCount++;
    return 0;
}

s32 osRecvMesg(OSMesgQueue* mq, OSMesg* msg, s32 flag) {
    if (MQ_IS_EMPTY(mq)) {
        if (flag == OS_MESG_NOBLOCK) {
            return -1;
        }
        WaitWhile(mq, false, "osRecvMesg");
    }

    if (msg != NULL) {
        *msg = mq->msg[mq->first];
    }
    mq->first = (mq->first + 1) % mq->msgCount;
    mq->validCount--;
    return 0;
}

// Event messages (SI, VI, PRENMI, ...) are registered here and sent by the port's main loop and device shims.

typedef struct EventMesg {
    OSMesgQueue* mq;
    OSMesg msg;
} EventMesg;

static EventMesg sEventMesgs[OS_NUM_EVENTS];

void osSetEventMesg(OSEvent e, OSMesgQueue* mq, OSMesg msg) {
    PORT_ASSERT(e < OS_NUM_EVENTS);
    sEventMesgs[e].mq = mq;
    sEventMesgs[e].msg = msg;
}

/** Sends the message registered for event `e`, if any. Returns true if one was registered. */
bool Port_SendEventMesg(OSEvent e) {
    PORT_ASSERT(e < OS_NUM_EVENTS);
    if (sEventMesgs[e].mq == NULL) {
        return false;
    }
    osSendMesg(sEventMesgs[e].mq, sEventMesgs[e].msg, OS_MESG_NOBLOCK);
    return true;
}
