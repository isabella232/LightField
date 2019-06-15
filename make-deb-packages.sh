#!/bin/bash -e

VERSION=1.0.1
PACKAGE_BUILD_ROOT=/home/lumen/Volumetric/LightField/packaging

#########################################################
##                                                     ##
##     No user-serviceable parts below this point.     ##
##                                                     ##
#########################################################

function clear () {
    echo -ne "\x1B[0m\x1B[H\x1B[J\x1B[3J"
}

function blue-bar () {
    echo -e "\r\x1B[1;37;44m$*\x1B[K\x1B[0m" 1>&2
}

PRINTRUN_SRC=/home/lumen/Volumetric/printrun
LIGHTFIELD_SRC=/home/lumen/Volumetric/LightField
MOUNTMON_SRC=${LIGHTFIELD_SRC}/mountmon
USBDRIVER_SRC=${LIGHTFIELD_SRC}/usb-driver
PACKAGE_BUILD_DIR=${PACKAGE_BUILD_ROOT}/${VERSION}
DEB_BUILD_DIR=${PACKAGE_BUILD_DIR}/deb
LIGHTFIELD_PACKAGE=${DEB_BUILD_DIR}/lightfield-${VERSION}
LIGHTFIELD_FILES=${LIGHTFIELD_PACKAGE}/files

if [ "$1" = "-q" ]
then
	VERBOSE=
else
	VERBOSE=-v
fi

clear

blue-bar • Setting up build environment

[ -d ${PACKAGE_BUILD_ROOT} ] || mkdir ${VERBOSE} -p  ${PACKAGE_BUILD_ROOT}
[ -d ${DEB_BUILD_DIR}      ] && rm    ${VERBOSE} -rd ${DEB_BUILD_DIR}

mkdir ${VERBOSE} -p ${LIGHTFIELD_FILES}

cp ${VERBOSE} -ar ${LIGHTFIELD_SRC}/debian ${LIGHTFIELD_PACKAGE}/

blue-bar • Creating directories

mkdir ${VERBOSE} -p ${LIGHTFIELD_FILES}/var/cache/lightfield/print-jobs
mkdir ${VERBOSE} -p ${LIGHTFIELD_FILES}/var/lib/lightfield/model-library
mkdir ${VERBOSE} -p ${LIGHTFIELD_FILES}/var/log/lightfield

blue-bar • Installing system files

cd ${LIGHTFIELD_SRC}
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/etc/sudoers.d/                               -m 644 system-stuff/lumen-lightfield
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/lib/systemd/system/                          -m 644 system-stuff/lightfield.service
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/lib/udev/rules.d/                            -m 644 usb-driver/90-dlpc350.rules
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/                -m 755 system-stuff/reset-lumen-arduino-port
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/ -m 644 stdio-shepherd/printer.py
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/ -m 755 stdio-shepherd/stdio-shepherd.py
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/share/X11/xorg.conf.d/                   -m 644 system-stuff/99-waveshare.conf

blue-bar • Building and installing debugging version of LightField

./rebuild -x
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/bin-debug/ -m 755 build/lf

blue-bar • Building and installing release version of LightField

./rebuild -rx
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/bin-release/ -m 755 -s build/lf

blue-bar • Installing printrun

cd ${PRINTRUN_SRC}
python3 setup.py build_ext --inplace
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/lib/lightfield                                      -m 755 -s printrun/gcoder_line.cpython-37m-x86_64-linux-gnu.so
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/         -m 644    printrun/__init__.py printrun/eventhandler.py printrun/gcoder.py printrun/printcore.py printrun/utils.py
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/plugins/ -m 644    printrun/plugins/__init__.py
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/Util/             -m 644    Util/constants.py

cd ${MOUNTMON_SRC}

blue-bar • Building and installing debugging version of Mountmon

./rebuild -x
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/bin-debug/   -m 755    build/mountmon

blue-bar • Building and installing release version of Mountmon

./rebuild -rx
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/bin-release/ -m 755 -s build/mountmon

blue-bar • Building and installing set-projector-power

cd ${USBDRIVER_SRC}

rm ${VERBOSE} -f set-projector-power || true
g++ -o set-projector-power -pipe -O3 -DNDEBUG -std=gnu++1z -Wall -W -D_REENTRANT -fPIC -l hidapi-libusb dlpc350_usb.cpp dlpc350_api.cpp main.cpp
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/bin-release/ -m 755 -s set-projector-power

rm ${VERBOSE} -f set-projector-power || true
g++ -o set-projector-power -pipe -g -Og -D_DEBUG -std=gnu++1z -Wall -W -D_REENTRANT -fPIC -l hidapi-libusb dlpc350_usb.cpp dlpc350_api.cpp main.cpp
install ${VERBOSE} -Dt ${LIGHTFIELD_FILES}/usr/bin-debug/ -m 755 set-projector-power

blue-bar • Building Debian packages
cd ${LIGHTFIELD_PACKAGE}
dpkg-buildpackage --build=any,all
