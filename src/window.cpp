#include "pch.h"

#include "window.h"

#include "pngdisplayer.h"
#include "printjob.h"
#include "printmanager.h"
#include "printprofilemanager.h"
#include "shepherd.h"
#include "signalhandler.h"
#include "upgrademanager.h"
#include "usbmountmanager.h"

#include "filetab.h"
#include "preparetab.h"
#include "tilingtab.h"
#include "printtab.h"
#include "statustab.h"
#include "advancedtab.h"
#include "profilestab.h"
#include "systemtab.h"

namespace {

    std::initializer_list<int> signalList {
        SIGHUP,
        SIGINT,
        SIGQUIT,
        SIGTERM,
#if defined _DEBUG
        SIGUSR1,
#endif // defined _DEBUG
        SIGUSR2,
    };

}

Window::Window( QWidget* parent ): QMainWindow( parent ) {
#if defined _DEBUG
    _isPrinterPrepared = g_settings.pretendPrinterIsPrepared;
#endif // _DEBUG
    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
    setFixedSize( MainWindowSize );
    move( g_settings.mainWindowPosition );

    _pngDisplayer = new PngDisplayer;
    QObject::connect( _pngDisplayer, &PngDisplayer::terminationRequested, static_cast<App*>( qApp ), &App::terminate, Qt::QueuedConnection );
    _pngDisplayer->show( );

    _signalHandler = new SignalHandler;
    QObject::connect( _signalHandler, &SignalHandler::signalReceived, this, &Window::signalHandler_signalReceived );
    _signalHandler->subscribe( signalList );

    _shepherd = new Shepherd { parent };
    QObject::connect( _shepherd, &Shepherd::shepherd_started,     this, &Window::shepherd_started     );
    QObject::connect( _shepherd, &Shepherd::shepherd_startFailed, this, &Window::shepherd_startFailed );
    QObject::connect( _shepherd, &Shepherd::shepherd_terminated,  this, &Window::shepherd_terminated  );

    _printProfileManager = new PrintProfileManager;
    _printProfileManager->reload();

    _printJob = QSharedPointer<PrintJob>(new PrintJob);
    _printJob->setPrintProfile(_printProfileManager->activeProfile());
    _printJob->baseSlices.layerCount = 2;
    _printJob->baseSlices.layerThickness = 100;
    _printJob->bodySlices.layerThickness = 100;


    _upgradeManager      = new UpgradeManager;
    _usbMountManager     = new UsbMountManager;

    _printManager = new PrintManager( _shepherd, this );

    QObject::connect( _usbMountManager, &UsbMountManager::ready, _upgradeManager, [this] ( ) {
        QObject::connect( _usbMountManager, &UsbMountManager::filesystemMounted, _upgradeManager, &UpgradeManager::checkForUpgrades );
        _upgradeManager->checkForUpgrades( _usbMountManager->mountPoint( ) );
    } );

    std::vector<TabBase*> tabs {
        _fileTab     = new FileTab,
        _prepareTab  = new PrepareTab,
        _tilingTab   = new TilingTab,
        _printTab    = new PrintTab,
        _statusTab   = new StatusTab,
        _advancedTab = new AdvancedTab,
        _profilesTab = new ProfilesTab,
        _systemTab   = new SystemTab,
    };

    QObject::connect(_printProfileManager, &PrintProfileManager::activeProfileChanged,
        _advancedTab, &AdvancedTab::loadPrintProfile);
    QObject::connect(_printProfileManager, &PrintProfileManager::activeProfileChanged,
        _prepareTab, &PrepareTab::loadPrintProfile);

    for (const auto &tabA: tabs) {
        tabA->setPrintJob(_printJob);
        QObject::connect(this, &Window::printJobChanged, tabA, &TabBase::setPrintJob);
        QObject::connect(this, &Window::printManagerChanged, tabA, &TabBase::setPrintManager);
        QObject::connect(this, &Window::shepherdChanged, tabA, &TabBase::setShepherd);
        QObject::connect(tabA, &TabBase::uiStateChanged, this, &Window::tab_uiStateChanged, Qt::QueuedConnection);
        QObject::connect(tabA, &TabBase::iconChanged,
            [this] (TabIndex const sender, QIcon const& icon) {
                _tabWidget->setTabIcon(+sender, icon);
            }
        );

        for (const auto &tabB: tabs) {
            QObject::connect(tabA, &TabBase::uiStateChanged, tabB, &TabBase::tab_uiStateChanged,
                Qt::QueuedConnection);            
        }
    }

    emit shepherdChanged( _shepherd );
    emit printJobChanged( _printJob );
    emit printManagerChanged( _printManager );

    _fileTab    ->setUsbMountManager    ( _usbMountManager     );
    _prepareTab ->setUsbMountManager    ( _usbMountManager );
    _profilesTab->setUsbMountManager    ( _usbMountManager );
    _advancedTab->setPngDisplayer       ( _pngDisplayer        );
    _systemTab  ->setUpgradeManager     ( _upgradeManager      );
    _systemTab  ->setUsbMountManager    ( _usbMountManager     );

    _shepherd->start( );

    //
    // "File" tab
    //

    _fileTab->setContentsMargins( { } );
    _fileTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _fileTab, &FileTab::modelSelected, this, &Window::fileTab_modelSelected );

    //
    // "Prepare" tab
    //

    _prepareTab->setContentsMargins( { } );
    _prepareTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _prepareTab, &PrepareTab::preparePrinterStarted,      this,         &Window::prepareTab_preparePrinterStarted  );
    QObject::connect( _prepareTab, &PrepareTab::preparePrinterComplete,     this,         &Window::prepareTab_preparePrinterComplete );
    QObject::connect( _prepareTab, &PrepareTab::slicingNeeded,              this,         &Window::prepareTab_slicingNeeded          );
    QObject::connect( _prepareTab, &PrepareTab::printerAvailabilityChanged, _printTab,    &PrintTab::setPrinterAvailable             );
    QObject::connect( _prepareTab, &PrepareTab::printerAvailabilityChanged, _statusTab,   &StatusTab::setPrinterAvailable            );
    QObject::connect( _prepareTab, &PrepareTab::printerAvailabilityChanged, _advancedTab, &AdvancedTab::setPrinterAvailable          );
    QObject::connect( _prepareTab, &PrepareTab::printerAvailabilityChanged, _systemTab,   &SystemTab::setPrinterAvailable            );


    _tilingTab->setContentsMargins( { } );
    _tilingTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // "Print" tab
    //

    _printTab->setContentsMargins( { } );
    _printTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _printTab, &PrintTab::printRequested,             this,         &Window::startPrinting                    );
    QObject::connect( _printTab, &PrintTab::printerAvailabilityChanged, _prepareTab,  &PrepareTab::setPrinterAvailable          );
    QObject::connect( _printTab, &PrintTab::printerAvailabilityChanged, _statusTab,   &StatusTab::setPrinterAvailable           );
    QObject::connect( _printTab, &PrintTab::printerAvailabilityChanged, _advancedTab, &AdvancedTab::setPrinterAvailable         );
    QObject::connect( _printTab, &PrintTab::projectorPowerLevelChanged, _advancedTab, &AdvancedTab::projectorPowerLevel_changed );
    QObject::connect( _printTab, &PrintTab::printerAvailabilityChanged, _systemTab,   &SystemTab::setPrinterAvailable           );
    QObject::connect( this,      &Window::modelRendered,                _printTab,    &PrintTab::setModelRendered               );
    QObject::connect( this,      &Window::printerPrepared,              _printTab,    &PrintTab::setPrinterPrepared             );


    //
    // "Status" tab
    //

    _statusTab->setContentsMargins( { } );
    _statusTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _statusTab, &StatusTab::printRequested, this,       &Window::startPrinting         );
    QObject::connect( this,       &Window::modelRendered,     _statusTab, &StatusTab::setModelRendered   );
    QObject::connect( this,       &Window::printerPrepared,   _statusTab, &StatusTab::setPrinterPrepared );

    //
    // "Advanced" tab
    //

    _advancedTab->setContentsMargins( { } );
    _advancedTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _advancedTab, &AdvancedTab::printerAvailabilityChanged, _prepareTab, &PrepareTab::setPrinterAvailable       );
    QObject::connect( _advancedTab, &AdvancedTab::printerAvailabilityChanged, _printTab,   &PrintTab::setPrinterAvailable         );
    QObject::connect( _advancedTab, &AdvancedTab::projectorPowerLevelChanged, _printTab,   &PrintTab::projectorPowerLevel_changed );
    QObject::connect( _advancedTab, &AdvancedTab::advancedExposureTimeChanged, _printTab,  &PrintTab::changeExpoTimeSliders       );
    QObject::connect( _advancedTab, &AdvancedTab::printerAvailabilityChanged, _statusTab,  &StatusTab::setPrinterAvailable        );
    QObject::connect( _advancedTab, &AdvancedTab::printerAvailabilityChanged, _systemTab,  &SystemTab::setPrinterAvailable        );


    //
    // "Profiles" tab
    //

    _profilesTab->setContentsMargins( { } );
    _profilesTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // "System" tab
    //

    _systemTab->setContentsMargins( { } );
    _systemTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _systemTab, &SystemTab::printerAvailabilityChanged, _prepareTab,  &PrepareTab::setPrinterAvailable  );
    QObject::connect( _systemTab, &SystemTab::printerAvailabilityChanged, _printTab,    &PrintTab::setPrinterAvailable    );
    QObject::connect( _systemTab, &SystemTab::printerAvailabilityChanged, _statusTab,   &StatusTab::setPrinterAvailable   );
    QObject::connect( _systemTab, &SystemTab::printerAvailabilityChanged, _advancedTab, &AdvancedTab::setPrinterAvailable );

    //
    // Tab widget
    //

    auto fontNormalSize { ModifyFont( font( ), NormalFontSize ) };
    auto fontLargeSize  { ModifyFont( font( ), LargeFontSize  ) };

    _tabWidget->setFont( fontLargeSize );
    QObject::connect( _tabWidget, &QTabWidget::currentChanged, this, &Window::tabs_currentChanged );

    _helpButton->setContentsMargins( { } );
    _helpButton->setEnabled( false );
    _helpButton->setFont( fontLargeSize );
    _helpButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _helpButton->setText( "?" );
    QObject::connect( _helpButton, &QPushButton::clicked, this, &Window::helpButton_clicked );
    //_tabWidget->setCornerWidget( _helpButton, Qt::TopRightCorner );

    for ( auto tab : tabs ) {
        _tabWidget->addTab( tab, ToString( tab->tabIndex( ) ) );
        tab->setFont( fontNormalSize );

        tab->setPrintProfileManager(_printProfileManager);
    }

    setCentralWidget( _tabWidget );
}

Window::~Window( ) {
    /*empty*/
}

void Window::_setPrinterPrepared( bool const value ) {
    auto newValue =
#if defined _DEBUG
        g_settings.pretendPrinterIsPrepared ? true :
#endif // _DEBUG
        value;

    debug( "+ Window::_setPrinterPrepared: old value: %s; new value: %s\n", ToString( _isPrinterPrepared ), ToString( newValue ) );
    _isPrinterPrepared = newValue;
    emit printerPrepared( newValue );

    update( );
}

void Window::_setModelRendered( bool const value ) {
    debug( "+ Window::_setModelRendered: old value: %s; new value: %s\n", ToString( _isModelRendered ), ToString( value ) );
    _isModelRendered = value;
    emit modelRendered( value );

    update( );
}

void Window::closeEvent( QCloseEvent* event ) {
    debug( "+ Window::closeEvent\n" );
    event->ignore( );

    emit terminationRequested( );
}

void Window::terminate( ) {
    debug( "+ Window::terminate\n" );
    if ( _shepherd ) {
        _shepherd->doTerminate( );
    }

    if ( _printManager ) {
        _printManager->terminate( );
    }

    if ( _usbMountManager ) {
        _fileTab   ->setUsbMountManager( nullptr );
        _prepareTab->setUsbMountManager( nullptr );
        _profilesTab->setUsbMountManager( nullptr );
        _systemTab ->setUsbMountManager( nullptr );

        QObject::disconnect( _usbMountManager );
        _usbMountManager->deleteLater( );
        _usbMountManager = nullptr;
    }

    if ( _upgradeManager ) {
        if ( _systemTab ) {
            _systemTab->setUpgradeManager( nullptr );
        }
        QObject::disconnect( _upgradeManager );
        delete _upgradeManager;
        _upgradeManager = nullptr;
    }

    if ( _pngDisplayer ) {
        _pngDisplayer->close( );
        _pngDisplayer->deleteLater( );
        _pngDisplayer = nullptr;
    }

    if ( _signalHandler ) {
        QObject::disconnect( _signalHandler );
        _signalHandler->unsubscribe( signalList );
        _signalHandler->deleteLater( );
        _signalHandler = nullptr;
    }

    update( );
}

void Window::startPrinting()
{
    _tabWidget->setCurrentIndex(+TabIndex::Status);
    update();

    const auto& baseSlices = _printJob->baseSlices;
    const auto& bodySlices = _printJob->bodySlices;
    const auto& baseLayerParameters = _printJob->baseLayerParameters();
    const auto& bodyLayerParameters = _printJob->bodyLayerParameters();

    debug(
        "+ Window::startPrinting: print job %p:\n"
        "  + modelFileName:              '%s'\n"
        "  + modelHash:                  %s\n"
        "  + totalLayerCount:            %d\n"
        "  + base slices:\n"
        "    + sliceDirectory:           '%s'\n"
        "    + isPreSliced:              %s\n"
        "    + layerCount:               %d\n"
        "    + layerThickness:           %d\n"
        "    + startLayer:               %d\n"
        "    + endLayer:                 %d\n"
        "  + body slices:\n"
        "    + sliceDirectory:           '%s'\n"
        "    + isPreSliced:              %s\n"
        "    + layerCount:               %d\n"
        "    + layerThickness:           %d\n"
        "    + startLayer:               %d\n"
        "    + endLayer:                 %d\n"
        "  + layer parameters: (calculated parameters are marked with *)\n"

        "",

        _printJob,
        _printJob->modelFileName.toUtf8().data(),
        _printJob->modelHash.toUtf8().data(),
        _printJob->totalLayerCount(),

        baseSlices.sliceDirectory.toUtf8().data(),
        ToString( baseSlices.isPreSliced ),
        baseSlices.layerCount,
        baseSlices.layerThickness,
        _printJob->baseLayerStart(),
        _printJob->baseLayerEnd(),

        bodySlices.sliceDirectory.toUtf8().data(),
        ToString( bodySlices.isPreSliced ),
        bodySlices.layerCount,
        bodySlices.layerThickness,
        _printJob->bodyLayerStart(),
        _printJob->bodyLayerEnd()
    );

    debug(
        "    + baseLayerCount:           %d\n"
        "    + baseLayersPumpingEnabled: %s\n"
        "    + base layer parameters:\n"
        "      + pumpUpDistance:         %.2f mm\n"
        "      + pumpUpVelocity:        *%.2f mm/min\n"
        "      + pumpUpPause:            %d ms\n"
        "      + pumpDownDistance:      *%.2f mm\n"
        "      + pumpDownVelocity:      *%.2f mm/min\n"
        "      + pumpDownPause:          %d ms\n"
        "      + noPumpUpVelocity:       %.2f mm/min\n"
        "      + pumpEveryNthLayer:      %d\n"
        "      + layerThickness:         %d µm\n"
        "      + layerExposureTime:      %.2f s\n"
        "      + powerLevel:             %.1f%%\n"
        "",

        _printJob->baseSlices.layerCount,
        ToString( baseLayerParameters.isPumpingEnabled( ) ),
        baseLayerParameters.pumpUpDistance( ),
        baseLayerParameters.pumpUpVelocity_Effective( ),
        baseLayerParameters.pumpUpPause( ),
        baseLayerParameters.pumpDownDistance_Effective( ),
        baseLayerParameters.pumpDownVelocity_Effective( ),
        baseLayerParameters.pumpDownPause( ),
        baseLayerParameters.noPumpUpVelocity( ),
        baseLayerParameters.pumpEveryNthLayer( ),
        baseLayerParameters.layerThickness( ),
        baseSlices.exposureTime,
        baseLayerParameters.powerLevel( )
    );


    debug(
        "    + bodyLayersPumpingEnabled: %s\n"
        "    + body layer parameters:\n"
        "      + pumpUpDistance:         %.2f mm\n"
        "      + pumpUpVelocity:        *%.2f mm/min\n"
        "      + pumpUpPause:            %d ms\n"
        "      + pumpDownDistance:      *%.2f mm\n"
        "      + pumpDownVelocity:      *%.2f mm/min\n"
        "      + pumpDownPause:          %d ms\n"
        "      + noPumpUpVelocity:       %.2f mm/min\n"
        "      + pumpEveryNthLayer:      %d\n"
        "      + layerThickness:         %d µm\n"
        "      + layerExposureTime:      %.2f s\n"
        "      + powerLevel:             %.1f%%\n"
        "",


        ToString( bodyLayerParameters.isPumpingEnabled( ) ),
        bodyLayerParameters.pumpUpDistance( ),
        bodyLayerParameters.pumpUpVelocity_Effective( ),
        bodyLayerParameters.pumpUpPause( ),
        bodyLayerParameters.pumpDownDistance_Effective( ),
        bodyLayerParameters.pumpDownVelocity_Effective( ),
        bodyLayerParameters.pumpDownPause( ),
        bodyLayerParameters.noPumpUpVelocity( ),
        bodyLayerParameters.pumpEveryNthLayer( ),
        bodyLayerParameters.layerThickness( ),
        bodySlices.exposureTime,
        bodyLayerParameters.powerLevel( )
    );

    QSharedPointer<PrintJob> job(_printJob);
    _printJob.reset(new PrintJob(*_printJob));

    PrintManager* oldPrintManager = _printManager;

    _printManager = new PrintManager( _shepherd, this );
    _printManager->setPngDisplayer( _pngDisplayer );

    QObject::connect( _printManager, &PrintManager::printStarting, this, &Window::printManager_printStarting );
    QObject::connect( _printManager, &PrintManager::printComplete, this, &Window::printManager_printComplete );
    QObject::connect( _printManager, &PrintManager::printAborted,  this, &Window::printManager_printAborted  );

    emit printJobChanged( _printJob );
    emit printManagerChanged( _printManager );

    _printManager->print( job );

    if ( oldPrintManager ) {
        QObject::disconnect( oldPrintManager );
        oldPrintManager->deleteLater( );
    }
}

void Window::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ Window::tab_uiStateChanged: from %sTab: %s => %s [PP? %s MR? %s current tab %s]\n", ToString( sender ), ToString( _uiState ), ToString( state ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ), ToString( static_cast<TabIndex>( _tabWidget->currentIndex( ) ) ) );

    _uiState = state;
    switch ( _uiState ) {
        case UiState::SelectStarted:
            _setModelRendered(false);
            break;

        case UiState::SelectCompleted:
            _setModelRendered(this->_printJob->directoryMode);
            if (_tabWidget->currentIndex() == +TabIndex::File ||
                _tabWidget->currentIndex() == +TabIndex::Tiling) {
                _tabWidget->setCurrentIndex(+TabIndex::Prepare);
                update();
            }
            break;

        case UiState::SliceStarted:
            _setModelRendered(false);
            break;

        case UiState::SliceCompleted:
            _setModelRendered(true);
            if (_isModelRendered && _isPrinterPrepared &&
                _tabWidget->currentIndex() == +TabIndex::Prepare) {
                _tabWidget->setCurrentIndex( +TabIndex::Print);
                update( );
            }
            break;

        case UiState::TilingClicked:
            _tabWidget->setCurrentIndex(+TabIndex::Tiling);
            update();
            break;

        default:
            break;
    }
}

void Window::tabs_currentChanged(int index)
{
    //debug( "+ Window::tabs_currentChanged: new tab is '%s' [%d]\n", ToString( static_cast<TabIndex>( index ) ), index );
    (void)index;
    update();
}

void Window::helpButton_clicked( bool ) {
    debug( "+ Window::helpButton_clicked\n" );
    // TODO
}

void Window::shepherd_started( ) {
    debug( "+ Window::shepherd_started\n" );
}

void Window::shepherd_startFailed( ) {
    debug( "+ Window::shepherd_startFailed\n" );
    // TODO
}

void Window::shepherd_terminated( bool const expected, bool const cleanExit ) {
    debug( "+ Window::shepherd_terminated: expected? %s; clean? %s\n", ToString( expected ), ToString( cleanExit ) );
    // TODO
}

void Window::printManager_printStarting( ) {
    debug( "+ Window::printManager_printStarting\n" );
    // TODO
}

void Window::printManager_printComplete( bool const success ) {
    debug( "+ Window::printManager_printComplete: success? %s; is model rendered? %s; is printer prepared? %s\n", ToString( success ), ToString( _isModelRendered ), ToString( _isPrinterPrepared ) );
    // TODO
}

void Window::printManager_printAborted( ) {
    debug( "+ Window::printManager_printAborted: forwarding to printManager_printComplete\n" );

    printManager_printComplete( false );

    update( );
}

void Window::fileTab_modelSelected( ModelSelectionInfo const* modelSelection ) {
    if ( _modelSelection ) {
        delete _modelSelection;
    }
    _modelSelection = new ModelSelectionInfo { *modelSelection };

    debug(
        "+ Window::fileTab_modelSelected:\n"
        "  + file name:        '%s'\n"
        "  + vertex count:     %5zu\n"
        "  + X min, max, size: %.2f..%.2f, %.2f\n"
        "  + Y min, max, size: %.2f..%.2f, %.2f\n"
        "  + Z min, max, size: %.2f..%.2f, %.2f\n"
        "  + estimated volume: %.2f mL\n"
        "",
        _modelSelection->fileName.toUtf8( ).data( ),
        _modelSelection->vertexCount,
        _modelSelection->x.min, _modelSelection->x.max, _modelSelection->x.size,
        _modelSelection->y.min, _modelSelection->y.max, _modelSelection->y.size,
        _modelSelection->z.min, _modelSelection->z.max, _modelSelection->z.size,
        _modelSelection->estimatedVolume
    );

    _printJob->directoryMode   = false;
    _printJob->vertexCount     = _modelSelection->vertexCount;
    _printJob->x               = _modelSelection->x;
    _printJob->y               = _modelSelection->y;
    _printJob->z               = _modelSelection->z;
    _printJob->estimatedVolume = _modelSelection->estimatedVolume;
    _printJob->modelFileName   = _modelSelection->fileName;

    _setModelRendered( false );
}

void Window::prepareTab_preparePrinterStarted( ) {
    debug( "+ Window::prepareTab_preparePrinterStarted\n" );
    _setPrinterPrepared( false );
}

void Window::prepareTab_preparePrinterComplete( bool const success ) {
    debug( "+ Window::prepareTab_preparePrinterComplete: %s; [PP? %s MR? %s current tab %s]\n", SucceededString( success ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ), ToString( static_cast<TabIndex>( _tabWidget->currentIndex( ) ) ) );
    _setPrinterPrepared( success );

    if ( _isModelRendered && _isPrinterPrepared && ( _tabWidget->currentIndex( ) == +TabIndex::Prepare ) ) {
        debug( "+ Window::prepareTab_preparePrinterComplete: switching to Print tab\n" );
        _tabWidget->setCurrentIndex( +TabIndex::Print );

        update( );
    }
}

void Window::prepareTab_slicingNeeded( bool const needed ) {
    debug( "+ Window::prepareTab_slicingNeeded: %s; [PP? %s MR? %s current tab %s]\n", YesNoString( needed ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ), ToString( static_cast<TabIndex>( _tabWidget->currentIndex( ) ) ) );
    _setModelRendered( !needed );
}

void Window::signalHandler_signalReceived( siginfo_t const& info ) {
    debug( "+ Window::signalHandler_signalReceived: received signal %s [%d]\n", ::strsignal( info.si_signo ), info.si_signo );

    if ( ( SIGUSR2 == info.si_signo ) && ( getpid( ) != info.si_pid ) ) {
        sigval_t val;
        val.sival_int = LIGHTFIELD_VERSION_CODE;
        sigqueue( info.si_pid, SIGUSR2, val );
        return;
    }

#if defined _DEBUG
    if ( SIGUSR1 == info.si_signo ) {
        debug( "+ Window::signalHandler_signalReceived: object information dump:\n" );
        dumpObjectInfo( );
        debug( "+ Window::signalHandler_signalReceived: object tree dump:\n" );
        dumpObjectTree( );
        return;
    }
#endif // defined _DEBUG

    close( );
}

void Window::showEvent( QShowEvent* aShowEvent )
{
    QMainWindow::showEvent(aShowEvent);
    activateWindow();
}
