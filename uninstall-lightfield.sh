#!/bin/bash

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

# Undo systemd configuration changes
systemctl set-default graphical.target
systemctl disable --now set-projector-power.service clean-up-mount-points.service
systemctl disable getty@tty1.service

# Remove these files
rm -f                                                         \
    /etc/apt/sources.list.d/volumetric-lightfield.list        \
    /etc/apt/trusted.gpg.d/volumetric-keyring.gpg             \
    /etc/sudoers.d/lumen-lightfield                           \
    /etc/systemd/system/getty.target.wants/getty@tty1.service \
    /home/lumen/.bash_profile                                 \
    /home/lumen/.real_bash_profile                            \
    /home/lumen/.gnupg/pubring.gpg                            \
    /home/lumen/.gnupg/trustdb.gpg                            \
    /lib/systemd/system/clean-up-mount-points.service         \
    /lib/systemd/system/set-projector-power.service           \
    /lib/udev/rules.d/90-dlpc350.rules                        \
    /usr/bin/lf                                               \
    /usr/bin/mountmon                                         \
    /usr/bin/set-projector-power                              \
    /usr/share/X11/xorg.conf.d/99-waveshare.conf

# Remove these directories and everything in them
rm -rf                                                        \
    /etc/systemd/system/getty@tty1.service.d/                 \
    /usr/lib/lightfield/                                      \
    /usr/share/lightfield/                                    \
    /var/cache/lightfield/                                    \
    /var/lib/lightfield/                                      \
    /var/log/lightfield/

# Remove these directories, but only if they're empty
rmdir                                                         \
    /etc/systemd/system/getty.target.wants                    \
    /home/lumen/.gnupg

systemctl daemon-reload
