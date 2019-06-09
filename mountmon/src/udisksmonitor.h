#if ! defined __UDISKSMONITOR_H__
#define __UDISKSMONITOR_H__

#include <QObject>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus/QDBusArgument>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusObjectPath>

using InterfaceList = QMap<QString, QVariantMap>;
Q_DECLARE_METATYPE( InterfaceList )

#include "udisks.h"

class UDisksMonitor: public QObject {

    Q_OBJECT

public:

    UDisksMonitor( QObject* parent = nullptr );
    virtual ~UDisksMonitor( ) override;

protected:

private:

    QMap<QDBusObjectPath, UDrive*>          _drives;
    QMap<QDBusObjectPath, UBlockDevice*>    _blockDevices;
    QMap<QDBusObjectPath, UPartitionTable*> _partitionTables;
    QMap<QDBusObjectPath, UPartition*>      _partitions;
    QMap<QDBusObjectPath, UFilesystem*>     _filesystems;

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
