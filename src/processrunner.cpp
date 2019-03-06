#include "pch.h"

#include "processrunner.h"
#include "strings.h"

#include <atomic>

/*
Bad command startup
===================
+ ProcessRunner::processStateChanged: new state Starting [1]
+ ProcessRunner::processStateChanged: new state NotRunning [0]
+ ProcessRunner::processErrorOccurred: error FailedToStart [0]

When a process exits normally
=============================
+ ProcessRunner::processStateChanged: new state Starting [1]
+ ProcessRunner::processStateChanged: new state Running [2]
+ ProcessRunner::processStarted
[time passes]
+ ProcessRunner::processStateChanged: new state NotRunning [0]
+ ProcessRunner::processFinished: exitCode: 0, exitStatus: NormalExit [0]

When a process exits abnormally
===============================
+ ProcessRunner::processStateChanged: new state Starting [1]
+ ProcessRunner::processStateChanged: new state Running [2]
+ ProcessRunner::processStarted
[time passes]
+ ProcessRunner::processErrorOccurred: error Crashed [1]
+ ProcessRunner::processStateChanged: new state NotRunning [0]
+ ProcessRunner::processFinished: exitCode: 15, exitStatus: CrashExit [1]
*/

namespace {

    std::atomic_int InstanceId { 0 };

}

ProcessRunner::ProcessRunner( QObject* parent ): QObject( parent ) {
    QObject::connect( &_process, &QProcess::errorOccurred,                                        this, &ProcessRunner::processErrorOccurred           );
    QObject::connect( &_process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &ProcessRunner::processFinished                );
    QObject::connect( &_process, &QProcess::readyReadStandardOutput,                              this, &ProcessRunner::processReadyReadStandardOutput );
    QObject::connect( &_process, &QProcess::readyReadStandardError,                               this, &ProcessRunner::processReadyReadStandardError  );
    _instanceId = ++InstanceId;
}

ProcessRunner::~ProcessRunner( ) {
    /*empty*/
}

void ProcessRunner::start( QString const& program, QStringList const& arguments, QProcess::OpenMode const mode ) {
    _process.start( program, arguments, mode );
}

void ProcessRunner::processErrorOccurred( QProcess::ProcessError error ) {
    debug( "+ ProcessRunner[%d]::processErrorOccurred: %s [%d]\n", _instanceId, ToString( error ), error );

    if ( QProcess::FailedToStart == error ) {
        // won't get signal "finished" if failed to start, so emit signal "failed" here
        debug( "+ ProcessRunner[%d]::processErrorOccurred: process failed to start\n", _instanceId );
        emit failed( QProcess::FailedToStart );
    } else if ( QProcess::Crashed == error ) {
        if ( _process.state( ) != QProcess::NotRunning ) {
            debug( "+ ProcessRunner[%d]::processErrorOccurred: killing undead crashed process\n", _instanceId );
            _process.kill( );
        }
    }
}

void ProcessRunner::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    if ( ( exitStatus == QProcess::CrashExit ) || ( exitCode != 0 ) ) {
        debug( "+ ProcessRunner[%d]::processFinished: process failed: exit status: %s [%d]; exit code: %d\n", _instanceId, ToString( exitStatus ), exitStatus, exitCode );
        emit failed( QProcess::Crashed );
    } else {
        emit succeeded( );
    }
}

void ProcessRunner::processReadyReadStandardOutput( ) {
    _process.setReadChannel( QProcess::StandardOutput );
    QString data = _process.readAllStandardOutput( );
    if ( data.length( ) ) {
        emit readyReadStandardOutput( data );
    }
}

void ProcessRunner::processReadyReadStandardError( ) {
    _process.setReadChannel( QProcess::StandardError );
    QString data = _process.readAllStandardError( );
    if ( data.length( ) ) {
        emit readyReadStandardError( data );
    }
}
