#!/bin/bash

mkdir /var/{cache,lib}/lightfield
mkdir -p /usr/share/lightfield/libexec
ln -s /home/lumen/Volumetric/working /var/cache/lightfield/print-jobs
ln -s /home/lumen/Volumetric/model-library /var/lib/lightfield/model-library
ln -s /home/lumen/Volumetric/debug /var/log/lightfield
ln -s /home/lumen/Volumetric/printrun /usr/share/lightfield/libexec/printrun
ln -s /home/lumen/Volumetric/LightField/stdio-shepherd /usr/share/lightfield/libexec/stdio-shepherd
chown -hR lumen:lumen /var/{cache,lib,log}/lightfield
