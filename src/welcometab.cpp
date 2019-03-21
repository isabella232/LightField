#include "pch.h"

#include "welcometab.h"

WelcomeTab::WelcomeTab( QWidget* parent ): QWidget( parent ) {
    _logoLabel->setPixmap( QPixmap( QString( ":images/dark-logo.png" ) ) );
    _versionLabel->setText( QString( "1.0.0" ) );

    _layout->addStretch( );
    _layout->addWidget( _logoLabel );
    _layout->addStretch( );
    _layout->addWidget( _versionLabel );
    _layout->addStretch( );

    setLayout( _layout );
}

WelcomeTab::~WelcomeTab( ) {
    /*empty*/
}

void WelcomeTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ WelcomeTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;
}

void WelcomeTab::setShepherd( Shepherd* newShepherd ) {
    _shepherd = newShepherd;
}
