#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <libudev.h>
#include <QMap>
#include <QSocketNotifier>
#include "pch.h"
#include "usbmountmanager.h"
#include "debug.h"

static const QMap<QString, unsigned long> supported_fs_types = {
    { "vfat", MS_SYNCHRONOUS },
    { "ntfs", MS_SYNCHRONOUS },
    { "ext4", MS_SYNCHRONOUS }
};

UsbMountManager::UsbMountManager(QObject* parent): QObject(parent)
{
    struct stat st;

    Q_ASSERT(unshare(CLONE_FS) == 0);

    if (stat("/media/lumen", &st) != -1) {
        mkdir("/media/lumen", 0755);
    }

    _udev = udev_new();
    Q_ASSERT(_udev);

    _mon = udev_monitor_new_from_netlink(_udev, "udev");
    Q_ASSERT(_mon);

    udev_monitor_filter_add_match_subsystem_devtype(_mon, UDEV_SUBSYSTEM, UDEV_DEVTYPE);
    udev_monitor_enable_receiving(_mon);

    _notifier = new QSocketNotifier(udev_monitor_get_fd(_mon), QSocketNotifier::Read);
    QObject::connect(_notifier, &QSocketNotifier::activated, this, &UsbMountManager::processDevice);
    _notifier->setEnabled(true);

    emit ready();
}

void UsbMountManager::enumerateDevices()
{
    struct udev_list_entry *devices;
    struct udev_list_entry *entry;
    struct udev_enumerate *enumerate = udev_enumerate_new(_udev);
    Q_ASSERT(enumerate);

    udev_enumerate_add_match_subsystem(enumerate, UDEV_SUBSYSTEM);
    udev_enumerate_add_match_property(enumerate, UDEV_BUS_PROP, UDEV_BUS_ID);
    udev_enumerate_scan_devices(enumerate);

    devices = udev_enumerate_get_list_entry(enumerate);
    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device* udev_dev = udev_device_new_from_syspath(_udev, path);
        if (strcmp(UDEV_DEVTYPE, udev_device_get_devtype(udev_dev)) == 0) {
            UsbDevice dev(udev_dev);
            _devList.insert(dev.getDevPath(), dev);
            tryMount(dev);
        }
    }

    udev_enumerate_unref(enumerate);
}

void UsbMountManager::processDevice(int fd)
{
    (void)fd;

    struct udev_device *udev_dev = udev_monitor_receive_device(_mon);

    if (strcmp(UDEV_BUS_ID, udev_device_get_property_value(udev_dev, UDEV_BUS_PROP)) == 0) {
        const char *action = udev_device_get_action(udev_dev);

        if (strcmp(action, "add") == 0) {
            UsbDevice dev(udev_dev);
            _devList.insert(dev.getDevPath(), dev);
            tryMount(dev);
        } else if (strcmp(action, "remove") == 0) {
            struct stat st;
            const char *path = udev_device_get_property_value(udev_dev, UDEV_DEVNAME);
            if (!_devList.contains(path))
                return;

            QString mountpoint = _devList.take(path).getMountpoint();

            if (stat(mountpoint.toUtf8().data(), &st) != -1) {
                rmdir(mountpoint.toUtf8().data());
            }
            emit filesystemUnmounted(mountpoint);
        }
    }
}

void UsbMountManager::tryMount(UsbDevice &dev)
{
    struct stat st;
    QString fstype = dev.getFstype();
    QString path = dev.getDevPath();
    QString mountpoint = dev.getMountpoint();

    if (!supported_fs_types.contains(fstype)) {
        debug("+ UsbMountManager::tryMount: unsupported filesystem type %s at path %s - skipping\n",
            fstype, path);
        _devList.remove(path);
        emit filesystemMountFailed(path, EINVAL);
        return;
    }

    unsigned long flags = supported_fs_types[fstype] | MS_RDONLY;

    if (stat(mountpoint.toUtf8().data(), &st) == -1) {
        mkdir(mountpoint.toUtf8().data(), 0755);
    }

    int ret = mount(path.toUtf8().data(), mountpoint.toUtf8().data(), fstype.toUtf8().data(), flags,
        nullptr);

    if (ret == 0) {
        dev.setMounted(true);
        emit filesystemMounted(dev, false);
        return;
    }

    debug("+ UsbMountManager::tryMount: failed to mount %s %s read only: %s - skipping\n",
        fstype, path, strerror(errno));
    _devList.remove(path);
    emit filesystemMountFailed(path, errno);
}

UsbMountManager::~UsbMountManager()
{
    udev_monitor_unref(_mon);
    udev_unref(_udev);
    _notifier->deleteLater();
}

void UsbMountManager::remountDevice(UsbDevice &dev, bool writable)
{
    QString fstype = dev.getFstype();
    QString path = dev.getDevPath();
    QString mountpoint = dev.getMountpoint();
    unsigned long flags = supported_fs_types[fstype] | MS_REMOUNT;
    if (!writable)
        flags |= MS_RDONLY;

    int ret = mount(path.toUtf8().data(), mountpoint.toUtf8().data(), fstype.toUtf8().data(), flags,
        nullptr);

    if (ret == 0) {
        emit filesystemRemounted(dev, true);
        dev.setWritable(writable);
        return;
    }

    emit filesystemRemounted(dev, false);
    debug("+ UsbMountManager::tryMount: failed to remount %s %s %s\n",
        fstype, path, writable ? "RW" : "RO");
}

QList<UsbDevice> UsbMountManager::getDeviceList()
{
    return _devList.values();
}

void UsbMountManager::remount(bool writable)
{
    if (_devList.empty())
        return;

    remountDevice(_devList.first(), writable);
}

bool UsbMountManager::isWritable()
{
    if (_devList.empty())
        return false;

    return _devList.first().getWritable();
}

QString UsbMountManager::mountPoint()
{
    if (_devList.empty())
        return QString();

    return _devList.first().getMountpoint();
}
