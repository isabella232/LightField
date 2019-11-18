#!/bin/bash

VERSION=1.0.9.4
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
Usage: $(basename $0) [-q] BUILDTYPE
Where: -q         build quietly
and:   BUILDTYPE  is one of
                  release  create a release-build kit
                  debug    create a debug-build kit
                  both     create both kits

If the build is successful, the requested upgrade kit(s) will be found in
  ${KIT_DIR}/lightfield-BUILDTYPE_${VERSION}_amd64.kit
EOF
}

trap error-trap ERR
set -e

LIGHTFIELD_SRC="/home/lumen/Volumetric/LightField"
PACKAGE_BUILD_DIR="${PACKAGE_BUILD_ROOT}/${VERSION}"
DEB_BUILD_DIR="${PACKAGE_BUILD_DIR}/deb"
LIGHTFIELD_PACKAGE="${DEB_BUILD_DIR}/lightfield-${VERSION}"
LIGHTFIELD_FILES="${LIGHTFIELD_PACKAGE}/files"

KIT_DIR="${PACKAGE_BUILD_DIR}/kit"

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
    ARG=$(if [ -z "${VERBOSE}" ]; then echo -q; fi)
    $0 ${ARG} release || exit $?
    $0 ${ARG} debug   || exit $?
    exit 0
fi

REPO_DIR="${PACKAGE_BUILD_DIR}/repo"
DISTRIBUTION=cosmic

RELEASEDATE=$(date "+%Y-%m-%d")

cd "${PACKAGE_BUILD_DIR}"

blue-bar • Creating LightField "${VERSION}" "${BUILDTYPE}"-build update kit

[ -d "${REPO_DIR}"      ] && rm ${VERBOSE} -rf "${REPO_DIR}"

mkdir ${VERBOSE} -p "${REPO_DIR}"
mkdir ${VERBOSE} -p "${KIT_DIR}"

install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${LIGHTFIELD_SRC}/fonts-montserrat_7.200_all.deb"
install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-common_${VERSION}_all.deb"

if [ "${BUILDTYPE}" = "release" ]
then
    install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-release_${VERSION}_amd64.deb"
elif [ "${BUILDTYPE}" = "debug" ]
then
    install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-debug_${VERSION}_amd64.deb"
    if [ -f "${DEB_BUILD_DIR}/lightfield-debug-dbgsym_${VERSION}_amd64.deb" ]
    then
        install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-debug-dbgsym_${VERSION}_amd64.deb"
    elif [ -f "${DEB_BUILD_DIR}/lightfield-debug-dbgsym_${VERSION}_amd64.ddeb" ]
    then
        install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${DEB_BUILD_DIR}/lightfield-debug-dbgsym_${VERSION}_amd64.ddeb"
    else
	red-bar "!!! Unable to find either"
	red-bar "!!!    ${DEB_BUILD_DIR}/lightfield-debug-dbgsym_${VERSION}_amd64.deb"
	red-bar "!!! or"
	red-bar "!!!    ${DEB_BUILD_DIR}/lightfield-debug-dbgsym_${VERSION}_amd64.ddeb"
	red-bar "!!! Build failed!"
    fi
fi

cd "${REPO_DIR}"

dpkg-scanpackages . | tee Packages | xz -ceT0 > Packages.xz

apt-ftparchive --config-file ${LIGHTFIELD_SRC}/apt-files/release.conf release . | tee Release | xz -ceT0 > Release.xz

gpg                                                               \
    ${VERBOSE}                                                    \
    --batch                                                       \
    --armor                                                       \
    --local-user "${REPO_KEY_ID}"                                 \
    --output InRelease                                            \
    --clearsign                                                   \
    Release

gpg                                                               \
    ${VERBOSE}                                                    \
    --batch                                                       \
    --armor                                                       \
    --local-user "${REPO_KEY_ID}"                                 \
    --output Release.gpg                                          \
    --detach-sign                                                 \
    Release

rm ${VERBOSE} -f                                                  \
    version.inf                                                   \
    version.inf.sig

sha256sum -b * | sed -r -e 's/^/ /' -e 's/ +\*/ /' > .hashes
(
    sed -e "s/@@VERSION@@/${VERSION}/g" -e "s/@@BUILDTYPE@@/${BUILDTYPE}/g" -e "s/@@RELEASEDATE@@/${RELEASEDATE}/g" "${LIGHTFIELD_SRC}/apt-files/version.inf.in"

    # extract description from ${LIGHTFIELD_SRC}/debian/changelog
    linecount=$(grep -n '^ -- LightField packager' ${LIGHTFIELD_SRC}/debian/changelog | head -1 | cut -d: -f1 || echo 0)
    if [ -z "${linecount}" -o \( "${linecount}" -lt 1 \) ]
    then
        red-bar " *** Can't find end of first change in ${LIGHTFIELD_SRC}/debian/changelog, aborting"
        exit 1
    fi
    head -$((linecount - 2)) ${LIGHTFIELD_SRC}/debian/changelog | tail +3 | perl -lpe 's/\s+$//; s/^$/./; s/^/ /'

    echo 'Checksums-SHA256:'
    cat .hashes
) > version.inf
rm .hashes

gpg                                                               \
    ${VERBOSE}                                                    \
    --batch                                                       \
    --armor                                                       \
    --local-user "${PKG_KEY_ID}"                                  \
    --output version.inf.sig                                      \
    --detach-sign                                                 \
    version.inf

rm                                                                \
    ${VERBOSE}                                                    \
    -f                                                            \
    "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_amd64.kit"     \
    "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_amd64.kit.sig" \
    "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_amd64.kit.zip"

tar                                                               \
    ${VERBOSE} ${VERBOSE}                                         \
    -c                                                            \
    -f "${KIT_DIR}/lightfield-${BUILDTYPE}_${VERSION}_amd64.kit"  \
    --owner=root                                                  \
    --group=root                                                  \
    --sort=name                                                   \
    *

cd ${KIT_DIR}

gpg                                                               \
    ${VERBOSE}                                                    \
    --batch                                                       \
    --armor                                                       \
    --local-user "${PKG_KEY_ID}"                                  \
    --output "lightfield-${BUILDTYPE}_${VERSION}_amd64.kit.sig"   \
    --detach-sign                                                 \
    "lightfield-${BUILDTYPE}_${VERSION}_amd64.kit"

zip                                                               \
    -0joq                                                         \
    "lightfield-${BUILDTYPE}_${VERSION}_amd64.kit.zip"            \
    "lightfield-${BUILDTYPE}_${VERSION}_amd64.kit"                \
    "lightfield-${BUILDTYPE}_${VERSION}_amd64.kit.sig"

blue-bar • Cleaning up

cd ..

rm ${VERBOSE} -rf ${REPO_DIR}

blue-bar ""
blue-bar "• Done!"
blue-bar ""
