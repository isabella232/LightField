#!/bin/bash

mkdir -pv /usr/lib/lightfield /var/{cache,lib}/lightfield

ln -sv /home/lumen/Volumetric/printrun                  /usr/lib/lightfield/printrun
ln -sv /home/lumen/Volumetric/LightField/stdio-shepherd /usr/lib/lightfield/stdio-shepherd
ln -sv /home/lumen/Volumetric/working                   /var/cache/lightfield/print-jobs
ln -sv /home/lumen/Volumetric/model-library             /var/lib/lightfield/model-library
ln -sv /home/lumen/Volumetric/debug                     /var/log/lightfield

chown -chR lumen:lumen /var/{cache,lib,log}/lightfield
