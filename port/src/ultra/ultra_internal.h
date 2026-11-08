/**
 * Internal interfaces between the libultra shim modules (port/src/ultra).
 */
#pragma once

#include "ultra64.h"

#include <stdbool.h>

/**
 * Called when the single game thread would block on `mq` (osRecvMesg on an empty queue, osSendMesg on a full one).
 * Waits for and fires pending timers that target `mq`. Returns true if a message was delivered to `mq`.
 * See docs/port-macos/adr/0003-modelo-de-threads.md
 */
bool Port_TimersWaitForQueue(OSMesgQueue* mq);

/** Fires every timer whose deadline has passed. Called by the main loop. */
void Port_TimersService(void);

/** Overrides the clock (nanoseconds since start) for tests; pass a negative value to go back to the real clock. */
void Port_TimeOverrideNs(int64_t ns);
