#include "pch.h"

#include "udisksmonitor.h"

namespace {

    //QVariant dbusProperty( QDBusObjectPath const& path, QString const& interface, QString const& property ) {
    //    auto getProperty = QDBusMessage::createMethodCall( UDisks2::Service( ), path.path( ), Interface::Properties( ), "Get" );
    //    getProperty.setArguments( QVariantList { } << interface << property );
    //
    //    QDBusReply<QVariant> reply = QDBusConnection::systemBus( ).call( getProperty, QDBus::Block, 500 );
    //    if ( !reply.isValid( ) ) {
    //        debug( "+ dbusProperty: failed to access property %s on interface %s at path %s: %s\n", property.toUtf8( ).data( ), interface.toUtf8( ).data( ), path.path( ).toUtf8( ).data( ), reply.error( ).message( ).toUtf8( ).data( ) );
    //        return { };
    //    }
    //    return reply.value( );
    //}
    //
    //QString introspect( const QString& path, int replyTimeout ) {
    //    QDBusMessage introspect = QDBusMessage::createMethodCall( UDisks2::Service( ), path, Interface::Introspectable( ), "Introspect" );
    //
    //    QDBusReply<QString> reply = QDBusConnection::systemBus( ).call( QDBusMessage::createMethodCall( UDisks2::Service( ), path, Interface::Introspectable( ), "Introspect" ), QDBus::Block, replyTimeout );
    //    if ( !reply.isValid( ) ) {
    //        debug( "+ introspect: failed on path %s: %s\n", path.toUtf8( ).data( ), reply.error( ).message( ).toUtf8( ).data( ) );
    //        return { };
    //    }
    //    return reply.value( );
    //}
    //
    //bool hasInterface( const QDBusObjectPath& path, const QString& interface, int replyTimeout ) {
    //    QXmlStreamReader xml( introspect( path.path( ), replyTimeout ) );
    //    while ( !xml.atEnd( ) ) {
    //        xml.readNext( );
    //        if ( xml.isStartElement( ) && xml.name( ) == "interface" && xml.attributes( ).value( "name" ) == interface ) {
    //            return true;
    //        }
    //    }
    //
    //    return false;
    //}
    //
    //void dumpVariant( QVariant const& value, QString const& name ) {
    //    QString valueString;
    //    QString typeName { value.typeName( ) };
    //    if ( typeName == "QDBusObjectPath" ) {
    //        valueString = value.value<QDBusObjectPath>( ).path( );
    //    } else if ( typeName == "QDBusArgument" ) {
    //        debug( "    + %s %s =>\n", value.typeName( ), name.toUtf8( ).data( ) );
    //        QDBusArgument arg = value.value<QDBusArgument>( );
    //        while ( !arg.atEnd( ) ) {
    //            auto type = arg.currentType( );
    //            if ( QDBusArgument::UnknownType == type ) {
    //                debug( "      + unknown type\n" );
    //                continue;
    //            }
    //
    //            QDBusVariant dv;
    //            arg >> dv;
    //            debug( "      + [%s] %s => '%s'\n", QDBusArgumentElementTypeStrings[type], dv.variant( ).typeName( ), dv.variant( ).toString( ).toUtf8( ).data( ) );
    //        }
    //        return;
    //    } else {
    //        valueString = value.toString( );
    //    }
    //    debug( "    + %s %s => '%s'\n", value.typeName( ), name.toUtf8( ).data( ), valueString.toUtf8( ).data( ) );
    //}
    
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
    qDBusRegisterMetaType<InterfaceList>( );

    auto systemBus = QDBusConnection::systemBus( );
    debug( "+ UDisksMonitor::`ctor: systemBus.isConnected %s\n", systemBus.isConnected( ) ? "true" : "false" );

    bool result;

    debug( "  + attempting to connect to signals on interface %s on service %s at path %s\n", Interface::ObjectManager( ).toUtf8( ).data( ), UDisks2::Service( ).toUtf8( ).data( ), UDisks2::Path( ).toUtf8( ).data( ) );
    result = systemBus.connect(
        UDisks2::Service( ),
        UDisks2::Path( ),
        Interface::ObjectManager( ),
        "InterfacesAdded",
        this,
        SLOT( _interfacesAdded( QDBusObjectPath const&, InterfaceList const& ) )
    );
    debug( "  + connect to InterfacesAdded:   success? %s\n", result ? "true" : "false" );

    result = systemBus.connect(
        UDisks2::Service( ),
        UDisks2::Path( ),
        Interface::ObjectManager( ),
        "InterfacesRemoved",
        this,
        SLOT( _interfacesRemoved( QDBusObjectPath const&, QStringList const& ) )
    );
    debug( "  + connect to InterfacesRemoved: success? %s\n", result ? "true" : "false" );
}

UDisksMonitor::~UDisksMonitor( ) {
    /*empty*/
}

void UDisksMonitor::_interfacesAdded( QDBusObjectPath const& path, InterfaceList const& interfaces ) {
    debug( "+ UDisksMonitor::_interfacesAdded: %d interface%s at path %s\n", interfaces.count( ), ( interfaces.count( ) == 1 ? "" : "s" ), path.path( ).toUtf8( ).data( ) );

    UDrive*          drive          { };
    UBlockDevice*    blockDevice    { };
    UPartitionTable* partitionTable { };
    UPartition*      partition      { };
    UFilesystem*     filesystem     { };

    for ( auto interfaceName : interfaces.keys( ) ) {
        debug( "  + interface: %s\n", interfaceName.toUtf8( ).data( ) );
        auto values = interfaces[interfaceName];

        if ( isDrive( interfaceName ) ) {
            debug( "    + gained drive %s\n", path.path( ).toUtf8( ).data( ) );
            drive = new UDrive( path, values, this );
            _drives.insert( path, drive );
        }

        if ( isBlockDevice( interfaceName ) ) {
            debug( "    + gained block device %s\n", path.path( ).toUtf8( ).data( ) );
            blockDevice = new UBlockDevice( path, values, this );
            _blockDevices.insert( path, blockDevice );
        }

        if ( isPartitionTable( interfaceName ) ) {
            debug( "    + gained partition table %s\n", path.path( ).toUtf8( ).data( ) );
            partitionTable = new UPartitionTable( path, values, this );
            _partitionTables.insert( path, partitionTable );
        }

        if ( isPartition( interfaceName ) ) {
            debug( "    + gained partition %s\n", path.path( ).toUtf8( ).data( ) );
            partition = new UPartition( path, values, this );
            _partitions.insert( path, partition );
        }

        if ( isFilesystem( interfaceName ) ) {
            debug( "    + gained filesystem %s\n", path.path( ).toUtf8( ).data( ) );
            filesystem = new UFilesystem( path, values, this );
            _filesystems.insert( path, filesystem );
        }
    }

    if ( drive ) {
        emit driveAdded( path, drive );
    }
    if ( blockDevice ) {
        emit blockDeviceAdded( path, blockDevice );
    }
    if ( partition ) {
        emit partitionAdded( path, partition );
    }
    if ( partitionTable ) {
        emit partitionTableAdded( path, partitionTable );
    }
    if ( filesystem ) {
        emit filesystemAdded( path, filesystem );
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
