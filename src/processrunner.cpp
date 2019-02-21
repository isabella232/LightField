#include "pch.h"

#include "processrunner.h"
#include "strings.h"

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

ProcessRunner::ProcessRunner( QObject* parent ): QObject( parent ) {
    QObject::connect( &_process, &QProcess::errorOccurred,                                        this, &ProcessRunner::processErrorOccurred );
    QObject::connect( &_process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &ProcessRunner::processFinished      );
}

ProcessRunner::~ProcessRunner( ) {
    /*empty*/
}

void ProcessRunner::start( QString const& program, QStringList const& arguments, QProcess::OpenMode const mode ) {
    _process.start( program, arguments, mode );
}

void ProcessRunner::processErrorOccurred( QProcess::ProcessError error ) {
    debug( "+ ProcessRunner::processErrorOccurred: %d\n", static_cast<int>( error ) );
    // won't get signal "finished" if FailedToStart, so emit signal "failed" here for that case
    if ( error == QProcess::FailedToStart ) {
        debug( "+ ProcessRunner::processErrorOccurred: process failed to start\n" );
        emit failed( QProcess::FailedToStart );
    }
}

void ProcessRunner::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    if ( ( exitStatus == QProcess::CrashExit ) || ( exitCode != 0 ) ) {
        debug( "+ ProcessRunner::processFinished: process failed: exit status: %s [%d]; exit code: %d\n", ToString( exitStatus ), exitStatus, exitCode );
        emit failed( QProcess::Crashed );
    } else {
        emit succeeded( );
    }
}
