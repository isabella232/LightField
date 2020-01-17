#!/bin/bash
# shellcheck disable=SC2103
# shellcheck disable=SC2164

LIGHTFIELD_ROOT=/home/lumen/Volumetric/LightField
PACKAGE_BUILD_ROOT=${LIGHTFIELD_ROOT}/packaging
USE_KEY_SET=current
BUILDTYPE=

#########################################################
##                                                     ##
##     No user-serviceable parts below this point.     ##
##                                                     ##
#########################################################

function usage () {
    cat <<EOF
Usage: $(basename "$0") [-q] BUILDTYPE
Where: -q         build quietly
       BUILDTYPE  is one of
                  release  create a release-build kit
                  debug    create a debug-build kit
                  both     create both kits

If the build is successful, the requested upgrade kit(s) will be found in
  ${KIT_DIR}/lightfield$([ "${DEFAULT_RELEASE_TRAIN}" = "base" ] || echo "-${DEFAULT_RELEASE_TRAIN}" )-BUILDTYPE_${VERSION}_${DEFAULT_ARCHITECTURE}.kit
EOF
    exit 1
}

# shellcheck disable=SC1090
source "${LIGHTFIELD_ROOT}/shared-stuff.sh"

VERBOSE=-v

ARGS=$(getopt -n 'make-upgrade-kit.sh' -o 'q' -- "$@")
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

if [ "${BUILDTYPE}" = "both" ]
then
    "${0}" "${ARGS}" release || exit $?
    "${0}" "${ARGS}" debug   || exit $?
    exit 0
fi

if [ "${USE_KEY_SET}" = "current" ]
then
    REPO_KEY_ID=E91BD3361F39D49C78B1E3A2B55C0E8D4B632A66
    PKG_KEY_ID=78DAD29978EB392992D7FE0423025033D9E840F7
#elif [ "${USE_KEY_SET}" = "future" ]
#then
# no future keys currently
else
    red-bar "The key set '${USE_KEY_SET}' is unrecognized."
    usage
fi

PACKAGE_BUILD_DIR="${PACKAGE_BUILD_ROOT}/${VERSION}-${SUFFIX}"
DEB_BUILD_DIR="${PACKAGE_BUILD_DIR}/deb"
KIT_DIR="${PACKAGE_BUILD_DIR}/kit"
REPO_DIR="${PACKAGE_BUILD_DIR}/repo"
RELEASEDATE=$(date "+%Y-%m-%d")

cd "${PACKAGE_BUILD_DIR}"

blue-bar "• Creating LightField ${VERSION} ${RELEASE_TRAIN} ${BUILDTYPE}-build update kit"

[ -d "${REPO_DIR}" ] && rm ${VERBOSE} -rf "${REPO_DIR}"

mkdir ${VERBOSE} -p "${REPO_DIR}"
mkdir ${VERBOSE} -p "${KIT_DIR}"

install ${VERBOSE} -Dt "${REPO_DIR}/" -m 644 "${LIGHTFIELD_ROOT}/fonts-montserrat_7.200_all.deb"
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

apt-ftparchive --config-file ${LIGHTFIELD_ROOT}/apt-files/release.conf release . | tee Release | xz -ceT0 > Release.xz

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
        "${LIGHTFIELD_ROOT}/apt-files/version.inf.in"

    # extract description from ${LIGHTFIELD_ROOT}/debian/changelog
    linecount=$(grep -n '^ -- LightField packager' ${LIGHTFIELD_ROOT}/debian/changelog | head -1 | cut -d: -f1 || echo 0)
    if [ -z "${linecount}" ] || [ "${linecount}" -lt 1 ]
    then
        red-bar "!!! Can't find end of first change in ${LIGHTFIELD_ROOT}/debian/changelog, aborting"
        exit 1
    fi
    head -$((linecount - 2)) ${LIGHTFIELD_ROOT}/debian/changelog | tail +3 | perl -lpe 's/\s+$//; s/^$/./; s/^/ /'

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
