#include "slicer.h"

Slicer::Slicer( QString const& /*fileName*/, QString const& outputPath, float layerThickness, QObject* parent ): QObject( parent ) {
    char const* baseDirectory = getenv( "DEBUGGING_ON_VIOLET" ) ? "/home/icekarma/devel/work/VolumetricLumen/fstl/stdio-shepherd" : "/home/lumen/Volumetric";
    fprintf( stderr, "+ Slicer::`ctor: Slicer base directory: '%s'\n", baseDirectory );

    _process = new QProcess( this );
    QObject::connect( _process, &QProcess::errorOccurred,           this, &Slicer::processErrorOccurred   );
    QObject::connect( _process, &QProcess::started,                 this, &Slicer::processStarted         );
    QObject::connect( _process, &QProcess::stateChanged,            this, &Slicer::processStateChanged    );
    QObject::connect( _process, &QProcess::readyReadStandardError,  this, &Slicer::processReadyRead       );
    QObject::connect( _process, &QProcess::readyReadStandardOutput, this, &Slicer::processReadyRead       );
    QObject::connect( _process, QOverload<int, QProcess::ExitStatus>::of( &QProcess::finished ), this, &Slicer::processFinished );

    auto env = _process->processEnvironment( );
    if ( env.isEmpty( ) ) {
        env = QProcessEnvironment::systemEnvironment( );
    }
    env.insert( "PYTHONUNBUFFERED", "x" );
    _process->setProcessEnvironment( env );
    _process->setWorkingDirectory( baseDirectory );
    _process->start( QString( "./stdio-shepherd.py" ) );
    emit starting( );
}

Slicer::~Slicer( ) {

}

void Slicer::processErrorOccurred( QProcess::ProcessError error ) {
    fprintf( stderr, "+ Slicer::processErrorOccurred: error %s [%d]\n", ProcessErrorStrings[error], error );
    emit error( );
}

void Slicer::processStarted( ) {
    fprintf( stderr, "+ Slicer::processStarted\n" );
}

void Slicer::processStateChanged( QProcess::ProcessState newState ) {
    fprintf( stderr, "+ Slicer::processStateChanged: new state %s [%d]\n", ProcessStateStrings[newState], newState );
}

void Slicer::processReadyRead( ) {
    QString input;

    _process->setReadChannel( QProcess::StandardError );
    input = _process->readAllStandardError( );
    if ( input.length( ) ) {
        fprintf( stderr,
            "+ Slicer::processReadyReadStdout\n"
            "  + received from stderr: >>%s<<\n",
            input.toUtf8( ).data( )
        );
    }

    _process->setReadChannel( QProcess::StandardOutput );
    input = _process->readAllStandardOutput( );
    if ( input.length( ) ) {
        fprintf( stderr,
            "+ Slicer::processReadyReadStdout\n"
            "  + received from stdout: >>%s<<\n",
            input.toUtf8( ).data( )
        );
    }

    _process->setReadChannel( QProcess::StandardOutput );
}

void Slicer::processFinished( int exitCode, QProcess::ExitStatus exitStatus ) {
    fprintf( stderr, "+ Slicer::processFinished: exitCode: %d, exitStatus: %s [%d]\n", exitCode, ExitStatusStrings[exitStatus], exitStatus );
    emit finished( );
}
