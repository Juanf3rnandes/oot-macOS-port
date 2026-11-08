/**
 * Byte-order helpers.
 *
 * Golden rule of the port (docs/port-macos/05-problemas-de-portabilidade.md §4): memory that the RSP/RDP would read as
 * bytes (textures, palettes, 16-bit framebuffers, binary data from the ROM) stays in N64 byte order (big-endian).
 * Natively compiled C structs (vertices, matrices, Gfx commands, asset structs) are in native order.
 * The graphics and audio interpreters read big-endian bytes with these helpers.
 *
 * Usable from C and C++.
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline uint16_t BSWAP16(uint16_t x) {
    return __builtin_bswap16(x);
}

static inline uint32_t BSWAP32(uint32_t x) {
    return __builtin_bswap32(x);
}

static inline uint64_t BSWAP64(uint64_t x) {
    return __builtin_bswap64(x);
}

/** Reads a big-endian 16-bit value from possibly unaligned memory. */
static inline uint16_t BE16(const void* p) {
    const uint8_t* b = (const uint8_t*)p;
    return (uint16_t)((b[0] << 8) | b[1]);
}

/** Reads a big-endian 32-bit value from possibly unaligned memory. */
static inline uint32_t BE32(const void* p) {
    const uint8_t* b = (const uint8_t*)p;
    return ((uint32_t)b[0] << 24) | ((uint32_t)b[1] << 16) | ((uint32_t)b[2] << 8) | (uint32_t)b[3];
}

/** Reads a big-endian 64-bit value from possibly unaligned memory. */
static inline uint64_t BE64(const void* p) {
    const uint8_t* b = (const uint8_t*)p;
    return ((uint64_t)BE32(b) << 32) | BE32(b + 4);
}

/** Writes a 16-bit value in big-endian order to possibly unaligned memory. */
static inline void WriteBE16(void* p, uint16_t v) {
    uint8_t* b = (uint8_t*)p;
    b[0] = (uint8_t)(v >> 8);
    b[1] = (uint8_t)v;
}

/** Writes a 32-bit value in big-endian order to possibly unaligned memory. */
static inline void WriteBE32(void* p, uint32_t v) {
    uint8_t* b = (uint8_t*)p;
    b[0] = (uint8_t)(v >> 24);
    b[1] = (uint8_t)(v >> 16);
    b[2] = (uint8_t)(v >> 8);
    b[3] = (uint8_t)v;
}

#ifdef __cplusplus
}
#endif
