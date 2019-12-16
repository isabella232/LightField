#!/bin/bash

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
    cat <<HERE
Usage: $(basename "$0") [-a <architecture>] [-t <release-train>] <a.b.c[.d]>
Changes the LightField version in all the relevant places in the source.

  -a <architecture>    Sets the architecture. Valid values: amd64 arm7l.
                       Default: ${DEFAULT_ARCHITECTURE}
  -t <release-train>   Sets the release train. Default: ${DEFAULT_RELEASE_TRAIN}
HERE
}

function apply-atsign-substitution () {
    perl -lpi -e "s/@@${1}@@/${2}/g;" $3-
}

function apply-assignment-substitution () {
    perl -lpi -e "s/^(\\s*)${1}=.*$/\$1${1}=${2}/g;" $3-
}

LIGHTFIELD_ROOT="/home/lumen/Volumetric/LightField"
LIGHTFIELD_SRC="${LIGHTFIELD_ROOT}/src"

RELEASE_TRAIN=base
ARCHITECTURE=$(uname -m)
VERSION=
[ "${ARCHITECTURE}" = "x86_64" ] && ARCHITECTURE=amd64

DEFAULT_RELEASE_TRAIN=${RELEASE_TRAIN}
DEFAULT_ARCHITECTURE=${ARCHITECTURE}

if [ -z "$1" ]
then
    usage
    exit 1
fi

if ! getopt -Q -q -n 'change-version-number.sh' -o 'a:t:' -- "$@"
then
    usage
    return
fi

ARGS=$(getopt -Q -q -n 'change-version-number.sh' -o 'a:t:' -- "$@")
eval set -- "$ARGS"

while [ -n "${1}" ]
do
    case "${1}" in
        '-a')
            ARCHITECTURE="${2}"
            shift
        ;;

        '-t')
            RELEASE_TRAIN="${2}"
            shift
        ;;

        *)
            VERSION="${1}"
        ;;
    esac
    shift
done

if [ -z "${VERSION}" ]
then
    usage
    exit 1
fi

# shellcheck disable=SC2207
VER=( $(IFS="."; echo "${VERSION}") )
COUNT=${#VER[@]}
if [ "${COUNT}" -lt 3 ]
then
    red-bar 'Too few components in version number -- must be at least three.'
    exit 1
elif [ "${COUNT}" -gt 4 ]
then
    red-bar 'Too many components in version number -- must be at most four.'
    exit 1
fi
if [ "${COUNT}" -eq 3 ]
then
    VER[3]=0
fi
STRINGVER="${VER[0]}.${VER[1]}.${VER[2]}.${VER[3]}"

# shellcheck disable=SC2164
cd "${LIGHTFIELD_SRC}"

blue-bar 'Generating src/version.h from src/version.h.in'

sed                                                                \
    -e "s/@@VERSION_STRING@@/${STRINGVER}/g"                       \
    -e  "s/@@VERSION_MAJOR@@/${VER[0]}/g"                          \
    -e  "s/@@VERSION_MINOR@@/${VER[1]}/g"                          \
    -e  "s/@@VERSION_TEENY@@/${VER[2]}/g"                          \
    -e  "s/@@VERSION_BUILD@@/${VER[3]}/g"                          \
    -e  "s/@@RELEASE_TRAIN@@/${RELEASE_TRAIN}/g"                   \
    < version.h.in                                                 \
    > version.h

# shellcheck disable=SC2164
cd "${LIGHTFIELD_ROOT}"

if [ -z "${RELEASE_TRAIN}" ]
then
    SUFFIX=-${BUILDTYPE}
else
    SUFFIX=-${RELEASE_TRAIN}-${BUILDTYPE}
fi

if [ "${BUILDTYPE}" = release ]
then
    ANTIBUILDTYPE=debug
else
    ANTIBUILDTYPE=release
fi

cp ${VERBOSE} debian/control.in debian/control
for a in install preinst postinst prerm postrm
do
    cp ${VERBOSE} "debian/lightfield.${a}.in" "debian/lightfield-${SUFFIX}.${a}"
done

blue-bar 'Updating build and packaging scripts'

perl                                                               \
    -lpi                                                           \
    -e "s/^\\s*ANTIBUILDTYPE=.*$/ANTIBUILDTYPE=\"${ANTIBUILDTYPE}\"/g;" \
    -e  "s/@@ARCHITECTURE@@/${ARCHITECTURE}/g;"                    \
    -e     "s/@@BUILDTYPE@@/${BUILDTYPE}/g;"                       \
    -e "s/@@RELEASE_TRAIN@@/${RELEASE_TRAIN}/g;"                   \
    -e       "s/@@VERSION@@/${STRINGVER}/g;"                       \
    -e   "s/(?:amd64|arm7l)/${ARCHITECTURE}/g;"                    \
    install-lightfield.sh                                          \
    make-deb-package.sh                                            \
    make-upgrade-kit.sh                                            \
    unpack-kit-manually.sh                                         \

perl                                                               \
    -lpi                                                           \
    -e "s/@@ANTIBUILDTYPE@@/${ANTIBUILDTYPE}/g;"                   \
    -e  "s/@@ARCHITECTURE@@/${ARCHITECTURE}/g;"                    \
    -e     "s/@@BUILDTYPE@@/${BUILDTYPE}/g;"                       \
    -e "s/@@RELEASE_TRAIN@@/${RELEASE_TRAIN}/g;"                   \
    -e       "s/@@VERSION@@/${STRINGVER}/g;"                       \
    -e   "s/(?:amd64|arm7l)/${ARCHITECTURE}/g;"                    \
    "debian/lightfield-${SUFFIX}.install"                          \
    "debian/lightfield-${SUFFIX}.preinst"                          \
    "debian/lightfield-${SUFFIX}.postinst"                         \
    "debian/lightfield-${SUFFIX}.prerm"                            \
    "debian/lightfield-${SUFFIX}.postrm"

perl                                                               \
    -lpi                                                           \
    -e  "s/@@ARCHITECTURE@@/ARCHITECTURE=\"${ARCHITECTURE}\"/g;"   \
    -e     "s/@@BUILDTYPE@@/BUILDTYPE=\"${BUILDTYPE}\"/g;"         \
    -e "s/@@RELEASE_TRAIN@@/RELEASE_TRAIN=\"${RELEASE_TRAIN}\"/g;" \
    -e       "s/@@VERSION@@/VERSION=${STRINGVER}/g;"               \
    -e   "s/(?:amd64|arm7l)/${ARCHITECTURE}/g;"                    \
    debian/control
