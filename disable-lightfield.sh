#!/bin/bash

sudo systemctl stop getty@tty1
sudo systemctl disable getty@tty1
sudo systemctl set-default graphical.target
sudo rm -rf /etc/systemd/system/getty@tty1*
sudo systemctl daemon-reload
sudo rm /usr/share/X11/xorg.conf.d/99-waveshare.conf
sudo systemctl reboot
