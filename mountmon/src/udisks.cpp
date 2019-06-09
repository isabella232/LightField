#include "pch.h"

#include "udisks.h"

//
// Class UDrive
//

UDrive::UDrive( QDBusObjectPath const& path, QVariantMap& values, QObject* parent ): QObject ( parent ) {
    Path                  = path;
    CanPowerOff           = values["CanPowerOff"          ].value<bool         >( );
    Configuration         = values["Configuration"        ].value<QDBusArgument>( );
    ConnectionBus         = values["ConnectionBus"        ].value<QString      >( );
    Ejectable             = values["Ejectable"            ].value<bool         >( );
    Id                    = values["Id"                   ].value<QString      >( );
    Media                 = values["Media"                ].value<QString      >( );
    MediaAvailable        = values["MediaAvailable"       ].value<bool         >( );
    MediaChangeDetected   = values["MediaChangeDetected"  ].value<bool         >( );
    MediaCompatibility    = values["MediaCompatibility"   ].value<QStringList  >( );
    MediaRemovable        = values["MediaRemovable"       ].value<bool         >( );
    Model                 = values["Model"                ].value<QString      >( );
    Optical               = values["Optical"              ].value<bool         >( );
    OpticalBlank          = values["OpticalBlank"         ].value<bool         >( );
    OpticalNumAudioTracks = values["OpticalNumAudioTracks"].value<uint         >( );
    OpticalNumDataTracks  = values["OpticalNumDataTracks" ].value<uint         >( );
    OpticalNumSessions    = values["OpticalNumSessions"   ].value<uint         >( );
    OpticalNumTracks      = values["OpticalNumTracks"     ].value<uint         >( );
    Removable             = values["Removable"            ].value<bool         >( );
    Revision              = values["Revision"             ].value<QString      >( );
    RotationRate          = values["RotationRate"         ].value<int          >( );
    Seat                  = values["Seat"                 ].value<QString      >( );
    Serial                = values["Serial"               ].value<QString      >( );
    SiblingId             = values["SiblingId"            ].value<QString      >( );
    Size                  = values["Size"                 ].value<qulonglong   >( );
    SortKey               = values["SortKey"              ].value<QString      >( );
    TimeDetected          = values["TimeDetected"         ].value<qulonglong   >( );
    TimeMediaDetected     = values["TimeMediaDetected"    ].value<qulonglong   >( );
    Vendor                = values["Vendor"               ].value<QString      >( );
    WWN                   = values["WWN"                  ].value<QString      >( );
}

UDrive::~UDrive( ) {
    /*empty*/
}

//
// Class UBlockDevice
//

UBlockDevice::UBlockDevice( QDBusObjectPath const& path, QVariantMap& values, QObject* parent ): QObject( parent ) {
    Path                  = path;
    Configuration         = values["Configuration"        ].value<QDBusArgument  >( );
    CryptoBackingDevice   = values["CryptoBackingDevice"  ].value<QDBusObjectPath>( );
    Device                = values["Device"               ].value<QByteArray     >( );
    DeviceNumber          = values["DeviceNumber"         ].value<qulonglong     >( );
    Drive                 = values["Drive"                ].value<QDBusObjectPath>( );
    HintAuto              = values["HintAuto"             ].value<bool           >( );
    HintIconName          = values["HintIconName"         ].value<QString        >( );
    HintIgnore            = values["HintIgnore"           ].value<bool           >( );
    HintName              = values["HintName"             ].value<QString        >( );
    HintPartitionable     = values["HintPartitionable"    ].value<bool           >( );
    HintSymbolicIconName  = values["HintSymbolicIconName" ].value<QString        >( );
    HintSystem            = values["HintSystem"           ].value<bool           >( );
    Id                    = values["Id"                   ].value<QString        >( );
    IdLabel               = values["IdLabel"              ].value<QString        >( );
    IdType                = values["IdType"               ].value<QString        >( );
    IdUUID                = values["IdUUID"               ].value<QString        >( );
    IdUsage               = values["IdUsage"              ].value<QString        >( );
    IdVersion             = values["IdVersion"            ].value<QString        >( );
    MDRaid                = values["MDRaid"               ].value<QDBusObjectPath>( );
    MDRaidMember          = values["MDRaidMember"         ].value<QDBusObjectPath>( );
    PreferredDevice       = values["PreferredDevice"      ].value<QByteArray     >( );
    ReadOnly              = values["ReadOnly"             ].value<bool           >( );
    Size                  = values["Size"                 ].value<qulonglong     >( );
    Symlinks              = values["Symlinks"             ].value<QDBusArgument  >( );
    UserspaceMountOptions = values["UserspaceMountOptions"].value<QStringList    >( );
}

UBlockDevice::~UBlockDevice( ) {
    /*empty*/
}

//
// Class UPartitionTable
//

UPartitionTable::UPartitionTable( QDBusObjectPath const& path, QVariantMap& values, QObject* parent ): QObject( parent ) {
    Path       = path;
    Partitions = values["Partitions"].value<QDBusArgument>( );
    Type       = values["Type"      ].value<QString      >( );
}

UPartitionTable::~UPartitionTable( ) {
    /*empty*/
}

//
// Class UPartition
//

UPartition::UPartition( QDBusObjectPath const& path, QVariantMap& values, QObject* parent ): QObject( parent ) {
    Path        = path;
    Flags       = values["Flags"      ].value<qulonglong     >( );
    IsContained = values["IsContained"].value<bool           >( );
    IsContainer = values["IsContainer"].value<bool           >( );
    Name        = values["Name"       ].value<QString        >( );
    Number      = values["Number"     ].value<uint           >( );
    Offset      = values["Offset"     ].value<qulonglong     >( );
    Size        = values["Size"       ].value<qulonglong     >( );
    Table       = values["Table"      ].value<QDBusObjectPath>( );
    Type        = values["Type"       ].value<QString        >( );
    UUID        = values["UUID"       ].value<QString        >( );
}

UPartition::~UPartition( ) {
    /*empty*/
}

//
// Class UFilesystem
//

UFilesystem::UFilesystem( QDBusObjectPath const& path, QVariantMap& values, QObject* parent ): QObject( parent ) {
    Path        = path;
    MountPoints = values["MountPoints"].value<QDBusArgument>( );
    Size        = values["Size"       ].value<qulonglong   >( );
}

UFilesystem::~UFilesystem( ) {
    /*empty*/
}
