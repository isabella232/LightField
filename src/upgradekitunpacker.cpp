#include "pch.h"

#include "upgradekitunpacker.h"

#include "processrunner.h"

UpgradeKitUnpacker::UpgradeKitUnpacker( QObject* parent ): QObject( parent ) {
    /*empty*/
}

UpgradeKitUnpacker::~UpgradeKitUnpacker( ) {
    if ( _processRunner ) {
        _processRunner->deleteLater( );
        _processRunner = nullptr;
    }
}

void UpgradeKitUnpacker::startUnpacking( QString const& fileName, QString const& targetDirectory ) {
    _processRunner = new ProcessRunner( this );

    QObject::connect( _processRunner, &ProcessRunner::succeeded,               this, &UpgradeKitUnpacker::tar_succeeded               );
    QObject::connect( _processRunner, &ProcessRunner::failed,                  this, &UpgradeKitUnpacker::tar_failed                  );
#if defined _DEBUG
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this, &UpgradeKitUnpacker::tar_readyReadStandardOutput );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardError,  this, &UpgradeKitUnpacker::tar_readyReadStandardError  );
#endif // defined _DEBUG

    _processRunner->start(
        { "tar" },
        {
            "-C",  targetDirectory,
            "-xf", fileName,
        }
    );
}

void UpgradeKitUnpacker::tar_succeeded( ) {
    _processRunner->deleteLater( );
    _processRunner = nullptr;

    debug( "+ UpgradeKitUnpacker::tar_succeeded\n" );

#if defined _DEBUG
    emit unpackComplete( true, _tarOutput, _tarError );
#else
    emit unpackComplete( true );
#endif // defined _DEBUG
}

void UpgradeKitUnpacker::tar_failed( int const exitCode, QProcess::ProcessError const error ) {
    _processRunner->deleteLater( );
    _processRunner = nullptr;

    debug( "+ UpgradeKitUnpacker::tar_failed: unpacking upgrade kit: exit status %d\n", exitCode );

#if defined _DEBUG
    emit unpackComplete( false, _tarOutput, _tarError );
#else
    emit unpackComplete( false );
#endif // defined _DEBUG
}

#if defined _DEBUG
void UpgradeKitUnpacker::tar_readyReadStandardOutput( QString const& data ) {
    _tarOutput.append( data );
}

void UpgradeKitUnpacker::tar_readyReadStandardError( QString const& data ) {
    _tarError.append( data );
}
#endif // defined _DEBUG

int UpgradeKitUnpacker::instanceId( ) const {
    return _processRunner ? _processRunner->instanceId( ) : -1;
}

QProcess::ProcessState UpgradeKitUnpacker::state( ) const {
    return _processRunner ? _processRunner->state( ) : QProcess::ProcessState::NotRunning;
}
