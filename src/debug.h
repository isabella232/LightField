#ifndef __DEBUG_H__
#define __DEBUG_H__

#if defined _DEBUG
inline void debug( char const* fmt ) {
    fputs( fmt, stderr );
}

template<typename... Args>
inline void debug( char const* fmt, Args... args ) {
    fprintf( stderr, fmt, args... );
}
#else
inline void debug( ... ) {
    /*empty*/
}
#endif // _DEBUG

#endif // __DEBUG_H__
