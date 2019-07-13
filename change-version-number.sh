#!/bin/bash

if [ -z "$1" ]
then
    echo 'Usage: change-version-number.sh <newversion>'
    exit 1
fi

VER=( $(IFS="."; echo $1) )

if [ "${#VER[@]}" -lt 3 ]
then
    echo 'Too few components to version number -- must be at least three.'
    exit 1
elif [ "${#VER[@]}" -eq 3 ]
then
    VER[3]=0
elif [ "${#VER[@]}" -gt 4 ]
then
    echo 'Too many components to version number -- must be at most four.'
    exit 1
fi

LIGHTFIELD_ROOT="/home/lumen/Volumetric/LightField"
LIGHTFIELD_SRC="${LIGHTFIELD_ROOT}/src"

cd "${LIGHTFIELD_SRC}"

sed                                                                              \
    -e s/@@LIGHTFIELD_VERSION_STRING@@/${VER[0]}.${VER[1]}.${VER[2]}.${VER[3]}/g \
    -e  s/@@LIGHTFIELD_VERSION_MAJOR@@/${VER[0]}/g                               \
    -e  s/@@LIGHTFIELD_VERSION_MINOR@@/${VER[1]}/g                               \
    -e  s/@@LIGHTFIELD_VERSION_TEENY@@/${VER[2]}/g                               \
    -e  s/@@LIGHTFIELD_VERSION_BUILD@@/${VER[3]}/g                               \
    < "version.h.in"                                                             \
    > "version.h"

cd "${LIGHTFIELD_ROOT}"

perl                                                                                                       \
    -lpi                                                                                                   \
    -e "s/^\s*VERSION=\\d+\\.\\d+\\.\\d+(\\.\\d+)?\s*$/VERSION=${VER[0]}.${VER[1]}.${VER[2]}.${VER[3]}/g;" \
    install-lightfield.sh                                                                                  \
    make-deb-package.sh                                                                                    \
    make-upgrade-kit.sh                                                                                    \
    unpack-kit-manually.sh
