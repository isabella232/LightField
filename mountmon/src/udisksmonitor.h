#if ! defined __UDISKSMONITOR_H__
#define __UDISKSMONITOR_H__

using InterfaceList = QMap<QString, QVariantMap>;

#include "udisks.h"

class UDisksMonitor: public QObject {

    Q_OBJECT

public:

    UDisksMonitor( QObject* parent = nullptr );
    virtual ~UDisksMonitor( ) override;

    void probeForExistingDevices( );

protected:

private:

    QMap<QDBusObjectPath, UDrive*>          _drives;
    QMap<QDBusObjectPath, UBlockDevice*>    _blockDevices;
    QMap<QDBusObjectPath, UPartitionTable*> _partitionTables;
    QMap<QDBusObjectPath, UPartition*>      _partitions;
    QMap<QDBusObjectPath, UFilesystem*>     _filesystems;

    void _createObject( QDBusObjectPath const& path, QString const& interface, QVariantMap const& values );

signals:
    ;

    void driveAdded            ( QDBusObjectPath const& path, UDrive*          drive          );
    void blockDeviceAdded      ( QDBusObjectPath const& path, UBlockDevice*    blockDevice    );
    void partitionTableAdded   ( QDBusObjectPath const& path, UPartitionTable* partitionTable );
    void partitionAdded        ( QDBusObjectPath const& path, UPartition*      partition      );
    void filesystemAdded       ( QDBusObjectPath const& path, UFilesystem*     filesystem     );

    void driveRemoved          ( QDBusObjectPath const& path, UDrive*          drive          );
    void blockDeviceRemoved    ( QDBusObjectPath const& path, UBlockDevice*    blockDevice    );
    void partitionTableRemoved ( QDBusObjectPath const& path, UPartitionTable* partitionTable );
    void partitionRemoved      ( QDBusObjectPath const& path, UPartition*      partition      );
    void filesystemRemoved     ( QDBusObjectPath const& path, UFilesystem*     filesystem     );

    void ready( );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void _interfacesAdded( QDBusObjectPath const& path, InterfaceList const& interfaces );
    void _interfacesRemoved( QDBusObjectPath const& path, QStringList const& interfaces );

};

#endif // ! defined __UDISKSMONITOR_H__
