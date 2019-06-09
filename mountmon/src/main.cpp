#include "pch.h"

#include "udisksmonitor.h"
#include "usbdevicemounter.h"

int main( int argc, char** argv ) {
    setvbuf( stdout, nullptr, _IONBF, 0 );

    auto app              { QCoreApplication { argc, argv }                                                   };
    auto udisksMonitor    { std::shared_ptr<UDisksMonitor>   ( new UDisksMonitor )                            };
    auto usbDeviceMounter { std::shared_ptr<UsbDeviceMounter>( new UsbDeviceMounter( udisksMonitor.get( ) ) ) };

    app.exec( );
}
