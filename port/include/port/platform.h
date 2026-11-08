/**
 * C API between the game/shim (layers 1-2) and the port services (layer 3).
 * See docs/port-macos/04-arquitetura-do-port.md §7
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Called by Graph_TaskSet00 instead of sending the task to the scheduler. */
void Port_SubmitGfxTask(const void* masterDisplayList);

/** Called by the AI shim: pushes interleaved stereo s16 samples to the output ring buffer. */
void Port_AudioPush(const int16_t* stereoSamples, uint32_t frameCount);
/** Number of stereo frames queued in the output ring buffer. */
uint32_t Port_AudioQueuedFrames(void);

#ifdef __cplusplus
}
#endif
