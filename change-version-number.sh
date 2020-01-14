#!/bin/bash

function blue-bar () {
    echo -e "\r\e[1;37;44m$*\e[K\e[0m" 1>&2
}

function red-bar () {
    echo -e "\r\e[1;33;41m$*\e[K\e[0m" 1>&2
}

function error-trap () {
    red-bar 'Failed!'
    exit 1
}

function usage () {
    cat 1>&2 <<HERE
Usage: $(basename "$0") [-t <release-train>] <a.b.c[.d]>
Changes the LightField version in all the relevant places in the source.

  -a <architecture>    Sets the architecture. Default: ${DEFAULT_ARCHITECTURE}
  -t <release-train>   Sets the release train. Default: ${DEFAULT_RELEASE_TRAIN}
HERE
    exit 1
}

function apply-atsign-substitution () {
    local CODE='\e[0;30;48;2;146;208;80m'
    local RST='\e[0m'
    # shellcheck disable=SC2145
    echo -e "Substituting value ${CODE}${2}${RST} for token ${CODE}@@${1}@@${RST} in files ${CODE}${@:3}${RST}"
    sed -i "s/@@${1}@@/${2}/g" "${@:3}"
}

function apply-assignment-substitution () {
    local CODE='\e[0;30;48;2;0;146;208m'
    local RST='\e[0m'
    # shellcheck disable=SC2145
    echo -e "Substituting value ${CODE}${2}${RST} into variable assignment ${CODE}${1}${RST} in files ${CODE}${@:3}${RST}"
    sed -i 's/^'"${1}"'=.*$/'"${1}"'='"${2}"'/g' "${@:3}"
}

LIGHTFIELD_ROOT="/home/lumen/Volumetric/LightField"
LIGHTFIELD_SRC="${LIGHTFIELD_ROOT}/src"

DEFAULT_RELEASE_TRAIN=base
DEFAULT_ARCHITECTURE=$(uname -m)
if [ "${DEFAULT_ARCHITECTURE}" = "x86_64" ]
then
    DEFAULT_ARCHITECTURE=amd64
fi

ARCHITECTURE=${DEFAULT_ARCHITECTURE}
RELEASE_TRAIN=${DEFAULT_RELEASE_TRAIN}
VERSION=

#################################################

ARGS=$(getopt -n 'change-version-number.sh' -o 'a:t:' -- "${@}")
# shellcheck disable=SC2181
if [ ${?} -ne 0 ]
then
    usage
fi
eval set -- "$ARGS"

while [ -n "${1}" ]
do
    case "${1}" in
        '-a')
            if [ "${2}" != "amd64" ] && [ "${2}" != "arm7l" ]
            then
                red-bar "Unrecognized argument to -a switch; valid values are amd64 and arm7l."
                usage
            fi
            ARCHITECTURE="${2}"
            shift
        ;;

        '-t')
            RELEASE_TRAIN="${2}"
            shift
        ;;

        '--')
            shift
            break
        ;;
    esac
    shift
done

if [ -z "${1}" ]
then
    red-bar 'No version number given.'
    usage
elif [ -n "${2}" ]
then
    red-bar 'Too many arguments given.'
    usage
else
    VERSION="${1}"
fi

mapfile -t -d. VER < <(echo -n "${VERSION}")
COUNT=${#VER[@]}

if [ "${COUNT}" -lt 3 ]
then
    red-bar 'Too few components in version number -- must be at least three.'
    usage
elif [ "${COUNT}" -gt 4 ]
then
    red-bar 'Too many components in version number -- must be at most four.'
    usage
fi

if [ "${COUNT}" -eq 3 ]
then
    VER[3]=0
fi

STRINGVER="${VER[0]}.${VER[1]}.${VER[2]}.${VER[3]}"

if [ -z "${RELEASE_TRAIN}" ]
then
    RELEASE_TRAIN=base
fi

cat <<HERE
ARCHITECTURE:  ${ARCHITECTURE}
RELEASE_TRAIN: ${RELEASE_TRAIN}
VERSION:       ${VERSION}
STRINGVER:     ${STRINGVER}
HERE

# shellcheck disable=SC2164
cd "${LIGHTFIELD_SRC}"

blue-bar 'Generating src/version.h from src/version.h.in'

cp version.h.in version.h

apply-atsign-substitution VERSION_STRING "${STRINGVER}"     version.h
apply-atsign-substitution VERSION_MAJOR  "${VER[0]}"        version.h
apply-atsign-substitution VERSION_MINOR  "${VER[1]}"        version.h
apply-atsign-substitution VERSION_TEENY  "${VER[2]}"        version.h
apply-atsign-substitution VERSION_BUILD  "${VER[3]}"        version.h
apply-atsign-substitution RELEASE_TRAIN  "${RELEASE_TRAIN}" version.h

# shellcheck disable=SC2164
cd "${LIGHTFIELD_ROOT}"

blue-bar 'Updating build and packaging scripts'

apply-assignment-substitution ARCHITECTURE  "${ARCHITECTURE}"  install-lightfield.sh make-deb-package.sh make-upgrade-kit.sh
apply-assignment-substitution RELEASE_TRAIN "${RELEASE_TRAIN}" install-lightfield.sh make-deb-package.sh make-upgrade-kit.sh
apply-assignment-substitution VERSION       "${STRINGVER}"     install-lightfield.sh make-deb-package.sh make-upgrade-kit.sh

blue-bar 'Done!'
