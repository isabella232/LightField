#include "pch.h"

#include "usbmountmanager.h"

#include "processrunner.h"
#include "strings.h"

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
    debug( "+ UsbMountManager::`dtor\n" );
    if ( _mountmonProcess ) {
        debug( "  + cleaning up after Mountmon\n" );
        QObject::disconnect( _mountmonProcess, nullptr, this, nullptr );
        _mountmonProcess->kill( );
        _mountmonProcess->deleteLater( );
        _mountmonProcess = nullptr;
    }
}

void UsbMountManager::mountmon_failed( int const exitCode, QProcess::ProcessError const error ) {
    debug( "+ UsbMountManager::mountmon_failed: exit code: %d, error: %s [%d]\n", exitCode, ToString( error ), static_cast<int>( error ) );
}

void UsbMountManager::mountmon_readyReadStandardOutput( QString const& data ) {
    _mountmonStdoutBuffer += data;

    auto index = _mountmonStdoutBuffer.lastIndexOf( '\n' );
    if ( -1 == index ) {
        return;
    }

    auto lines = _mountmonStdoutBuffer.left( index ).split( NewLineRegex );
    _mountmonStdoutBuffer.remove( 0, index + 1 );
    for ( auto const& line : lines ) {
        debug( "[mountmon/stdout] %s\n", line.toUtf8( ).data( ) );

        auto tokens = line.split( ':' );
        if ( tokens.count( ) < 2 ) {
            debug( "UsbMountManager::mountmon_readyReadStandardOutput: short line\n" );
            continue;
        }

        if ( tokens[0] == "mounted" ) {
            _mountPoint = tokens[1];
            emit filesystemMounted( tokens[1] );
        } else if ( tokens[0] == "unmounted" ) {
            _mountPoint.clear( );
            emit filesystemUnmounted( tokens[1] );
        } else {
            debug( "UsbMountManager::mountmon_readyReadStandardOutput: unknown verb '%s'\n", tokens[0].toUtf8( ).data( ) );
        }
    }
}

void UsbMountManager::mountmon_readyReadStandardError( QString const& data ) {
    _mountmonStderrBuffer += data;

    auto index = _mountmonStderrBuffer.lastIndexOf( '\n' );
    if ( -1 == index ) {
        return;
    }

    auto lines = _mountmonStderrBuffer.left( index ).replace( NewLineRegex, "\n[mountmon/stderr] " );
    _mountmonStderrBuffer.remove( 0, index + 1 );
    debug( "[mountmon/stderr] %s\n", lines.toUtf8( ).data( ) );
}

void UsbMountManager::mount_readyReadStandardOutput( QString const& data ) {
    _mountStdoutBuffer += data;

    auto index = _mountStdoutBuffer.lastIndexOf( '\n' );
    if ( -1 == index ) {
        return;
    }

    auto lines = _mountStdoutBuffer.left( index ).replace( NewLineRegex, "\n[mount/stdout] " );
    _mountStdoutBuffer.remove( 0, index + 1 );
    debug( "[mount/stdout] %s\n", lines.toUtf8( ).data( ) );
}

void UsbMountManager::mount_readyReadStandardError( QString const& data ) {
    _mountStderrBuffer += data;

    auto index = _mountStderrBuffer.lastIndexOf( '\n' );
    if ( -1 == index ) {
        return;
    }

    auto lines = _mountStderrBuffer.left( index ).replace( NewLineRegex, "\n[mount/stderr] " );
    _mountStderrBuffer.remove( 0, index + 1 );
    debug( "[mount/stderr] %s\n", lines.toUtf8( ).data( ) );
}

void UsbMountManager::remount( bool const writable ) {
    if ( _isWritable == writable ) {
        debug( "+ UsbMountManager::remount: already mounted read-%s, returning success\n", writable ? "write" : "only" );
        emit filesystemRemounted( true, writable );
        return;
    }

    debug( "+ UsbMountManager::remount: starting `/bin/mount` to remount mount point '%s' read-%s\n", _mountPoint.toUtf8( ).data( ), writable ? "write" : "only" );

    _mountProcess = new ProcessRunner;

    QObject::connect( _mountProcess, &ProcessRunner::succeeded, this, [ this, writable ] ( ) {
        debug( "+ UsbMountManager::remount: remount read-%s succeeded\n", writable ? "write" : "only" );
        _mountProcess->deleteLater( );
        _mountProcess = nullptr;
        _isWritable   = writable;
        emit filesystemRemounted( true, writable );
    } );

    QObject::connect( _mountProcess, &ProcessRunner::failed, this, [ this, writable ] ( ) {
        debug( "+ UsbMountManager::remount: remount read-%s failed\n", writable ? "write" : "only" );
        _mountProcess->deleteLater( );
        _mountProcess = nullptr;
        emit filesystemRemounted( false, writable );
    } );

    QObject::connect( _mountProcess, &ProcessRunner::readyReadStandardOutput, this, &UsbMountManager::mount_readyReadStandardOutput );
    QObject::connect( _mountProcess, &ProcessRunner::readyReadStandardError,  this, &UsbMountManager::mount_readyReadStandardError  );

    _mountProcess->start(
        QString { "sudo" },
        QStringList {
            "/bin/mount",
            "-o",
            "remount,r" % QChar { writable ? 'w' : 'o' },
            _mountPoint
        }
    );
}
