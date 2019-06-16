#!/bin/bash

/usr/bin/sudo /bin/rm -rd                           \
	/etc/sudoers.d/lumen-lightfield                 \
	/etc/systemd/system/getty@tty1*                 \
	/home/lumen/.bash_profile                       \
	/lib/systemd/system/set-projector-power.service \
	/lib/udev/rules.d/90-dlpc350.rules              \
	/usr/bin/lf                                     \
	/usr/bin/mountmon                               \
	/usr/bin/set-projector-power                    \
	/usr/share/lightfield/                          \
	/usr/share/X11/xorg.conf.d/99-waveshare.conf    \
	/var/cache/lightfield/                          \
	/var/lib/lightfield/                            \
	/var/log/lightfield/
