#ifndef __DEBUG_H__
#define __DEBUG_H__

template<typename... Args>
inline void debug( char const* fmt, Args... args ) {
#if defined _DEBUG
    fprintf( stderr, fmt, args... );
#endif // _DEBUG
}

#endif // __DEBUG_H__
