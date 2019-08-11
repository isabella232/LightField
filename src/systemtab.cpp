#include "pch.h"

#include "systemtab.h"

#include "debuglogcopier.h"
#include "shepherd.h"
#include "upgrademanager.h"
#include "upgradeselector.h"
#include "window.h"

namespace {

    QString VersionMessage {
        "<span style='font-size: 22pt;'>%1</span><br>"
        "<span style='font-size: 16pt;'>Version %2</span><br>"
        "<span style='font-size: 12pt;'>Firmware version %3</span><br>"
        "<span>© 2019 %4</span>"
    };

}

SystemTab::SystemTab( UsbMountManager* manager, QWidget* parent ): InitialShowEventMixin<SystemTab, TabBase>( parent ) {
    _usbMountManager = manager;

    auto origFont = font( );
    auto font16pt = ModifyFont( origFont, 18.0 );
    auto font22pt = ModifyFont( origFont, LargeFontSize );

    //
    // Main content
    //

    _logoLabel->setAlignment( Qt::AlignCenter );
    _logoLabel->setContentsMargins( { } );
    _logoLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    _logoLabel->setPixmap( QPixmap { QString { ":images/transparent-dark-logo.png" } } );

    _versionLabel->setAlignment( Qt::AlignCenter );
    _versionLabel->setContentsMargins( { } );
    _versionLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    _versionLabel->setTextFormat( Qt::RichText );
    _versionLabel->setText(
        VersionMessage
        .arg( QCoreApplication::applicationName( )    )
        .arg( QCoreApplication::applicationVersion( ) )
        .arg( "(unknown)" )
        .arg( QCoreApplication::organizationName( )   )
    );

    auto versionInfoLayout = WrapWidgetsInHBox( { nullptr, _logoLabel, nullptr, _versionLabel, nullptr } );
    versionInfoLayout->setContentsMargins( { } );


    _copyrightsLabel->setAlignment( Qt::AlignCenter );
    _copyrightsLabel->setTextFormat( Qt::RichText );
    _copyrightsLabel->setText( ReadWholeFile( ":text/copyright-message.txt" ) );


    _copyLogsButton->setEnabled( false );
    _copyLogsButton->setFont( font16pt );
    _copyLogsButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _copyLogsButton->setText( "Copy logs to USB" );
    QObject::connect( _copyLogsButton, &QPushButton::clicked, this, &SystemTab::copyLogsButton_clicked );

    auto copyLogsButtonLayout = WrapWidgetsInHBox( { nullptr, _copyLogsButton, nullptr } );
    copyLogsButtonLayout->setContentsMargins( { } );


    _updateSoftwareButton->setEnabled( false );
    _updateSoftwareButton->setFont( font16pt );
    _updateSoftwareButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _updateSoftwareButton->setText( "Update software" );
    QObject::connect( _updateSoftwareButton, &QPushButton::clicked, this, &SystemTab::updateSoftwareButton_clicked );

    _updateFirmwareButton->setEnabled( false );
    _updateFirmwareButton->setFont( font16pt );
    _updateFirmwareButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _updateFirmwareButton->setText( "Update firmware" );
    QObject::connect( _updateFirmwareButton, &QPushButton::clicked, this, &SystemTab::updateFirmwareButton_clicked );

    auto updateButtonsLayout = WrapWidgetsInHBox( { nullptr, _updateSoftwareButton, nullptr, _updateFirmwareButton, nullptr } );
    updateButtonsLayout->setContentsMargins( { } );


    _restartButton->setFont( font16pt );
    _restartButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _restartButton->setText( "Restart" );
    QObject::connect( _restartButton, &QPushButton::clicked, this, &SystemTab::restartButton_clicked );

    _shutDownButton->setFont( font16pt );
    _shutDownButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _shutDownButton->setText( "Shut down" );
    QObject::connect( _shutDownButton, &QPushButton::clicked, this, &SystemTab::shutDownButton_clicked );

    auto mainButtonsLayout = WrapWidgetsInHBox( { nullptr, _restartButton, nullptr, _shutDownButton, nullptr } );
    mainButtonsLayout->setContentsMargins( { } );

    //
    // Top level
    //

    _layout->setAlignment( Qt::AlignCenter );
    _layout->setContentsMargins( { } );
    _layout->addLayout( versionInfoLayout );
    _layout->addWidget( _copyrightsLabel );
    _layout->addStretch( 1 );
    _layout->addLayout( copyLogsButtonLayout );
    _layout->addStretch( 2 );
    _layout->addLayout( updateButtonsLayout );
    _layout->addStretch( 2 );
    _layout->addLayout( mainButtonsLayout );
    _layout->addStretch( 1 );
    setLayout( _layout );
}

SystemTab::~SystemTab( ) {
    /*empty*/
}

void SystemTab::_connectShepherd( ) {
    if ( _shepherd ) {
        QObject::connect( _shepherd, &Shepherd::printer_online,                this, &SystemTab::printer_online                 );
        QObject::connect( _shepherd, &Shepherd::printer_offline,               this, &SystemTab::printer_offline                );
        QObject::connect( _shepherd, &Shepherd::printer_firmwareVersionReport, this, &SystemTab::shepherd_firmwareVersionReport );
    }
}

void SystemTab::_initialShowEvent( QShowEvent* event ) {
    QSize newSize = maxSize( maxSize( maxSize( maxSize( _updateSoftwareButton->size( ), _updateFirmwareButton->size( ) ), _restartButton->size( ) ), _shutDownButton->size( ) ), _copyLogsButton->size( ) ) + ButtonPadding;
    _copyLogsButton      ->setFixedSize( newSize );
    _updateSoftwareButton->setFixedSize( newSize );
    _updateFirmwareButton->setFixedSize( newSize );
    _restartButton       ->setFixedSize( newSize );
    _shutDownButton      ->setFixedSize( newSize );

    event->accept( );

    update( );
}

void SystemTab::_updateButtons( ) {
    _copyLogsButton      ->setEnabled( !_mountPoint.isEmpty( )                                                );
    _updateSoftwareButton->setEnabled( _isSoftwareUpgradeAvailable && _isPrinterAvailable                     );
    _updateFirmwareButton->setEnabled( _isFirmwareUpgradeAvailable && _isPrinterAvailable && _isPrinterOnline );
    _restartButton       ->setEnabled(                                _isPrinterAvailable                     );
    _shutDownButton      ->setEnabled(                                _isPrinterAvailable                     );

    update( );
}

bool SystemTab::_yesNoPrompt( QString const& title, QString const& text ) {
    QMessageBox messageBox { this };
    messageBox.setIcon( QMessageBox::Question );
    messageBox.setText( title );
    messageBox.setInformativeText( text );
    messageBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    messageBox.setDefaultButton( QMessageBox::No );
    messageBox.setFont( ModifyFont( messageBox.font( ), 16.0 ) );

    auto mainWindow = getMainWindow( );
    mainWindow->hide( );
    auto result = static_cast<QMessageBox::StandardButton>( messageBox.exec( ) );
    mainWindow->show( );

    return ( result == QMessageBox::Yes );
}

void SystemTab::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ SystemTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch ( _uiState ) {
        case UiState::SelectStarted:
        case UiState::SelectCompleted:
        case UiState::SliceStarted:
        case UiState::SliceCompleted:
        case UiState::PrintCompleted:
            setPrinterAvailable( true );
            break;

        case UiState::PrintStarted:
            setPrinterAvailable( false );
            break;
    }
}

void SystemTab::usbMountManager_filesystemMounted( QString const& mountPoint ) {
    debug( "+ SystemTab::usbMountManager_filesystemMounted: mount point '%s'\n", mountPoint.toUtf8( ).data( ) );
    _mountPoint = mountPoint;
    _updateButtons( );
}

void SystemTab::usbMountManager_filesystemUnmounted( QString const& mountPoint ) {
    debug( "+ SystemTab::usbMountManager_filesystemUnmounted: mount point '%s'\n", mountPoint.toUtf8( ).data( ) );
    _mountPoint.clear( );
    _updateButtons( );
}

void SystemTab::shepherd_firmwareVersionReport( QString const& version ) {
    _versionLabel->setText(
        VersionMessage
        .arg( QCoreApplication::applicationName( )    )
        .arg( QCoreApplication::applicationVersion( ) )
        .arg( version )
        .arg( QCoreApplication::organizationName( )   )
    );
}

void SystemTab::upgradeManager_upgradeCheckStarting( ) {
    debug( "+ SystemTab::upgradeManager_upgradeCheckStarting\n" );

    _isSoftwareUpgradeAvailable = false;
    _updateButtons( );
}

void SystemTab::upgradeManager_upgradeCheckComplete( bool const upgradesFound ) {
    debug( "+ SystemTab::upgradeManager_upgradeCheckComplete: upgrades available? %s\n", YesNoString( upgradesFound ) );

    _isSoftwareUpgradeAvailable = upgradesFound;
    _updateButtons( );
}

void SystemTab::upgradeSelector_canceled( ) {
    debug( "+ SystemTab::upgradeSelector_canceled\n" );

    getMainWindow( )->show( );
    _upgradeSelector->hide( );

    _upgradeSelector->deleteLater( );
    _upgradeSelector = nullptr;

    setPrinterAvailable( true );
    emit printerAvailabilityChanged( true );
}

void SystemTab::upgradeSelector_kitSelected( UpgradeKitInfo const& kit ) {
    debug( "+ SystemTab::upgradeSelector_kitSelected: version %s (%s)\n", kit.versionString.toUtf8( ).data( ), ToString( kit.buildType ) );

    setPrinterAvailable( false );
    emit printerAvailabilityChanged( false );

    _upgradeSelector->showInProgressMessage( );
    _upgradeManager->installUpgradeKit( kit );
}

void SystemTab::upgradeManager_upgradeFailed( ) {
    debug( "+ SystemTab::upgradeManager_upgradeFailed\n" );

    _upgradeSelector->showFailedMessage( );
}

void SystemTab::setPrinterAvailable( bool const value ) {
    _isPrinterAvailable = value;
    debug( "+ SystemTab::setPrinterAvailable: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updateButtons( );
}

void SystemTab::printer_online( ) {
    _isPrinterOnline = true;
    debug( "+ SystemTab::printer_online: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updateButtons( );
}

void SystemTab::printer_offline( ) {
    _isPrinterOnline = false;
    debug( "+ SystemTab::printer_offline: PO? %s PA? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ) );

    _updateButtons( );
}

void SystemTab::copyLogsButton_clicked( bool ) {
    debug( "+ SystemTab::copyLogsButton_clicked: mount point is '%s'\n", _mountPoint.toUtf8( ).data( ) );

    auto debugLogCopier { new DebugLogCopier { _usbMountManager, this } };
    QObject::connect( debugLogCopier, &DebugLogCopier::finished, debugLogCopier, &DebugLogCopier::deleteLater );
    debugLogCopier->copyTo( _mountPoint );
}

void SystemTab::updateSoftwareButton_clicked( bool ) {
    debug( "+ SystemTab::updateSoftwareButton_clicked\n" );

    _upgradeSelector = new UpgradeSelector( _upgradeManager, this );
    QObject::connect( _upgradeSelector, &UpgradeSelector::canceled,    this, &SystemTab::upgradeSelector_canceled    );
    QObject::connect( _upgradeSelector, &UpgradeSelector::kitSelected, this, &SystemTab::upgradeSelector_kitSelected );

    _upgradeSelector->show( );
    getMainWindow( )->hide( );
}

void SystemTab::updateFirmwareButton_clicked( bool ) {
    debug( "+ SystemTab::updateFirmwareButton_clicked\n" );
}

void SystemTab::restartButton_clicked( bool ) {
    if ( _yesNoPrompt( "Confirm", "Are you sure you want to restart?" ) ) {
        system( "sudo systemctl reboot" );
    }
}

void SystemTab::shutDownButton_clicked( bool ) {
    if ( _yesNoPrompt( "Confirm", "Are you sure you want to shut down?" ) ) {
        system( "sudo systemctl poweroff" );
    }
}

void SystemTab::setUpgradeManager( UpgradeManager* upgradeManager ) {
    if ( upgradeManager ) {
        _upgradeManager = upgradeManager;
        QObject::connect( _upgradeManager, &UpgradeManager::upgradeCheckStarting, this, &SystemTab::upgradeManager_upgradeCheckStarting );
        QObject::connect( _upgradeManager, &UpgradeManager::upgradeCheckComplete, this, &SystemTab::upgradeManager_upgradeCheckComplete );
        QObject::connect( _upgradeManager, &UpgradeManager::upgradeFailed,        this, &SystemTab::upgradeManager_upgradeFailed        );
    } else {
        QObject::disconnect( _upgradeManager, nullptr, this, nullptr );
        _upgradeManager = nullptr;
    }
}
