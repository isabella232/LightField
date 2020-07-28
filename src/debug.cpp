#include "pch.h"
#include <ctime>
#include <iomanip>
#include "debug.h"

namespace {

    FILE* DebugLog       {        };
    FILE* OriginalStderr { stderr };

}

DebugManager::DebugManager( ) {
    ::unlink( DebugLogPaths[5] );
    for ( int n = 5; n > 0; --n ) {
        ::rename( DebugLogPaths[n - 1], DebugLogPaths[n] );
    }

    DebugLog = ::fopen( DebugLogPaths[0], "wtx" );
    if ( !DebugLog ) {
        error_t err = errno;
        ::fprintf( stderr, "failed to open log file '%s': %s [%d]", DebugLogPaths[0], strerror( err ), err );
    } else {
        // save the original stderr
        int fd = ::dup( 2 );

        // redirect stderr to the debug log
        ::dup2( ::fileno( DebugLog ), 2 );

        // get a FILE* for the original stderr
        OriginalStderr = ::fdopen( fd, "wt" );

        // disable buffering on both FILE*:s
        ::setvbuf( DebugLog,       nullptr, _IONBF, 0 );
        ::setvbuf( OriginalStderr, nullptr, _IONBF, 0 );
    }
}

DebugManager::~DebugManager( ) {
    /*empty*/
}

void debug( QString str ) {
    const char* buff = str.toUtf8().data();

    debug(buff);
}

void debug( char const* str ) {
    time_t timeNow = std::time(nullptr);

    std::stringstream ss;

    ss << std::put_time(std::localtime(&timeNow), "%OH:%OM:%OS | ") << str;

    const std::string formattedOutput = ss.str();
    const char* cstr = formattedOutput.c_str();

    if ( DebugLog ) {
        ::fputs(cstr, DebugLog);
    }

    ::fputs(cstr, OriginalStderr);
}
