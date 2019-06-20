#!/bin/bash


stty -F /dev/ttyACM0 speed 1200
stty -F /dev/ttyACM0 speed 115200
avrdude -p atmega2560 -c wiring -P /dev/ttyACM0 -b 250000 -U flash:w:Marlin.Volumetric.hex:i -D

