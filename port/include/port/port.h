/**
 * Common port definitions. Usable from C and C++.
 */
#pragma once

#include "port/log.h"

/** Invariant check, active in every build type (the port is still young; turn off per-config later if needed). */
#define PORT_ASSERT(cond, ...)                                    \
    do {                                                          \
        if (!(cond)) {                                            \
            Port_Fatal(__FILE__, __LINE__, "assertion failed: %s", #cond); \
        }                                                         \
    } while (0)

/** Marks a libultra function the port does not implement (yet). Turns link errors into a runtime to-do list. */
#define PORT_UNIMPLEMENTED(name) Port_Fatal(__FILE__, __LINE__, "unimplemented: %s", name)

/** Lowest valid native pointer: the first 4 GiB are __PAGEZERO on 64-bit macOS (see segmented_address.h). */
#define PORT_NATIVE_POINTER_MIN 0x100000000ull
