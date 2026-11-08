// OSMesgQueue semantics (docs/port-macos/10-estrategia-de-testes.md §3.2).
#include "helpers.h"

#include <gtest/gtest.h>

namespace {
void* M(uintptr_t v) {
    return reinterpret_cast<void*>(v);
}
} // namespace

TEST(MesgQueue, FifoOrder) {
    MesgQueueStorage mq;
    void* buf[3];
    void* out = nullptr;

    ASSERT_LE(Helpers_SizeofMesgQueue(), sizeof(mq));
    Helpers_CreateMesgQueue(&mq, buf, 3);
    EXPECT_EQ(Helpers_SendMesg(&mq, M(1), 0), 0);
    EXPECT_EQ(Helpers_SendMesg(&mq, M(2), 0), 0);
    EXPECT_EQ(Helpers_MesgCount(&mq), 2);
    EXPECT_EQ(Helpers_RecvMesg(&mq, &out, 0), 0);
    EXPECT_EQ(out, M(1));
    EXPECT_EQ(Helpers_SendMesg(&mq, M(3), 0), 0);
    EXPECT_EQ(Helpers_SendMesg(&mq, M(4), 0), 0); // wraps around the ring
    EXPECT_EQ(Helpers_RecvMesg(&mq, &out, 0), 0);
    EXPECT_EQ(out, M(2));
    EXPECT_EQ(Helpers_RecvMesg(&mq, &out, 0), 0);
    EXPECT_EQ(out, M(3));
    EXPECT_EQ(Helpers_RecvMesg(&mq, &out, 0), 0);
    EXPECT_EQ(out, M(4));
}

TEST(MesgQueue, NonBlockingFailsWhenFullOrEmpty) {
    MesgQueueStorage mq;
    void* buf[1];
    void* out = M(0xAA);

    Helpers_CreateMesgQueue(&mq, buf, 1);
    EXPECT_EQ(Helpers_RecvMesg(&mq, &out, 0), -1);
    EXPECT_EQ(out, M(0xAA));
    EXPECT_EQ(Helpers_SendMesg(&mq, M(1), 0), 0);
    EXPECT_EQ(Helpers_SendMesg(&mq, M(2), 0), -1);
    EXPECT_EQ(Helpers_JamMesg(&mq, M(2), 0), -1);
    EXPECT_EQ(Helpers_MesgCount(&mq), 1);
}

TEST(MesgQueue, JamPutsMessageFirst) {
    MesgQueueStorage mq;
    void* buf[4];
    void* out = nullptr;

    Helpers_CreateMesgQueue(&mq, buf, 4);
    Helpers_SendMesg(&mq, M(1), 0);
    Helpers_SendMesg(&mq, M(2), 0);
    EXPECT_EQ(Helpers_JamMesg(&mq, M(9), 0), 0);
    Helpers_RecvMesg(&mq, &out, 0);
    EXPECT_EQ(out, M(9));
    Helpers_RecvMesg(&mq, &out, 0);
    EXPECT_EQ(out, M(1));
}

TEST(MesgQueue, RecvWithNullMessagePointer) {
    MesgQueueStorage mq;
    void* buf[1];

    Helpers_CreateMesgQueue(&mq, buf, 1);
    Helpers_SendMesg(&mq, M(1), 0);
    EXPECT_EQ(Helpers_RecvMesg(&mq, nullptr, 0), 0);
    EXPECT_EQ(Helpers_MesgCount(&mq), 0);
}

TEST(MesgQueueDeathTest, BlockingRecvOnEmptyQueueAbortsInsteadOfHanging) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    MesgQueueStorage mq;
    void* buf[1];

    Helpers_CreateMesgQueue(&mq, buf, 1);
    EXPECT_DEATH(Helpers_RecvMesg(&mq, nullptr, 1), "would block forever");
}

TEST(MesgQueueDeathTest, BlockingSendOnFullQueueAbortsInsteadOfHanging) {
    GTEST_FLAG_SET(death_test_style, "threadsafe");
    MesgQueueStorage mq;
    void* buf[1];

    Helpers_CreateMesgQueue(&mq, buf, 1);
    Helpers_SendMesg(&mq, M(1), 0);
    EXPECT_DEATH(Helpers_SendMesg(&mq, M(2), 1), "would block forever");
}
