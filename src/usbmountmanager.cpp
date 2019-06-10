#include "pch.h"

#include "processrunner.h"
#include "strings.h"
#include "usbmountmanager.h"

UsbMountManager::UsbMountManager( QObject* parent ): QObject( parent ) {
    _mountmonProcess = new ProcessRunner;
    QObject::connect( _mountmonProcess, &ProcessRunner::failed,                  this, &UsbMountManager::mountmon_failed                  );
    QObject::connect( _mountmonProcess, &ProcessRunner::readyReadStandardOutput, this, &UsbMountManager::mountmon_readyReadStandardOutput );
    QObject::connect( _mountmonProcess, &ProcessRunner::readyReadStandardError,  this, &UsbMountManager::mountmon_readyReadStandardError  );
    _mountmonProcess->start(
        QString     { "sudo" },
        QStringList {
            MountmonCommand,
        }
    );
}

UsbMountManager::~UsbMountManager( ) {
    QObject::disconnect( _mountmonProcess, nullptr, this, nullptr );
    if ( _mountmonProcess ) {
        _mountmonProcess->kill( );
        _mountmonProcess->deleteLater( );
        _mountmonProcess = nullptr;
    }
}

void UsbMountManager::mountmon_failed( int const exitCode, QProcess::ProcessError const error ) {
    debug( "+ UsbMountManager::mountmon_failed: exit code: %d, error: %s [%d]\n", exitCode, ToString( error ), static_cast<int>( error ) );
}

void UsbMountManager::mountmon_readyReadStandardOutput( QString const& data ) {
    debug( "+ UsbMountManager::mountmon_readyReadStandardOutput:\n" );
    _stdoutBuffer += data;

    auto index = _stdoutBuffer.lastIndexOf( '\n' );
    if ( -1 == index ) {
        debug( "  + no whole lines in buffer, done for now\n" );
        return;
    }

    auto lines = _stdoutBuffer.left( index ).split( NewLineRegex );
    _stdoutBuffer.remove( 0, index + 1 );
    for ( auto const& line : lines ) {
        debug( "  + output from mountmon: '%s'\n", line.toUtf8( ).data( ) );
        auto tokens = _stdoutBuffer.split( ':' );
        if ( tokens.count( ) < 2 ) {
            debug( "    + short line\n" );
            continue;
        }

        if ( tokens[0] == "mounted" ) {
            emit filesystemMounted( tokens[1] );
        } else if ( tokens[1] == "unmounted" ) {
            emit filesystemUnmounted( tokens[1] );
        } else {
            debug( "    + unknown verb '%s'\n", tokens[0].toUtf8( ).data( ) );
        }
    }
}

void UsbMountManager::mountmon_readyReadStandardError( QString const& data ) {
    debug( "+ UsbMountManager::mountmon_readyReadStandardError\n" );
    _stderrBuffer += data;

    auto index = _stderrBuffer.lastIndexOf( '\n' );
    if ( -1 == index ) {
        debug( "  + no whole lines in buffer, done for now\n" );
        return;
    }

    auto lines = _stderrBuffer.left( index ).split( NewLineRegex );
    _stderrBuffer.remove( 0, index + 1 );
    for ( auto const& line : lines ) {
        debug( "[mountmon] %s\n", line.toUtf8( ).data( ) );
    }
}
