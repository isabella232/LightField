#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <cstdio>
#include <cstring>
#include <cerrno>

#include "shepherd.h"

namespace {

    FILE* debugfile = stderr;

    template<typename... Args>
    void debug( char const* format, Args... args ) {
        fprintf( debugfile, format, args... );
    }

    void _close( int& fd ) {
        if ( -1 != fd ) {
            close( fd );
            fd = -1;
        }
    }

}

Shepherd::Shepherd( ) {
}

Shepherd::~Shepherd( ) {
}

bool Shepherd::_CreatePipes( ) {
    if ( -1 == pipe( _stdinPipe ) ) {
        error_t err = errno;
        debug( "Shepherd::_CreatePipes: pipe/1 failed: %s [%d]\n", strerror( err ), err );
        return false;
    }

    if ( -1 == pipe( _stdoutPipe ) ) {
        error_t err = errno;

        _close( _stdinPipe[0] );
        _close( _stdinPipe[1] );

        debug( "Shepherd::_CreatePipes: pipe/2 failed: %s [%d]\n", strerror( err ), err );
        return false;
    }

    if ( -1 == pipe( _stderrPipe ) ) {
        error_t err = errno;

        _close( _stdinPipe[0] ); _close( _stdoutPipe[0] );
        _close( _stdinPipe[1] ); _close( _stdoutPipe[1] );

        debug( "Shepherd::_CreatePipes: pipe/3 failed: %s [%d]\n", strerror( err ), err );
        return false;
    }

    return true;
}

bool Shepherd::Start( ) {
    if ( !_CreatePipes( ) ) {
        return false;
    }

    return true;
}
