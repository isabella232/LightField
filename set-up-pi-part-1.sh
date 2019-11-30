#!/bin/bash

[ "${USER}" != root ] && exec sudo "$0"

cd /etc
sed -i 's/\<pi\>/lumen/g; s/lumen-greeter/pi-greeter/g' \
    passwd shadow group gshadow subuid subgid lightdm/lightdm.conf polkit-1/localauthority.conf.d/60-desktop-policy.conf profile.d/sshpwd.sh sudoers.d/010_pi-nopasswd systemd/system/autologin@.service \
    systemd/system/getty@tty1.service.d/autologin.conf xdg/lxsession/LXDE-pi/sshpwd.sh
mv sudoers.d/010_pi-nopasswd sudoers.d/010_lumen-nopasswd

groupadd plugdev 1>/dev/null 2>&1
usermod -a -G dialout,plugdev,tty,uucp lumen

cd /home
mv pi lumen

#systemctl reboot
