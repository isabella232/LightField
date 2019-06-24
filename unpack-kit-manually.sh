#!/bin/bash

if [ "${UID}" != "0" ]
then
    echo This script must be run as root.
    exit 1
fi

UPDATES_DIR=/var/lib/lightfield/software-updates

rm    -v -dr "${UPDATES_DIR}"
mkdir -v -p  "${UPDATES_DIR}"

for ver in 1.0.1
do
	dir="lightfield-debug_${ver}_amd64"
	updatedir="${UPDATES_DIR}/${dir}"
	rm    -frv     "${updatedir}"
	mkdir -v       "${updatedir}"
	tar   -v -v -C "${updatedir}" -xf ~/"${dir}.kit"
done

chown -c -R  lumen:lumen "${UPDATES_DIR}"

echo "deb file:/var/lib/lightfield/software-updates/lightfield-debug_1.0.1_amd64 ./" > /etc/apt/sources.list.d/volumetric-lightfield.list
chown -c lumen:lumen /etc/apt/sources.list.d/volumetric-lightfield.list

cp    -v ~/volumetric-keyring.gpg /etc/apt/trusted.gpg.d/
