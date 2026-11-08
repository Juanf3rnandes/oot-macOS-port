// Sizes and Gfx layout. The static checks live in types_check.c (compiled with the game's ABI); these tests check
// the values that only exist at run time.
#include "helpers.h"

#include <gtest/gtest.h>

namespace {
constexpr uintptr_t kG_DL = 0xDE;
constexpr uintptr_t kG_VTX = 0x01;
constexpr uintptr_t kG_ENDDL = 0xDF;
constexpr uintptr_t kG_SETTIMG = 0xFD;
constexpr uintptr_t kG_MOVEWORD = 0xDB;

uintptr_t Opcode(const void* dl, int i) {
    return (Helpers_GfxWord0(dl, i) >> 24) & 0xFF;
}
} // namespace

TEST(Types, StaticDisplayListHoldsNativePointers) {
    const void* dl = TypesCheck_GetTestDL();

    EXPECT_EQ(Opcode(dl, 0), kG_VTX);
    EXPECT_EQ(Helpers_GfxWord1(dl, 0), reinterpret_cast<uintptr_t>(TypesCheck_GetTestVtx()));
    EXPECT_EQ(Opcode(dl, 1), kG_DL);
    EXPECT_EQ(Helpers_GfxWord1(dl, 1), reinterpret_cast<uintptr_t>(TypesCheck_GetTestSubDL()));
    EXPECT_EQ(Opcode(dl, 2), kG_ENDDL);
    // w0 only ever carries the 32-bit command
    EXPECT_EQ(Helpers_GfxWord0(dl, 1) >> 32, 0u);
}

TEST(Types, RuntimeDisplayListHoldsNativePointers) {
    alignas(16) unsigned char buf[16 * 8] = {};
    int dl = 0, vtx = 0, tex = 0;

    Helpers_BuildRuntimeDL(buf, &dl, &vtx, &tex);
    EXPECT_EQ(Opcode(buf, 0), kG_DL);
    EXPECT_EQ(Helpers_GfxWord1(buf, 0), reinterpret_cast<uintptr_t>(&dl));
    EXPECT_EQ(Opcode(buf, 1), kG_VTX);
    EXPECT_EQ(Helpers_GfxWord1(buf, 1), reinterpret_cast<uintptr_t>(&vtx));
    EXPECT_EQ(Opcode(buf, 2), kG_SETTIMG);
    EXPECT_EQ(Helpers_GfxWord1(buf, 2), reinterpret_cast<uintptr_t>(&tex));
    // gSPSegment passes VIRTUAL_TO_PHYSICAL-style addresses; natively that is the full pointer
    EXPECT_EQ(Opcode(buf, 3), kG_MOVEWORD);
    EXPECT_EQ(Helpers_GfxWord1(buf, 3), reinterpret_cast<uintptr_t>(&tex));
    EXPECT_EQ(Opcode(buf, 4), kG_ENDDL);
}

TEST(Types, AcmdHoldsNativePointers) {
    int book = 0;
    EXPECT_EQ(Helpers_AcmdLoadADPCM(&book), reinterpret_cast<uintptr_t>(&book));
}
