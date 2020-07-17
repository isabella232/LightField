#!/bin/bash
# shellcheck disable=SC2103
# shellcheck disable=SC2164

BUILDTYPE=
LIGHTFIELD_ROOT="${PWD}"
PACKAGE_BUILD_ROOT=${LIGHTFIELD_ROOT}/packaging

#########################################################
##                                                     ##
##     No user-serviceable parts below this point.     ##
##                                                     ##
#########################################################

function usage () {
    cat 1>&2 <<EOF
Usage: $(basename "$0") [-q] [-X] BUILDTYPE
Where: -q         build quietly
       -X         don't force rebuild
       BUILDTYPE  is one of
                  release  create release-build package set
                  debug    create debug-build package set
                  both     create both package sets

If the build is successful, the requested package set(s) will be found in a
subdirectory of ${PACKAGE_BUILD_ROOT}/ .
EOF
    exit 1
}

# shellcheck disable=SC1090
source "${LIGHTFIELD_ROOT}/shared-stuff.sh"

VERBOSE=-v
CHXXXVERBOSE=-c
FORCEREBUILD=-x

ARGS=$(getopt -n 'make-deb-package.sh' -o 'qX' -- "$@")
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

        --)
            shift
            break
        ;;

        *)
            echo "Unknown parameter '${1}'."
            usage
        ;;
    esac
    shift
done

if [ -z "${1}" ]
then
    red-bar 'No build type given.'
    usage
elif [ -n "${2}" ]
then
    red-bar 'Too many arguments given.'
    usage
else
    if [ "${1}" = "debug" ] || [ "${1}" = "release" ]
    then
        BUILDTYPE="${1}"
    elif [ "${1}" = "both" ]
    then
        "$0" "${ARGS}" release || exit $?
        "$0" "${ARGS}" debug   || exit $?
        exit 0
    else
        red-bar "Unknown build type '${1}'."
        usage
    fi
fi

if [ -z "${RELEASE_TRAIN}" ]
then
    RELEASE_TRAIN=base
fi

if [ "${RELEASE_TRAIN}" = "base" ]
then
    SUFFIX=${BUILDTYPE}
else
    SUFFIX=${RELEASE_TRAIN}-${BUILDTYPE}
fi

if [ "${BUILDTYPE}" = debug ]
then
    ANTIBUILDTYPE=release
else
    ANTIBUILDTYPE=debug
fi

sed -i "1s/(.*)/($VERSION)/" ${LIGHTFIELD_ROOT}/debian/changelog
PRINTRUN_SRC="${LIGHTFIELD_ROOT}/printrun"
MOUNTMON_SRC="${LIGHTFIELD_ROOT}/mountmon"
PRINTPROFILES_SRC="${LIGHTFIELD_ROOT}/print-profiles"
if [ "${RELEASE_TRAIN}" = "base" ] || [ "${RELEASE_TRAIN}" = "xbase" ]
then
    PROJECTOR_SRC=${LIGHTFIELD_ROOT}/usb-driver
elif [ "${RELEASE_TRAIN}" = "dlp4710" ] || [ "${RELEASE_TRAIN}" = "xdlp4710" ] || [ "${RELEASE_TRAIN}" = "xdlp4710-20um" ]
then
    PROJECTOR_SRC=${LIGHTFIELD_ROOT}/dlp4710
fi

PACKAGE_BUILD_DIR="${PACKAGE_BUILD_ROOT}/${VERSION}-${SUFFIX}"
DEB_BUILD_DIR="${PACKAGE_BUILD_DIR}/deb"
LIGHTFIELD_PACKAGE="${DEB_BUILD_DIR}/lightfield-${VERSION}"
LIGHTFIELD_FILES="${LIGHTFIELD_PACKAGE}/files"

#cat <<EOF
#LIGHTFIELD_ROOT:    ${LIGHTFIELD_ROOT}
#PACKAGE_BUILD_ROOT: ${PACKAGE_BUILD_ROOT}
#VERBOSE:            ${VERBOSE}
#CHXXXVERBOSE:       ${CHXXXVERBOSE}
#FORCEREBUILD:       ${FORCEREBUILD}
#BUILDTYPE:          ${BUILDTYPE}
#RELEASE_TRAIN:      ${RELEASE_TRAIN}
#SUFFIX:             ${SUFFIX}
#ANTIBUILDTYPE:      ${ANTIBUILDTYPE}
#PRINTRUN_SRC:       ${PRINTRUN_SRC}
#MOUNTMON_SRC:       ${MOUNTMON_SRC}
#PRINTPROFILES_SRC:  ${PRINTPROFILES_SRC}
#PROJECTOR_SRC:      ${PROJECTOR_SRC}
#PACKAGE_BUILD_DIR:  ${PACKAGE_BUILD_DIR}
#DEB_BUILD_DIR:      ${DEB_BUILD_DIR}
#LIGHTFIELD_PACKAGE: ${LIGHTFIELD_PACKAGE}
#LIGHTFIELD_FILES:   ${LIGHTFIELD_FILES}
#EOF
#exit

##################################################

blue-bar "• Setting up build environment"

[ -d "${PACKAGE_BUILD_ROOT}"        ] || mkdir ${VERBOSE} -p  "${PACKAGE_BUILD_ROOT}"
[ -h "${PACKAGE_BUILD_ROOT}/latest" ] && rm    ${VERBOSE}     "${PACKAGE_BUILD_ROOT}/latest"
[ -d "${DEB_BUILD_DIR}"             ] && rm    ${VERBOSE} -rf "${DEB_BUILD_DIR}"

mkdir ${VERBOSE} -p  "${LIGHTFIELD_FILES}"
cp    ${VERBOSE} -ar "${LIGHTFIELD_ROOT}/debian" "${LIGHTFIELD_PACKAGE}/"
ln    ${VERBOSE} -s  "${PACKAGE_BUILD_DIR}"      "${PACKAGE_BUILD_ROOT}/latest"

[ -d "${LIGHTFIELD_FILES}/usr/bin" ] || mkdir ${VERBOSE} -p "${LIGHTFIELD_FILES}/usr/bin"

##################################################

blue-bar "• Building ${BUILDTYPE} version of set-projector-power"

echo ${PROJECTOR_SRC}
cd "${PROJECTOR_SRC}"

# shellcheck disable=SC2015
[ -n "${FORCEREBUILD}" ] && make BUILD=debug clean || true
make BUILD=debug

##################################################

blue-bar "• Building ${BUILDTYPE} version of Mountmon"

cd "${MOUNTMON_SRC}"

if [ "${BUILDTYPE}" = "debug" ]
then
    ./rebuild ${FORCEREBUILD}
elif [ "${BUILDTYPE}" = "release" ]
then
    ./rebuild ${FORCEREBUILD} -r
fi

install ${VERBOSE} -DT -m 755 build/mountmon "${LIGHTFIELD_FILES}/usr/bin/mountmon"

##################################################

blue-bar "• Building ${BUILDTYPE} version of LightField"

cd "${LIGHTFIELD_ROOT}"

if [ "${BUILDTYPE}" = "debug" ]
then
    ./rebuild ${FORCEREBUILD}
elif [ "${BUILDTYPE}" = "release" ]
then
    ./rebuild ${FORCEREBUILD} -r
fi

install ${VERBOSE} -DT -m 755 build/lf "${LIGHTFIELD_FILES}/usr/bin/lf"

##################################################

blue-bar "• Copying LightField files into packaging directory"

install     ${VERBOSE} -DT -m  644 system-stuff/99untrustworthy-clock               "${LIGHTFIELD_FILES}/etc/apt/apt.conf.d/99untrustworthy-clock"
install     ${VERBOSE} -DT -m  644 gpg/pubring.gpg                                  "${LIGHTFIELD_FILES}/etc/apt/trusted.gpg.d/volumetric-keyring.gpg"
install     ${VERBOSE} -DT -m  440 system-stuff/lumen-lightfield                    "${LIGHTFIELD_FILES}/etc/sudoers.d/lumen-lightfield"
install     ${VERBOSE} -DT -m  644 system-stuff/getty@tty1.service.d_override.conf  "${LIGHTFIELD_FILES}/etc/systemd/system/getty@tty1.service.d/override.conf"
install     ${VERBOSE} -DT -m  600 system-stuff/lumen-bash_profile                  "${LIGHTFIELD_FILES}/home/lumen/.bash_profile"
install     ${VERBOSE} -DT -m  600 system-stuff/lumen-real_bash_profile             "${LIGHTFIELD_FILES}/home/lumen/.real_bash_profile"
install     ${VERBOSE} -DT -m  600 gpg/pubring.gpg                                  "${LIGHTFIELD_FILES}/home/lumen/.gnupg/pubring.gpg"
install     ${VERBOSE} -DT -m  600 gpg/pubring.kbx                                  "${LIGHTFIELD_FILES}/home/lumen/.gnupg/pubring.kbx"
install     ${VERBOSE} -DT -m  600 gpg/trustdb.gpg                                  "${LIGHTFIELD_FILES}/home/lumen/.gnupg/trustdb.gpg"
install     ${VERBOSE} -DT -m  644 system-stuff/clean-up-mount-points.service       "${LIGHTFIELD_FILES}/lib/systemd/system/clean-up-mount-points.service"
install     ${VERBOSE} -DT -m  755 system-stuff/reset-lumen-arduino-port            "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/reset-lumen-arduino-port"
install     ${VERBOSE} -DT -m  644 stdio-shepherd/printer.py                        "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/printer.py"
install     ${VERBOSE} -DT -m  755 stdio-shepherd/stdio-shepherd.py                 "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/stdio-shepherd/stdio-shepherd.py"

if [ "${RELEASE_TRAIN}" = "base" ] || [ "${RELEASE_TRAIN}" = "xbase" ]
then
    install ${VERBOSE} -DT -m  644 system-stuff/dlpc350-set-projector-power.service "${LIGHTFIELD_FILES}/lib/systemd/system/set-projector-power.service"
    install ${VERBOSE} -DT -m  644 usb-driver/90-dlpc350.rules                      "${LIGHTFIELD_FILES}/lib/udev/rules.d/90-dlpc350.rules"
    install ${VERBOSE} -DT -m  755 usb-driver/set-projector-power                   "${LIGHTFIELD_FILES}/usr/bin/set-projector-power"
    install ${VERBOSE} -DT -m  644 system-stuff/99-waveshare-dlpc350.conf           "${LIGHTFIELD_FILES}/usr/share/X11/xorg.conf.d/99-waveshare.conf"
elif [ "${RELEASE_TRAIN}" = "dlp4710" ] || [ "${RELEASE_TRAIN}" = "xdlp4710" ] || [ "${RELEASE_TRAIN}" = "xdlp4710-20um" ]
then
    install ${VERBOSE} -DT -m  644 system-stuff/dlp4710-set-projector-power.service "${LIGHTFIELD_FILES}/lib/systemd/system/set-projector-power.service"
    install ${VERBOSE} -DT -m  644 dlp4710/90-dlp4710.rules                         "${LIGHTFIELD_FILES}/lib/udev/rules.d/90-dlp4710.rules"
    install ${VERBOSE} -DT -m 4555 dlp4710/set-projector-power                      "${LIGHTFIELD_FILES}/usr/bin/set-projector-power.real"
    install ${VERBOSE} -DT -m  755 dlp4710/set-projector-power.wrapper              "${LIGHTFIELD_FILES}/usr/bin/set-projector-power"
    install ${VERBOSE} -DT -m  755 system-stuff/dlp4710-reset-lumen-projector-port  "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/reset-lumen-projector-port"
    install ${VERBOSE} -DT -m  644 system-stuff/99-waveshare-dlp4710.conf           "${LIGHTFIELD_FILES}/usr/share/X11/xorg.conf.d/99-waveshare.conf"
fi

chmod ${CHXXXVERBOSE} -R go= "${LIGHTFIELD_FILES}/home/lumen/.gnupg"

##################################################

blue-bar "• Copying printrun files into packaging directory"

cd "${PRINTRUN_SRC}"

install     ${VERBOSE} -DT -m  644 printrun/__init__.py                             "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/__init__.py"
install     ${VERBOSE} -DT -m  644 printrun/eventhandler.py                         "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/eventhandler.py"
install     ${VERBOSE} -DT -m  644 printrun/gcoder.py                               "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/gcoder.py"
install     ${VERBOSE} -DT -m  644 printrun/printcore.py                            "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/printcore.py"
install     ${VERBOSE} -DT -m  644 printrun/utils.py                                "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/utils.py"
install     ${VERBOSE} -DT -m  644 printrun/plugins/__init__.py                     "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/printrun/plugins/__init__.py"
install     ${VERBOSE} -DT -m  644 Util/constants.py                                "${LIGHTFIELD_FILES}/usr/share/lightfield/libexec/printrun/Util/constants.py"

##################################################
cd ${PRINTPROFILES_SRC}

install     ${VERBOSE} -DT -m  666 print-profiles.json		    "${LIGHTFIELD_FILES}/var/lib/lightfield/print-profiles.json"
##################################################

blue-bar "• Building Debian packages"

cd "${LIGHTFIELD_PACKAGE}"

mv debian/control.in                                   debian/control
mv debian/lightfield-"${RELEASE_TRAIN}".install        debian/lightfield-"${SUFFIX}".install
mv debian/lightfield.preinst                           debian/lightfield-"${SUFFIX}".preinst
mv debian/lightfield.prerm                             debian/lightfield-"${SUFFIX}".prerm
mv debian/lightfield.postinst.in                       debian/lightfield-"${SUFFIX}".postinst
mv debian/lightfield.postrm                            debian/lightfield-"${SUFFIX}".postrm
if [ "${RELEASE_TRAIN}" != "base" ]
then
    mv debian/lightfield-common.preinst                debian/lightfield-common-"${RELEASE_TRAIN}".preinst
    mv debian/lightfield-common.postinst               debian/lightfield-common-"${RELEASE_TRAIN}".postinst
    mv debian/lightfield-common.postrm                 debian/lightfield-common-"${RELEASE_TRAIN}".postrm
fi

apply-atsign-substitution     BUILDTYPE     "${BUILDTYPE}"      debian/control
apply-atsign-substitution     ANTIBUILDTYPE "${ANTIBUILDTYPE}"  debian/control
if [ "${RELEASE_TRAIN}" = "base" ]
then
    apply-atsign-substitution RELEASE_TRAIN ""                  debian/control
else
    apply-atsign-substitution RELEASE_TRAIN "-${RELEASE_TRAIN}" debian/control
fi

apply-assignment-substitution ARCHITECTURE  "${ARCHITECTURE}"   debian/lightfield-"${SUFFIX}".postinst
apply-assignment-substitution BUILDTYPE     "${BUILDTYPE}"      debian/lightfield-"${SUFFIX}".postinst
apply-assignment-substitution RELEASE_TRAIN "${RELEASE_TRAIN}"  debian/lightfield-"${SUFFIX}".postinst
apply-assignment-substitution VERSION       "${VERSION}"        debian/lightfield-"${SUFFIX}".postinst

dpkg-buildpackage --build=any,all --no-sign

##################################################

blue-bar "• Cleaning up"

cd ..

rm ${VERBOSE} -rf "${LIGHTFIELD_PACKAGE}"
git checkout ${LIGHTFIELD_ROOT}/debian/changelog

blue-bar ""
blue-bar "• Done!"
blue-bar ""
