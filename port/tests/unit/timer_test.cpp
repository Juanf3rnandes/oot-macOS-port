// Time conversion and timers (src/libultra/os/settimer.c semantics, driven by a fake clock).
#include "helpers.h"

#include <gtest/gtest.h>

namespace {

constexpr uint64_t kCpuCounter = 46875000; // OS_CPU_COUNTER

class TimerTest : public ::testing::Test {
protected:
    void SetUp() override {
        Helpers_TimeOverrideNs(0);
        Helpers_SetTime(0);
    }
    void TearDown() override {
        Helpers_TimeOverrideNs(-1);
    }
    void AdvanceTo(int64_t ns) {
        Helpers_TimeOverrideNs(ns);
    }
};

} // namespace

TEST_F(TimerTest, ClockIsInCpuCounterCycles) {
    AdvanceTo(1000000000);
    EXPECT_EQ(Helpers_GetTime(), kCpuCounter);
    EXPECT_EQ(Helpers_GetCount(), kCpuCounter);
    AdvanceTo(64);
    EXPECT_EQ(Helpers_GetTime(), 3u);
}

TEST_F(TimerTest, SetTimeOffsetsGetTimeButNotGetCount) {
    AdvanceTo(1000000000);
    Helpers_SetTime(5);
    EXPECT_EQ(Helpers_GetTime(), 5u);
    EXPECT_EQ(Helpers_GetCount(), kCpuCounter);
    AdvanceTo(2000000000);
    EXPECT_EQ(Helpers_GetTime(), 5u + kCpuCounter);
}

TEST_F(TimerTest, OneShotFiresAtDeadline) {
    MesgQueueStorage mq;
    TimerStorage timer;
    void* buf[2];
    void* out = nullptr;

    ASSERT_LE(Helpers_SizeofTimer(), sizeof(timer));
    Helpers_CreateMesgQueue(&mq, buf, 2);
    Helpers_SetTimer(&timer, kCpuCounter / 1000, 0, &mq, reinterpret_cast<void*>(7)); // 1 ms

    AdvanceTo(500000);
    Helpers_TimersService();
    EXPECT_EQ(Helpers_MesgCount(&mq), 0);

    AdvanceTo(1000000);
    Helpers_TimersService();
    ASSERT_EQ(Helpers_RecvMesg(&mq, &out, 0), 0);
    EXPECT_EQ(out, reinterpret_cast<void*>(7));

    AdvanceTo(5000000);
    Helpers_TimersService();
    EXPECT_EQ(Helpers_MesgCount(&mq), 0);
    EXPECT_EQ(Helpers_StopTimer(&timer), -1); // already expired
}

TEST_F(TimerTest, PeriodicTimerRearms) {
    MesgQueueStorage mq;
    TimerStorage timer;
    void* buf[8];

    Helpers_CreateMesgQueue(&mq, buf, 8);
    Helpers_SetTimer(&timer, 0, kCpuCounter / 60, &mq, nullptr); // countdown 0 → first shot after one interval

    AdvanceTo(1000000000 / 60 * 3 + 1000);
    Helpers_TimersService();
    EXPECT_EQ(Helpers_MesgCount(&mq), 3);
    EXPECT_EQ(Helpers_StopTimer(&timer), 0);
    AdvanceTo(1000000000);
    Helpers_TimersService();
    EXPECT_EQ(Helpers_MesgCount(&mq), 3);
}

TEST_F(TimerTest, BlockingRecvWaitsForTimer) {
    // This is how Sleep_Cycles (src/libc64/sleep.c) works on a single thread.
    MesgQueueStorage mq;
    TimerStorage timer;
    void* buf[1];

    Helpers_CreateMesgQueue(&mq, buf, 1);
    Helpers_SetTimer(&timer, kCpuCounter, 0, &mq, nullptr); // 1 s
    EXPECT_EQ(Helpers_RecvMesg(&mq, nullptr, 1), 0);
    EXPECT_GE(Helpers_GetTime(), kCpuCounter);
}

TEST(RealClock, IsMonotonic) {
    Helpers_TimeOverrideNs(-1);
    uint64_t a = Helpers_GetTime();
    uint64_t b = Helpers_GetTime();
    EXPECT_LE(a, b);
}
