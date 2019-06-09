#ifndef __USBDEVICEMOUNTER_H__
#define __USBDEVICEMOUNTER_H__

#include <QObject>
#include <QtDBus/QDBusObjectPath>

class UDisksMonitor;
class UDrive;
class UPartitionTable;
class UPartition;
class UFilesystem;
class UBlockDevice;

class UsbDeviceMounter: public QObject {

    Q_OBJECT

public:

    UsbDeviceMounter( UDisksMonitor* monitor, QObject* parent = nullptr );
    virtual ~UsbDeviceMounter( ) override;

protected:

private:

    UDisksMonitor*                          _monitor;
    QMap<QDBusObjectPath, UDrive*>          _drives;
    QMap<QDBusObjectPath, UBlockDevice*>    _blockDevices;
    QMap<QDBusObjectPath, UFilesystem*>     _filesystems;

    void _mount( QDBusObjectPath const& path, UFilesystem* filesystem );

signals:
    ;

    void filesystemMounted( QString const& path );

public slots:
    ;

protected slots:
    ;

private slots:
    ;

    void _driveAdded         ( QDBusObjectPath const& path, UDrive*       drive       );
    void _blockDeviceAdded   ( QDBusObjectPath const& path, UBlockDevice* blockDevice );
    void _filesystemAdded    ( QDBusObjectPath const& path, UFilesystem*  filesystem  );

    void _driveRemoved       ( QDBusObjectPath const& path );
    void _blockDeviceRemoved ( QDBusObjectPath const& path );
    void _filesystemRemoved  ( QDBusObjectPath const& path );

};

#endif // __USBDEVICEMOUNTER_H__
