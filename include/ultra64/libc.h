#ifndef ULTRA64_LIBC_H
#define ULTRA64_LIBC_H

#include "stddef.h"

void osSyncPrintf(const char* fmt, ...);

#if TARGET_NATIVE
// port: use the host declarations (size_t lengths; the macOS SDK may also define these as fortified macros)
#include <strings.h>
#elif defined(__GNUC__) && defined(NON_MATCHING)
void bzero(void* __s, unsigned int __n);
int bcmp(const void* __sl, const void* __s2, unsigned int __n);
void bcopy(const void* __src, void* __dest, unsigned int __n);
#else
void bzero(void* __s, int __n);
int bcmp(const void* __sl, const void* __s2, int __n);
void bcopy(const void* __src, void* __dest, int __n);
#endif

#endif
