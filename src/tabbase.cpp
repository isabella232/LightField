#include "pch.h"

#include "tabbase.h"

#include "printjob.h"
#include "shepherd.h"

TabBase::TabBase( QWidget* parent ): QWidget( parent ) {
    /*empty*/
}

TabBase::~TabBase( ) {
    /*empty*/
}

void TabBase::setPrintJob( PrintJob* printJob ) {
    debug( "+ StatusTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;
}

void TabBase::setShepherd( Shepherd* newShepherd ) {
    if ( _shepherd ) {
        QObject::disconnect( _shepherd, nullptr, this, nullptr );
    }

    _shepherd = newShepherd;
}

void TabBase::_disconnectShepherd( ) {
    QObject::disconnect( _shepherd, nullptr, this, nullptr );
}

void TabBase::_connectShepherd( ) {
    /*empty*/
}
