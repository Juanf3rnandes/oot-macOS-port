// Port logging (port/include/port/log.h).
#include "port/log.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace {

PortLogLevel gLevel = PORT_LOG_INFO;

const char* LevelName(PortLogLevel level) {
    switch (level) {
        case PORT_LOG_TRACE:
            return "trace";
        case PORT_LOG_DEBUG:
            return "debug";
        case PORT_LOG_INFO:
            return "info";
        case PORT_LOG_WARN:
            return "warn";
        case PORT_LOG_ERROR:
            return "error";
    }
    return "?";
}

const char* BaseName(const char* path) {
    const char* slash = std::strrchr(path, '/');
    return slash != nullptr ? slash + 1 : path;
}

} // namespace

extern "C" {

void Port_LogSetLevel(PortLogLevel level) {
    gLevel = level;
}

PortLogLevel Port_LogGetLevel(void) {
    return gLevel;
}

void Port_LogV(PortLogLevel level, const char* file, int line, const char* fmt, va_list args) {
    if (level < gLevel) {
        return;
    }
    std::fprintf(stderr, "[%s] %s:%d: ", LevelName(level), BaseName(file), line);
    std::vfprintf(stderr, fmt, args);
    std::fputc('\n', stderr);
}

void Port_Log(PortLogLevel level, const char* file, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Port_LogV(level, file, line, fmt, args);
    va_end(args);
}

void Port_Fatal(const char* file, int line, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    std::fprintf(stderr, "[fatal] %s:%d: ", BaseName(file), line);
    std::vfprintf(stderr, fmt, args);
    std::fputc('\n', stderr);
    va_end(args);
    std::fflush(stderr);
    std::abort();
}

} // extern "C"
