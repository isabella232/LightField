#include "pch.h"

#include "udisksmonitor.h"
#include "usbdevicemounter.h"

int main( int argc, char** argv ) {
    setvbuf( stdout, nullptr, _IONBF, 0 );

    auto             app              { QCoreApplication { argc, argv } };
    UDisksMonitor    udisksMonitor    { &app                            };
    UsbDeviceMounter usbDeviceMounter { udisksMonitor, &app             };

    app.exec( );
}
