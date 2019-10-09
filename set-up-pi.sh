#!/bin/bash

cd /etc
sudo sed -i -e 's/\<pi\>/lumen/g' -e 's/lumen-greeter/pi-greeter/g' lightdm/lightdm.conf
sudo sed -i -e 's/\<pi\>/lumen/g' passwd shadow group gshadow subuid subgid polkit-1/localauthority.conf.d/60-desktop-policy.conf profile.d/sshpwd.sh sudoers.d/010_pi-nopasswd systemd/system/autologin@.service systemd/system/getty@tty1.service.d/autologin.conf xdg/lxsession/LXDE-pi/sshpwd.sh
sudo mv sudoers.d/010_pi-nopasswd sudoers.d/010_lumen-nopasswd

cd /home
sudo mv pi lumen

sudo apt install deborphan
sudo apt purge --autoremove --purge $(deborphan --guess-all)
