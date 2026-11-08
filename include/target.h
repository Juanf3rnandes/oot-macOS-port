#ifndef TARGET_H
#define TARGET_H

/**
 * Compilation target axis, independent from the game version macros (OOT_VERSION, PLATFORM_*).
 *
 * TARGET_NATIVE is 1 when building the native executable (port/, see docs/port-macos/11-convencoes.md §2)
 * and 0 when building the N64 ROM. The ROM build never defines it, so this header must not change anything
 * the ROM build sees.
 */

#ifndef TARGET_NATIVE
#define TARGET_NATIVE 0
#endif

#ifndef TARGET_MACOS
#define TARGET_MACOS 0
#endif

#endif
