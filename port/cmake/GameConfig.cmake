# Game version configuration, mirroring the version table in the root Makefile.
#
# Input:  OOT_VERSION (cache variable, e.g. gc-eu-mq-dbg)
# Output: OOT_GAME_DEFINES       - compile definitions the game code sees (same macros as the matching build)
#         OOT_VERSION_PLATFORM   - N64 | GC | IQUE
#         OOT_VERSION_DEBUG      - 0 | 1
#         OOT_BUILD_CREATOR, OOT_BUILD_DATE, OOT_BUILD_TIME - strings for src/boot/build.c

# version|region|platform|debug_features|revision|build_creator|build_date|build_time
set(_oot_versions
    "ntsc-1.0|JP|N64|0|0|zelda@srd44|98-10-21|04:56:31"
    "ntsc-1.1|JP|N64|0|1|zelda@srd44|98-10-26|10:58:45"
    "pal-1.0|EU|N64|0|0|zelda@srd44|98-11-10|14:34:22"
    "ntsc-1.2|JP|N64|0|2|zelda@srd44|98-11-12|18:17:03"
    "pal-1.1|EU|N64|0|1|zelda@srd44|98-11-18|17:36:49"
    "gc-jp|JP|GC|0|15|zelda@srd022j|02-10-29|23:49:53"
    "gc-jp-mq|JP|GC|0|15|zelda@srd022j|02-10-30|00:15:15"
    "gc-us|US|GC|0|15|zelda@srd022j|02-12-19|13:28:09"
    "gc-us-mq|US|GC|0|15|zelda@srd022j|02-12-19|14:05:42"
    "gc-eu-mq-dbg|EU|GC|1|15|zelda@srd022j|03-02-21|00:16:31"
    "gc-eu|EU|GC|0|15|zelda@srd022j|03-02-21|20:12:23"
    "gc-eu-mq|EU|GC|0|15|zelda@srd022j|03-02-21|20:37:19"
    "gc-jp-ce|JP|GC|0|15|zelda@srd022j|03-10-08|21:53:00"
    "gc-eu-dbg|EU|GC|1|0|zelda@srd022j|03-02-21|00:49:18"
    "gc-eu-dbg-2|EU|GC|1|15|zelda@srd022j|03-02-13|19:46:49"
)

set(_found FALSE)
foreach(_row IN LISTS _oot_versions)
    string(REPLACE "|" ";" _entry "${_row}")
    list(GET _entry 0 _v)
    if(_v STREQUAL OOT_VERSION)
        list(GET _entry 1 _region)
        list(GET _entry 2 _platform)
        list(GET _entry 3 _debug)
        list(GET _entry 4 _revision)
        list(GET _entry 5 OOT_BUILD_CREATOR)
        list(GET _entry 6 OOT_BUILD_DATE)
        list(GET _entry 7 OOT_BUILD_TIME)
        set(_found TRUE)
    endif()
endforeach()

if(NOT _found)
    message(FATAL_ERROR "Unsupported OOT_VERSION '${OOT_VERSION}' (iQue is out of scope, see docs/port-macos/01-visao-geral.md §5)")
endif()

# Overrides that exist in the Makefile (REGION ?= ..., DEBUG_FEATURES ?= ...)
if(DEFINED OOT_REGION_OVERRIDE AND NOT OOT_REGION_OVERRIDE STREQUAL "")
    set(_region ${OOT_REGION_OVERRIDE})
endif()

# ntsc-1.0 → NTSC_1_0
string(TOUPPER "${OOT_VERSION}" _version_macro)
string(REGEX REPLACE "[-.]" "_" _version_macro "${_version_macro}")

if(_platform STREQUAL "N64")
    set(_libultra_version I)
    set(_libultra_patch 1)
    set(_platform_defs PLATFORM_N64=1 PLATFORM_GC=0 PLATFORM_IQUE=0)
else()
    set(_libultra_version L)
    set(_libultra_patch 0)
    set(_platform_defs PLATFORM_N64=0 PLATFORM_GC=1 PLATFORM_IQUE=0)
endif()

set(OOT_VERSION_PLATFORM ${_platform})
set(OOT_VERSION_DEBUG ${_debug})

set(OOT_GAME_DEFINES
    _LANGUAGE_C
    TARGET_NATIVE=1
    TARGET_MACOS=1
    NON_MATCHING
    AVOID_UB
    ${_platform_defs}
    OOT_VERSION=${_version_macro}
    OOT_REVISION=${_revision}
    OOT_REGION=REGION_${_region}
    LIBULTRA_VERSION=LIBULTRA_VERSION_${_libultra_version}
    LIBULTRA_PATCH=${_libultra_patch}
    F3DEX_GBI_2
)
if(NOT _platform STREQUAL "N64")
    list(APPEND OOT_GAME_DEFINES F3DEX_GBI_PL GBI_DOWHILE)
endif()
if(_debug)
    list(APPEND OOT_GAME_DEFINES DEBUG_FEATURES=1 GBI_DEBUG)
else()
    list(APPEND OOT_GAME_DEFINES DEBUG_FEATURES=0 NDEBUG)
endif()
