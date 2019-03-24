#include "pch.h"

#include "debug.h"

#if defined _DEBUG

namespace {

    FILE* DebugLog       { stderr };
    FILE* OriginalStderr { stderr };

    char const* DebugLogPath = "/var/log/lightfield";

}

DebugManager::DebugManager( ) {
    ::mkdir( DebugLogPath, 0700 );

    time_t currentTimeT = ::time( nullptr );

    struct tm currentTm;
    ::gmtime_r( &currentTimeT, &currentTm );

    char timestamp[128] { };
    ::strftime( timestamp, sizeof( timestamp ), "%Y%m%d%H%M%S", &currentTm );

    QString debugLogFileName = QString( "%1/%2.log" ).arg( DebugLogPath ).arg( timestamp );

    DebugLog = ::fopen( debugLogFileName.toUtf8( ).data( ), "wtx" );
    if ( !DebugLog ) {
        error_t err = errno;
        fprintf( stderr, "failed to open debug log file %s.log: %s [%d]", timestamp, strerror( err ), err );
        DebugLog = stderr;
    } else {
        // save the original stderr
        int fd = dup( 2 );

        // redirect stderr to the debug log
        dup2( ::fileno( DebugLog ), 2 );

        // get a FILE* for the original stderr
        OriginalStderr = fdopen( fd, "wt" );

        // disable buffering on both FILE*:s
        setvbuf( DebugLog,       nullptr, _IONBF, 0 );
        setvbuf( OriginalStderr, nullptr, _IONBF, 0 );
    }
}

DebugManager::~DebugManager( ) {
    /*empty*/
}

void debug( char const* str ) {
    fputs( str, DebugLog       );
    fputs( str, OriginalStderr );
}

#endif // defined _DEBUG
