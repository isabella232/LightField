#include "pch.h"

#include "processrunner.h"

#include "debug.h"

ProcessRunner::ProcessRunner( ) {
    QObject::connect( &_process, &QProcess::errorOccurred, this, &ProcessRunner::processErrorOccurred );
    QObject::connect( &_process, &QProcess::started,       this, &ProcessRunner::processStarted       );
    QObject::connect( &_process, &QProcess::stateChanged,  this, &ProcessRunner::processStateChanged  );
    QObject::connect( &_process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &ProcessRunner::processFinished );
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

void ProcessRunner::processErrorOccurred( QProcess::ProcessError error ) {
    fprintf( stderr, "ProcessRunner::processErrorOccurred: %d\n", static_cast<int>( error ) );
    emit failed( error );
}

void ProcessRunner::processStarted( ) {
    fprintf( stderr, "ProcessRunner::processStarted\n" );
}

void ProcessRunner::processStateChanged( QProcess::ProcessState newState ) {
    fprintf( stderr, "ProcessRunner::processStateChanged: %d\n", static_cast<int>( newState ) );
}

void ProcessRunner::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    if ( ( exitStatus == QProcess::CrashExit ) || ( exitCode != 0 ) ) {
        emit failed( QProcess::Crashed );
    } else {
        emit succeeded( );
    }
}
