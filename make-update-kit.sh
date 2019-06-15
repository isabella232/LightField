#!/bin/bash -e

VERSION=1.0.1
PACKAGE_BUILD_ROOT=/home/lumen/Volumetric/LightField/packaging

#########################################################
##                                                     ##
##     No user-serviceable parts below this point.     ##
##                                                     ##
#########################################################

function clear () {
    echo -ne "\x1B[0m\x1B[H\x1B[J\x1B[3J"
}

function blue-bar () {
    echo -e "\r\x1B[1;37;44m$*\x1B[K\x1B[0m"
}

function usage () {
    cat <<EOF
Usage: $(basename $0) [-q] BUILDTYPE
Where: -q         build quietly
and:   BUILDTYPE  is one of
                  release  create a release-build package
                  debug    create a debugging-build package

If the build is successful, the requested upgrade kit will be found in file
${KIT_DIR}/lightfield-release_BUILDTYPE_amd64.kit
EOF
}

LIGHTFIELD_SRC="/home/lumen/Volumetric/LightField"
PACKAGE_BUILD_DIR="${PACKAGE_BUILD_ROOT}/${VERSION}"
DEB_BUILD_DIR="${PACKAGE_BUILD_DIR}/deb"
LIGHTFIELD_PACKAGE="${DEB_BUILD_DIR}/lightfield-${VERSION}"
LIGHTFIELD_FILES="${LIGHTFIELD_PACKAGE}/files"

KIT_DIR="${PACKAGE_BUILD_DIR}/kit"

VERBOSE=-v
BUILDTYPE=

while [ -n "$1" ]
do
    if [ "$1" = "-q" ]
    then
	VERBOSE=
    elif [ "$1" = "release" -o "$1" = "debug" ]
    then
	BUILDTYPE="$1"
    else
	usage
	exit 1
    fi
    shift
done

if [ -z "${BUILDTYPE}" ]
then
    usage
    exit 1
fi

RELEASEDATE=$(date "+%Y-%m-%d")
REPO_DIR="${PACKAGE_BUILD_DIR}/repo/${BUILDTYPE}"
APT_CACHE_DIR="${PACKAGE_BUILD_DIR}/repo/apt-cache/${BUILDTYPE}"

clear

cd "${PACKAGE_BUILD_DIR}"

blue-bar • Creating LightField "${VERSION}" "${BUILDTYPE}" build update kit


[ -d "${REPO_DIR}"      ] && rm ${VERBOSE} -rd "${REPO_DIR}"
[ -d "${APT_CACHE_DIR}" ] && rm ${VERBOSE} -rd "${APT_CACHE_DIR}"

mkdir ${VERBOSE} -p "${REPO_DIR}/dists/lightfield/main/binary-amd64"
mkdir ${VERBOSE} -p "${REPO_DIR}/dists/lightfield/main/binary-all"
mkdir ${VERBOSE} -p "${APT_CACHE_DIR}"
mkdir ${VERBOSE} -p "${KIT_DIR}"

install ${VERBOSE} -Dt "${REPO_DIR}/pool/main/f/" -m 644 "${DEB_BUILD_DIR}/fonts-montserrat_7.200_all.deb"
install ${VERBOSE} -Dt "${REPO_DIR}/pool/main/l/" -m 644 "${DEB_BUILD_DIR}/lightfield-common_1.0.1_all.deb"

if [ "${BUILDTYPE}" = "release" ]
then
    install ${VERBOSE} -Dt "${REPO_DIR}/pool/main/l/" -m 644 "${DEB_BUILD_DIR}/lightfield-release_1.0.1_amd64.deb"
elif [ "${BUILDTYPE}" = "debug" ]
then
    install ${VERBOSE} -Dt "${REPO_DIR}/pool/main/l/" -m 644 "${DEB_BUILD_DIR}/lightfield-debug_1.0.1_amd64.deb"
    install ${VERBOSE} -Dt "${REPO_DIR}/pool/main/l/" -m 644 "${DEB_BUILD_DIR}/lightfield-debug-dbgsym_1.0.1_amd64.deb"
fi

apt-ftparchive                                                             \
    generate <(                                                            \
	sed                                                                \
	    -e "s:@@REPO_DIR@@:${REPO_DIR}:g"                              \
	    -e "s:@@APT_CACHE_DIR@@:${APT_CACHE_DIR}:g"                    \
	    "${LIGHTFIELD_SRC}/apt-files/apt-ftparchive.conf.in"           \
    )

apt-ftparchive                                                             \
    -c "${LIGHTFIELD_SRC}/apt-files/release.conf"                          \
    release "${REPO_DIR}/dists/lightfield"                                 \
    > "${REPO_DIR}/dists/lightfield/Release"

gpg                                                                        \
    ${VERBOSE}                                                             \
    --detach-sign                                                          \
    --armor                                                                \
    --local-user "lightfield-repo-maint@volumetricbio.com"                 \
    --output "${REPO_DIR}/dists/lightfield/Release.gpg"                    \
    "${REPO_DIR}/dists/lightfield/Release"

cd "${REPO_DIR}"
sed                                                                        \
    -e "s/@@VERSION@@/${VERSION}/g"                                        \
    -e "s/@@BUILDTYPE@@/${BUILDTYPE}/g"                                    \
    -e "s/@@RELEASEDATE@@/${RELEASEDATE}/g"                                \
    "${LIGHTFIELD_SRC}/apt-files/version.inf.in"                           \
    > "${REPO_DIR}/version.inf"

gpg                                                                        \
    ${VERBOSE}                                                             \
    --detach-sign                                                          \
    --armor                                                                \
    --local-user "lightfield-packager@volumetricbio.com"                   \
    --output "${REPO_DIR}/version.inf.sig"                                 \
    "${REPO_DIR}/version.inf"

tar                                                                        \
    ${VERBOSE} ${VERBOSE}                                                  \
    -c                                                                     \
    -f "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_amd64.kit"           \
    --owner=root                                                           \
    --group=root                                                           \
    *

gpg                                                                        \
    ${VERBOSE}                                                             \
    --detach-sign                                                          \
    --armor                                                                \
    --local-user "lightfield-packager@volumetricbio.com"                   \
    --output "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_amd64.kit.sig" \
    "${KIT_DIR}/lightfield-release_${VERSION}_amd64.kit"

blue-bar ""
blue-bar "• Done!"
blue-bar ""
