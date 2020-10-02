#ifndef __USBMOUNTMANAGER_H__
#define __USBMOUNTMANAGER_H__

#include <QtCore>
#include <QSocketNotifier>
#include <libudev.h>

#define UDEV_SUBSYSTEM "block"
#define UDEV_DEVTYPE "partition"
#define UDEV_BUS_PROP "ID_BUS"
#define UDEV_DEVNAME "DEVNAME"
#define UDEV_FS_TYPE "ID_FS_TYPE"
#define UDEV_BUS_ID "usb"
#define UDEV_DEV_MODEL "ID_MODEL"

class UsbDevice
{
public:
    UsbDevice(): _dev(nullptr) {}
    UsbDevice(struct udev_device *dev): _dev(dev) {}
    ~UsbDevice() {
        udev_device_unref(_dev);
    }

    QString getDevPath() const {
        if (_dev == nullptr)
            return QString();
        return udev_device_get_property_value(_dev, UDEV_DEVNAME);
    }

    QString getModel() const {
        if (_dev == nullptr)
            return QString();
        return udev_device_get_property_value(_dev, UDEV_DEV_MODEL);
    }

    QString getFstype() const {
        if (_dev == nullptr)
            return QString();
        return udev_device_get_property_value(_dev, UDEV_FS_TYPE);
    }

    QString getDevName() const {
        return QString(getDevPath()).split('/').takeLast();
    }

    QString getMountpoint() const {

        return QString("/dev/lumen/%1").arg(getDevName());
    }

    void setMounted(bool mounted) {
        _mounted = mounted;
    }

    bool getMounted() const {
        return _mounted;
    }

    void setWritable(bool writable) {
        _writable = _mounted && writable;
    }

    bool getWritable() const {
        return _mounted && _writable;
    }

protected:
    struct udev_device *_dev;
    bool _mounted { false };
    bool _writable { false };
};


class UsbMountManager: public QObject
{
    Q_OBJECT

public:
    UsbMountManager(QObject* parent = nullptr);
    virtual ~UsbMountManager( ) override;

    void remount(bool writable);
    bool isWritable();
    QString mountPoint();

    void remountDevice(UsbDevice &dev, bool writable);
    QList<UsbDevice> getDeviceList();

protected:
    QMap<QString, UsbDevice> _devList;
    QSocketNotifier *_notifier;
    struct udev *_udev;
    struct udev_monitor *_mon;

    void enumerateDevices();
    void tryMount(UsbDevice &dev);

signals:
    void ready();
    void filesystemMounted(UsbDevice const &dev, bool writable);
    void filesystemMountFailed(QString path, int error);
    void filesystemRemounted(UsbDevice const &dev, bool succeeded);
    void filesystemUnmounted(QString const& mountPoint);

protected slots:
    void processDevice(int fd);
};

#endif // __USBMOUNTMANAGER_H__
