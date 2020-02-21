#include "pch.h"

#include "systemtab.h"

#include "debuglogcopier.h"
#include "shepherd.h"
#include "upgrademanager.h"
#include "upgradeselector.h"
#include "usbmountmanager.h"
#include "window.h"

namespace {

    QString VersionMessage {
        "<span style='font-size: 22pt;'>%1</span><br>"
        "<span style='font-size: 16pt;'>Version %2</span><br>"
        "<span style='font-size: 12pt;'>Firmware version %3</span><br>"
        "<span>© 2019 %4</span>"
    };

}

SystemTab::SystemTab( QWidget* parent ): InitialShowEventMixin<SystemTab, TabBase>( parent ) {
    auto origFont = font( );
    auto font16pt = ModifyFont( origFont, 18.0 );
    auto font22pt = ModifyFont( origFont, LargeFontSize );

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


    _copyrightsLabel->setAlignment( Qt::AlignCenter );
    _copyrightsLabel->setTextFormat( Qt::RichText );
    _copyrightsLabel->setText( ReadWholeFile( ":text/copyright-message.txt" ) );


    _updateSoftwareButton->setEnabled( false );
    _updateSoftwareButton->setFont( font16pt );
    _updateSoftwareButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _updateSoftwareButton->setText( "Update software" );
    QObject::connect( _updateSoftwareButton, &QPushButton::clicked, this, &SystemTab::updateSoftwareButton_clicked );

    _copyLogsButton->setEnabled( false );
    _copyLogsButton->setFont( font16pt );
    _copyLogsButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _copyLogsButton->setText( "Copy logs to USB" );
    QObject::connect( _copyLogsButton, &QPushButton::clicked, this, &SystemTab::copyLogsButton_clicked );


    _restartButton->setFont( font16pt );
    _restartButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _restartButton->setText( "Restart" );
    QObject::connect( _restartButton, &QPushButton::clicked, this, &SystemTab::restartButton_clicked );

    _shutDownButton->setFont( font16pt );
    _shutDownButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _shutDownButton->setText( "Shut down" );
    QObject::connect( _shutDownButton, &QPushButton::clicked, this, &SystemTab::shutDownButton_clicked );

    _layout = WrapWidgetsInVBox(
        WrapWidgetsInHBox( nullptr, _logoLabel,            nullptr, _versionLabel,   nullptr ),
        _copyrightsLabel,
        nullptr,
        WrapWidgetsInHBox( nullptr, _updateSoftwareButton, nullptr, _copyLogsButton, nullptr ),
        nullptr,
        WrapWidgetsInHBox( nullptr, _restartButton,        nullptr, _shutDownButton, nullptr ),
        nullptr
    );
    _layout->setAlignment( Qt::AlignCenter );

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
    QSize newSize = maxSize( _updateSoftwareButton->size( ), _copyLogsButton->size( ), _restartButton->size( ), _shutDownButton->size( ) ) + ButtonPadding;
    _updateSoftwareButton->setFixedSize( newSize );
    _copyLogsButton      ->setFixedSize( newSize );
    _restartButton       ->setFixedSize( newSize );
    _shutDownButton      ->setFixedSize( newSize );

    event->accept( );

    update( );
}

void SystemTab::_connectUsbMountManager( ) {
    QObject::connect( _usbMountManager, &UsbMountManager::filesystemMounted,   this, &SystemTab::usbMountManager_filesystemMounted   );
    QObject::connect( _usbMountManager, &UsbMountManager::filesystemUnmounted, this, &SystemTab::usbMountManager_filesystemUnmounted );
}

void SystemTab::_updateButtons( ) {
    _updateSoftwareButton->setEnabled( _isSoftwareUpgradeAvailable && _isPrinterAvailable );
    _copyLogsButton      ->setEnabled( !_mountPoint.isEmpty( )                            );
    _restartButton       ->setEnabled(                                _isPrinterAvailable );
    _shutDownButton      ->setEnabled(                                _isPrinterAvailable );

    update( );
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

        case UiState::SelectedDirectory:
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
    bool newVersionAvailable = false;

    _isSoftwareUpgradeAvailable = upgradesFound;
    if ( _isSoftwareUpgradeAvailable ) {
        for ( auto const& kit : _upgradeManager->availableUpgrades( ) ) {
            newVersionAvailable |= ( kit.version > LIGHTFIELD_VERSION_CODE );
        }
    }

    _updateButtons( );

    emit iconChanged( TabIndex::System, newVersionAvailable ? QIcon { ":images/new-version-available.png" } : QIcon { } );
}

void SystemTab::upgradeSelector_canceled( ) {
    debug( "+ SystemTab::upgradeSelector_canceled\n" );

    App::mainWindow( )->show( );
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

void SystemTab::updateSoftwareButton_clicked( bool ) {
    debug( "+ SystemTab::updateSoftwareButton_clicked\n" );

    _upgradeSelector = new UpgradeSelector( _upgradeManager, this );
    QObject::connect( _upgradeSelector, &UpgradeSelector::canceled,    this, &SystemTab::upgradeSelector_canceled    );
    QObject::connect( _upgradeSelector, &UpgradeSelector::kitSelected, this, &SystemTab::upgradeSelector_kitSelected );

    _upgradeSelector->show( );
    App::mainWindow( )->hide( );
}

void SystemTab::copyLogsButton_clicked( bool ) {
    debug( "+ SystemTab::copyLogsButton_clicked: mount point is '%s'\n", _mountPoint.toUtf8( ).data( ) );

    auto debugLogCopier { new DebugLogCopier { _usbMountManager, this } };
    QObject::connect( debugLogCopier, &DebugLogCopier::finished, debugLogCopier, &DebugLogCopier::deleteLater );
    debugLogCopier->copyTo( _mountPoint );
}

void SystemTab::restartButton_clicked( bool ) {
    App::mainWindow( )->hide( );
    if ( YesNoPrompt( this, "Confirm", "Are you sure you want to restart?" ) ) {
        RebootPrinter( );
    } else {
        App::mainWindow( )->show( );
    }
}

void SystemTab::shutDownButton_clicked( bool ) {
    App::mainWindow( )->hide( );
    if ( YesNoPrompt( this, "Confirm", "Are you sure you want to shut down?" ) ) {
        ShutDownPrinter( );
    } else {
        App::mainWindow( )->show( );
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
