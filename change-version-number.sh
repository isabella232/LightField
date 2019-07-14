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

if [ -z "$1" ]
then
    red-bar 'Usage: change-version-number.sh <newversion>'
    exit 1
fi

VER=( $(IFS="."; echo $1) )

if [ "${#VER[@]}" -lt 3 ]
then
    red-bar 'Too few components to version number -- must be at least three.'
    exit 1
elif [ "${#VER[@]}" -gt 3 ]
then
    red-bar 'Too many components to version number -- must be at most three.'
    exit 1
fi

LIGHTFIELD_ROOT="/home/lumen/Volumetric/LightField"
LIGHTFIELD_SRC="${LIGHTFIELD_ROOT}/src"

cd "${LIGHTFIELD_SRC}"

blue-bar 'Generating src/version.h from src/version.h.in'

sed                                                                                                                  \
    -e s/@@LIGHTFIELD_VERSION_STRING@@/${VER[0]}.${VER[1]}.${VER[2]}/g                                               \
    -e  s/@@LIGHTFIELD_VERSION_MAJOR@@/${VER[0]}/g                                                                   \
    -e  s/@@LIGHTFIELD_VERSION_MINOR@@/${VER[1]}/g                                                                   \
    -e  s/@@LIGHTFIELD_VERSION_TEENY@@/${VER[2]}/g                                                                   \
    < "version.h.in"                                                                                                 \
    > "version.h"

cd "${LIGHTFIELD_ROOT}"

blue-bar 'Updating build and packaging scripts'
perl                                                                                                                 \
    -lpi                                                                                                             \
    -e "s/^\s*VERSION=\\d+\\.\\d+\\.\\d+\s*$/VERSION=${VER[0]}.${VER[1]}.${VER[2]}/g;"                               \
    install-lightfield.sh                                                                                            \
    make-deb-package.sh                                                                                              \
    make-upgrade-kit.sh                                                                                              \
    unpack-kit-manually.sh

perl                                                                                                                 \
    -lpi                                                                                                             \
    -e "s/lightfield-(debug|release)_\\d+\\.\\d+\\.\\d+_amd64/lightfield-\$1_${VER[0]}.${VER[1]}.${VER[2]}_amd64/g;" \
    debian/lightfield-debug.postinst                                                                                 \
    debian/lightfield-release.postinst
