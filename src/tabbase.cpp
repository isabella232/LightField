#include "pch.h"

#include "tabbase.h"

#include "printjob.h"
#include "printmanager.h"
#include "printprofilemanager.h"
#include "firmwarecontroller.h"
#include "usbmountmanager.h"

TabBase::TabBase(QWidget* parent): QWidget( parent ) {
    /*empty*/
}

TabBase::~TabBase( ) {
    /*empty*/
}

void TabBase::_disconnectPrintManager( ) {
    QObject::disconnect( _printManager, nullptr, this, nullptr );
}

void TabBase::_connectPrintManager( ) {
    /*empty*/
}

void TabBase::_disconnectPrintProfileManager( ) {
    /*empty*/
}

void TabBase::_connectPrintProfileManager( ) {
    /*empty*/
}

void TabBase::_disconnectFirmwareController( ) {
    QObject::disconnect( _firmwareController, nullptr, this, nullptr );
}

void TabBase::_connectFirmwareController( ) {
    /*empty*/
}

void TabBase::_disconnectUsbMountManager( ) {
    QObject::disconnect( _usbMountManager, nullptr, this, nullptr );
}

void TabBase::_connectUsbMountManager( ) {
    /*empty*/
}

void TabBase::setPrintManager( PrintManager* printManager ) {
    _disconnectPrintManager( );
    _printManager = printManager;
    _connectPrintManager( );
}

void TabBase::setPrintProfileManager(PrintProfileManager* printProfileManager)
{
    _disconnectPrintProfileManager( );
    _printProfileManager = printProfileManager;
    _connectPrintProfileManager( );
}

void TabBase::setFirmwareController(FirmwareController *controller) {
    _firmwareController = controller;
}

void TabBase::setUsbMountManager( UsbMountManager* usbMountManager ) {
    _disconnectUsbMountManager( );
    _usbMountManager = usbMountManager;
    _connectUsbMountManager( );
}
