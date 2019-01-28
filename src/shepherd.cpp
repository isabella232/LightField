#include "shepherd.h"
#include "window.h"

#if 0
auto _baseDirectory = "/home/icekarma/devel/work/VolumetricLumen";
#else
auto _baseDirectory = "/home/lumen/Volumetric";
#endif

Shepherd::Shepherd( QObject* parent ): QObject( parent ) {
    _process = new QProcess( this );
    _process->setWorkingDirectory( _baseDirectory );
    _process->setProgram( "printer.py" );
    QObject::connect( _process, &QProcess::started,       this, &Shepherd::processStarted       );
    QObject::connect( _process, &QProcess::errorOccurred, this, &Shepherd::processErrorOccurred );
    _process->start( );
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
}

void Shepherd::processErrorOccurred( QProcess::ProcessError ) {
}
