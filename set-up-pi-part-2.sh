#!/bin/bash

[ "${USER}" != root ] && exec sudo "$0"

BUILDTYPE=debug
VERSION=1.1.0.0
ARCHITECTURE=armhf
INSTALLKITDIR=/var/lib/lightfield/software-updates/lightfield-${BUILDTYPE}_${VERSION}_${ARCHITECTURE}

apt-get -y purge $(<rpi4_packages-to-purge.txt)

rm -drv \
    /etc/sgml                                  \
    /etc/vnc                                   \
    /etc/wpa_supplicant                        \
    /etc/xml                                   \
    /usr/share/fonts/truetype/freefont         \
    /usr/share/fonts/truetype/liberation2      \
    /usr/share/fonts/truetype/piboto           \
    /usr/share/fonts/truetype/quicksand        \
    /usr/share/raspi-ui-overrides/applications \
    /var/run/avahi-daemon

[ -f /etc/apt/sources.list   ] && mv -v /etc/apt/sources.list   /etc/apt/old.sources.list.old
[ -d /etc/apt/sources.list.d ] && mv -v /etc/apt/sources.list.d /etc/apt/old.sources.list.d.old
echo "deb file:${INSTALLKITDIR} ./" > /etc/apt/sources.list
chmod -c 644 /etc/apt/sources.list

cp -v /mnt/pubring.gpg /etc/apt/trusted.gpg.d/volumetric-keyring.gpg
mkdir -v -p ${INSTALLKITDIR}
tar -vv -C ${INSTALLKITDIR} -xf /mnt/lightfield-debug_1.1.0.0_armhf.kit

apt-get update
#apt-get -y install autotools-dev libhidapi-dev libtool-bin libudev-dev libusb-1.0-0-dev
apt-get -y install dlocate deborphan feh fonts-font-awesome lightfield-debug pastebinit simplescreenrecorder

rm -v /etc/apt/sources.list
[ -f /etc/apt/old.sources.list.old   ] && mv -v /etc/apt/old.sources.list.old   /etc/apt/sources.list
[ -f /etc/apt/old.sources.list.d.old ] && mv -v /etc/apt/old.sources.list.d.old /etc/apt/sources.list.d
