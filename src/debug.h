#ifndef __DEBUG_H__
#define __DEBUG_H__

#if defined _DEBUG
void debug( char const* str );

template<typename... Args>
inline void debug( char const* fmt, Args... args ) {
    char* buf { };
    asprintf( &buf, fmt, args... );
    debug( buf );
    free( buf );
}
#else
inline void debug( ... ) {
    /*empty*/
}
#endif // _DEBUG

#endif // __DEBUG_H__
