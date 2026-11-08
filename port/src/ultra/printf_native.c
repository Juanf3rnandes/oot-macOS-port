/**
 * Formatted output.
 *
 * - _Printf (src/libultra/libc/xprintf.c) is the engine behind the game's sprintf, PrintUtils and GfxPrint. The
 *   original reads float signs through `((unsigned short*)&x)[0]`, which is wrong on a little-endian host, so the
 *   port formats with the host vsnprintf and hands the result to the callback, like the original does in pieces.
 * - src/boot/is_debug.c is an IS-Viewer (development cartridge) driver; it is replaced so osSyncPrintf goes to the
 *   port log.
 */
#include "ultra64.h"
#include "port/log.h"

#include <stdio.h>
#include <stdlib.h>

int _Printf(PrintCallback pfn, void* arg, const char* fmt, va_list ap) {
    char stackBuf[512];
    char* buf = stackBuf;
    va_list apCopy;
    int len;

    va_copy(apCopy, ap);
    len = vsnprintf(stackBuf, sizeof(stackBuf), fmt, apCopy);
    va_end(apCopy);
    if (len < 0) {
        return -1;
    }
    if ((size_t)len >= sizeof(stackBuf)) {
        buf = malloc((size_t)len + 1);
        if (buf == NULL) {
            return -1;
        }
        va_copy(apCopy, ap);
        vsnprintf(buf, (size_t)len + 1, fmt, apCopy);
        va_end(apCopy);
    }
    if (len > 0 && pfn(arg, buf, (size_t)len) == NULL) {
        len = -1;
    }
    if (buf != stackBuf) {
        free(buf);
    }
    return len;
}

// is_debug.c replacement. Output is written raw: the game's debug strings are often EUC-JP.

void isPrintfInit(void) {
}

void* is_proutSyncPrintf(void* arg, const char* str, size_t count) {
    (void)arg;
    fwrite(str, 1, count, stderr);
    return (void*)1;
}

static void SyncPrintfV(const char* fmt, va_list args) {
    if (Port_LogGetLevel() <= PORT_LOG_DEBUG) {
        _Printf(is_proutSyncPrintf, NULL, fmt, args);
    }
}

void osSyncPrintfUnused(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    SyncPrintfV(fmt, args);
    va_end(args);
}

void osSyncPrintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    SyncPrintfV(fmt, args);
    va_end(args);
}

void rmonPrintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    SyncPrintfV(fmt, args);
    va_end(args);
}

void func_80002384(const char* exp, const char* file, int line) {
    Port_Fatal(file, line, "%s", exp);
}
