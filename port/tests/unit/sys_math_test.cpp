// src/code/sys_math.c compiled natively: the M0 proof that game code runs on the host.
#include "helpers.h"

#include <cmath>
#include <gtest/gtest.h>

TEST(SysMath, Factorial) {
    EXPECT_EQ(Helpers_MathFactorialF(0.0f), 1.0f);
    EXPECT_EQ(Helpers_MathFactorialF(5.0f), 120.0f);
    EXPECT_EQ(Helpers_MathFactorial(5), 120.0f);
    EXPECT_EQ(Helpers_MathFactorial(12), 479001600.0f);
    EXPECT_FLOAT_EQ(Helpers_MathFactorial(13), 6227020800.0f);
}

TEST(SysMath, PowF) {
    EXPECT_EQ(Helpers_MathPowF(2.0f, 10), 1024.0f);
    EXPECT_EQ(Helpers_MathPowF(3.0f, 0), 1.0f);
}

TEST(SysMath, SinCosUseTheSineTable) {
    // Math_SinF goes through sins() (a 1024-entry table), so it is only accurate to ~1e-3.
    for (float x = -3.0f; x <= 3.0f; x += 0.25f) {
        EXPECT_NEAR(Helpers_MathSinF(x), std::sin(x), 2e-3) << x;
        EXPECT_NEAR(Helpers_MathCosF(x), std::cos(x), 2e-3) << x;
    }
    EXPECT_EQ(Helpers_MathSinF(0.0f), 0.0f);
}
