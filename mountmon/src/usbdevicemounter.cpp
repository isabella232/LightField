#include "pch.h"

#include "usbdevicemounter.h"

#include "signalhandler.h"
#include "udisksmonitor.h"

namespace {

    char const* DriveSizeUnits[] = { "iB", "KiB", "MiB", "GiB", "TiB", "PiB", "EiB", "ZiB", "YiB" };

    std::initializer_list<int> signalList {
        SIGHUP,
        SIGINT,
        SIGPIPE,
        SIGQUIT,
        SIGTERM,
    };

}


UsbDeviceMounter::UsbDeviceMounter( UDisksMonitor* monitor, QObject* parent ):
    QObject  ( parent  ),
    _monitor ( monitor )
{
    QObject::connect( g_signalHandler, &SignalHandler::signalReceived,     this, &UsbDeviceMounter::_signalReceived     );
    g_signalHandler->subscribe( signalList );

    QObject::connect( _monitor,        &UDisksMonitor::driveAdded,         this, &UsbDeviceMounter::_driveAdded         );
    QObject::connect( _monitor,        &UDisksMonitor::filesystemAdded,    this, &UsbDeviceMounter::_filesystemAdded    );
    QObject::connect( _monitor,        &UDisksMonitor::blockDeviceAdded,   this, &UsbDeviceMounter::_blockDeviceAdded   );
                                       
    QObject::connect( _monitor,        &UDisksMonitor::driveRemoved,       this, &UsbDeviceMounter::_driveRemoved       );
    QObject::connect( _monitor,        &UDisksMonitor::filesystemRemoved,  this, &UsbDeviceMounter::_filesystemRemoved  );
    QObject::connect( _monitor,        &UDisksMonitor::blockDeviceRemoved, this, &UsbDeviceMounter::_blockDeviceRemoved );
}

UsbDeviceMounter::~UsbDeviceMounter( ) {
    QObject::disconnect( _monitor, nullptr, this, nullptr );
    _monitor = nullptr;
}

void UsbDeviceMounter::_mount( QDBusObjectPath const& path, UFilesystem* filesystem ) {
    debug( "+ UsbDeviceMounter::_mount: attempting to mount filesystem at path %s\n", path.path( ).toUtf8( ).data( ) );
    auto mount = QDBusMessage::createMethodCall( UDisks2::Service( ), path.path( ), Interface::UDisks2( "Filesystem" ), "Mount" );
    mount.setArguments( QVariantList { } << QVariantMap { { "options", QString { "ro,noexec,umask=0222" } } } );
    QDBusReply<QString> reply = QDBusConnection::systemBus( ).call( mount, QDBus::Block, -1 );
    if ( reply.isValid( ) ) {
        auto mountPoint = reply.value( );
        auto mountPointText = mountPoint.toUtf8( ).data( );
        debug( "+ UsbDeviceMounter::_mount: filesystem at path %s mounted at %s\n", path.path( ).toUtf8( ).data( ), mountPointText );
        printf( "mounted:%s\n", mountPointText );
        filesystem->OurMountPoint = mountPoint;
        chmod( mountPointText, 0555 );
        auto index = mountPoint.lastIndexOf( '/' );
        if ( index > 0 ) {
            chmod( mountPoint.left( index ).toUtf8( ).data( ), 0755 );
        }
    } else {
        debug( "+ UsbDeviceMounter::_mount: failed to mount filesystem at path %s: %s\n", path.path( ).toUtf8( ).data( ), reply.error( ).message( ).toUtf8( ).data( ) );
    }
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

    double driveSize = drive->Size;
    int unitIndex = 0;
    while ( driveSize > 1024.0 ) {
        ++unitIndex;
        driveSize /= 1024.0;
    }

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
        driveSize, DriveSizeUnits[unitIndex]
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

void UsbDeviceMounter::_signalReceived( int const signalNumber ) {
    debug( "+ UsbDeviceMounter::_signalReceived: signal %d\n", signalNumber );
    qApp->exit( );
}
