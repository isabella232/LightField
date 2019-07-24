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
        _mountmonProcess->write( "terminate\n" );
        QObject::disconnect( _mountmonProcess, nullptr, this, nullptr );
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

        if ( tokens[0] == "mount" ) {
            // "mount:<mountPoint>"

            if ( !_mountPoint.isEmpty( ) ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'mount' notification for mount point '%s', when we already have a mounted filesystem?\n", tokens[1].toUtf8( ).data( ) );
                continue;
            }

            _mountPoint = tokens[1];
            emit filesystemMounted( _mountPoint );
        } else if ( tokens[0] == "unmount" ) {
            // "unmount:<mountPoint>"

            if ( _mountPoint.isEmpty( ) ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'unmount' notification for mount point '%s', when we don't have a mounted filesystem?\n", tokens[1].toUtf8( ).data( ) );
                continue;
            }
            if ( _mountPoint != tokens[1] ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'unmount' notification for mount point '%s', which is not the mount point '%s' that we know about?\n", tokens[1].toUtf8( ).data( ), _mountPoint.toUtf8( ).data( ) );
                continue;
            }

            _mountPoint.clear( );
            emit filesystemUnmounted( _mountPoint );
        } else if ( tokens[0] == "remount" ) {
            // "remount:(failure|success):r[wo]:<mountPoint>"

            if ( _mountPoint.isEmpty( ) ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'remount-ro' notification for mount point '%s', when we don't have a mounted filesystem?\n", tokens[3].toUtf8( ).data( ) );
                continue;
            }
            if ( _mountPoint != tokens[3] ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'remount-ro' notification for mount point '%s', which is not the mount point '%s' that we know about?\n", tokens[3].toUtf8( ).data( ), _mountPoint.toUtf8( ).data( ) );
                continue;
            }

            bool succeeded = ( tokens[1] == "success" );
            if ( succeeded ) {
                _isWritable = ( tokens[2] == "rw" );
            }
            emit filesystemRemounted( succeeded, _isWritable );
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

void UsbMountManager::remount( bool const writable ) {
    if ( _mountPoint.isEmpty( ) ) {
        debug( "+ UsbMountManager::remount: asked to remount our filesystem, when there isn't one mounted?\n" );
        emit filesystemRemounted( false, writable );
        return;
    }

    debug( "+ UsbMountManager::remount: asking mountmon to remount mount point '%s' read-%s\n", _mountPoint.toUtf8( ).data( ), writable ? "write" : "only" );
    _mountmonProcess->write( ( QString { writable ? "remount-rw:" : "remount-ro:" } % _mountPoint % '\n' ).toUtf8( ).data( ) );
}
