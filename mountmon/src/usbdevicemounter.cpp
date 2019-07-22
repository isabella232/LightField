#include "pch.h"

#include "usbdevicemounter.h"

#include "processrunner.h"
#include "signalhandler.h"
#include "udisksmonitor.h"

extern uid_t LightFieldUserId;
extern gid_t LightFieldGroupId;

namespace {

    char const* DriveSizeUnits[] = { "iB", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB" };

    QRegularExpression const NewLineRegex { "\\r?\\n" };

    std::initializer_list<int> signalList {
        SIGHUP,
        SIGINT,
        SIGPIPE,
        SIGQUIT,
        SIGTERM,
    };

    void ScaleSize( qulonglong const inputSize, double& scaledSize, char const*& suffix ) {
        int unitIndex = 0;

        scaledSize = inputSize;
        while ( scaledSize > 1024.0 ) {
            ++unitIndex;
            scaledSize /= 1024.0;
        }
        suffix = DriveSizeUnits[unitIndex];
    }

}

UsbDeviceMounter::UsbDeviceMounter( UDisksMonitor& monitor, QObject* parent ):
    QObject  ( parent  ),
    _monitor ( monitor )
{
    _signalHandler = new SignalHandler;
    (void) QObject::connect( _signalHandler, &SignalHandler::signalReceived, this, &UsbDeviceMounter::_signalReceived );
    _signalHandler->subscribe( signalList );

    (void) QObject::connect( &_monitor, &UDisksMonitor::driveAdded,         this, &UsbDeviceMounter::_driveAdded         );
    (void) QObject::connect( &_monitor, &UDisksMonitor::filesystemAdded,    this, &UsbDeviceMounter::_filesystemAdded    );
    (void) QObject::connect( &_monitor, &UDisksMonitor::blockDeviceAdded,   this, &UsbDeviceMounter::_blockDeviceAdded   );

    (void) QObject::connect( &_monitor, &UDisksMonitor::driveRemoved,       this, &UsbDeviceMounter::_driveRemoved       );
    (void) QObject::connect( &_monitor, &UDisksMonitor::filesystemRemoved,  this, &UsbDeviceMounter::_filesystemRemoved  );
    (void) QObject::connect( &_monitor, &UDisksMonitor::blockDeviceRemoved, this, &UsbDeviceMounter::_blockDeviceRemoved );
}

UsbDeviceMounter::~UsbDeviceMounter( ) {
    QObject::disconnect( &_monitor );
}

void UsbDeviceMounter::_fixUpMountPoint( QString const& mountPoint ) {
    if ( -1 == chmod( mountPoint.toUtf8( ).constData( ), 0555 ) ) {
        error_t err = errno;
        debug( "+ UsbDeviceMounter:_fixUpMountPoint: couldn't change permissions on mount point directory '%s', continuing anyway: %s [%d]\n", mountPoint.toUtf8( ).constData( ), strerror( err ), err );
    }

    if ( -1 == chown( mountPoint.toUtf8( ).constData( ), LightFieldUserId, LightFieldGroupId ) ) {
        error_t err = errno;
        debug( "+ UsbDeviceMounter:_fixUpMountPoint: couldn't change owner and group of mount point directory '%s', continuing anyway: %s [%d]\n", mountPoint.toUtf8( ).constData( ), strerror( err ), err );
    }

    auto index = mountPoint.lastIndexOf( '/' );
    if ( index > 0 ) {
        auto mountParentDirText = mountPoint.left( index ).toUtf8( ).constData( );
        if ( -1 == chmod( mountParentDirText, 0755 ) ) {
            error_t err = errno;
            debug( "+ UsbDeviceMounter:_fixUpMountPoint: couldn't change permissions on user mount directory '%s', continuing anyway: %s [%d]\n", mountParentDirText, strerror( err ), err );
        }
    }
}

void UsbDeviceMounter::_mount( QDBusObjectPath const& path, UFilesystem* filesystem ) {
    debug( "+ UsbDeviceMounter::_mount: attempting to mount filesystem at D-Bus object path '%s'\n", path.path( ).toUtf8( ).data( ) );
    auto mount = QDBusMessage::createMethodCall( UDisks2::Service( ), path.path( ), Interface::UDisks2( "Filesystem" ), "Mount" );
    mount.setArguments( QVariantList { } << QVariantMap { { "options", "ro,noexec,umask=0002" } } );
    QDBusReply<QString> reply = QDBusConnection::systemBus( ).call( mount, QDBus::Block, -1 );
    if ( reply.isValid( ) ) {
        auto mountPoint = reply.value( );
        debug( "+ UsbDeviceMounter::_mount: filesystem at D-Bus object path '%s' mounted at '%s', fixing permissions\n", path.path( ).toUtf8( ).data( ), mountPoint.toUtf8( ).constData( ) );

        filesystem->OurMountPoint = mountPoint;
        filesystem->IsReadWrite   = false;

        _fixUpMountPoint( mountPoint );

        printf( "mounted:%s\n", mountPoint.toUtf8( ).constData( ) );
    } else {
        debug( "+ UsbDeviceMounter::_mount: failed to mount filesystem at D-Bus object path '%s': %s\n", path.path( ).toUtf8( ).data( ), reply.error( ).message( ).toUtf8( ).data( ) );
    }
}

void UsbDeviceMounter::_mount_readyReadStandardOutput( QString const& data ) {
    _mountStdoutBuffer += data;

    auto index = _mountStdoutBuffer.lastIndexOf( '\n' );
    if ( -1 == index ) {
        return;
    }

    debug( "[mount/stdout] %s\n", _mountStdoutBuffer.left( index ).replace( NewLineRegex, "\n[mount/stdout] " ).toUtf8( ).data( ) );
    _mountStdoutBuffer.remove( 0, index + 1 );
}

void UsbDeviceMounter::_mount_readyReadStandardError( QString const& data ) {
    _mountStderrBuffer += data;

    auto index = _mountStderrBuffer.lastIndexOf( '\n' );
    if ( -1 == index ) {
        return;
    }

    debug( "[mount/stderr] %s\n", _mountStderrBuffer.left( index ).replace( NewLineRegex, "\n[mount/stderr] " ).toUtf8( ).data( ) );
    _mountStderrBuffer.remove( 0, index + 1 );
}

void UsbDeviceMounter::_remount( QString const& path, bool const writable ) {
    UFilesystem* filesystem;

    auto iter = std::find_if( _filesystems.begin( ), _filesystems.end( ), [ path ] ( UFilesystem* item ) { return item->OurMountPoint == path; } );
    if ( iter == _filesystems.end( ) ) {
        debug( "+ UsbDeviceMounter::_remount: couldn't find mount point '%s' in our list of mounted filesystems, returning failure\n", path.toUtf8( ).data( ) );
        printf( "remount:failure:r%c:%s\n", writable ? 'w' : 'o', path.toUtf8( ).data( ) );
        return;
    } 
    filesystem = *iter;
    if ( filesystem->IsReadWrite == writable ) {
        debug( "+ UsbDeviceMounter::_remount: already mounted read-%s, returning success\n", writable ? "write" : "only" );
        printf( "remount:success:r%c:%s\n", writable ? 'w' : 'o', path.toUtf8( ).data( ) );
        return;
    }

    debug( "+ UsbDeviceMounter::_remount: starting `/bin/mount` to remount mount point '%s' read-%s\n", path.toUtf8( ).data( ), writable ? "write" : "only" );
    _mountProcess = new ProcessRunner;

    (void) QObject::connect( _mountProcess, &ProcessRunner::succeeded, this, [ this, filesystem, writable ] ( ) {
        debug( "+ UsbDeviceMounter::_remount: remount read-%s of %s succeeded\n", writable ? "write" : "only", filesystem->OurMountPoint.toUtf8( ).data( ) );

        _mountProcess->deleteLater( );
        _mountProcess = nullptr;

        filesystem->IsReadWrite = writable;

        _fixUpMountPoint( filesystem->OurMountPoint );

        printf( "remount:success:r%c:%s\n", writable ? 'w' : 'o', filesystem->OurMountPoint.toUtf8( ).data( ) );
    } );

    (void) QObject::connect( _mountProcess, &ProcessRunner::failed, this, [ this, filesystem, writable ] ( ) {
        char const* mountPointText = filesystem->OurMountPoint.toUtf8( ).data( );
        debug( "+ UsbDeviceMounter::_remount: remount read-%s of %s failed\n", writable ? "write" : "only", mountPointText );
        _mountProcess->deleteLater( );
        _mountProcess = nullptr;
        printf( "remount:failure:r%c:%s\n", writable ? 'w' : 'o', mountPointText );
    } );

    (void) QObject::connect( _mountProcess, &ProcessRunner::readyReadStandardOutput, this, &UsbDeviceMounter::_mount_readyReadStandardOutput );
    (void) QObject::connect( _mountProcess, &ProcessRunner::readyReadStandardError,  this, &UsbDeviceMounter::_mount_readyReadStandardError  );

    _mountProcess->start(
        QString { "/bin/mount" },
        QStringList {
            "-o",
            "remount,r" % QChar { writable ? 'w' : 'o' },
            path
        }
    );
}

void UsbDeviceMounter::_driveAdded( QDBusObjectPath const& path, UDrive* drive ) {
    debug( "+ UsbDeviceMounter::_driveAdded: path %s\n", path.path( ).toUtf8( ).data( ) );

    if ( 0 != drive->ConnectionBus.compare( "usb", Qt::CaseInsensitive ) ) {
        debug( "  + bad value for property ConnectionBus: '%s'\n", drive->ConnectionBus.toUtf8( ).data( ) );
        return;
    }
    if ( !drive->MediaAvailable ) {
        debug( "  + bad value for property MediaAvailable: '%s'\n", drive->MediaAvailable ? "true" : "false" );
        return;
    }

    char const* unit;
    double driveSize;
    ScaleSize( drive->Size, driveSize, unit );

    debug(
        "  + Media:  %s\n"
        "  + Vendor: %s\n"
        "  + Model:  %s\n"
        "  + Serial: %s\n"
        "  + Size:   %.2f %s\n"
        "",
        drive->Media.toUtf8( ).data( ),
        drive->Vendor.toUtf8( ).data( ),
        drive->Model.toUtf8( ).data( ),
        drive->Serial.toUtf8( ).data( ),
        driveSize, unit
    );

    drive->setParent( this );
    _drives.insert( path, drive );
}

void UsbDeviceMounter::_blockDeviceAdded( QDBusObjectPath const& path, UBlockDevice* blockDevice ) {
    debug( "+ UsbDeviceMounter::_blockDeviceAdded: path %s\n", path.path( ).toUtf8( ).data( ) );

    if ( !_drives.contains( blockDevice->Drive ) ) {
        debug( "  + not interested in this block device\n" );
        return;
    }

    if ( blockDevice->HintIgnore ) {
        debug( "  + block device says to ignore it, so ignoring\n" );
        return;
    }

    char const* unit;
    double driveSize;
    ScaleSize( blockDevice->Size, driveSize, unit );

    debug(
        "  + System? %s\n"
        "  + Size:   %.2f %s\n"
        "",
        blockDevice->HintSystem ? "yes" : "no",
        driveSize, unit
    );

    blockDevice->setParent( this );
    _blockDevices.insert( path, blockDevice );
}

void UsbDeviceMounter::_filesystemAdded( QDBusObjectPath const& path, UFilesystem* filesystem ) {
    debug( "+ UsbDeviceMounter::_filesystemAdded: path %s\n", path.path( ).toUtf8( ).data( ) );

    if ( !_blockDevices.contains( path ) ) {
        debug( "  + not interested in this filesystem\n" );
        return;
    }

    filesystem->setParent( this );
    _filesystems.insert( path, filesystem );

    _mount( path, filesystem );
}

void UsbDeviceMounter::_driveRemoved( QDBusObjectPath const& path ) {
    debug( "+ UsbDeviceMounter::_driveRemoved: path %s\n", path.path( ).toUtf8( ).data( ) );

    _drives.remove( path );
}

void UsbDeviceMounter::_blockDeviceRemoved( QDBusObjectPath const& path ) {
    debug( "+ UsbDeviceMounter::_blockDeviceRemoved: path %s\n", path.path( ).toUtf8( ).data( ) );

    _blockDevices.remove( path );
}

void UsbDeviceMounter::_filesystemRemoved( QDBusObjectPath const& path ) {
    debug( "+ UsbDeviceMounter::_filesystemRemoved: path %s\n", path.path( ).toUtf8( ).data( ) );

    if ( _filesystems.contains( path ) ) {
        UFilesystem* filesystem = _filesystems[path];
        printf( "unmounted:%s\n", filesystem->OurMountPoint.toUtf8( ).data( ) );
    }

    _filesystems.remove( path );
}

void UsbDeviceMounter::_signalReceived( siginfo_t const& info ) {
    debug( "+ UsbDeviceMounter::_signalReceived: signal %s [%d]\n", ::strsignal( info.si_signo ), info.si_signo );
    qApp->exit( );
}

void UsbDeviceMounter::commandReader_commandReceived( QStringList const& command ) {
    if ( command[0] == "terminate" ) {
        qApp->exit( 0 );
    } else if ( command[0] == "remount-ro" ) {
        _remount( command[1], false );
    } else if ( command[0] == "remount-rw" ) {
        _remount( command[1], true );
    } else {
        printf( "error:%s:Unrecognized command\n", command[0].toUtf8( ).data( ) );
    }
}
