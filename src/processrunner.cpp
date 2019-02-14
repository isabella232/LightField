#include "pch.h"

#include "processrunner.h"

ProcessRunner::ProcessRunner( ) {
    QObject::connect( &_process, &QProcess::errorOccurred,                                        this, &ProcessRunner::processErrorOccurred );
    QObject::connect( &_process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &ProcessRunner::processFinished      );
}

ProcessRunner::~ProcessRunner( ) {
    /*empty*/
}

void ProcessRunner::setProgram( QString const& program ) {
    _process.setProgram( program );
}

void ProcessRunner::setArguments( QStringList const& arguments ) {
    _process.setArguments( arguments );
}

void ProcessRunner::start( QProcess::OpenMode const mode ) {
    _process.start( mode );
}

void ProcessRunner::start( QString const& program, QStringList const& arguments, QProcess::OpenMode const mode ) {
    _process.start( program, arguments, mode );
}

void ProcessRunner::processErrorOccurred( QProcess::ProcessError error ) {
    debug( "+ ProcessRunner::processErrorOccurred: %d\n", static_cast<int>( error ) );
    // won't get signal "finished" if FailedToStart, so emit signal "failed" here for that case
    if ( error == QProcess::FailedToStart ) {
        emit failed( QProcess::FailedToStart );
    }
}

void ProcessRunner::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    if ( ( exitStatus == QProcess::CrashExit ) || ( exitCode != 0 ) ) {
        emit failed( QProcess::Crashed );
    } else {
        emit succeeded( );
    }
}
