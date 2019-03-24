#include "pch.h"

#include "maintenancetab.h"

#include "app.h"
#include "strings.h"
#include "utils.h"

MaintenanceTab::MaintenanceTab( QWidget* parent ): TabBase( parent ) {
    _initialShowEventFunc = std::bind( &MaintenanceTab::_initialShowEvent, this, _1 );

    auto origFont = font( );
    auto font16pt = ModifyFont( origFont, 16.0 );
    auto font22pt = ModifyFont( origFont, 22.0 );

    //
    // Main content
    //

    _logoLabel->setAlignment( Qt::AlignCenter );
    _logoLabel->setContentsMargins( { } );
    _logoLabel->setPixmap( QPixmap { QString { ":images/transparent-dark-logo.png" } } );
    _logoLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );

    _versionLabel->setAlignment( Qt::AlignCenter );
    _versionLabel->setContentsMargins( { } );
    _versionLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    _versionLabel->setTextFormat( Qt::RichText );
    _versionLabel->setText(
        QString {
            "<span style='font-size: 22pt;'>%1</span><br>"
            "<span style='font-size: 16pt;'>Version %2</span><br>"
            "<span>© 2019 Volumetric, Inc.</span>"
        }
        .arg( QCoreApplication::applicationName( ) )
        .arg( QCoreApplication::applicationVersion( ) )
    );

    auto versionInfoLayout = WrapWidgetsInHBox( { _logoLabel, _versionLabel } );
    versionInfoLayout->setContentsMargins( { } );


    _copyrightsLabel->setAlignment( Qt::AlignCenter );
    _copyrightsLabel->setTextFormat( Qt::RichText );
    _copyrightsLabel->setText( QString {
        "Based on <a href='https://github.com/mkeeter/fstl' style='color: white;'>fstl</a> by <a href='https://www.mattkeeter.com/' style='color: white;'>Matthew Keeter</a>.<br>"
        "© 2014-2018 Matthew Keeter. Licensed under the terms of the <a href='https://opensource.org/licenses/MIT' style='color: white;'>MIT license</a>.<br>"
        "Includes code derived from <a href='https://github.com/kliment/Printrun' style='color: white;'>Printrun</a> by <a href='https://0xfb.com/' style='color: white;'>Kliment Yanev</a>.<br>"
        "© 2011-2019 Kliment Yanev. Licensed under the terms of the <a href='https://opensource.org/licenses/GPL-3.0' style='color: white;'>GNU General Purpose License v3</a>.<br>"
        "<a href='https://github.com/Alexhuszagh/BreezeStyleSheets' style='color: white;'>BreezeStyleSheets</a> by <a href='https://github.com/Alexhuszagh/' style='color: white;'>Alex Huszagh</a>, based on <a href='https://github.com/ColinDuquesnoy/QDarkStyleSheet' style='color: white;'>QDarkStyleSheet</a> by <a href='https://github.com/ColinDuquesnoy' style='color: white;'>Colin Duquesnoy</a>.<br>"
        "© 2013-2018 Colin Duquesnoy. © 2015-2018 Alex Huszagh. Licensed under the terms of the <a href='https://opensource.org/licenses/MIT' style='color: white;'>MIT license</a>.<br>"
        "<a href='https://github.com/JulietaUla/Montserrat' style='color: white;'>Montserrat</a> typeface by <a href='https://github.com/JulietaUla' style='color: white;'>Julieta Ulanovsky</a>.<br>"
        "© 2011 The Montserrat Project Authors. Licensed under the terms of the <a href='https://github.com/JulietaUla/Montserrat/raw/master/OFL.txt' style='color: white;'>SIL Open Font License</a>."
    } );


    _restartButton->setFont( font16pt );
    _restartButton->setText( "Restart" );
    QObject::connect( _restartButton, &QPushButton::clicked, this, &MaintenanceTab::restartButton_clicked );

    _shutDownButton->setFont( font16pt );
    _shutDownButton->setText( "Shut down" );
    QObject::connect( _shutDownButton, &QPushButton::clicked, this, &MaintenanceTab::shutDownButton_clicked );

    auto mainButtonsLayout = WrapWidgetsInHBox( { nullptr, _restartButton, nullptr, _shutDownButton, nullptr } );
    mainButtonsLayout->setContentsMargins( { } );


    _mainLayout->addLayout( versionInfoLayout );
    _mainLayout->addWidget( _copyrightsLabel );
    _mainLayout->addStretch( );
    _mainLayout->addLayout( mainButtonsLayout );
    _mainLayout->setAlignment( Qt::AlignCenter );
    _mainLayout->setContentsMargins( { } );


    _mainContent->setContentsMargins( { } );
    _mainContent->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _mainContent->setVisible( true );
    _mainContent->setLayout( _mainLayout );

    //
    // Confirm restart content
    //

    _confirmRestartLabel->setAlignment( Qt::AlignCenter );
    _confirmRestartLabel->setFont( font22pt );
    _confirmRestartLabel->setText( "Are you sure you want to restart?" );


    _confirmRestartYesButton->setFont( font16pt );
    _confirmRestartYesButton->setText( "Yes" );
    QObject::connect( _confirmRestartYesButton, &QPushButton::clicked, this, &MaintenanceTab::confirmRestartYesButton_clicked );

    _confirmRestartNoButton->setFont( font16pt );
    _confirmRestartNoButton->setText( "No" );
    QObject::connect( _confirmRestartNoButton, &QPushButton::clicked, this, &MaintenanceTab::confirmRestartNoButton_clicked );

    auto confirmRestartButtonsLayout = WrapWidgetsInHBox( { nullptr, _confirmRestartYesButton, nullptr, _confirmRestartNoButton, nullptr } );
    confirmRestartButtonsLayout->setContentsMargins( { } );


    _confirmRestartLayout = WrapWidgetsInVBox( { nullptr, _confirmRestartLabel, nullptr } );
    _confirmRestartLayout->addLayout( confirmRestartButtonsLayout );
    _confirmRestartLayout->addStretch( );
    _confirmRestartLayout->setAlignment( Qt::AlignCenter );
    _confirmRestartLayout->setContentsMargins( { } );


    _confirmRestartContent->setContentsMargins( { } );
    _confirmRestartContent->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _confirmRestartContent->setVisible( false );
    _confirmRestartContent->setLayout( _confirmRestartLayout );

    //
    // Confirm shutdown content
    //

    _confirmShutdownLabel->setAlignment( Qt::AlignCenter );
    _confirmShutdownLabel->setFont( font22pt );
    _confirmShutdownLabel->setText( "Are you sure you want to shut down?" );


    _confirmShutdownYesButton->setFont( font16pt );
    _confirmShutdownYesButton->setText( "Yes" );
    QObject::connect( _confirmShutdownYesButton, &QPushButton::clicked, this, &MaintenanceTab::confirmShutdownYesButton_clicked );

    _confirmShutdownNoButton->setFont( font16pt );
    _confirmShutdownNoButton->setText( "No" );
    QObject::connect( _confirmShutdownNoButton, &QPushButton::clicked, this, &MaintenanceTab::confirmShutdownNoButton_clicked );

    auto confirmShutdownButtonsLayout = WrapWidgetsInHBox( { nullptr, _confirmShutdownYesButton, nullptr, _confirmShutdownNoButton, nullptr } );
    confirmShutdownButtonsLayout->setContentsMargins( { } );


    _confirmShutdownLayout = WrapWidgetsInVBox( { nullptr, _confirmShutdownLabel, nullptr } );
    _confirmShutdownLayout->addLayout( confirmShutdownButtonsLayout );
    _confirmShutdownLayout->addStretch( );
    _confirmShutdownLayout->setAlignment( Qt::AlignCenter );
    _confirmShutdownLayout->setContentsMargins( { } );


    _confirmShutdownContent->setContentsMargins( { } );
    _confirmShutdownContent->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _confirmShutdownContent->setVisible( false );
    _confirmShutdownContent->setLayout( _confirmShutdownLayout );

    //
    // Top level
    //

    _layout = WrapWidgetsInVBox( { _mainContent, _confirmRestartContent, _confirmShutdownContent } );
    setLayout( _layout );
}

MaintenanceTab::~MaintenanceTab( ) {
    /*empty*/
}

void MaintenanceTab::_initialShowEvent( QShowEvent* event ) {
    QSize newSize = _shutDownButton->size( );
    newSize.setWidth( newSize.width( ) + 20 );

    _restartButton           ->setFixedSize( newSize );
    _shutDownButton          ->setFixedSize( newSize );
    _confirmRestartYesButton ->setFixedSize( newSize );
    _confirmRestartNoButton  ->setFixedSize( newSize );
    _confirmShutdownYesButton->setFixedSize( newSize );
    _confirmShutdownNoButton ->setFixedSize( newSize );

    event->accept( );
}

void MaintenanceTab::restartButton_clicked( bool ) {
    _mainContent->setVisible( false );
    _confirmRestartContent->setVisible( true );
}

void MaintenanceTab::shutDownButton_clicked( bool ) {
    _mainContent->setVisible( false );
    _confirmShutdownContent->setVisible( true );
}

void MaintenanceTab::confirmRestartYesButton_clicked( bool ) {
    _confirmRestartContent->setVisible( false );
    _mainContent->setVisible( true );

    system( "sudo shutdown -r now" );
}

void MaintenanceTab::confirmRestartNoButton_clicked( bool ) {
    _confirmRestartContent->setVisible( false );
    _mainContent->setVisible( true );
}

void MaintenanceTab::confirmShutdownYesButton_clicked( bool ) {
    _confirmShutdownContent->setVisible( false );
    _mainContent->setVisible( true );

    system( "sudo shutdown -h now" );
}

void MaintenanceTab::confirmShutdownNoButton_clicked( bool ) {
    _confirmShutdownContent->setVisible( false );
    _mainContent->setVisible( true );
}
