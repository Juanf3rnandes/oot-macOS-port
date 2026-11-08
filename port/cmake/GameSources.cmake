# Game source list for the native build, generated from spec/ (docs/port-macos/adr/0005-build-cmake-separado.md).
#
# Output: OOT_GAME_SOURCES             absolute paths of the game .c files to compile
#         OOT_LIBULTRA_NATIVE_SOURCES  original libultra/libc sources that are pure C and compile natively
#         OOT_HAVE_ASSETS              TRUE if extracted/<version>/ exists (`make setup` was run)

find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(EXISTS "${OOT_EXTRACTED_DIR}/assets")
    set(OOT_HAVE_ASSETS TRUE)
else()
    set(OOT_HAVE_ASSETS FALSE)
    message(WARNING
        "extracted/${OOT_VERSION}/assets not found: only game sources that do not need ROM assets will be built.\n"
        "Run `make setup VERSION=${OOT_VERSION}` with baseroms/${OOT_VERSION}/baserom.z64 in place.")
endif()

# Version macros for preprocessing the spec, and flags so clang can resolve the game's includes.
set(_gen_args)
foreach(_def IN LISTS OOT_GAME_DEFINES)
    list(APPEND _gen_args -D ${_def})
endforeach()
list(APPEND _gen_args --check-assets
    --asset-dir "${OOT_EXTRACTED_DIR}" --asset-dir "${OOT_ASSET_BUILD_DIR}"
    "--cflag=-iquote" "--cflag=${OOT_REPO_ROOT}/include"
    "--cflag=-iquote" "--cflag=${OOT_REPO_ROOT}/src"
    "--cflag=-iquote" "--cflag=${OOT_REPO_ROOT}"
    "--cflag=-iquote" "--cflag=${OOT_ASSET_BUILD_DIR}"
    "--cflag=-iquote" "--cflag=${OOT_EXTRACTED_DIR}"
    "--cflag=-I${CMAKE_CURRENT_SOURCE_DIR}/include/libc_compat"
)
foreach(_def IN LISTS OOT_GAME_DEFINES)
    list(APPEND _gen_args "--cflag=-D${_def}")
endforeach()

set(_gen_output "${CMAKE_CURRENT_BINARY_DIR}/generated/game_sources.cmake")
execute_process(
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tools/gen_game_sources.py" ${_gen_args} -o "${_gen_output}"
    RESULT_VARIABLE _gen_result
)
if(NOT _gen_result EQUAL 0)
    message(FATAL_ERROR "port/tools/gen_game_sources.py failed")
endif()
include("${_gen_output}")

# Re-run the generator when the spec changes.
file(GLOB _spec_files "${OOT_REPO_ROOT}/spec/*")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${_spec_files})

# Game files replaced by the port (compiled from port/src instead). Keep each entry justified.
set(OOT_GAME_REPLACED
    src/boot/is_debug.c   # IS-Viewer driver → port/src/ultra/printf_native.c
    src/libc/memset.c     # host libc (a game definition would replace memset for the whole process)
    src/libc/memmove.c    # host libc (same reason)
    src/libc/fmodf.c      # compiled into oot_ultra, renamed to ultra_fmodf (port/include/libc_compat/math.h)
)

# Game files whose native version depends on later milestones. Keep each entry justified.
set(OOT_GAME_DEFERRED
    # M5: the tables store `name_Start`/`name_Size` linker symbols as ROM offsets and alias the table with
    # `#pragma weak a = b` (Mach-O has no symbol aliases). Needs the native audio asset table.
    src/audio/tables/sequence_table.c
    src/audio/tables/soundfont_table.c
    src/audio/tables/samplebank_table.c
)

set(OOT_GAME_SOURCES)
set(_skipped_for_assets 0)
foreach(_src IN LISTS OOT_SPEC_GAME_SOURCES)
    if(_src IN_LIST OOT_GAME_REPLACED OR _src IN_LIST OOT_GAME_DEFERRED)
        continue()
    endif()
    if(NOT OOT_HAVE_ASSETS AND _src IN_LIST OOT_SPEC_NEED_ASSETS)
        math(EXPR _skipped_for_assets "${_skipped_for_assets} + 1")
        continue()
    endif()
    list(APPEND OOT_GAME_SOURCES "${OOT_REPO_ROOT}/${_src}")
endforeach()

list(LENGTH OOT_GAME_SOURCES _n)
message(STATUS "OoT ${OOT_VERSION}: ${_n} game sources (${_skipped_for_assets} skipped: need extracted assets)")

# Original libultra and libc files that are plain C and behave the same on a little-endian 64-bit host.
# (sinf/cosf, mtxutil and xprintf are not: see port/src/ultra/libm_native.c, gu_mtx.c, printf_native.c)
set(OOT_LIBULTRA_NATIVE_SOURCES
    ${OOT_REPO_ROOT}/src/libultra/gu/coss.c
    ${OOT_REPO_ROOT}/src/libultra/gu/sins.c
    ${OOT_REPO_ROOT}/src/libultra/gu/lookat.c
    ${OOT_REPO_ROOT}/src/libultra/gu/lookathil.c
    ${OOT_REPO_ROOT}/src/libultra/gu/normalize.c
    ${OOT_REPO_ROOT}/src/libultra/gu/ortho.c
    ${OOT_REPO_ROOT}/src/libultra/gu/perspective.c
    ${OOT_REPO_ROOT}/src/libultra/gu/position.c
    ${OOT_REPO_ROOT}/src/libultra/gu/rotate.c
    ${OOT_REPO_ROOT}/src/libultra/gu/scale.c
    ${OOT_REPO_ROOT}/src/libultra/gu/translate.c
    ${OOT_REPO_ROOT}/src/libultra/gu/us2dex.c
    ${OOT_REPO_ROOT}/src/libc/fmodf.c
    # Video mode tables (pure data)
    ${OOT_REPO_ROOT}/src/libultra/io/vimodentsclan1.c
    ${OOT_REPO_ROOT}/src/libultra/io/vimodepallan1.c
    ${OOT_REPO_ROOT}/src/libultra/io/vimodempallan1.c
    ${OOT_REPO_ROOT}/src/libultra/io/vimodefpallan1.c
)
