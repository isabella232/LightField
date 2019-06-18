#!/bin/bash

VERSION=1.0.0
PACKAGE_BUILD_ROOT=/home/lumen/Volumetric/LightField/packaging

#########################################################
##                                                     ##
##     No user-serviceable parts below this point.     ##
##                                                     ##
#########################################################

function blue-bar () {
    echo -e "\r\x1B[1;37;44m$*\x1B[K\x1B[0m" 1>&2
}

function red-bar () {
    echo -e "\r\x1B[1;33;41m$*\x1B[K\x1B[0m" 1>&2
}

function error-trap () {
    red-bar Failed\!
    exit 1
}

trap error-trap ERR
set -e

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
    CHXXXVERBOSE=
else
    VERBOSE=-v
    CHXXXVERBOSE=-c
fi

##################################################

blue-bar • Setting up build environment

[ -d ${PACKAGE_BUILD_ROOT} ] || mkdir ${VERBOSE} -p  ${PACKAGE_BUILD_ROOT}
[ -d ${DEB_BUILD_DIR}      ] && rm    ${VERBOSE} -rf ${DEB_BUILD_DIR}

mkdir ${VERBOSE} -p ${LIGHTFIELD_FILES}

cp ${VERBOSE} -ar ${LIGHTFIELD_SRC}/debian ${LIGHTFIELD_PACKAGE}/

[ -d ${LIGHTFIELD_FILES}/usr/bin-release ] || mkdir ${VERBOSE} -p ${LIGHTFIELD_FILES}/usr/bin-release
[ -d ${LIGHTFIELD_FILES}/usr/bin-debug   ] || mkdir ${VERBOSE} -p ${LIGHTFIELD_FILES}/usr/bin-debug

##################################################

cd ${USBDRIVER_SRC}

##################################################

blue-bar • Building and installing release version of set-projector-power

g++ -o ${LIGHTFIELD_FILES}/usr/bin-release/set-projector-power -pipe -O3 -s -DNDEBUG -std=gnu++1z -Wall -W -D_GNU_SOURCE -fPIC dlpc350_usb.cpp dlpc350_api.cpp main.cpp -l hidapi-libusb

##################################################

blue-bar • Building and installing debug version of set-projector-power

g++ -o ${LIGHTFIELD_FILES}/usr/bin-debug/set-projector-power -pipe -g -Og -D_DEBUG -std=gnu++1z -Wall -W -D_GNU_SOURCE -fPIC dlpc350_usb.cpp dlpc350_api.cpp main.cpp -l hidapi-libusb

##################################################

cd ${MOUNTMON_SRC}

##################################################

blue-bar • Building and installing debugging version of Mountmon

./rebuild -x
install ${VERBOSE} -DT -m 755 build/mountmon ${LIGHTFIELD_FILES}/usr/bin-debug/mountmon

##################################################

blue-bar • Building and installing release version of Mountmon

./rebuild -rx
install ${VERBOSE} -DT -m 755 -s build/mountmon ${LIGHTFIELD_FILES}/usr/bin-release/mountmon

##################################################

cd ${LIGHTFIELD_SRC}

##################################################

blue-bar • Building and installing debugging version of LightField

./rebuild -x
install ${VERBOSE} -DT -m 755 build/lf ${LIGHTFIELD_FILES}/usr/bin-debug/lf

##################################################

blue-bar • Building and installing release version of LightField

./rebuild -rx
install ${VERBOSE} -DT -m 755 -s build/lf ${LIGHTFIELD_FILES}/usr/bin-release/lf

##################################################

blue-bar • Installing system files

install ${VERBOSE} -DT -m 644 apt-files/volumetric-lightfield.list            ${LIGHTFIELD_FILES}/etc/apt/sources.list.d/volumetric-lightfield.list
install ${VERBOSE} -DT -m 644 gpg/pubring.gpg                                 ${LIGHTFIELD_FILES}/etc/apt/trusted.gpg.d/volumetric-keyring.gpg
install ${VERBOSE} -DT -m 440 system-stuff/lumen-lightfield                   ${LIGHTFIELD_FILES}/etc/sudoers.d/lumen-lightfield
install ${VERBOSE} -DT -m 644 system-stuff/getty@tty1.service.d_override.conf ${LIGHTFIELD_FILES}/etc/systemd/system/getty@tty1.service.d/override.conf
install ${VERBOSE} -DT -m 600 system-stuff/lumen-bash_profile                 ${LIGHTFIELD_FILES}/home/lumen/.bash_profile
install ${VERBOSE} -DT -m 600 system-stuff/lumen-real_bash_profile            ${LIGHTFIELD_FILES}/home/lumen/.real_bash_profile
install ${VERBOSE} -DT -m 600 gpg/pubring.gpg                                 ${LIGHTFIELD_FILES}/home/lumen/.gnupg/pubring.gpg
install ${VERBOSE} -DT -m 600 gpg/trustdb.gpg                                 ${LIGHTFIELD_FILES}/home/lumen/.gnupg/trustdb.gpg
install ${VERBOSE} -DT -m 644 system-stuff/set-projector-power.service        ${LIGHTFIELD_FILES}/lib/systemd/system/set-projector-power.service
install ${VERBOSE} -DT -m 644 usb-driver/90-dlpc350.rules                     ${LIGHTFIELD_FILES}/lib/udev/rules.d/90-dlpc350.rules
install ${VERBOSE} -DT -m 755 system-stuff/reset-lumen-arduino-port           ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/reset-lumen-arduino-port
install ${VERBOSE} -DT -m 644 stdio-shepherd/printer.py                       ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/printer.py
install ${VERBOSE} -DT -m 755 stdio-shepherd/stdio-shepherd.py                ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/stdio-shepherd.py
install ${VERBOSE} -DT -m 644 system-stuff/99-waveshare.conf                  ${LIGHTFIELD_FILES}/usr/share/X11/xorg.conf.d/99-waveshare.conf

chmod ${CHXXXVERBOSE} -R go= ${LIGHTFIELD_FILES}/home/lumen/.gnupg

##################################################

cd ${PRINTRUN_SRC}

##################################################

blue-bar • Installing printrun

install ${VERBOSE} -DT -m 644 printrun/__init__.py                            ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/__init__.py
install ${VERBOSE} -DT -m 644 printrun/eventhandler.py                        ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/eventhandler.py
install ${VERBOSE} -DT -m 644 printrun/gcoder.py                              ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/gcoder.py
install ${VERBOSE} -DT -m 644 printrun/printcore.py                           ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/printcore.py
install ${VERBOSE} -DT -m 644 printrun/utils.py                               ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/utils.py
install ${VERBOSE} -DT -m 644 printrun/plugins/__init__.py                    ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/plugins/__init__.py
install ${VERBOSE} -DT -m 644 Util/constants.py                               ${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/Util/constants.py

##################################################

cd ${LIGHTFIELD_PACKAGE}

##################################################

blue-bar • Building Debian packages

dpkg-buildpackage --build=any,all

##################################################

blue-bar • Cleaning up

cd ..

rm ${VERBOSE} -rf ${LIGHTFIELD_PACKAGE}
