#include "pch.h"

#include "tabbase.h"

#include "printjob.h"
#include "printmanager.h"
#include "printprofilemanager.h"
#include "shepherd.h"
#include "usbmountmanager.h"

TabBase::TabBase( QWidget* parent ): QWidget( parent ) {
    /*empty*/
}

TabBase::~TabBase( ) {
    /*empty*/
}

void TabBase::_disconnectPrintJob( ) {
    /*empty*/
}

void TabBase::_connectPrintJob( ) {
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

void TabBase::_disconnectShepherd( ) {
    QObject::disconnect( _shepherd, nullptr, this, nullptr );
}

void TabBase::_connectShepherd( ) {
    /*empty*/
}

void TabBase::_disconnectUsbMountManager( ) {
    QObject::disconnect( _usbMountManager, nullptr, this, nullptr );
}

void TabBase::_connectUsbMountManager( ) {
    /*empty*/
}

void TabBase::setPrintJob( PrintJob* printJob ) {
    _disconnectPrintJob( );
    _printJob = printJob;
    _connectPrintJob( );
}

void TabBase::setPrintManager( PrintManager* printManager ) {
    _disconnectPrintManager( );
    _printManager = printManager;
    _connectPrintManager( );
}

void TabBase::setPrintProfileManager( PrintProfileManager* printProfileManager ) {
    _disconnectPrintProfileManager( );
    _printProfileManager = printProfileManager;
    _connectPrintProfileManager( );
}

void TabBase::setShepherd( Shepherd* shepherd ) {
    _disconnectShepherd( );
    _shepherd = shepherd;
    _connectShepherd( );
}

void TabBase::setManifestMgr( OrderManifestManager* manifestMgr ) {
    _manifestManager = manifestMgr;
}

void TabBase::setUsbMountManager( UsbMountManager* usbMountManager ) {
    _disconnectUsbMountManager( );
    _usbMountManager = usbMountManager;
    _connectUsbMountManager( );
}
