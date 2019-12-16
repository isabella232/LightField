#!/bin/bash

ARCHITECTURE=amd64
RELEASE_TRAIN=base
VERSION=1.0.10.0

PACKAGE_BUILD_ROOT=/home/lumen/Volumetric/LightField/packaging
USE_KEY_SET=current

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
Where: -q           build quietly
       -a <arch>    Sets the architecture. Valid values: amd64 arm7l.
                    Default: ${DEFAULT_ARCHITECTURE}
       -t <train>   Sets the release train. Default: ${DEFAULT_RELEASE_TRAIN}
       BUILDTYPE    is one of
                    release  create a release-build kit
                    debug    create a debug-build kit
                    both     create both kits

If the build is successful, the requested upgrade kit(s) will be found in
  ${KIT_DIR}/lightfield$([ "${RELEASE_TRAIN}" = "base" ] || echo "-${RELEASE_TRAIN}" )-BUILDTYPE_${VERSION}_${ARCHITECTURE}.kit
EOF
}

trap error-trap ERR
set -e

if [ "${USE_KEY_SET}" = "current" ]; then
    #REPO_KEY_ID=18DDFE4E607507208C9F6E6582768C36BD8725D2
    #PKG_KEY_ID=0EF6486549978C0C76B49E99C9FC781B66B69981
    REPO_KEY_ID=lightfield-repo-maint@volumetricbio.com
    PKG_KEY_ID=lightfield-packager@volumetricbio.com
elif [ "${USE_KEY_SET}" = "future" ]; then
    REPO_KEY_ID=E91BD3361F39D49C78B1E3A2B55C0E8D4B632A66
    PKG_KEY_ID=78DAD29978EB392992D7FE0423025033D9E840F7
else
    red-bar "The key set '${USE_KEY_SET}' is unrecognized."
fi

VERBOSE=-v

ARCHITECTURE=$(uname -m)
[ "${ARCHITECTURE}" = "x86_64" ] && ARCHITECTURE=amd64
BUILDTYPE=

DEFAULT_ARCHITECTURE=${ARCHITECTURE}
DEFAULT_RELEASE_TRAIN=${RELEASE_TRAIN}

if ! getopt -Q -q -n 'make-upgrade-kit.sh' -o 'qa:t:' -- "$@"
then
    usage
    exit 1
fi

ARGS=$(getopt -Q -q -n 'make-upgrade-kit.sh' -o 'qa:t:' -- "$@")
eval set -- "$ARGS"

while [ -n "$1" ]
do
    case "$1" in
        "-q")
            VERBOSE=
        ;;

        "-a")
            if [ "${2}" = "amd64" ] || [ "${2}" = "arm7l" ]
            then
                ARCHITECTURE="${2}"
                shift
            else
                usage
                exit 1
            fi
        ;;

        "-t")
            RELEASE_TRAIN="${2}"
            shift
        ;;

        "release" | "debug" | "both")
            BUILDTYPE="${1}"
            break
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

if [ "${RELEASE_TRAIN}" = "base" ]
then
    SUFFIX=${BUILDTYPE}
else
    SUFFIX=${RELEASE_TRAIN}-${BUILDTYPE}
fi

LIGHTFIELD_SRC="/home/lumen/Volumetric/LightField"
PACKAGE_BUILD_DIR="${PACKAGE_BUILD_ROOT}/${SUFFIX}-${VERSION}"
DEB_BUILD_DIR="${PACKAGE_BUILD_DIR}/deb"
#LIGHTFIELD_PACKAGE="${DEB_BUILD_DIR}/lightfield-${VERSION}"
#LIGHTFIELD_FILES="${LIGHTFIELD_PACKAGE}/files"

KIT_DIR="${PACKAGE_BUILD_DIR}/kit"

if [ "${BUILDTYPE}" = "both" ]
then
    "${0}" "${ARGS}" release || exit $?
    "${0}" "${ARGS}" debug   || exit $?
    exit 0
fi

REPO_DIR="${PACKAGE_BUILD_DIR}/repo"
RELEASEDATE=$(date "+%Y-%m-%d")

cd "${PACKAGE_BUILD_DIR}"

blue-bar "• Creating LightField ${VERSION} ${RELEASE_TRAIN} ${BUILDTYPE}-build update kit"

[ -d "${REPO_DIR}" ] && rm ${VERBOSE} -rf "${REPO_DIR}"

mkdir ${VERBOSE} -p "${REPO_DIR}"
mkdir ${VERBOSE} -p "${KIT_DIR}"

install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${LIGHTFIELD_SRC}/fonts-montserrat_7.200_all.deb"
install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-common_${VERSION}_all.deb"

if [ "${BUILDTYPE}" = "release" ]
then
    install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.deb"
elif [ "${BUILDTYPE}" = "debug" ]
then
    install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.deb"
    if [ -f "${DEB_BUILD_DIR}/lightfield-${SUFFIX}-dbgsym_${VERSION}_${ARCHITECTURE}.deb" ]
    then
        install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-${SUFFIX}-dbgsym_${VERSION}_${ARCHITECTURE}.deb"
    elif [ -f "${DEB_BUILD_DIR}/lightfield-${SUFFIX}-dbgsym_${VERSION}_${ARCHITECTURE}.ddeb" ]
    then
        install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-${SUFFIX}-dbgsym_${VERSION}_${ARCHITECTURE}.ddeb"
    else
    red-bar "!!! Unable to find either"
    red-bar "!!!    ${DEB_BUILD_DIR}/lightfield-${SUFFIX}-dbgsym_${VERSION}_${ARCHITECTURE}.deb"
    red-bar "!!! or"
    red-bar "!!!    ${DEB_BUILD_DIR}/lightfield-${SUFFIX}-dbgsym_${VERSION}_${ARCHITECTURE}.ddeb"
    red-bar "!!! Build failed!"
    fi
fi

cd "${REPO_DIR}"

dpkg-scanpackages . | tee Packages | xz -ceT0 > Packages.xz

apt-ftparchive --config-file ${LIGHTFIELD_SRC}/apt-files/release.conf release . | tee Release | xz -ceT0 > Release.xz

gpg                               \
    ${VERBOSE}                    \
    --batch                       \
    --armor                       \
    --local-user "${REPO_KEY_ID}" \
    --output InRelease            \
    --clearsign                   \
    Release

gpg                               \
    ${VERBOSE}                    \
    --batch                       \
    --armor                       \
    --local-user "${REPO_KEY_ID}" \
    --output Release.gpg          \
    --detach-sign                 \
    Release

rm ${VERBOSE} -f                  \
    version.inf                   \
    version.inf.sig

sha256sum -- -b * | sed -r -e 's/^/ /' -e 's/ +\*/ /' > .hashes
(
    sed                                              \
        -e  "s/@@ARCHITECTURE@@/${ARCHITECTURE}/g"   \
        -e     "s/@@BUILDTYPE@@/${BUILDTYPE}/g"      \
        -e   "s/@@RELEASEDATE@@/${RELEASEDATE}/g"    \
        -e "s/@@RELEASE_TRAIN@@/${RELEASE_TRAIN}/g"  \
        -e       "s/@@VERSION@@/${VERSION}/g"        \
        "${LIGHTFIELD_SRC}/apt-files/version.inf.in"

    # extract description from ${LIGHTFIELD_SRC}/debian/changelog
    linecount=$(grep -n '^ -- LightField packager' ${LIGHTFIELD_SRC}/debian/changelog | head -1 | cut -d: -f1 || echo 0)
    if [ -z "${linecount}" ] || [ "${linecount}" -lt 1 ]
    then
        red-bar "!!! Can't find end of first change in ${LIGHTFIELD_SRC}/debian/changelog, aborting"
        exit 1
    fi
    head -$((linecount - 2)) ${LIGHTFIELD_SRC}/debian/changelog | tail +3 | perl -lpe 's/\s+$//; s/^$/./; s/^/ /'

    echo 'Checksums-SHA256:'
    cat .hashes
) > version.inf
rm .hashes

gpg                                                                      \
    ${VERBOSE}                                                           \
    --batch                                                              \
    --armor                                                              \
    --local-user "${PKG_KEY_ID}"                                         \
    --output version.inf.sig                                             \
    --detach-sign                                                        \
    version.inf

rm                                                                       \
    ${VERBOSE}                                                           \
    -f                                                                   \
    "${KIT_DIR}/lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.kit"     \
    "${KIT_DIR}/lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.kit.sig" \
    "${KIT_DIR}/lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.kit.zip"

tar                                                                      \
    ${VERBOSE} ${VERBOSE}                                                \
    -c                                                                   \
    -f "${KIT_DIR}/lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.kit"  \
    --owner=root                                                         \
    --group=root                                                         \
    --sort=name                                                          \
    --                                                                   \
    *

cd "${KIT_DIR}"

gpg                                                                      \
    ${VERBOSE}                                                           \
    --batch                                                              \
    --armor                                                              \
    --local-user "${PKG_KEY_ID}"                                         \
    --output "lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.kit.sig"   \
    --detach-sign                                                        \
    "lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.kit"

zip                                                                      \
    -0joq                                                                \
    "lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.kit.zip"            \
    "lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.kit"                \
    "lightfield-${SUFFIX}_${VERSION}_${ARCHITECTURE}.kit.sig"

blue-bar "• Cleaning up"

cd ..

rm ${VERBOSE} -rf "${REPO_DIR}"

blue-bar ""
blue-bar "• Done!"
blue-bar ""
