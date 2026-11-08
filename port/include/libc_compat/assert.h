/**
 * port: native replacement for include/libc/assert.h.
 *
 * The game's assert.h is not the standard one: it provides ASSERT(cond, msg, file, line) and declares the game's
 * own __assert (src/boot/assert.c). The system <assert.h> is deliberately not included, because macOS declares a
 * conflicting __assert.
 */
#ifndef PORT_LIBC_COMPAT_ASSERT_H
#define PORT_LIBC_COMPAT_ASSERT_H

__attribute__((noreturn)) void __assert(const char* assertion, const char* file, int line);

#ifndef NDEBUG
#define ASSERT(cond, msg, file, line) ((cond) ? ((void)0) : __assert(#cond, __FILE__, __LINE__))
#define assert(cond) ASSERT(cond, #cond, __FILE__, __LINE__)
#else
#define ASSERT(cond, msg, file, line) ((void)0)
#define assert(cond) ((void)0)
#endif

#ifndef __cplusplus
#define static_assert(cond, msg) _Static_assert(cond, msg)
#endif

#endif
