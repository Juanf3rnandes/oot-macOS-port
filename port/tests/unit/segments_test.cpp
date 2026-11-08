// SEGMENTED_TO_VIRTUAL native semantics (docs/port-macos/05-problemas-de-portabilidade.md §5).
#include "helpers.h"
#include "port/port.h"

#include <gtest/gtest.h>
#include <memory>

namespace {
int gSomeGlobal;
}

TEST(Segments, PageZeroGuaranteesNativePointersAreAbove4GiB) {
    // The whole native segmented-address scheme relies on this (adr/0004-assets-e-segmentos.md).
    EXPECT_GE(reinterpret_cast<uintptr_t>(&gSomeGlobal), PORT_NATIVE_POINTER_MIN);
    EXPECT_GE(reinterpret_cast<uintptr_t>(&Helpers_Segments), PORT_NATIVE_POINTER_MIN);
    int local = 0;
    EXPECT_GE(reinterpret_cast<uintptr_t>(&local), PORT_NATIVE_POINTER_MIN);
    auto heap = std::make_unique<int>(0);
    EXPECT_GE(reinterpret_cast<uintptr_t>(heap.get()), PORT_NATIVE_POINTER_MIN);
}

TEST(Segments, NativePointerPassesThrough) {
    EXPECT_EQ(Helpers_SegmentedToVirtual(reinterpret_cast<uintptr_t>(&gSomeGlobal)), &gSomeGlobal);
}

TEST(Segments, SegmentedAddressIsResolvedThroughTable) {
    static unsigned char objectData[0x2000];
    uintptr_t* segments = Helpers_Segments();

    segments[6] = reinterpret_cast<uintptr_t>(objectData);
    EXPECT_EQ(Helpers_SegmentedToVirtual(0x06001234), objectData + 0x1234);
    EXPECT_EQ(Helpers_SegmentNumber(0x06001234), 6u);
    segments[6] = 0;
}

TEST(Segments, SegmentNumberOfNativePointerDoesNotTruncate) {
    uintptr_t p = reinterpret_cast<uintptr_t>(&gSomeGlobal);
    EXPECT_EQ(Helpers_SegmentNumber(p), (p >> 24) & 0xF);
}
