#if ! defined __USBDEVICEMOUNTER_H__
#define __USBDEVICEMOUNTER_H__

class ProcessRunner;
class SignalHandler;
class UDisksMonitor;
class UDrive;
class UPartitionTable;
class UPartition;
class UFilesystem;
class UBlockDevice;

class UsbDeviceMounter: public QObject {

    Q_OBJECT

public:

    UsbDeviceMounter( UDisksMonitor& monitor, QObject* parent = nullptr );
    virtual ~UsbDeviceMounter( ) override;

protected:

private:

    SignalHandler*                       _signalHandler     { };
    UDisksMonitor&                       _monitor;
    QMap<QDBusObjectPath, UDrive*>       _drives;
    QMap<QDBusObjectPath, UBlockDevice*> _blockDevices;
    QMap<QDBusObjectPath, UFilesystem*>  _filesystems;

    ProcessRunner*                       _mountProcess      { };
    QString                              _mountStdoutBuffer;
    QString                              _mountStderrBuffer;

    void _fixUpMountPoint( QString const& mountPoint );
    void _mount( QDBusObjectPath const& path, UFilesystem* filesystem );
    void _remount( QString const& path, bool const writable );

signals:
    ;

    void filesystemMounted( QString const& path );

public slots:
    ;

    void commandReader_commandReceived( QStringList const& command );

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

    void _signalReceived     ( siginfo_t const& info );

    void _mount_readyReadStandardOutput( QString const& data );
    void _mount_readyReadStandardError( QString const& data );

};

#endif // ! defined __USBDEVICEMOUNTER_H__
