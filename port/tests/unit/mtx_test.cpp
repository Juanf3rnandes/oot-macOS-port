// Fixed-point matrix layout (docs/port-macos/05-problemas-de-portabilidade.md §4.3).
#include "helpers.h"

#include <gtest/gtest.h>

TEST(Mtx, F2LUsesIndexedHalves) {
    float mf[4][4] = {};
    MtxStorage m;

    mf[0][0] = 1.0f;
    mf[0][1] = 1.5f;
    mf[1][2] = -1.5f;
    mf[3][3] = 0.25f;
    Helpers_MtxF2L(mf, &m);

    EXPECT_EQ(Helpers_MtxIntPart(&m, 0, 0), 1);
    EXPECT_EQ(Helpers_MtxFracPart(&m, 0, 0), 0);
    EXPECT_EQ(Helpers_MtxIntPart(&m, 0, 1), 1);
    EXPECT_EQ(Helpers_MtxFracPart(&m, 0, 1), 0x8000);
    EXPECT_EQ(Helpers_MtxIntPart(&m, 1, 2), 0xFFFE); // -1.5 = 0xFFFE8000
    EXPECT_EQ(Helpers_MtxFracPart(&m, 1, 2), 0x8000);
    EXPECT_EQ(Helpers_MtxIntPart(&m, 3, 3), 0);
    EXPECT_EQ(Helpers_MtxFracPart(&m, 3, 3), 0x4000);
}

TEST(Mtx, RoundTrip) {
    float in[4][4];
    float out[4][4];
    MtxStorage m;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            in[i][j] = (i * 4 + j - 7) * 13.37f;
        }
    }
    Helpers_MtxF2L(in, &m);
    Helpers_MtxL2F(out, &m);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            EXPECT_NEAR(out[i][j], in[i][j], 1.0 / 65536) << i << "," << j;
        }
    }
}

TEST(Mtx, OriginalGuTranslateWritesTranslationRow) {
    // gu/translate.c is the original libultra source compiled natively.
    MtxStorage m;

    Helpers_Translate(&m, 10.0f, -2.0f, 0.5f);
    EXPECT_EQ(Helpers_MtxIntPart(&m, 3, 0), 10);
    EXPECT_EQ(Helpers_MtxIntPart(&m, 3, 1), 0xFFFE);
    EXPECT_EQ(Helpers_MtxFracPart(&m, 3, 2), 0x8000);
    EXPECT_EQ(Helpers_MtxIntPart(&m, 0, 0), 1);
    EXPECT_EQ(Helpers_MtxIntPart(&m, 3, 3), 1);
}
