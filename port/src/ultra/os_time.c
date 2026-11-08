/**
 * Time and timers (src/libultra/os/{gettime,getcount,settime,settimer,stoptimer}.c).
 *
 * The game measures time in CPU counter cycles (OS_CPU_COUNTER = 46.875 MHz). The host monotonic clock is converted
 * to that unit, so loops that busy-wait on osGetCount keep working. Timers are kept in a list and fired by the main
 * loop (Port_TimersService) or when the single game thread would otherwise block (Port_TimersWaitForQueue).
 */
#include "ultra64.h"
#include "port/port.h"
#include "ultra_internal.h"

#include <mach/mach_time.h>
#include <time.h>

static uint64_t sStartTicks;
static mach_timebase_info_data_t sTimebase;
static int64_t sOverrideNs = -1;
static OSTime sTimeOffset; // set by osSetTime

static uint64_t NowNs(void) {
    if (sOverrideNs >= 0) {
        return (uint64_t)sOverrideNs;
    }
    if (sTimebase.denom == 0) {
        mach_timebase_info(&sTimebase);
        sStartTicks = mach_absolute_time();
    }
    uint64_t ticks = mach_absolute_time() - sStartTicks;
    return ticks * sTimebase.numer / sTimebase.denom;
}

void Port_TimeOverrideNs(int64_t ns) {
    sOverrideNs = ns;
}

static OSTime CyclesSinceStart(void) {
    uint64_t ns = NowNs();
    // ns * 46875000 / 1e9 == ns * 3 / 64, exact and overflow-free for ~190 years
    return (OSTime)(ns / 64 * 3 + (ns % 64) * 3 / 64);
}

OSTime osGetTime(void) {
    return CyclesSinceStart() + sTimeOffset;
}

void osSetTime(OSTime time) {
    sTimeOffset = time - CyclesSinceStart();
}

u32 osGetCount(void) {
    return (u32)CyclesSinceStart();
}

// Active timers form a circular doubly linked list through OSTimer.next/prev, like libultra, so osStopTimer can tell
// whether a timer is active by its `next` pointer. `value` holds the absolute deadline (osGetTime units).
static OSTimer sTimerList = { &sTimerList, &sTimerList, 0, 0, NULL, NULL };

static void InsertTimer(OSTimer* timer) {
    OSTimer* at = sTimerList.next;

    while (at != &sTimerList && at->value <= timer->value) {
        at = at->next;
    }
    timer->next = at;
    timer->prev = at->prev;
    at->prev->next = timer;
    at->prev = timer;
}

static void RemoveTimer(OSTimer* timer) {
    timer->prev->next = timer->next;
    timer->next->prev = timer->prev;
    timer->next = NULL;
    timer->prev = NULL;
}

s32 osSetTimer(OSTimer* timer, OSTime countdown, OSTime interval, OSMesgQueue* mq, OSMesg msg) {
    timer->interval = interval;
    timer->value = osGetTime() + ((countdown != 0) ? countdown : interval);
    timer->mq = mq;
    timer->msg = msg;
    InsertTimer(timer);
    return 0;
}

s32 osStopTimer(OSTimer* timer) {
    if (timer->next == NULL) {
        return -1;
    }
    RemoveTimer(timer);
    return 0;
}

/** Fires the first timer in the list. */
static void FireFirstTimer(void) {
    OSTimer* timer = sTimerList.next;

    RemoveTimer(timer);
    if (timer->interval != 0) {
        timer->value += timer->interval;
        InsertTimer(timer);
    }
    if (timer->mq != NULL) {
        osSendMesg(timer->mq, timer->msg, OS_MESG_NOBLOCK);
    }
}

void Port_TimersService(void) {
    OSTime now = osGetTime();

    while (sTimerList.next != &sTimerList && sTimerList.next->value <= now) {
        FireFirstTimer();
    }
}

static void SleepUntil(OSTime deadline) {
    while (true) {
        OSTime now = osGetTime();
        if (now >= deadline) {
            return;
        }
        if (sOverrideNs >= 0) {
            // Tests drive the clock: jump straight to the deadline.
            sOverrideNs = (int64_t)(OS_CYCLES_TO_NSEC(deadline - sTimeOffset) + 1);
            continue;
        }
        uint64_t ns = OS_CYCLES_TO_NSEC(deadline - now);
        struct timespec ts = { (time_t)(ns / 1000000000ull), (long)(ns % 1000000000ull) };
        nanosleep(&ts, NULL);
    }
}

bool Port_TimersWaitForQueue(OSMesgQueue* mq) {
    s32 countBefore = mq->validCount;
    OSTimer* timer;

    // Is there a timer that will eventually deliver to this queue?
    for (timer = sTimerList.next; timer != &sTimerList; timer = timer->next) {
        if (timer->mq == mq) {
            break;
        }
    }
    if (timer == &sTimerList) {
        return false;
    }

    // Fire timers in deadline order until one reaches `mq`.
    while (mq->validCount == countBefore && sTimerList.next != &sTimerList) {
        SleepUntil(sTimerList.next->value);
        FireFirstTimer();
    }
    return mq->validCount != countBefore;
}
