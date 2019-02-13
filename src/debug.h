#ifndef __DEBUG_H__
#define __DEBUG_H__

#if defined _DEBUG
template<typename... Args>
inline void debug( char const* fmt, Args... args ) {
    fprintf( stderr, fmt, args... );
}
#else
template<typename... Args>
inline void debug( char const*, Args... ) {
    /*empty*/
}
#endif // _DEBUG

#endif // __DEBUG_H__
