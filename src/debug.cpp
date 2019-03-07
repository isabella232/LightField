#include "pch.h"

#include "debug.h"

FILE* DebugLog { stderr };

namespace {

    char const* DebugLogPath = "/home/lumen/Volumetric/debug";

    class DebugManager {

    public:

        DebugManager( ) {
            ::mkdir( DebugLogPath, 0700 );

            time_t currentTimeT = ::time( nullptr );

            struct tm currentTm;
            ::gmtime_r( &currentTimeT, &currentTm );

            char timestamp[128] { };
            ::strftime( timestamp, sizeof( timestamp ), "%Y%m%d%H%M%S", &currentTm );

            QString debugLogFileName = QString( "%1/%2.log" ).arg( DebugLogPath ).arg( timestamp );

            DebugLog = ::fopen( debugLogFileName.toUtf8( ).data( ), "wt" );
            if ( !DebugLog ) {
                DebugLog = stderr;
            } else {
                dup2( 2, ::fileno( DebugLog ) );
            }
        }

        ~DebugManager( ) {
            fclose( DebugLog );
        }

    };

    DebugManager _debugManager;

}