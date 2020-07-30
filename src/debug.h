#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <cstdlib>
#include <cstdio>

class DebugManager {

    DebugManager( DebugManager const& ) = delete;
    DebugManager( DebugManager&& ) = delete;
    DebugManager& operator=( DebugManager const& ) = delete;
    DebugManager& operator=( DebugManager&& ) = delete;

public:

    DebugManager( );
    ~DebugManager( );

};

#define DEBUG(fmt, ...) debug(" + " __PRETTY_FUNCTION __ ": " str "\n", __VA_ARGS__)

void debug(QString str);
void debug( char const* str );

template<typename... Args>
inline void debug( char const* fmt, Args... args ) {
    if ( char* buf; asprintf( &buf, fmt, args... ) > 0 ) {
        debug( buf );
        free( buf );
    }
}

#endif // __DEBUG_H__
