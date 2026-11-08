# Unit tests (docs/port-macos/10-estrategia-de-testes.md). They need neither a ROM nor a window.
include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.tar.gz
    URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# C helpers compiled with the game's ABI: C++ test code cannot include the game headers directly, so these expose
# what the tests need through plain functions (and hold the compile-time checks).
add_library(oot_test_game_helpers STATIC
    unit/types_check.c
    unit/game_helpers.c
)
oot_use_game_abi(oot_test_game_helpers)
target_link_libraries(oot_test_game_helpers PUBLIC oot_game_proof oot_ultra)

add_executable(oot_unit_tests
    unit/endian_test.cpp
    unit/types_test.cpp
    unit/segments_test.cpp
    unit/mesg_queue_test.cpp
    unit/timer_test.cpp
    unit/mtx_test.cpp
    unit/libm_test.cpp
    unit/sys_math_test.cpp
)
target_link_libraries(oot_unit_tests PRIVATE oot_test_game_helpers oot_port_core GTest::gtest_main)
gtest_discover_tests(oot_unit_tests PROPERTIES LABELS unit)
