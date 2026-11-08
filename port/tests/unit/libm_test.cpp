// The game's libm (src/libc64/fp.s, src/libc/fmodf.c, src/libultra/gu/sinf.c) differs from the host's on purpose.
#include "helpers.h"

#include <cmath>
#include <gtest/gtest.h>

TEST(GameLibm, SinCosMatchTheMathWithinFloatPrecision) {
    // If the (hi, lo) word order of the constants were wrong, results would be wildly off.
    for (float x = -20.0f; x <= 20.0f; x += 0.173f) {
        EXPECT_NEAR(Helpers_GameSinf(x), std::sin(static_cast<double>(x)), 2e-6) << x;
        EXPECT_NEAR(Helpers_GameCosf(x), std::cos(static_cast<double>(x)), 2e-6) << x;
    }
    EXPECT_EQ(Helpers_GameSinf(0.0f), 0.0f);
    EXPECT_EQ(Helpers_GameSinf(1e-5f), 1e-5f); // tiny angles are returned as-is
    EXPECT_TRUE(std::isnan(Helpers_GameSinf(NAN)));
}

TEST(GameLibm, RoundfIsFloorOfXPlusHalf) {
    EXPECT_EQ(Helpers_GameRoundf(2.5f), 3.0f);
    EXPECT_EQ(Helpers_GameRoundf(-2.5f), -2.0f); // host roundf gives -3
    EXPECT_EQ(std::roundf(-2.5f), -3.0f);        // and the host libm is untouched
}

TEST(GameLibm, FloorfGoesThroughA32BitInteger) {
    EXPECT_EQ(Helpers_GameFloorf(-1.5f), -2.0f);
    EXPECT_EQ(Helpers_GameFloorf(3e9f), 2147483648.0f); // invalid conversion → 0x7FFFFFFF
}

TEST(GameLibm, FmodfTruncatesThroughAnInteger) {
    EXPECT_FLOAT_EQ(Helpers_GameFmodf(5.5f, 2.0f), 1.5f);
    EXPECT_FLOAT_EQ(Helpers_GameFmodf(-5.5f, 2.0f), -1.5f);
    EXPECT_EQ(Helpers_GameFmodf(1.0f, 0.0f), 0.0f);
}

TEST(GameLibm, PiIsAFloatConstant) {
    EXPECT_EQ(Helpers_GamePi(), 3.14159265358979323846f);
}

TEST(GameLibm, SinsTable) {
    EXPECT_EQ(Helpers_Sins(0), 0);
    EXPECT_EQ(Helpers_Sins(0x4000), 0x7FFF);
    EXPECT_EQ(Helpers_Sins(0xC000), -0x7FFF);
}
