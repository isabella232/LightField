#include "pch.h"

#include "usbmountmanager.h"

#include "processrunner.h"
#include "stdiologger.h"

// ===============================================
// Mountmon messages:
// "terminate:<reason>"
// "mount:<mountPoint>"
// "unmount:<mountPoint>"
// "remount:(failure|success):r[wo]:<mountPoint>"
// ===============================================

UsbMountManager::UsbMountManager( QObject* parent ): QObject( parent ) {
    _stderrLogger  = new StdioLogger   { "mountmon/stderr", this };
    _stdoutLogger  = new StdioLogger   { "mountmon/stdout", this };
    _processRunner = new ProcessRunner {                    this };

    QObject::connect( _processRunner, &ProcessRunner::failed,                  this,          &UsbMountManager::mountmon_failed                  );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardError,  _stderrLogger, &StdioLogger::read                                 );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, _stdoutLogger, &StdioLogger::read                                 );
    QObject::connect( _processRunner, &ProcessRunner::readyReadStandardOutput, this,          &UsbMountManager::mountmon_readyReadStandardOutput );

    _processRunner->start( QString { "sudo" }, QStringList { MountmonCommand } );
}

UsbMountManager::~UsbMountManager( ) {
    if ( _stderrLogger ) {
        QObject::disconnect( _stderrLogger );
        _stderrLogger->deleteLater( );
        _stderrLogger = nullptr;
    }
    if ( _stdoutLogger ) {
        QObject::disconnect( _stdoutLogger );
        _stdoutLogger->deleteLater( );
        _stdoutLogger = nullptr;
    }
    if ( _processRunner ) {
        _processRunner->write( "terminate\n" );
        QObject::disconnect( _processRunner );
        _processRunner->deleteLater( );
        _processRunner = nullptr;
    }
}

void UsbMountManager::mountmon_failed( int const exitCode, QProcess::ProcessError const error ) {
    debug( "+ UsbMountManager::mountmon_failed: exit code: %d, error: %s [%d]\n", exitCode, ToString( error ), static_cast<int>( error ) );
}

void UsbMountManager::mountmon_readyReadStandardOutput( QString const& data ) {
    _stdoutBuffer += data;

    auto index = _stdoutBuffer.lastIndexOf( '\n' );
    if ( -1 == index ) {
        return;
    }

    auto lines = _stdoutBuffer.left( index ).split( NewLineRegex );
    _stdoutBuffer.remove( 0, index + 1 );
    for ( auto const& line : lines ) {
        auto tokens = line.split( ':' );
        if ( tokens.count( ) < 2 ) {
            debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: short line\n" );
            continue;
        }

        if ( tokens[0] == "terminate" ) {
            debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: mountmon terminated: reason '%s'\n", tokens[1].toUtf8( ).data( ) );

            _stdoutBuffer.clear( );
            break;
        } else if ( tokens[0] == "mount" ) {
            if ( !_mountPoint.isEmpty( ) ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'mount' notification for mount point '%s', when we already have a mounted filesystem?\n", tokens[1].toUtf8( ).data( ) );
                continue;
            }
            _mountPoint = tokens[1];

            emit filesystemMounted( _mountPoint );
        } else if ( tokens[0] == "unmount" ) {
            if ( _mountPoint.isEmpty( ) ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'unmount' notification for mount point '%s', when we don't have a mounted filesystem?\n", tokens[1].toUtf8( ).data( ) );
                continue;
            }
            if ( _mountPoint != tokens[1] ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'unmount' notification for mount point '%s', which is not the mount point '%s' that we know about?\n", tokens[1].toUtf8( ).data( ), _mountPoint.toUtf8( ).data( ) );
                continue;
            }

            auto mountPoint = _mountPoint;
            _mountPoint.clear( );

            emit filesystemUnmounted( mountPoint );
        } else if ( tokens[0] == "remount" ) {
            if ( tokens.count( ) < 4 ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received short 'remount' notification with only %d fields\n", tokens.count( ) );
                continue;
            }
            if ( _mountPoint.isEmpty( ) ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'remount' notification for mount point '%s', when we don't have a mounted filesystem?\n", tokens[3].toUtf8( ).data( ) );
                continue;
            }
            if ( _mountPoint != tokens[3] ) {
                debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: received 'remount' notification for mount point '%s', which is not the mount point '%s' that we know about?\n", tokens[3].toUtf8( ).data( ), _mountPoint.toUtf8( ).data( ) );
                continue;
            }

            bool succeeded = ( tokens[1] == "success" );
            if ( succeeded ) {
                _isWritable = ( tokens[2] == "rw" );
            }

            emit filesystemRemounted( succeeded, _isWritable );
        } else {
            debug( "+ UsbMountManager::mountmon_readyReadStandardOutput: unknown verb '%s'\n", tokens[0].toUtf8( ).data( ) );
        }
    }
}

void UsbMountManager::remount( bool const writable ) {
    if ( _mountPoint.isEmpty( ) ) {
        debug( "+ UsbMountManager::remount: asked to remount our filesystem, when we don't have one?\n" );
        emit filesystemRemounted( false, writable );
        return;
    }

    debug( "+ UsbMountManager::remount: asking Mountmon to remount mount point '%s' read-%s\n", _mountPoint.toUtf8( ).data( ), writable ? "write" : "only" );
    _processRunner->write( QString { "remount-r%1:%2\n" }.arg( writable ? 'w' : 'r' ).arg( _mountPoint ).toUtf8( ) );
}
