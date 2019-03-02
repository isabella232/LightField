#include "pch.h"

#include "slicer.h"

#include "strings.h"

namespace {

    char const* BaseDirectory = "/home/lumen/Volumetric/printrun";

}

Slicer::Slicer( QString const& /*fileName*/, QString const& /*outputPath*/, float /*layerThickness*/, QObject* parent ): QObject( parent ) {
    debug( "+ Slicer::`ctor: Slicer base directory: '%s'\n", BaseDirectory );

    _process = new QProcess( this );
    _process->setWorkingDirectory( BaseDirectory );

    auto env = _process->processEnvironment( );
    if ( env.isEmpty( ) ) {
        env = QProcessEnvironment::systemEnvironment( );
    }
    env.insert( "PYTHONUNBUFFERED", "x" );
    _process->setProcessEnvironment( env );

    QObject::connect( _process, &QProcess::errorOccurred,           this, &Slicer::processErrorOccurred );
    QObject::connect( _process, &QProcess::started,                 this, &Slicer::processStarted       );
    QObject::connect( _process, &QProcess::stateChanged,            this, &Slicer::processStateChanged  );
    QObject::connect( _process, &QProcess::readyReadStandardError,  this, &Slicer::processReadyRead     );
    QObject::connect( _process, &QProcess::readyReadStandardOutput, this, &Slicer::processReadyRead     );
    QObject::connect( _process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &Slicer::processFinished );
}

Slicer::~Slicer( ) {

}

void Slicer::processErrorOccurred( QProcess::ProcessError processError ) {
    debug( "+ Slicer::processErrorOccurred: error %s [%d]\n", ToString( processError ), processError );
    emit error( );
}

void Slicer::processStarted( ) {
    debug( "+ Slicer::processStarted\n" );
}

void Slicer::processStateChanged( QProcess::ProcessState newState ) {
    debug( "+ Slicer::processStateChanged: new state %s [%d]\n", ToString( newState ), newState );
}

void Slicer::processReadyRead( ) {
    QString input;

    _process->setReadChannel( QProcess::StandardError );
    input = _process->readAllStandardError( );
    if ( input.length( ) ) {
        debug(
            "+ Slicer::processReadyReadStdout\n"
            "  + received from stderr: >>%s<<\n",
            input.toUtf8( ).data( )
        );
    }

    _process->setReadChannel( QProcess::StandardOutput );
    input = _process->readAllStandardOutput( );
    if ( input.length( ) ) {
        debug(
            "+ Slicer::processReadyReadStdout\n"
            "  + received from stdout: >>%s<<\n",
            input.toUtf8( ).data( )
        );
    }

    _process->setReadChannel( QProcess::StandardOutput );
}

void Slicer::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    debug( "+ Slicer::processFinished: exitStatus: %s [%d], exitCode: %d\n", ToString( exitStatus ), exitStatus, exitCode );
    emit finished( );
}

void Slicer::start( ) {
    //TODO
    if ( _process->state( ) == QProcess::NotRunning ) {
        //_process->start( "./stdio-shepherd.py" );
    }

    emit starting( );
}
