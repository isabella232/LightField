#!/bin/bash

VERSION=1.0.11.0
PACKAGE_BUILD_ROOT=/home/lumen/Volumetric/LightField/packaging

#########################################################
##                                                     ##
##     No user-serviceable parts below this point.     ##
##                                                     ##
#########################################################

if [ "${UID}" != "0" ]
then
    echo This script must be run as root.
    exit 1
fi

UPDATES_DIR=/var/lib/lightfield/software-updates

rm    -v -dr "${UPDATES_DIR}"
mkdir -v -p  "${UPDATES_DIR}"

dir="lightfield-debug_${VERSION}_amd64"
updatedir="${UPDATES_DIR}/${dir}"
rm    -v -fr "${updatedir}"
mkdir -v     "${updatedir}"
tar   -vv -C "${updatedir}" -xf "${PACKAGE_BUILD_ROOT}/${VERSION}/kit/${dir}.kit"

chown -c -R  lumen:lumen "${UPDATES_DIR}"

echo "deb file:/var/lib/lightfield/software-updates/lightfield-debug_${VERSION}_amd64 ./" > /etc/apt/sources.list.d/volumetric-lightfield.list
chown -c lumen:lumen /etc/apt/sources.list.d/volumetric-lightfield.list
