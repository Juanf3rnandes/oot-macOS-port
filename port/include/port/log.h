/**
 * Port logging. Usable from C and C++. The game's PRINTF/osSyncPrintf output is routed here too.
 */
#pragma once

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum PortLogLevel {
    PORT_LOG_TRACE,
    PORT_LOG_DEBUG,
    PORT_LOG_INFO,
    PORT_LOG_WARN,
    PORT_LOG_ERROR,
} PortLogLevel;

void Port_LogSetLevel(PortLogLevel level);
PortLogLevel Port_LogGetLevel(void);
void Port_Log(PortLogLevel level, const char* file, int line, const char* fmt, ...) __attribute__((format(printf, 4, 5)));
void Port_LogV(PortLogLevel level, const char* file, int line, const char* fmt, va_list args);

/** Logs a fatal error and aborts. */
__attribute__((noreturn)) void Port_Fatal(const char* file, int line, const char* fmt, ...)
    __attribute__((format(printf, 3, 4)));

#ifdef __cplusplus
}
#endif

#define PORT_LOG(level, ...) Port_Log(PORT_LOG_##level, __FILE__, __LINE__, __VA_ARGS__)
