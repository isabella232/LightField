#!/bin/bash


stty -F /dev/lumen-arduino speed 1200
stty -F /dev/lumen-arduino speed 115200
avrdude -p atmega2560 -c wiring -P /dev/lumen-arduino -b 250000 -U flash:w:Marlin.Volumetric.hex:i -D

