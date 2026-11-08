/**
 * Threads (src/libultra/os/{createthread,startthread,...}.c).
 *
 * Phase 1 of the port runs everything on one host thread (docs/port-macos/adr/0003-modelo-de-threads.md): the bodies
 * of the original threads are called from the port's main loop. These functions only keep the bookkeeping the game
 * reads back (id, priority, state) and remember entry points so Port_Main can find them.
 */
#include "ultra64.h"
#include "port/port.h"

#define MAX_THREADS 32

typedef struct ThreadInfo {
    OSThread* thread;
    void (*entry)(void*);
    void* arg;
} ThreadInfo;

static ThreadInfo sThreads[MAX_THREADS];
static OSThread sMainThread = { NULL, OS_PRIORITY_APPMAX, NULL, NULL, OS_STATE_RUNNING, 0, 1, 0, NULL };
static OSThread* sCurrentThread = &sMainThread;

static ThreadInfo* FindThread(OSThread* thread) {
    for (int i = 0; i < MAX_THREADS; i++) {
        if (sThreads[i].thread == thread) {
            return &sThreads[i];
        }
    }
    return NULL;
}

void osCreateThread(OSThread* thread, OSId id, void (*entry)(void*), void* arg, void* sp, OSPri pri) {
    ThreadInfo* info = FindThread(thread);

    (void)sp;
    if (info == NULL) {
        info = FindThread(NULL);
        PORT_ASSERT(info != NULL);
    }
    info->thread = thread;
    info->entry = entry;
    info->arg = arg;

    thread->next = NULL;
    thread->queue = NULL;
    thread->tlnext = NULL;
    thread->priority = pri;
    thread->id = id;
    thread->state = OS_STATE_STOPPED;
    thread->flags = 0;
    thread->fp = 0;
    thread->thprof = NULL;
    PORT_LOG(DEBUG, "osCreateThread id=%d pri=%d", (int)id, (int)pri);
}

void osStartThread(OSThread* thread) {
    // Not run: the port main loop drives the original thread bodies directly.
    thread->state = OS_STATE_RUNNABLE;
    PORT_LOG(DEBUG, "osStartThread id=%d (not started: single-threaded port)", (int)thread->id);
}

void osStopThread(OSThread* thread) {
    if (thread == NULL) {
        thread = sCurrentThread;
    }
    thread->state = OS_STATE_STOPPED;
}

void osDestroyThread(OSThread* thread) {
    ThreadInfo* info = FindThread(thread);

    if (info != NULL) {
        info->thread = NULL;
    }
}

void osYieldThread(void) {
}

OSId osGetThreadId(OSThread* thread) {
    return (thread != NULL ? thread : sCurrentThread)->id;
}

void osSetThreadPri(OSThread* thread, OSPri pri) {
    (thread != NULL ? thread : sCurrentThread)->priority = pri;
}

OSPri osGetThreadPri(OSThread* thread) {
    return (thread != NULL ? thread : sCurrentThread)->priority;
}

/** Returns the entry point and argument registered for `thread` by osCreateThread. */
bool Port_GetThreadEntry(OSThread* thread, void (**entry)(void*), void** arg) {
    ThreadInfo* info = FindThread(thread);

    if (info == NULL) {
        return false;
    }
    *entry = info->entry;
    *arg = info->arg;
    return true;
}

/** Marks which original thread the main loop is currently running the body of (for osGetThreadId(NULL) etc.). */
void Port_SetCurrentThread(OSThread* thread) {
    sCurrentThread = (thread != NULL) ? thread : &sMainThread;
}
