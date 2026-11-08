#include "port/endian.h"

#include <gtest/gtest.h>

TEST(Endian, ByteSwap) {
    EXPECT_EQ(BSWAP16(0x1234), 0x3412);
    EXPECT_EQ(BSWAP32(0x12345678u), 0x78563412u);
    EXPECT_EQ(BSWAP64(0x0123456789ABCDEFull), 0xEFCDAB8967452301ull);
}

TEST(Endian, ReadBigEndian) {
    const uint8_t bytes[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x55 };

    EXPECT_EQ(BE16(bytes), 0x0123);
    EXPECT_EQ(BE32(bytes), 0x01234567u);
    EXPECT_EQ(BE64(bytes), 0x0123456789ABCDEFull);
    // Unaligned reads are allowed
    EXPECT_EQ(BE16(bytes + 1), 0x2345);
    EXPECT_EQ(BE32(bytes + 5), 0xABCDEF55u);
}

TEST(Endian, WriteBigEndian) {
    uint8_t bytes[6] = {};

    WriteBE16(bytes, 0xBEEF);
    WriteBE32(bytes + 2, 0xCAFEF00Du);
    EXPECT_EQ(bytes[0], 0xBE);
    EXPECT_EQ(bytes[1], 0xEF);
    EXPECT_EQ(BE32(bytes + 2), 0xCAFEF00Du);
    EXPECT_EQ(bytes[2], 0xCA);
}
