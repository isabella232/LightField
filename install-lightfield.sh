#!/bin/bash

LIGHTFIELD_ROOT="${PWD}"

#########################################################
##                                                     ##
##     No user-serviceable parts below this point.     ##
##                                                     ##
#########################################################

[ "${UID}" != "0" ] && exec sudo "${0}" "${@}"

function usage () {
    cat <<EOF
Usage: $(basename "$0") [-q] [-x]
Where: -q  Build quietly.
       -x  Force rebuild all.
EOF
}

# shellcheck disable=SC1090
source "${LIGHTFIELD_ROOT}/shared-stuff.sh"

PRINTRUN_SRC=/home/lumen/Volumetric/printrun
MOUNTMON_SRC=${LIGHTFIELD_ROOT}/mountmon

VERBOSE=-v
CHXXXVERBOSE=-c
BUILDQUIETLY=
FORCEREBUILD=

ARGS=$(getopt -n 'install-lightfield.sh' -o 'qx' -- "${@}")
# shellcheck disable=SC2181
if [ ${?} -ne 0 ]
then
    usage
    exit 1
fi
eval set -- "$ARGS"

while [ -n "$1" ]
do
    case "$1" in
        '-q')
            VERBOSE=
            CHXXXVERBOSE=
            BUILDQUIETLY=-q
        ;;

        '-x')
            FORCEREBUILD=-x
        ;;

        '--')
            shift
            break
        ;;

        *)
            usage
            exit 1
        ;;
    esac
    shift
done

if [ -z "${RELEASE_TRAIN}" ]
then
    RELEASE_TRAIN=base
fi

if [ "${RELEASE_TRAIN}" = "base" ]
then
    SUFFIX=debug
else
    SUFFIX=${RELEASE_TRAIN}-debug
fi

if [ "${RELEASE_TRAIN}" = "base" ]
then
    PROJECTOR_SRC=${LIGHTFIELD_ROOT}/usb-driver
elif [ "${RELEASE_TRAIN}" = "dlp4710" ]
then
    PROJECTOR_SRC=${LIGHTFIELD_ROOT}/dlp4710
else
    red-bar "Unrecognized release train name '${RELEASE_TRAIN}'."
    usage
fi

blue-bar • Building debugging version of set-projector-power
# shellcheck disable=SC2164
cd "${PROJECTOR_SRC}"
# shellcheck disable=SC2015
[ "${FORCEREBUILD}" = "-x" ] && make BUILD=debug clean || true
make BUILD=debug

blue-bar • Building debugging version of Mountmon
# shellcheck disable=SC2164
cd "${MOUNTMON_SRC}"
./rebuild ${FORCEREBUILD} ${BUILDQUIETLY}

blue-bar • Building debugging version of LightField
# shellcheck disable=SC2164
cd "${LIGHTFIELD_ROOT}"
./rebuild ${FORCEREBUILD} ${BUILDQUIETLY}

chown ${CHXXXVERBOSE} -R lumen:lumen "${PROJECTOR_SRC}"
chown ${CHXXXVERBOSE} -R lumen:lumen "${MOUNTMON_SRC}/build"
chown ${CHXXXVERBOSE} -R lumen:lumen "${LIGHTFIELD_ROOT}/build"

blue-bar • Creating any missing directories
[ ! -d /var/cache/lightfield/print-jobs     ] && mkdir ${VERBOSE} -p /var/cache/lightfield/print-jobs
[ ! -d /var/lib/lightfield/model-library    ] && mkdir ${VERBOSE} -p /var/lib/lightfield/model-library
[ ! -d /var/lib/lightfield/software-updates ] && mkdir ${VERBOSE} -p /var/lib/lightfield/software-updates
[ ! -d /var/log/lightfield                  ] && mkdir ${VERBOSE} -p /var/log/lightfield

chown ${CHXXXVERBOSE} -R lumen:lumen /var/cache/lightfield
chown ${CHXXXVERBOSE} -R lumen:lumen /var/lib/lightfield
chown ${CHXXXVERBOSE} -R lumen:lumen /var/log/lightfield

blue-bar • Installing files
install ${VERBOSE} -DT -m 644                   system-stuff/99untrustworthy-clock               /etc/apt/apt.conf.d/99untrustworthy-clock
install ${VERBOSE} -DT -m 644                   gpg/pubring.gpg                                  /etc/apt/trusted.gpg.d/volumetric-keyring.gpg
install ${VERBOSE} -DT -m 440                   system-stuff/lumen-lightfield                    /etc/sudoers.d/lumen-lightfield
install ${VERBOSE} -DT -m 644                   system-stuff/getty@tty1.service.d_override.conf  /etc/systemd/system/getty@tty1.service.d/override.conf
install ${VERBOSE} -DT -m 644 -o lumen -g lumen system-stuff/lumen-bash_profile                  /home/lumen/.bash_profile
install ${VERBOSE} -DT -m 644 -o lumen -g lumen system-stuff/lumen-real_bash_profile             /home/lumen/.real_bash_profile
install ${VERBOSE} -DT -m 600 -o lumen -g lumen gpg/pubring.gpg                                  /home/lumen/.gnupg/pubring.gpg
install ${VERBOSE} -DT -m 600 -o lumen -g lumen gpg/trustdb.gpg                                  /home/lumen/.gnupg/trustdb.gpg
install ${VERBOSE} -DT -m 644                   system-stuff/clean-up-mount-points.service       /lib/systemd/system/clean-up-mount-points.service
install ${VERBOSE} -DT -m 755                   build/lf                                         /usr/bin/lf
install ${VERBOSE} -DT -m 755                   mountmon/build/mountmon                          /usr/bin/mountmon
install ${VERBOSE} -DT -m 644                   stdio-shepherd/printer.py                        /usr/share/lightfield/libexec/stdio-shepherd/printer.py
install ${VERBOSE} -DT -m 755                   stdio-shepherd/stdio-shepherd.py                 /usr/share/lightfield/libexec/stdio-shepherd/stdio-shepherd.py
install ${VERBOSE} -DT -m 755                   system-stuff/reset-lumen-arduino-port            /usr/share/lightfield/libexec/reset-lumen-arduino-port

if [ "${RELEASE_TRAIN}" = "base" ]
then
    install ${VERBOSE} -DT -m 644               system-stuff/dlpc350-set-projector-power.service /lib/systemd/system/set-projector-power.service
    install ${VERBOSE} -DT -m 644               usb-driver/90-dlpc350.rules                      /lib/udev/rules.d/90-dlpc350.rules
    install ${VERBOSE} -DT -m 755               usb-driver/set-projector-power                   /usr/bin/set-projector-power
    install ${VERBOSE} -DT -m 644               system-stuff/99-waveshare-dlpc350.conf           /usr/share/X11/xorg.conf.d/99-waveshare.conf
elif [ "${RELEASE_TRAIN}" = "dlp4710" ]
then
    install ${VERBOSE} -DT -m 644               system-stuff/dlp4710-set-projector-power.service /lib/systemd/system/set-projector-power.service
    install ${VERBOSE} -DT -m 644               dlp4710/90-dlp4710.rules                         /lib/udev/rules.d/90-dlp4710.rules
    install ${VERBOSE} -DT -m 4555              dlp4710/set-projector-power                      /usr/bin/set-projector-power
    install ${VERBOSE} -DT -m 755               system-stuff/dlp4710-reset-lumen-projector-port  /usr/share/lightfield/libexec/reset-lumen-projector-port
    install ${VERBOSE} -DT -m 644               system-stuff/99-waveshare-dlp4710.conf           /usr/share/X11/xorg.conf.d/99-waveshare.conf
fi

[ -f /home/lumen/.gnupg/pubring.kbx ] && rm ${VERBOSE} /home/lumen/.gnupg/pubring.kbx
chmod ${CHXXXVERBOSE} -R go= /home/lumen/.gnupg

# shellcheck disable=SC2164
cd ${PRINTRUN_SRC}
install ${VERBOSE} -DT -m 644                   printrun/__init__.py                             /usr/share/lightfield/libexec/printrun/printrun/__init__.py
install ${VERBOSE} -DT -m 644                   printrun/eventhandler.py                         /usr/share/lightfield/libexec/printrun/printrun/eventhandler.py
install ${VERBOSE} -DT -m 644                   printrun/gcoder.py                               /usr/share/lightfield/libexec/printrun/printrun/gcoder.py
install ${VERBOSE} -DT -m 644                   printrun/printcore.py                            /usr/share/lightfield/libexec/printrun/printrun/printcore.py
install ${VERBOSE} -DT -m 644                   printrun/utils.py                                /usr/share/lightfield/libexec/printrun/printrun/utils.py
install ${VERBOSE} -DT -m 644                   printrun/plugins/__init__.py                     /usr/share/lightfield/libexec/printrun/printrun/plugins/__init__.py
install ${VERBOSE} -DT -m 644                   Util/constants.py                                /usr/share/lightfield/libexec/printrun/Util/constants.py

blue-bar • Configuring system

perl -lpi -e 's/^(?!##LF## )/##LF## /;' /etc/apt/sources.list /etc/apt/sources.list.d/* 2>/dev/null

echo "deb file:/var/lib/lightfield/software-updates/lightfield${SUFFIX}_${VERSION}_${ARCHITECTURE} ./" > /etc/apt/sources.list.d/volumetric-lightfield.list
chown ${CHXXXVERBOSE} lumen:lumen /etc/apt/sources.list.d/volumetric-lightfield.list

systemctl daemon-reload
systemctl set-default multi-user.target
systemctl enable --now set-projector-power.service clean-up-mount-points.service
systemctl enable getty@tty1.service
systemctl daemon-reload

blue-bar ''
blue-bar 'Done!'
blue-bar ''
