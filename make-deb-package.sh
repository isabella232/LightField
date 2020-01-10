#!/bin/bash

VERSION=1.0.11.0
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

function usage () {
    cat <<EOF
Usage: $(basename "$0") [-q] BUILDTYPE
Where: -q         build quietly
       -X         don't force rebuild
       BUILDTYPE  is one of
                  release  create a release-build kit
                  debug    create a debug-build kit
                  both     create both kits

If the build is successful, the requested package set(s) will be found in
  ${DEB_BUILD_DIR}/
EOF
}

trap error-trap ERR
set -e

PRINTRUN_SRC=/home/lumen/Volumetric/printrun
LIGHTFIELD_SRC=/home/lumen/Volumetric/LightField
MOUNTMON_SRC="${LIGHTFIELD_SRC}/mountmon"
PROJECTORDRIVER_SRC="${LIGHTFIELD_SRC}/dlp4710"
PACKAGE_BUILD_DIR="${PACKAGE_BUILD_ROOT}/${VERSION}"
DEB_BUILD_DIR="${PACKAGE_BUILD_DIR}/deb"
LIGHTFIELD_PACKAGE="${DEB_BUILD_DIR}/lightfield-${VERSION}"
LIGHTFIELD_FILES="${LIGHTFIELD_PACKAGE}/files"

VERBOSE=-v
CHXXXVERBOSE=-c
FORCEREBUILD=-x
BUILDTYPE=

while [ -n "$1" ]
do
    case "$1" in
        "-q")
            VERBOSE=
            CHXXXVERBOSE=
        ;;

        "-X")
            FORCEREBUILD=
        ;;

        "release" | "debug" | "both")
            BUILDTYPE="$1"
        ;;

        *)
            usage
            exit 1
        ;;
    esac
    shift
done

if [ -z "${BUILDTYPE}" ]
then
    usage
    exit 1
fi

if [ "${BUILDTYPE}" = "both" ]
then
    ARG=$([ -z ${VERBOSE} ] && echo -q)
    $0 "${ARG}" release || exit $?
    $0 "${ARG}" debug   || exit $?
    exit 0
fi

##################################################

blue-bar • Setting up build environment

[ -d "${PACKAGE_BUILD_ROOT}"        ] || mkdir ${VERBOSE} -p  "${PACKAGE_BUILD_ROOT}"
[ -h "${PACKAGE_BUILD_ROOT}/latest" ] && rm    ${VERBOSE}     "${PACKAGE_BUILD_ROOT}/latest"
[ -d "${DEB_BUILD_DIR}"             ] && rm    ${VERBOSE} -rf "${DEB_BUILD_DIR}"

mkdir ${VERBOSE} -p  "${LIGHTFIELD_FILES}"
cp    ${VERBOSE} -ar "${LIGHTFIELD_SRC}/debian" "${LIGHTFIELD_PACKAGE}/"
ln    ${VERBOSE} -s  "${PACKAGE_BUILD_DIR}"     "${PACKAGE_BUILD_ROOT}/latest"

[ -d "${LIGHTFIELD_FILES}/usr/bin" ] || mkdir ${VERBOSE} -p "${LIGHTFIELD_FILES}/usr/bin"

##################################################

cd "${PROJECTORDRIVER_SRC}"

##################################################

blue-bar • Building "${BUILDTYPE}" version of set-projector-power

cd ${PROJECTORDRIVER_SRC}
[ -n "${FORCEREBUILD}" ] && make BUILD="${BUILDTYPE}" clean
make BUILD="${BUILDTYPE}"
install ${VERBOSE} -DT -m 755 dlp4710/set-projector-power "${LIGHTFIELD_FILES}/usr/bin/set-projector-power"

##################################################

cd "${MOUNTMON_SRC}"

##################################################

blue-bar • Building "${BUILDTYPE}" version of Mountmon

if [ "${BUILDTYPE}" = "debug" ]
then
    ./rebuild ${FORCEREBUILD}
elif [ "${BUILDTYPE}" = "release" ]
then
    ./rebuild ${FORCEREBUILD} -r
fi

install ${VERBOSE} -DT -m 755 build/mountmon "${LIGHTFIELD_FILES}/usr/bin/mountmon"

##################################################

cd "${LIGHTFIELD_SRC}"

##################################################

blue-bar • Building "${BUILDTYPE}" version of LightField

if [ "${BUILDTYPE}" = "debug" ]
then
    ./rebuild ${FORCEREBUILD}
elif [ "${BUILDTYPE}" = "release" ]
then
    ./rebuild ${FORCEREBUILD} -r
fi
chown ${CHXXXVERBOSE} -R lumen:lumen build

install ${VERBOSE} -DT -m 755 build/lf "${LIGHTFIELD_FILES}/usr/bin/lf"

##################################################

blue-bar • Copying files into packaging directory

install ${VERBOSE} -DT -m 644 system-stuff/99untrustworthy-clock              "${LIGHTFIELD_FILES}/etc/apt/apt.conf.d/99untrustworthy-clock"
install ${VERBOSE} -DT -m 644 gpg/new-pubring.gpg                             "${LIGHTFIELD_FILES}/etc/apt/trusted.gpg.d/volumetric-keyring.gpg"
install ${VERBOSE} -DT -m 440 system-stuff/lumen-lightfield                   "${LIGHTFIELD_FILES}/etc/sudoers.d/lumen-lightfield"
install ${VERBOSE} -DT -m 644 system-stuff/getty@tty1.service.d_override.conf "${LIGHTFIELD_FILES}/etc/systemd/system/getty@tty1.service.d/override.conf"
install ${VERBOSE} -DT -m 600 system-stuff/lumen-bash_profile                 "${LIGHTFIELD_FILES}/home/lumen/.bash_profile"
install ${VERBOSE} -DT -m 600 system-stuff/lumen-real_bash_profile            "${LIGHTFIELD_FILES}/home/lumen/.real_bash_profile"
install ${VERBOSE} -DT -m 600 gpg/new-pubring.gpg                             "${LIGHTFIELD_FILES}/home/lumen/.gnupg/pubring.gpg"
install ${VERBOSE} -DT -m 600 gpg/new-pubring.kbx                             "${LIGHTFIELD_FILES}/home/lumen/.gnupg/pubring.kbx"
install ${VERBOSE} -DT -m 600 gpg/trustdb.gpg                                 "${LIGHTFIELD_FILES}/home/lumen/.gnupg/trustdb.gpg"
install ${VERBOSE} -DT -m 644 system-stuff/clean-up-mount-points.service      "${LIGHTFIELD_FILES}/lib/systemd/system/clean-up-mount-points.service"
install ${VERBOSE} -DT -m 644 system-stuff/set-projector-power.service        "${LIGHTFIELD_FILES}/lib/systemd/system/set-projector-power.service"
install ${VERBOSE} -DT -m 644 dlp4710/90-dlp4710.rules                        "${LIGHTFIELD_FILES}/lib/udev/rules.d/90-dlp4710.rules"
install ${VERBOSE} -DT -m 755 system-stuff/reset-lumen-arduino-port           "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/reset-lumen-arduino-port"
install ${VERBOSE} -DT -m 755 system-stuff/reset-lumen-projector-port         "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/reset-lumen-projector-port"
install ${VERBOSE} -DT -m 644 stdio-shepherd/printer.py                       "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/printer.py"
install ${VERBOSE} -DT -m 755 stdio-shepherd/stdio-shepherd.py                "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/stdio-shepherd.py"
install ${VERBOSE} -DT -m 644 system-stuff/99-waveshare.conf                  "${LIGHTFIELD_FILES}/usr/share/X11/xorg.conf.d/99-waveshare.conf"

chmod ${CHXXXVERBOSE} -R go= "${LIGHTFIELD_FILES}/home/lumen/.gnupg"

##################################################

cd "${PRINTRUN_SRC}"

##################################################

blue-bar • Copying printrun files into packaging directory

install ${VERBOSE} -DT -m 644 printrun/__init__.py                            "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/__init__.py"
install ${VERBOSE} -DT -m 644 printrun/eventhandler.py                        "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/eventhandler.py"
install ${VERBOSE} -DT -m 644 printrun/gcoder.py                              "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/gcoder.py"
install ${VERBOSE} -DT -m 644 printrun/printcore.py                           "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/printcore.py"
install ${VERBOSE} -DT -m 644 printrun/utils.py                               "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/utils.py"
install ${VERBOSE} -DT -m 644 printrun/plugins/__init__.py                    "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/plugins/__init__.py"
install ${VERBOSE} -DT -m 644 Util/constants.py                               "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/Util/constants.py"

##################################################

cd "${LIGHTFIELD_PACKAGE}"

##################################################

blue-bar • Building Debian packages

cp -v "debian/${BUILDTYPE}-control" debian/control
dpkg-buildpackage --build=any,all --no-sign

##################################################

blue-bar • Cleaning up

cd ..

rm ${VERBOSE} -rf "${LIGHTFIELD_PACKAGE}"

blue-bar ""
blue-bar "• Done!"
blue-bar ""
