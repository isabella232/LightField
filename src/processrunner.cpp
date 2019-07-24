#include "pch.h"

#include "processrunner.h"
#include "strings.h"

#include <atomic>

/*
Bad command startup
===================
+ ProcessRunner::process_stateChanged: new state Starting [1]
+ ProcessRunner::process_stateChanged: new state NotRunning [0]
+ ProcessRunner::process_errorOccurred: error FailedToStart [0]

When a process exits normally
=============================
+ ProcessRunner::process_stateChanged: new state Starting [1]
+ ProcessRunner::process_stateChanged: new state Running [2]
+ ProcessRunner::process_started
[time passes]
+ ProcessRunner::process_stateChanged: new state NotRunning [0]
+ ProcessRunner::process_finished: exitCode: 0, exitStatus: NormalExit [0]

When a process exits abnormally
===============================
+ ProcessRunner::process_stateChanged: new state Starting [1]
+ ProcessRunner::process_stateChanged: new state Running [2]
+ ProcessRunner::process_started
[time passes]
+ ProcessRunner::process_errorOccurred: error Crashed [1]
+ ProcessRunner::process_stateChanged: new state NotRunning [0]
+ ProcessRunner::process_finished: exitCode: 15, exitStatus: CrashExit [1]
*/

namespace {

    std::atomic_int InstanceId { 0 };

}

ProcessRunner::ProcessRunner( QObject* parent ): QObject( parent ) {
    QObject::connect( &_process, &QProcess::errorOccurred,                                        this, &ProcessRunner::process_errorOccurred           );
    QObject::connect( &_process, &QProcess::readyReadStandardOutput,                              this, &ProcessRunner::process_readyReadStandardOutput );
    QObject::connect( &_process, &QProcess::readyReadStandardError,                               this, &ProcessRunner::process_readyReadStandardError  );
    QObject::connect( &_process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &ProcessRunner::process_finished                );
    _instanceId = ++InstanceId;
}

ProcessRunner::~ProcessRunner( ) {
    /*empty*/
}

void ProcessRunner::start( QString const& program, QStringList const& arguments ) {
    _process.start( program, arguments, QProcess::ReadWrite );

    auto commandLine { program.trimmed( ) };
    auto joinedArgs { arguments.join( Space ).trimmed( ) };
    if ( !joinedArgs.isEmpty( ) ) {
        commandLine += Space % joinedArgs;
    }
    debug( "+ ProcessRunner[%d]::start: pid %lld, command: '%s'\n", _instanceId, _process.processId( ), commandLine.toUtf8( ).data( ) );
}

qint64 ProcessRunner::write( char const* data, qint64 const maxSize ) {
    return _process.write( data, maxSize );
}

qint64 ProcessRunner::write( char const* data ) {
    return _process.write( data );
}

qint64 ProcessRunner::write( QByteArray const& byteArray ) {
    return _process.write( byteArray );
}

void ProcessRunner::process_errorOccurred( QProcess::ProcessError error ) {
    debug( "+ ProcessRunner[%d]::process_errorOccurred: %s [%d]\n", _instanceId, ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        // won't get signal "finished" if failed to start, so emit signal "failed" here
        debug( "+ ProcessRunner[%d]::process_errorOccurred: process failed to start\n", _instanceId );
        emit failed( -1, QProcess::FailedToStart );
    } else if ( QProcess::Crashed == error ) {
        if ( _process.state( ) != QProcess::NotRunning ) {
            debug( "+ ProcessRunner[%d]::process_errorOccurred: killing undead crashed process\n", _instanceId );
            _process.kill( );
        }
    }
}

void ProcessRunner::process_finished( int exitCode, QProcess::ExitStatus exitStatus ) {
    debug( "+ ProcessRunner[%d]::process_finished: exit status %s [%d], exit code %d\n", _instanceId, ToString( exitStatus ), exitStatus, exitCode );

    if ( ( exitStatus == QProcess::CrashExit ) || ( exitCode != 0 ) ) {
        debug( "+ ProcessRunner[%d]::process_finished: process failed: exit status: %s [%d]; exit code: %d\n", _instanceId, ToString( exitStatus ), exitStatus, exitCode );
        emit failed( exitCode, QProcess::Crashed );
    } else {
        emit succeeded( );
    }
}

void ProcessRunner::process_readyReadStandardOutput( ) {
    _process.setReadChannel( QProcess::StandardOutput );
    QString data = _process.readAllStandardOutput( );
    if ( data.length( ) ) {
        emit readyReadStandardOutput( data );
    }
}

void ProcessRunner::process_readyReadStandardError( ) {
    _process.setReadChannel( QProcess::StandardError );
    QString data = _process.readAllStandardError( );
    if ( data.length( ) ) {
        emit readyReadStandardError( data );
    }
}
