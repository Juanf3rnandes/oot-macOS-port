# Native macOS build of Ocarina of Time (docs/port-macos/adr/0005-build-cmake-separado.md).
#
#   cmake --preset debug            (or: cmake -S port -B build-native -G Ninja -DOOT_VERSION=gc-eu-mq-dbg)
#   cmake --build build-native
#   ctest --test-dir build-native
cmake_minimum_required(VERSION 3.24)
project(OoTNative LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON) # gnu11: the game uses GNU extensions (statement expressions, anonymous structs)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set(OOT_VERSION "gc-eu-mq-dbg" CACHE STRING "Game version to build (same names as the Makefile VERSION)")
option(OOT_BUILD_TESTS "Build the unit tests" ON)
set(OOT_SANITIZE "" CACHE STRING "Sanitizers to enable, e.g. address,undefined (empty: none)")

set(OOT_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
cmake_path(NORMAL_PATH OOT_REPO_ROOT)
set(OOT_EXTRACTED_DIR "${OOT_REPO_ROOT}/extracted/${OOT_VERSION}")
set(OOT_ASSET_BUILD_DIR "${OOT_REPO_ROOT}/build/${OOT_VERSION}")

include(cmake/GameConfig.cmake)
include(cmake/GameSources.cmake)

if(OOT_SANITIZE)
    add_compile_options(-fsanitize=${OOT_SANITIZE} -fno-omit-frame-pointer -fno-sanitize-recover=all)
    add_link_options(-fsanitize=${OOT_SANITIZE})
endif()

# ---------------------------------------------------------------------------------------------------------------------
# Compile settings for code built with the game's ABI (game sources and the libultra shim).
# See docs/port-macos/05-problemas-de-portabilidade.md §6 for the reason behind each flag.
# ---------------------------------------------------------------------------------------------------------------------
set(OOT_GAME_COMPILE_OPTIONS
    -funsigned-char          # char is unsigned on MIPS/IDO
    -fno-strict-aliasing     # IDO did not exploit aliasing rules; the game relies on that
    -fwrapv                  # signed overflow wraps like on MIPS
    -ffp-contract=off        # the VR4300 has no FMA; fused multiply-add changes float results
    -fno-common
    # The game's headers live in include/ and are only ever included with quotes. -iquote keeps them from shadowing
    # system headers (include/sched.h, include/printf.h, include/vis.h collide with the macOS SDK).
    "SHELL:-iquote ${OOT_REPO_ROOT}/include"
    "SHELL:-iquote ${OOT_REPO_ROOT}/src"
    "SHELL:-iquote ${OOT_REPO_ROOT}"
    "SHELL:-iquote ${OOT_ASSET_BUILD_DIR}"
    "SHELL:-iquote ${OOT_EXTRACTED_DIR}"
    -Wno-unknown-pragmas     # #pragma increment_block_number etc. only matter to IDO
    -Wno-missing-braces
    -Wno-microsoft-anon-tag
    -Wno-gnu-anonymous-struct
)

# Native replacements for include/libc (found before the system headers, after the game's -iquote dirs).
set(OOT_GAME_INCLUDE_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/libc_compat"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

function(oot_use_game_abi target)
    target_compile_definitions(${target} PRIVATE ${OOT_GAME_DEFINES})
    target_compile_options(${target} PRIVATE ${OOT_GAME_COMPILE_OPTIONS})
    target_include_directories(${target} PRIVATE ${OOT_GAME_INCLUDE_DIRS})
endfunction()

# ---------------------------------------------------------------------------------------------------------------------
# Layer 3: port services (C++)
# ---------------------------------------------------------------------------------------------------------------------
add_library(oot_port_core STATIC
    src/core/log.cpp
)
target_include_directories(oot_port_core PUBLIC include)
target_compile_options(oot_port_core PRIVATE -Wall -Wextra)

# ---------------------------------------------------------------------------------------------------------------------
# Layer 2: libultra shim (C, game ABI)
# ---------------------------------------------------------------------------------------------------------------------
add_library(oot_ultra STATIC
    src/ultra/os_mesg.c
    src/ultra/os_time.c
    src/ultra/os_thread.c
    src/ultra/os_misc.c
    src/ultra/gu_mtx.c
    src/ultra/libm_native.c
    src/ultra/printf_native.c
    src/ultra/io_devices.c
    ${OOT_LIBULTRA_NATIVE_SOURCES}
)
oot_use_game_abi(oot_ultra)
target_link_libraries(oot_ultra PUBLIC oot_port_core)

# ---------------------------------------------------------------------------------------------------------------------
# Layer 1: game code
# ---------------------------------------------------------------------------------------------------------------------

# Proof that game code compiles and runs natively (M0): pure functions used by the unit tests.
add_library(oot_game_proof STATIC
    ${OOT_REPO_ROOT}/src/code/sys_math.c
    ${OOT_REPO_ROOT}/src/libc/fmodf.c
)
oot_use_game_abi(oot_game_proof)
target_link_libraries(oot_game_proof PUBLIC oot_ultra)

# Every game source of the selected version. Not part of `all` until it compiles cleanly (milestone M1):
#   cmake --build build-native --target oot_game -- -k 0
add_library(oot_game STATIC EXCLUDE_FROM_ALL ${OOT_GAME_SOURCES})
oot_use_game_abi(oot_game)
set_property(SOURCE "${OOT_REPO_ROOT}/src/boot/build.c" APPEND PROPERTY COMPILE_DEFINITIONS
    "BUILD_CREATOR=\"${OOT_BUILD_CREATOR}\"" "BUILD_DATE=\"${OOT_BUILD_DATE}\"" "BUILD_TIME=\"${OOT_BUILD_TIME}\"")

if(OOT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
