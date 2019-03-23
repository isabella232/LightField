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
    _logoLabel->setPixmap( QPixmap { QString { ":images/transparent-dark-logo.png" } } );


    _versionLabel->setAlignment( Qt::AlignCenter );
    _versionLabel->setFont( font16pt );
    _versionLabel->setText( QString { "Version " } + QCoreApplication::applicationVersion( ) );


    _restartButton->setFont( font16pt );
    _restartButton->setText( "Restart" );
    QObject::connect( _restartButton, &QPushButton::clicked, this, &MaintenanceTab::restartButton_clicked );

    _shutDownButton->setFont( font16pt );
    _shutDownButton->setText( "Shut down" );
    QObject::connect( _shutDownButton, &QPushButton::clicked, this, &MaintenanceTab::shutDownButton_clicked );

    auto mainButtonsLayout = WrapWidgetsInHBox( { nullptr, _restartButton, nullptr, _shutDownButton, nullptr } );
    mainButtonsLayout->setContentsMargins( { } );


    _mainLayout = WrapWidgetsInVBox( { _logoLabel, _versionLabel, nullptr } );
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
