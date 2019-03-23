#include "pch.h"

#include "maintenancetab.h"

#include "app.h"
#include "utils.h"

MaintenanceTab::MaintenanceTab( QWidget* parent ): TabBase( parent ) {
    _logoLabel->setAlignment( Qt::AlignCenter );
    if ( g_settings.theme == Theme::Dark ) {
        _logoLabel->setPixmap( QPixmap( QString( ":images/dark-logo.png" ) ) );
    } else {
        _logoLabel->setPixmap( QPixmap( QString( ":images/light-logo.png" ) ) );
    }

    _versionLabel->setAlignment( Qt::AlignCenter );
    _versionLabel->setFont( ModifyFont( font( ), 16.0 ) );
    _versionLabel->setText( QString { "Version " } + QCoreApplication::applicationVersion( ) );

    _restartButton->setText( "Restart" );
    QObject::connect( _restartButton, &QPushButton::clicked, this, &MaintenanceTab::restartButton_clicked );

    _shutDownButton->setText( "Shut down" );
    QObject::connect( _shutDownButton, &QPushButton::clicked, this, &MaintenanceTab::shutDownButton_clicked );

    _layout = WrapWidgetsInVBox( { _logoLabel, _versionLabel, nullptr } );
    _layout->addLayout( WrapWidgetsInHBox( { nullptr, _restartButton, nullptr, _shutDownButton, nullptr } ) );
    setLayout( _layout );
}

MaintenanceTab::~MaintenanceTab( ) {
    /*empty*/
}

void MaintenanceTab::restartButton_clicked( bool ) {
    system( "sudo shutdown -r now" );
}

void MaintenanceTab::shutDownButton_clicked( bool ) {
    system( "sudo shutdown -h now" );
}
