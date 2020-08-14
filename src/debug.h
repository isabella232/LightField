#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <cstdlib>
#include <cstdio>

enum class DebugType {
    APP,
    FIRMWARE
};

class DebugManager {

    DebugManager(DebugManager const&) = delete;
    DebugManager(DebugManager&&) = delete;
    DebugManager& operator=(DebugManager const&) = delete;
    DebugManager& operator=(DebugManager&&) = delete;

public:

    DebugManager(DebugType type, const char *paths[]);
    ~DebugManager( );

    void rotate();

protected:
    DebugType _type;
    const char **_paths;
};

#define DEBUG(fmt, ...) debug(" + " __PRETTY_FUNCTION __ ": " str "\n", __VA_ARGS__)

void debug(char const* str);

template<typename... Args>
inline void debug(char const* fmt, Args... args) {
    if (char* buf; asprintf(&buf, fmt, args...) > 0) {
        debug(buf);
        free(buf);
    }
}

void firmware_debug(char const* str);

template<typename... Args>
inline void firmware_debug(char const* fmt, Args... args) {
    if (char* buf; asprintf(&buf, fmt, args...) > 0) {
        firmware_debug(buf);
        free(buf);
    }
}

#endif // __DEBUG_H__
