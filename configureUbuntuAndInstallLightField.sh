sudo apt-get update
sudo apt install net-tools openssh-server -y
### AT THIS POINT YOU CAN RUN ifconfig TO FIND THE IP ADDRESS, SSH IN, BRING OVER THIS INSTALLATION SCRIPT, chmod +x it, AND FINALLY sudo RUN IT.
sudo usermod -a -G tty lumen
sudo usermod -a -G tty lumen
sudo usermod -a -G dialout lumen
sudo usermod -a -G uucp lumen
sudo groupadd plugdev
sudo usermod -a -G plugdev lumen


sudo apt dist-upgrade -y




mkdir -p ~/Volumetric
mkdir -p ~/Downloads
cd ~/Volumetric


sudo apt install git -y


git clone https://github.com/VolumetricBio/hidapi.git
sudo apt install libudev-dev libusb-1.0-0-dev autotools-dev autoconf automake libtool build-essential libhidapi-libusb0 pkg-config libusb-dev -y


cd hidapi
./bootstrap
./configure
make
sudo make install




## INSTALL FONT-AWESOME for the proper Unicode chars for the LightField software
sudo apt install fonts-font-awesome -y



## INSTALL pastebinit for posting log files easily
sudo apt install pastebinit -y



cd ~/Volumetric

git clone --recursive https://github.com/VolumetricBio/LightField.git




cd ~/Volumetric/LightField/login-user-stuff/
cp .bash_profile ~/.bash_profile
cp .bashrc ~/.bashrc
cp .profile ~/.profile




sudo apt install qt5-default libqt5opengl5-dev qtchooser -y




sudo apt install python3-venv -y
cd ~/Volumetric
python3 -m venv envTouchPrint
. ./envTouchPrint/bin/activate


cd ~/Volumetric
rm -rf printrun
git clone https://github.com/VolumetricBio/printrun.git
cd printrun


python -m pip install Cython
sudo apt install python3-serial python3-numpy cython3 python3-libxml2 python3-gi python3-dbus python3-psutil python3-cairosvg libpython3-dev python3-appdirs python3-wxgtk4.0 -y
sudo apt install python3-pip -y
pip3 install --user pyglet


pip install pyglet


pip3 install appdirs
pip3 install psutil


pip install --upgrade pip appdirs Cython pillow pyserial pygame


sudo apt install git virtualenv build-essential python3-dev libdbus-glib-1-dev libgirepository1.0-dev -y


pip install dbus-python


python setup.py build_ext --inplace






### Install Slic3r
sudo apt install slic3r -y




sudo apt install samba -y


sudo apt install feh graphicsmagick scrot -y


sudo apt install libatlas-base-dev librsvg2-bin imagemagick inkscape netatalk -y
pip3 install numpy-stl




mkdir -p ~/Downloads
cd ~/Downloads
wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb
sudo dpkg -i google-chrome-stable_current_amd64.deb




sudo apt install simplescreenrecorder -y
# use with: 



# INSTALL LIGHTFIELD!
cd ~/Volumetric/LightField
chmod +x ./install-lightfield.sh
./install-lightfield.sh


# FLASH THE FIRMWARE ONTO THE ARDUINO
sudo apt install avrdude -y
cd ~/Volumetric/LightField/system-stuff/firmware
chmod +x *.sh
./firmware-flash-during-install.sh

# COPY STARTUP FILES FOR WHEN RUNNING GNOME
sudo apt install arandr -y
cd ~/Volumetric/LightField/login-user-stuff/
mkdir -p ~/.config/autostart
chmod +x trustDotDesktopFiles.sh
./trustDotDesktopFiles.sh
cp lumenProjectorPower.desktop ~/.config/autostart/
cp lumenLightField.desktop ~/.config/autostart/
cp lumenTouchscreenXinput.desktop ~/.config/autostart/
cp lumenLockPrimaryDisplay.desktop ~/.config/autostart/
cp burn-in.desktop ~/Desktop/
cp burn-in-noHeat.desktop ~/Desktop/
cp burn-in-noFan.desktop ~/Desktop/
cp burn-in-noMotor.desktop ~/Desktop/

# REMOVE MODEM MANAGER SO WE CAN CONNECT PROPERLY TO THE EINSY MOTHERBOARD
sudo apt remove modemmanager -y

# SET PERMISSIONS FOR ALL USER FILES
sudo chown -R lumen:lumen ~/

# ALSO PROPERLY SET THE DOT FILES
sudo chown -R lumen:lumen ~/.*


sudo apt autoremove -y

# clean all caches
sudo apt clean
sudo rm -rf ~/.cache/


## Delete Network Passwords
cd /etc/NetworkManager/system-connections
sudo rm -rf *


## Delete Shell History
# https://askubuntu.com/questions/191999/how-to-clear-bash-history-completely
sudo chown lumen:lumen ~/.bash_history
cat /dev/null > ~/.bash_history && history -c && exit