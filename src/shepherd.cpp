#include "window.h"

namespace {

    char const* ShepherdBaseDirectory = "/home/lumen/Volumetric/fstl/stdio-shepherd";

    char const* ProcessErrorStrings[] {
        "FailedToStart",
        "Crashed",
        "Timedout",
        "WriteError",
        "ReadError",
        "UnknownError",
    };

    char const* ProcessStateStrings[] {
        "NotRunning",
        "Starting",
        "Running",
    };

    char const* ExitStatusStrings[] {
        "NormalExit",
        "CrashExit"
    };

}

Shepherd::Shepherd( QObject* parent ): QObject( parent ) {
    fprintf( stderr, "+ Shepherd::`ctor: Shepherd base directory: '%s'\n", ShepherdBaseDirectory );

    _process = new QProcess( this );
    QObject::connect( _process, &QProcess::errorOccurred,           this, &Shepherd::processErrorOccurred   );
    QObject::connect( _process, &QProcess::started,                 this, &Shepherd::processStarted         );
    QObject::connect( _process, &QProcess::stateChanged,            this, &Shepherd::processStateChanged    );
    QObject::connect( _process, &QProcess::readyReadStandardError,  this, &Shepherd::processReadyRead       );
    QObject::connect( _process, &QProcess::readyReadStandardOutput, this, &Shepherd::processReadyRead       );
    QObject::connect( _process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &Shepherd::processFinished );

    auto env = _process->processEnvironment( );
    if ( env.isEmpty( ) ) {
        env = QProcessEnvironment::systemEnvironment( );
    }
    env.insert( "PYTHONUNBUFFERED", "x" );
    _process->setProcessEnvironment( env );
    _process->setWorkingDirectory( ShepherdBaseDirectory );
    _process->start( QString( "./stdio-shepherd.py" ) );
}

Shepherd::~Shepherd( ) {
    /*empty*/
}

void Shepherd::processErrorOccurred( QProcess::ProcessError error ) {
    fprintf( stderr, "+ Shepherd::processErrorOccurred: error %s [%d]\n", ProcessErrorStrings[error], error );
    emit shepherd_ProcessError( error );
}

void Shepherd::processStarted( ) {
    fprintf( stderr, "+ Shepherd::processStarted\n" );
    emit shepherd_Started( );
}

void Shepherd::processStateChanged( QProcess::ProcessState newState ) {
    fprintf( stderr, "+ Shepherd::processStateChanged: new state %s [%d]\n", ProcessStateStrings[newState], newState );
}

void Shepherd::processReadyRead( ) {
    QString input;

    _process->setReadChannel( QProcess::StandardError );
    input = _process->readAllStandardError( );
    if ( input.length( ) ) {
        fprintf( stderr,
            "+ Shepherd::processReadyReadStdout\n"
            "  + received from stderr: >>%s<<\n",
            input.toUtf8( ).data( )
        );
    }

    _process->setReadChannel( QProcess::StandardOutput );
    input = _process->readAllStandardOutput( );
    if ( input.length( ) ) {
        fprintf( stderr,
            "+ Shepherd::processReadyReadStdout\n"
            "  + received from stdout: >>%s<<\n",
            input.toUtf8( ).data( )
        );
    }

    _process->setReadChannel( QProcess::StandardOutput );
}

void Shepherd::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    fprintf( stderr, "+ Shepherd::processFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ExitStatusStrings[exitStatus], exitStatus );
    emit shepherd_Finished( exitCode, exitStatus );
}

void Shepherd::doMove( float arg ) {
    _process->write( QString( "move %1\n" ).arg( arg ).toUtf8( ) );
}

void Shepherd::doMoveTo( float arg ) {
    _process->write( QString( "moveTo %1\n" ).arg( arg ).toUtf8( ) );
}

void Shepherd::doHome( ) {
    _process->write( "home\n" );
}

void Shepherd::doLift( float arg1, float arg2 ) {
    _process->write( QString( "lift %1 %2\n" ).arg( arg1 ).arg( arg2 ).toUtf8( ) );
}

void Shepherd::doAskTemp( ) {
    _process->write( "askTemp\n" );
}

void Shepherd::doSend( char const* arg ) {
    _process->write( QString( "send \"%1\"\n" ).arg( QString( arg ).replace( "\\", "\\\\" ).replace( "\"", "\\\"" ) ).toUtf8( ) );
}

void Shepherd::terminate( ) {
    _process->write( "terminate\n" );
    _process->waitForFinished( );
}
