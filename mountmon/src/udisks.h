#if ! defined __UDISKS_H__
#define __UDISKS_H__

namespace UDisks2 {

    inline constexpr char const* Service( )                       { return "org.freedesktop.UDisks2";                 }
    inline constexpr char const* Path( )                          { return "/org/freedesktop/UDisks2";                }
    inline           QString     Path( char const* subPath )      { return QString { Path( ) } % '/' % subPath;       }

}

namespace Interface {

    inline constexpr char const* UDisks2( )                       { return "org.freedesktop.UDisks2";                 }
    inline           QString     UDisks2( char const* interface ) { return QString { UDisks2( ) }  % '.' % interface; }
    inline constexpr char const* ObjectManager( )                 { return "org.freedesktop.DBus.ObjectManager";      }
    inline constexpr char const* Introspectable( )                { return "org.freedesktop.DBus.Introspectable";     }
    inline constexpr char const* Properties( )                    { return "org.freedesktop.DBus.Properties";         }

}

class UDrive;
class UBlockDevice;
class UPartitionTable;
class UPartition;
class UFilesystem;

class UDrive: public QObject {

    Q_OBJECT

public:

    UDrive( QDBusObjectPath const& path, QVariantMap const& values, QObject* parent = nullptr );
    virtual ~UDrive( ) override;

    bool                 CanPowerOff;
    QDBusArgument        Configuration;
    QString              ConnectionBus;
    bool                 Ejectable;
    QString              Id;
    QString              Media;
    bool                 MediaAvailable;
    bool                 MediaChangeDetected;
    QStringList          MediaCompatibility;
    bool                 MediaRemovable;
    QString              Model;
    bool                 Optical;
    bool                 OpticalBlank;
    uint                 OpticalNumAudioTracks;
    uint                 OpticalNumDataTracks;
    uint                 OpticalNumSessions;
    uint                 OpticalNumTracks;
    bool                 Removable;
    QString              Revision;
    int                  RotationRate;
    QString              Seat;
    QString              Serial;
    QString              SiblingId;
    qulonglong           Size;
    QString              SortKey;
    qulonglong           TimeDetected;
    qulonglong           TimeMediaDetected;
    QString              Vendor;
    QString              WWN;

    QList<UBlockDevice*> BlockDevices;
    QDBusObjectPath      Path;

protected:

private:

signals:
    ;

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

class UBlockDevice: public QObject {

    Q_OBJECT

public:

    UBlockDevice( QDBusObjectPath const& path, QVariantMap const& values, QObject* parent = nullptr );
    virtual ~UBlockDevice( ) override;

    QDBusArgument       Configuration;
    QDBusObjectPath     CryptoBackingDevice;
    QByteArray          Device;
    qulonglong          DeviceNumber;
    QDBusObjectPath     Drive;
    bool                HintAuto;
    QString             HintIconName;
    bool                HintIgnore;
    QString             HintName;
    bool                HintPartitionable;
    QString             HintSymbolicIconName;
    bool                HintSystem;
    QString             Id;
    QString             IdLabel;
    QString             IdType;
    QString             IdUUID;
    QString             IdUsage;
    QString             IdVersion;
    QDBusObjectPath     MDRaid;
    QDBusObjectPath     MDRaidMember;
    QByteArray          PreferredDevice;
    bool                ReadOnly;
    qulonglong          Size;
    QDBusArgument       Symlinks;
    QStringList         UserspaceMountOptions;

    QDBusObjectPath     Path;

protected:

private:

signals:
    ;

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

class UPartitionTable: public QObject {

    Q_OBJECT

public:

    UPartitionTable( QDBusObjectPath const& path, QVariantMap const& values, QObject* parent = nullptr );
    virtual ~UPartitionTable( ) override;

    QDBusArgument   Partitions;
    QString         Type;

    QDBusObjectPath Path;

protected:

private:

signals:
    ;

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

class UPartition: public QObject {

    Q_OBJECT

public:

    UPartition( QDBusObjectPath const& path, QVariantMap const& values, QObject* parent = nullptr );
    virtual ~UPartition( ) override;

    qulonglong      Flags;
    bool            IsContained;
    bool            IsContainer;
    QString         Name;
    uint            Number;
    qulonglong      Offset;
    qulonglong      Size;
    QDBusObjectPath Table;
    QString         Type;
    QString         UUID;

    QDBusObjectPath Path;

protected:

private:

signals:
    ;

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

class UFilesystem: public QObject {

    Q_OBJECT

public:

    UFilesystem( QDBusObjectPath const& path, QVariantMap const& values, QObject* parent = nullptr );
    virtual ~UFilesystem( ) override;

    QDBusArgument   MountPoints;
    qulonglong      Size;

    QDBusObjectPath Path;
    QString         OurMountPoint;

    bool            IsReadWrite   { };

protected:

private:

signals:
    ;

public slots:
    ;

protected slots:
    ;

private slots:
    ;

};

#endif // ! defined __UDISKS_H__
