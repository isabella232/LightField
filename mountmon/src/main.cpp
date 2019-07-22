#include "pch.h"

#include "commandreader.h"
#include "udisksmonitor.h"
#include "usbdevicemounter.h"

int main( int argc, char** argv ) {
    setvbuf( stdout, nullptr, _IONBF, 0 );

    auto             app              { QCoreApplication { argc, argv } };
    CommandReader    commandReader    { &app                            };
    UDisksMonitor    udisksMonitor    { &app                            };
    UsbDeviceMounter usbDeviceMounter { udisksMonitor, &app             };

    (void) QObject::connect( &commandReader, &CommandReader::commandReceived, &usbDeviceMounter, &UsbDeviceMounter::commandReader_commandReceived );

    app.exec( );
}
