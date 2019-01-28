#include "shepherd.h"
#include "window.h"

namespace {

    char const* ProcessErrorStrings[] {
        "FailedToStart",
        "Crashed",
        "Timedout",
        "WriteError",
        "ReadError",
        "UnknownError",
    };

}

Shepherd::Shepherd( QObject* parent ): QObject( parent ) {
    char const* baseDirectory = getenv( "DEBUGGING_ON_VIOLET" ) ? "/home/icekarma/devel/work/VolumetricLumen/fstl/stdio-shepherd" : "/home/lumen/Volumetric";
    fprintf( stderr, "+ Shepherd::`ctor: Base directory for launching stdio-shepherd.py: %s\n", baseDirectory );

    _process = new QProcess( this );
    _process->setWorkingDirectory( baseDirectory );
    QObject::connect( _process, &QProcess::started,       this, &Shepherd::processStarted       );
    QObject::connect( _process, &QProcess::errorOccurred, this, &Shepherd::processErrorOccurred );
    QObject::connect( _process, &QProcess::readyRead,     this, &Shepherd::processReadyRead     );
    _process->start( QString( "./stdio-shepherd.py" ) );
}

Shepherd::~Shepherd( ) {
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
    _process->write( QString( "send \"%1\"\n" ).arg( arg ).toUtf8( ) );
}

void Shepherd::processStarted( ) {
    fprintf( stderr, "+ Shepherd::processStarted\n" );
}

void Shepherd::processErrorOccurred( QProcess::ProcessError err ) {
    fprintf( stderr, "+ Shepherd::processErrorOccurred: error %s [%d]\n", ProcessErrorStrings[err], err );
}

void Shepherd::processReadyRead( ) {
    fprintf( stderr, "+ Shepherd::processReadyRead\n" );
}
