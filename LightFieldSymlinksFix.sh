#!/bin/bash

mkdir  -v /var/{cache,lib}/lightfield
mkdir -pv /usr/share/lightfield/libexec
ln -sv /home/lumen/Volumetric/working /var/cache/lightfield/print-jobs
ln -sv /home/lumen/Volumetric/model-library /var/lib/lightfield/model-library
ln -sv /home/lumen/Volumetric/debug /var/log/lightfield
ln -sv /home/lumen/Volumetric/printrun /usr/share/lightfield/libexec/printrun
ln -sv /home/lumen/Volumetric/LightField/stdio-shepherd /usr/share/lightfield/libexec/stdio-shepherd
chown -chR lumen:lumen /var/{cache,lib,log}/lightfield
