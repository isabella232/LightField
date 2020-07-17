#!/bin/bash
# shellcheck disable=SC2164

LIGHTFIELD_ROOT="${PWD}"

#########################################################
##                                                     ##
##     No user-serviceable parts below this point.     ##
##                                                     ##
#########################################################

function usage () {
    cat 1>&2 <<HERE
Usage: $(basename "$0") [-t <release-train>] <a.b.c[.d]>
Changes the LightField version in all the relevant places in the source.

  -t <release-train>   Sets the release train. Default: ${DEFAULT_RELEASE_TRAIN}
                       Available release trains:
                       base       	  LightField with DLPC350 support
                       dlp4710   	  LightField with DLP4710 support
                       xbase      	  LightField with DLPC350 support (experimental)
                       xdlp4710  	  LightField with DLP4710 support (experimental)
                       xdlp4710-20um  LightField with DLP4710 and 20 um pixel size support
  <a.b.c[.d]>          Version number. If the fourth element is omitted,
                       it defaults to 0.
HERE
    exit 1
}

# shellcheck disable=SC1090
source "${LIGHTFIELD_ROOT}/shared-stuff.sh"

ARGS=$(getopt -n 'change-version-number.sh' -o 't:' -- "${@}")
# shellcheck disable=SC2181
if [ ${?} -ne 0 ]
then
    usage
fi
eval set -- "$ARGS"

while [ -n "${1}" ]
do
    case "${1}" in
        '-t')
            if [ "${2}" = "base" ] || [ "${2}" = "xbase" ] || [ "${2}" = "dlp4710" ] || [ "${2}" = "xdlp4710" ] || [ "${2}" = "xdlp4710-20um" ]
            then
                RELEASE_TRAIN="${2}"
            else
                red-bar "Unknown release train name '${2}'."
                usage
            fi
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
    if [ -z "$(git describe --abbrev=0 2>/dev/null || true)" ]
    then
        VER[3]=$(git rev-list --count HEAD)
    else
        VER[3]=0
    fi
fi

STRINGVER="${VER[0]}.${VER[1]}.${VER[2]}.${VER[3]}"
GITHASH="$(git rev-parse HEAD)"

if [ -z "${RELEASE_TRAIN}" ]
then
    RELEASE_TRAIN=base
fi

#cat <<HERE
#ARCHITECTURE:  ${ARCHITECTURE}
#RELEASE_TRAIN: ${RELEASE_TRAIN}
#VERSION:       ${VERSION}
#STRINGVER:     ${STRINGVER}
#HERE

cd "${LIGHTFIELD_ROOT}/src"

blue-bar 'Generating src/version.h from src/version.h.in'

cp version.h.in version.h

apply-atsign-substitution VERSION_STRING "${STRINGVER}" version.h
apply-atsign-substitution VERSION_MAJOR "${VER[0]}" version.h
apply-atsign-substitution VERSION_MINOR "${VER[1]}" version.h
apply-atsign-substitution VERSION_TEENY "${VER[2]}" version.h
apply-atsign-substitution VERSION_BUILD "${VER[3]}" version.h
apply-atsign-substitution VERSION_GITHASH "${GITHASH}" version.h
apply-atsign-substitution RELEASE_TRAIN  "${RELEASE_TRAIN}" version.h

cd "${LIGHTFIELD_ROOT}"

blue-bar 'Updating build and packaging scripts'

apply-assignment-substitution ARCHITECTURE "${ARCHITECTURE}" shared-stuff.sh
apply-assignment-substitution RELEASE_TRAIN "${RELEASE_TRAIN}" shared-stuff.sh
apply-assignment-substitution VERSION "${STRINGVER}" shared-stuff.sh

blue-bar 'Done!'
