#!/bin/bash
# jmil hat-tip thanks to: https://askubuntu.com/questions/1070057/trust-desktop-icons-without-clicking-them-manually-in-ubuntu-18-04-gnome-3
# Trust all desktop files
for i in ~/Volumetric/TouchPrint/login-user-stuff/*.desktop; do
  [ -f "${i}" ] || break
  gio set "${i}" "metadata::trusted" yes
done

