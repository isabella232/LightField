To install necessary library:
$ cd
$ git clone git://github.com/signal11/hidapi.git
$ sudo apt install libudev-dev libusb-1.0-0-dev autotools-dev autoconf automake libtool build-essential libhidapi-libusb0 pkg-config libusb-dev -y
$ cd hidapi
$ ./bootstrap
$ ./configure
$ make
$ sudo make install

To install udev ruleset:
$ sudo cp 90-dlpc350.rules /lib/udev/rules.d

Image orientation and flipping can be changed in the main.cpp file with something like:
# FLIP HORIZONTALLY
DLPC350_SetLongAxisImageFlip(true);
# FLIP VERTICALLY
DLPC350_SetShortAxisImageFlip(true);

then, in this directory:
$ g++ main.cpp dlpc350_usb.cpp dlpc350_api.cpp -l hidapi-libusb -o setpower

INSTALL into /usr/local/bin:
$ sudo cp setpower /usr/local/bin/
$ which setpower
/usr/local/bin/setpower

To use:
$ setpower 120 (or any other value)

