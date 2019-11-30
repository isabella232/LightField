#!/bin/bash

VERSION=1.1.0.0
PACKAGE_BUILD_ROOT=/home/lumen/Volumetric/LightField/installer
PLATFORM=armhf

DEPS_DEB_DIR=/code/LightField/1.1.0.0-armhf/deps
LIGHTFIELD_DEB_DIR=/code/LightField/1.1.0.0-armhf/deb
MONTSERRAT_DEB_DIR=/code/LightField
QT_DEB_DIR=/arm/lightfield-arm/libqt-lightfield_5.13.1\~lf3

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
and:   BUILDTYPE  is one of
                  release  create a release-build kit
                  debug    create a debug-build kit
                  both     create both kits

If the build is successful, the requested upgrade kit(s) will be found in
  ${KIT_DIR}/lightfield-BUILDTYPE_${VERSION}_${PLATFORM}.kit
EOF
}

trap error-trap ERR
set -e

LIGHTFIELD_SRC="/home/lumen/Volumetric/LightField"
PACKAGE_BUILD_DIR="${PACKAGE_BUILD_ROOT}/${VERSION}"

KIT_DIR="${PACKAGE_BUILD_DIR}/kit"

VERBOSE=-v
BUILDTYPE=

while [ -n "$1" ]
do
    case "$1" in
        "-q")
            VERBOSE=
        ;;

        "release" | "debug" | "both")
            BUILDTYPE=$1
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
    ARG=$([ -z "${VERBOSE}" ] && echo -q)
    "$0" "${ARG}" release || exit $?
    "$0" "${ARG}" debug   || exit $?
    exit 0
fi

REPO_DIR="${PACKAGE_BUILD_DIR}/repo"

RELEASEDATE=$(date "+%Y-%m-%d")

(
[ -d "${PACKAGE_BUILD_DIR}" ] || mkdir -p "${PACKAGE_BUILD_DIR}"
cd "${PACKAGE_BUILD_DIR}"

blue-bar "• Creating LightField ${VERSION} ${BUILDTYPE}-build update kit"

[ -d "${REPO_DIR}" ] && rm ${VERBOSE} -rf "${REPO_DIR}"

mkdir ${VERBOSE} -p "${REPO_DIR}"
mkdir ${VERBOSE} -p "${KIT_DIR}"

install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEPS_DEB_DIR}"/*.deb
install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${MONTSERRAT_DEB_DIR}/fonts-montserrat_7.200_all.deb"
install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${QT_DEB_DIR}/libqt-lightfield_5.13.1~lf3_${PLATFORM}.deb"
install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${LIGHTFIELD_DEB_DIR}/lightfield-common_${VERSION}_all.deb"

if [ "${BUILDTYPE}" = "release" ]
then
    install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${LIGHTFIELD_DEB_DIR}/lightfield-release_${VERSION}_${PLATFORM}.deb"
elif [ "${BUILDTYPE}" = "debug" ]
then
    install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${LIGHTFIELD_DEB_DIR}/lightfield-debug_${VERSION}_${PLATFORM}.deb"
    if [ -f "${LIGHTFIELD_DEB_DIR}/lightfield-debug-dbgsym_${VERSION}_${PLATFORM}.deb" ]
    then
        install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${LIGHTFIELD_DEB_DIR}/lightfield-debug-dbgsym_${VERSION}_${PLATFORM}.deb"
    elif [ -f "${LIGHTFIELD_DEB_DIR}/lightfield-debug-dbgsym_${VERSION}_${PLATFORM}.ddeb" ]
    then
        install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${LIGHTFIELD_DEB_DIR}/lightfield-debug-dbgsym_${VERSION}_${PLATFORM}.ddeb"
    fi
fi
)

(
cd "${REPO_DIR}"

dpkg-scanpackages . | tee Packages | xz -ceT0 > Packages.xz

apt-ftparchive --config-file ${LIGHTFIELD_SRC}/apt-files/release.conf release . | tee Release | xz -ceT0 > Release.xz

gpg                                                        \
    ${VERBOSE}                                             \
    --batch                                                \
    --armor                                                \
    --local-user "lightfield-repo-maint@volumetricbio.com" \
    --output InRelease                                     \
    --clearsign                                            \
    Release

gpg                                                        \
    ${VERBOSE}                                             \
    --batch                                                \
    --armor                                                \
    --local-user "lightfield-repo-maint@volumetricbio.com" \
    --output Release.gpg                                   \
    --detach-sign                                          \
    Release

rm ${VERBOSE} -f                                           \
    version.inf                                            \
    version.inf.sig

sha256sum -b -- * | sed -r -e 's/^/ /' -e 's/ +\*/ /' > .hashes
(
    sed -e "s/@@VERSION@@/${VERSION}/g" -e "s/@@BUILDTYPE@@/${BUILDTYPE}/g" -e "s/@@RELEASEDATE@@/${RELEASEDATE}/g" "${LIGHTFIELD_SRC}/apt-files/version.inf.in"

    # extract description from ${LIGHTFIELD_SRC}/debian/changelog
    linecount=$( ( grep -n '^ -- LightField packager' ${LIGHTFIELD_SRC}/debian/changelog || echo 0: ) | head -1 | cut -d: -f1 )
    if [ -z "${linecount}" ] || [ "${linecount}" -lt 1 ]
    then
        red-bar " *** Can't find end of first change in ${LIGHTFIELD_SRC}/debian/changelog, aborting"
        exit 1
    fi
    head -$((linecount - 2)) ${LIGHTFIELD_SRC}/debian/changelog | tail +3 | perl -lpe 's/\s+$//; s/^$/./; s/^/ /'

    echo 'Checksums-SHA256:'
    cat .hashes
) > version.inf
rm .hashes

gpg                                                                     \
    ${VERBOSE}                                                          \
    --batch                                                             \
    --armor                                                             \
    --local-user "lightfield-packager@volumetricbio.com"                \
    --output version.inf.sig                                            \
    --detach-sign                                                       \
    version.inf

rm                                                                      \
    ${VERBOSE}                                                          \
    -f                                                                  \
    "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_${PLATFORM}.kit"     \
    "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_${PLATFORM}.kit.sig" \
    "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_${PLATFORM}.kit.zip"

tar                                                                     \
    ${VERBOSE} ${VERBOSE}                                               \
    -c                                                                  \
    -f "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_${PLATFORM}.kit"  \
    --owner=root                                                        \
    --group=root                                                        \
    --sort=name                                                         \
    --                                                                  \
    *
)

(
cd ${KIT_DIR}

gpg                                                                     \
    ${VERBOSE}                                                          \
    --batch                                                             \
    --armor                                                             \
    --local-user "lightfield-packager@volumetricbio.com"                \
    --output "lightfield-${BUILDTYPE}_${VERSION}_${PLATFORM}.kit.sig"   \
    --detach-sign                                                       \
    "lightfield-${BUILDTYPE}_${VERSION}_${PLATFORM}.kit"

zip                                                                     \
    -0joq                                                               \
    "lightfield-${BUILDTYPE}_${VERSION}_${PLATFORM}.kit.zip"            \
    "lightfield-${BUILDTYPE}_${VERSION}_${PLATFORM}.kit"                \
    "lightfield-${BUILDTYPE}_${VERSION}_${PLATFORM}.kit.sig"
)

blue-bar "• Cleaning up"

rm ${VERBOSE} -rf ${REPO_DIR}

blue-bar ""
blue-bar "• Done!"
blue-bar ""
