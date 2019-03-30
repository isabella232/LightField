#include "pch.h"

#include "tabbase.h"

#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"

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

void TabBase::_disconnectShepherd( ) {
    QObject::disconnect( _shepherd, nullptr, this, nullptr );
}

void TabBase::_connectShepherd( ) {
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

void TabBase::setShepherd( Shepherd* shepherd ) {
    _disconnectShepherd( );
    _shepherd = shepherd;
    _connectShepherd( );
}
