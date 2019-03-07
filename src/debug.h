#ifndef __DEBUG_H__
#define __DEBUG_H__

#if defined _DEBUG
inline void debug( char const* fmt ) {
    extern FILE* DebugLog;
    fputs( fmt, DebugLog );
}

template<typename... Args>
inline void debug( char const* fmt, Args... args ) {
    extern FILE* DebugLog;
    fprintf( DebugLog, fmt, args... );
}
#else
inline void debug( ... ) {
    /*empty*/
}
#endif // _DEBUG

#endif // __DEBUG_H__
