#!/bin/bash

VERSION=1.0.10.0
BUILDTYPE=
ARCHITECTURE=amd64
RELEASE_TRAIN=base
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
    red-bar "Failed!"
    exit 1
}

function usage () {
    cat 1>&2 <<EOF
Usage: $(basename "$0") [-q] BUILDTYPE
Where: -q           build quietly
       -X           don't force rebuild
       -t <train>   Sets the release train. Default: ${RELEASE_TRAIN}
       BUILDTYPE    is one of
                    release  create a release-build kit
                    debug    create a debug-build kit
                    both     create both kits

If the build is successful, the requested package set(s) will be found in a
subdirectory of ${PACKAGE_BUILD_ROOT}/ .
EOF
    exit 1
}

function apply-atsign-substitution () {
    local CODE='\e[0;30;48;2;146;208;80m»'
    local RST='«\e[0m'
    # shellcheck disable=SC2145
    echo "Substituting value ${CODE}${2}${RST} for token ${CODE}@@${1}@@${RST} in files ${CODE}${@:3}${RST}"
    perl -ilp -e "s/\\@\\@${1}\\@\\@/${2}/g;" "${@:3}"
}

function apply-assignment-substitution () {
    local CODE='\e[0;30;48;2;0;146;208m»'
    local RST='«\e[0m'
    # shellcheck disable=SC2145
    echo "Substituting value ${CODE}${2}${RST} into variable assignment ${CODE}${1}${RST} in files ${CODE}${@:3}${RST}"
    perl -ilp -e "s/^(\\s*)${1}=.*\$/\$1${1}=${2}/g;" "${@:3}"
}

trap error-trap ERR
set -e

PRINTRUN_SRC=/home/lumen/Volumetric/printrun
LIGHTFIELD_SRC=/home/lumen/Volumetric/LightField
MOUNTMON_SRC="${LIGHTFIELD_SRC}/mountmon"
USBDRIVER_SRC="${LIGHTFIELD_SRC}/usb-driver"

VERBOSE=-v
CHXXXVERBOSE=-c
FORCEREBUILD=-x

ARGS=$(getopt -n 'make-deb-package.sh' -o 'qXa:t:' -- "$@")
# shellcheck disable=SC2181
if [ ${?} -ne 0 ]
then
    usage
fi
eval set -- "$ARGS"

while [ -n "$1" ]
do
    case "$1" in
        '-q')
            VERBOSE=
            CHXXXVERBOSE=
        ;;

        '-X')
            FORCEREBUILD=
        ;;

        '-t')
            RELEASE_TRAIN="${2}"
            shift
        ;;

        'release' | 'debug' | 'both')
            if [ -z "${BUILDTYPE}" ]
            then
                BUILDTYPE="${1}"
            else
                echo "Too many build types specified -- use 'both' to build debug and release packages." 1>&2
                usage
            fi
            break
        ;;

        *)
            echo "Unknown parameter '${1}'."
            usage
        ;;
    esac
    shift
done

if [ -z "${BUILDTYPE}" ]
then
    usage
fi

if [ "${BUILDTYPE}" = "both" ]
then
    "$0" "${ARGS}" release || exit $?
    "$0" "${ARGS}" debug   || exit $?
    exit 0
fi

if [ -z "${RELEASE_TRAIN}" ] || [ "${RELEASE_TRAIN}" = "base" ]
then
    SUFFIX=${BUILDTYPE}
    RELEASE_TRAIN=
else
    SUFFIX=${RELEASE_TRAIN}-${BUILDTYPE}
fi

if [ "${BUILDTYPE}" = debug ]
then
    ANTIBUILDTYPE=release
else
    ANTIBUILDTYPE=debug
fi

PACKAGE_BUILD_DIR="${PACKAGE_BUILD_ROOT}/${SUFFIX}-${VERSION}"
DEB_BUILD_DIR="${PACKAGE_BUILD_DIR}/deb"
LIGHTFIELD_PACKAGE="${DEB_BUILD_DIR}/lightfield-${VERSION}"
LIGHTFIELD_FILES="${LIGHTFIELD_PACKAGE}/files"

##################################################

blue-bar "• Setting up build environment"

[ -d "${PACKAGE_BUILD_ROOT}"        ] || mkdir ${VERBOSE} -p  "${PACKAGE_BUILD_ROOT}"
[ -h "${PACKAGE_BUILD_ROOT}/latest" ] && rm    ${VERBOSE}     "${PACKAGE_BUILD_ROOT}/latest"
[ -d "${DEB_BUILD_DIR}"             ] && rm    ${VERBOSE} -rf "${DEB_BUILD_DIR}"

mkdir ${VERBOSE} -p  "${LIGHTFIELD_FILES}"
cp    ${VERBOSE} -ar "${LIGHTFIELD_SRC}/debian" "${LIGHTFIELD_PACKAGE}/"
ln    ${VERBOSE} -s  "${PACKAGE_BUILD_DIR}"     "${PACKAGE_BUILD_ROOT}/latest"

[ -d "${LIGHTFIELD_FILES}/usr/bin" ] || mkdir ${VERBOSE} -p "${LIGHTFIELD_FILES}/usr/bin"

##################################################

cd "${USBDRIVER_SRC}"

##################################################

blue-bar "• Building ${BUILDTYPE} version of set-projector-power"

if [ "${BUILDTYPE}" = "debug" ]
then
    OPTS="-g -Og -D_DEBUG"
elif [ "${BUILDTYPE}" = "release" ]
then
    OPTS="-s -O3 -DNDEBUG"
fi
# shellcheck disable=SC2086
g++ -o "${LIGHTFIELD_FILES}/usr/bin/set-projector-power" ${OPTS} -pipe -std=gnu++1z -Wall -W -D_GNU_SOURCE -fPIC dlpc350_usb.cpp dlpc350_api.cpp main.cpp -lhidapi-libusb

##################################################

cd "${MOUNTMON_SRC}"

blue-bar "• Building ${BUILDTYPE} version of Mountmon"

if [ "${BUILDTYPE}" = "debug" ]
then
    ./rebuild ${FORCEREBUILD}
elif [ "${BUILDTYPE}" = "release" ]
then
    ./rebuild ${FORCEREBUILD} -r
fi
chown ${CHXXXVERBOSE} -R lumen:lumen build

install ${VERBOSE} -DT -m 755 build/mountmon "${LIGHTFIELD_FILES}/usr/bin/mountmon"

##################################################

cd "${LIGHTFIELD_SRC}"

blue-bar "• Building ${BUILDTYPE} version of LightField"

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

blue-bar "• Copying LightField files into packaging directory"

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
install ${VERBOSE} -DT -m 644 usb-driver/90-dlpc350.rules                     "${LIGHTFIELD_FILES}/lib/udev/rules.d/90-dlpc350.rules"
install ${VERBOSE} -DT -m 755 system-stuff/reset-lumen-arduino-port           "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/reset-lumen-arduino-port"
install ${VERBOSE} -DT -m 644 stdio-shepherd/printer.py                       "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/printer.py"
install ${VERBOSE} -DT -m 755 stdio-shepherd/stdio-shepherd.py                "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/stdio-shepherd.py"
install ${VERBOSE} -DT -m 644 system-stuff/99-waveshare.conf                  "${LIGHTFIELD_FILES}/usr/share/X11/xorg.conf.d/99-waveshare.conf"

chmod ${CHXXXVERBOSE} -R go= "${LIGHTFIELD_FILES}/home/lumen/.gnupg"

##################################################

cd "${PRINTRUN_SRC}"

blue-bar "• Copying printrun files into packaging directory"

install ${VERBOSE} -DT -m 644 printrun/__init__.py                            "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/__init__.py"
install ${VERBOSE} -DT -m 644 printrun/eventhandler.py                        "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/eventhandler.py"
install ${VERBOSE} -DT -m 644 printrun/gcoder.py                              "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/gcoder.py"
install ${VERBOSE} -DT -m 644 printrun/printcore.py                           "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/printcore.py"
install ${VERBOSE} -DT -m 644 printrun/utils.py                               "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/utils.py"
install ${VERBOSE} -DT -m 644 printrun/plugins/__init__.py                    "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/plugins/__init__.py"
install ${VERBOSE} -DT -m 644 Util/constants.py                               "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/Util/constants.py"

##################################################

cd "${LIGHTFIELD_PACKAGE}"

blue-bar "• Building Debian packages"

cp debian/control.in debian/control
for a in install preinst postinst prerm postrm
do
    cp "${VERBOSE}" "debian/lightfield.${a}.in" "debian/lightfield-${SUFFIX}.${a}"
done

apply-atsign-substitution     BUILDTYPE     "${BUILDTYPE}"      debian/control
apply-atsign-substitution     ANTIBUILDTYPE "${ANTIBUILDTYPE}"  debian/control
if [ -n "${RELEASE_TRAIN}" ]
then
    apply-atsign-substitution RELEASE_TRAIN "-${RELEASE_TRAIN}" debian/control
else
    apply-atsign-substitution RELEASE_TRAIN ""                  debian/control
fi

apply-assignment-substitution ARCHITECTURE  "${ARCHITECTURE}"  debian/lightfield.postinst.in
apply-assignment-substitution BUILDTYPE     "${BUILDTYPE}"     debian/lightfield.postinst.in
apply-assignment-substitution RELEASE_TRAIN "${RELEASE_TRAIN}" debian/lightfield.postinst.in
apply-assignment-substitution VERSION       "${VERSION}"       debian/lightfield.postinst.in

dpkg-buildpackage --build=any,all --no-sign

##################################################

blue-bar "• Cleaning up"

cd ..

rm ${VERBOSE} -rf "${LIGHTFIELD_PACKAGE}"

blue-bar ""
blue-bar "• Done!"
blue-bar ""
