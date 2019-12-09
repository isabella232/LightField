#include "pch.h"

#include "udisksmonitor.h"

using GetManagedObjectsResult = QMap<QDBusObjectPath, InterfaceList>;

Q_DECLARE_METATYPE( GetManagedObjectsResult );
Q_DECLARE_METATYPE( InterfaceList )

namespace {

    bool isDrive( QString const& interface ) {
        return interface == Interface::UDisks2( "Drive" );
    }

    bool isBlockDevice( QString const& interface ) {
        return interface == Interface::UDisks2( "Block" );
    }

    bool isPartitionTable( QString const& interface ) {
        return interface == Interface::UDisks2( "PartitionTable" );
    }

    bool isPartition( QString const& interface ) {
        return interface == Interface::UDisks2( "Partition" );
    }

    bool isFilesystem( QString const& interface ) {
        return interface == Interface::UDisks2( "Filesystem" );
    }

}

UDisksMonitor::UDisksMonitor( QObject* parent ): QObject ( parent ) {
    qDBusRegisterMetaType<GetManagedObjectsResult>( );
    qDBusRegisterMetaType<InterfaceList>( );

    auto systemBus = QDBusConnection::systemBus( );
    debug( "+ UDisksMonitor::`ctor: systemBus.isConnected? %s\n", systemBus.isConnected( ) ? "true" : "false" );

    bool result;

    debug( "  + attempting to connect to signals on service %s at path %s via interface %s\n", UDisks2::Service( ), UDisks2::Path( ), Interface::ObjectManager( ) );
    result = systemBus.connect(
        UDisks2::Service( ),
        UDisks2::Path( ),
        Interface::ObjectManager( ),
        "InterfacesAdded",
        this,
        SLOT( _interfacesAdded( QDBusObjectPath const&, InterfaceList const& ) )
    );
    debug( "  + connect to InterfacesAdded:   %s\n", result ? "succeeded" : "failed" );

    result = systemBus.connect(
        UDisks2::Service( ),
        UDisks2::Path( ),
        Interface::ObjectManager( ),
        "InterfacesRemoved",
        this,
        SLOT( _interfacesRemoved( QDBusObjectPath const&, QStringList const& ) )
    );
    debug( "  + connect to InterfacesRemoved: %s\n", result ? "succeeded" : "failed" );
}

UDisksMonitor::~UDisksMonitor( ) {
    /*empty*/
}

void UDisksMonitor::_createObject( QDBusObjectPath const& path, QString const& interfaceName, QVariantMap const& values ) {
    debug( "+ UDisksMonitor::_createObject: path %s interface %s\n", path.path( ).toUtf8( ).data( ), interfaceName.toUtf8( ).data( ) );
    if ( isDrive( interfaceName ) ) {
        debug( "  + gained drive %s\n", path.path( ).toUtf8( ).data( ) );
        auto drive = new UDrive( path, values, this );
        _drives.insert( path, drive );
        emit driveAdded( path, drive );
    } else if ( isBlockDevice( interfaceName ) ) {
        debug( "  + gained block device %s\n", path.path( ).toUtf8( ).data( ) );
        auto blockDevice = new UBlockDevice( path, values, this );
        _blockDevices.insert( path, blockDevice );
        emit blockDeviceAdded( path, blockDevice );
    } else if ( isPartitionTable( interfaceName ) ) {
        debug( "  + gained partition table %s\n", path.path( ).toUtf8( ).data( ) );
        auto partitionTable = new UPartitionTable( path, values, this );
        _partitionTables.insert( path, partitionTable );
        emit partitionTableAdded( path, partitionTable );
    } else if ( isPartition( interfaceName ) ) {
        debug( "  + gained partition %s\n", path.path( ).toUtf8( ).data( ) );
        auto partition = new UPartition( path, values, this );
        _partitions.insert( path, partition );
        emit partitionAdded( path, partition );
    } else if ( isFilesystem( interfaceName ) ) {
        debug( "  + gained filesystem %s\n", path.path( ).toUtf8( ).data( ) );
        auto filesystem = new UFilesystem( path, values, this );
        _filesystems.insert( path, filesystem );
        emit filesystemAdded( path, filesystem );
    }
}

void UDisksMonitor::probeForExistingDevices( ) {
    debug( "+ UDisksMonitor::probeForExistingDevices: attempting to find existing objects by calling GetManagedObjects\n" );
    QDBusReply<GetManagedObjectsResult> reply {
        QDBusConnection::systemBus( ).call(
            QDBusMessage::createMethodCall(
                UDisks2::Service( ),
                UDisks2::Path( ),
                Interface::ObjectManager( ),
                "GetManagedObjects"
            )
        )
    };
    if ( !reply.isValid( ) ) {
        auto const& err = reply.error( );
        if ( err.isValid( ) ) {
            debug(
                "+ UDisksMonitor::probeForExistingDevices: GetManagedObjects call failed:\n"
                "  + error type:    %s\n"
                "  + error name:    %s\n"
                "  + error message: %s\n"
                "",
                QDBusError::errorString( err.type( ) ).toUtf8( ).data( ),
                err.name( ).toUtf8( ).data( ),
                err.message( ).toUtf8( ).data( )
            );
        } else {
            debug( "+ UDisksMonitor::probeForExistingDevices: GetManagedObjects call failed and detailed error information is not available\n" );
        }
        return;
    }

    debug( "+ UDisksMonitor::probeForExistingDevices: GetManagedObjects call results:\n" );
    auto objectPaths { reply.value( ) };

    // first, find Drive objects
    debug( "+ UDisksMonitor::probeForExistingDevices: Drive objects:\n" );
    for ( auto objectPathsIter = objectPaths.begin( ); objectPathsIter != objectPaths.end( ); ++objectPathsIter ) {
        auto const& objectPath    { objectPathsIter.key( )   };
        auto const& interfaceList { objectPathsIter.value( ) };
        for ( auto interfaceListIter = interfaceList.begin( ); interfaceListIter != interfaceList.end( ); ++interfaceListIter ) {
            auto const& interface  { interfaceListIter.key( )   };
            auto const& properties { interfaceListIter.value( ) };

            if ( isDrive( interface ) ) {
                debug( "+ UDisksMonitor::probeForExistingDevices: Object path: %s\n", objectPath.path( ).toUtf8( ).data( ) );
                _createObject( objectPath, interface, properties );
            }
        }
    }

    // then, find everything else
    debug( "+ UDisksMonitor::probeForExistingDevices: Other objects:\n" );
    for ( auto objectPathsIter = objectPaths.begin( ); objectPathsIter != objectPaths.end( ); ++objectPathsIter ) {
        auto const& objectPath    { objectPathsIter.key( )   };
        auto const& interfaceList { objectPathsIter.value( ) };
        for ( auto interfaceListIter = interfaceList.begin( ); interfaceListIter != interfaceList.end( ); ++interfaceListIter ) {
            auto const& interface { interfaceListIter.key( ) };
            auto const& properties { interfaceListIter.value( ) };

            if ( !isDrive( interface ) ) {
                debug( "+ UDisksMonitor::probeForExistingDevices: Object path: %s\n", objectPath.path( ).toUtf8( ).data( ) );
                _createObject( objectPath, interface, properties );
            }
        }
    }

    emit ready( );
}

void UDisksMonitor::_interfacesAdded( QDBusObjectPath const& path, InterfaceList const& interfaces ) {
    debug( "+ UDisksMonitor::_interfacesAdded: %d interface%s at path %s\n", interfaces.count( ), ( interfaces.count( ) == 1 ? "" : "s" ), path.path( ).toUtf8( ).data( ) );

    for ( auto interfaceName : interfaces.keys( ) ) {
        _createObject( path, interfaceName, interfaces[interfaceName] );
    }
}

void UDisksMonitor::_interfacesRemoved( QDBusObjectPath const& path, QStringList const& interfaces ) {
    debug( "+ UDisksMonitor::_interfacesRemoved: %d interface%s at path %s\n", interfaces.count( ), ( interfaces.count( ) == 1 ? "" : "s" ), path.path( ).toUtf8( ).data( ) );
    for ( auto interfaceName : interfaces ) {
        debug( "  + interface: %s\n", interfaceName.toUtf8( ).data( ) );

        if ( isDrive( interfaceName ) ) {
            debug( "    + lost drive %s\n", path.path( ).toUtf8( ).data( ) );
            emit driveRemoved( path, _drives.contains( path ) ? _drives[path] : nullptr );
            _drives.remove( path );
        }
        if ( isBlockDevice( interfaceName ) ) {
            debug( "    + lost block device %s\n", path.path( ).toUtf8( ).data( ) );
            emit blockDeviceRemoved( path, _blockDevices.contains( path ) ? _blockDevices[path] : nullptr );
            _blockDevices.remove( path );
        }
        if ( isPartitionTable( interfaceName ) ) {
            debug( "    + lost partition table %s\n", path.path( ).toUtf8( ).data( ) );
            emit partitionTableRemoved( path, _partitionTables.contains( path ) ? _partitionTables[path] : nullptr );
            _partitionTables.remove( path );
        }
        if ( isPartition( interfaceName ) ) {
            debug( "    + lost partition %s\n", path.path( ).toUtf8( ).data( ) );
            emit partitionRemoved( path, _partitions.contains( path ) ? _partitions[path] : nullptr );
            _partitions.remove( path );
        }
        if ( isFilesystem( interfaceName ) ) {
            debug( "    + lost file system %s\n", path.path( ).toUtf8( ).data( ) );
            emit filesystemRemoved( path, _filesystems.contains( path ) ? _filesystems[path] : nullptr );
            _filesystems.remove( path );
        }
    }
}
