#include "pch.h"

#include "window.h"

#include "app.h"
#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"
#include "signalhandler.h"
#include "strings.h"
#include "utils.h"

#include "filetab.h"
#include "preparetab.h"
#include "printtab.h"
#include "statustab.h"
#include "advancedtab.h"
#include "maintenancetab.h"

namespace {

    std::initializer_list<int> signalList {
        SIGHUP,
        SIGINT,
        SIGQUIT,
        SIGTERM,
#if defined _DEBUG
        SIGUSR1,
#endif // defined _DEBUG
    };

}

Window::Window( QWidget* parent ): QMainWindow/*InitialShowEventMixin<Window, QMainWindow>*/( parent ) {
    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
    setFixedSize( MainWindowSize );
    move( g_settings.mainWindowPosition );

    QObject::connect( g_signalHandler, &SignalHandler::signalReceived, this, &Window::signalHandler_signalReceived );
    g_signalHandler->subscribe( signalList );

#if defined _DEBUG
    if ( g_settings.pretendPrinterIsPrepared ) {
        _isPrinterPrepared = true;
    }
#endif // _DEBUG

    _printJob       = new PrintJob;
    _shepherd       = new Shepherd { parent };

    _fileTab        = new FileTab;
    _prepareTab     = new PrepareTab;
    _printTab       = new PrintTab;
    _statusTab      = new StatusTab;
    _advancedTab    = new AdvancedTab;
    _maintenanceTab = new MaintenanceTab;

    std::vector<TabBase*> tabs {
        _fileTab,
        _prepareTab,
        _printTab,
        _statusTab,
        _advancedTab,
        _maintenanceTab,
    };

    QObject::connect( this,            &Window::printJobChanged,        _fileTab,        &FileTab::setPrintJob            );
    QObject::connect( this,            &Window::printJobChanged,        _prepareTab,     &PrepareTab::setPrintJob         );
    QObject::connect( this,            &Window::printJobChanged,        _printTab,       &PrintTab::setPrintJob           );
    QObject::connect( this,            &Window::printJobChanged,        _statusTab,      &StatusTab::setPrintJob          );
    QObject::connect( this,            &Window::printJobChanged,        _advancedTab,    &AdvancedTab::setPrintJob        );
    QObject::connect( this,            &Window::printJobChanged,        _maintenanceTab, &MaintenanceTab::setPrintJob     );
                                       
    QObject::connect( this,            &Window::printManagerChanged,    _fileTab,        &FileTab::setPrintManager        );
    QObject::connect( this,            &Window::printManagerChanged,    _prepareTab,     &PrepareTab::setPrintManager     );
    QObject::connect( this,            &Window::printManagerChanged,    _printTab,       &PrintTab::setPrintManager       );
    QObject::connect( this,            &Window::printManagerChanged,    _statusTab,      &StatusTab::setPrintManager      );
    QObject::connect( this,            &Window::printManagerChanged,    _advancedTab,    &AdvancedTab::setPrintManager    );
    QObject::connect( this,            &Window::printManagerChanged,    _maintenanceTab, &MaintenanceTab::setPrintManager );
                                       
    QObject::connect( this,            &Window::shepherdChanged,        _fileTab,        &FileTab::setShepherd            );
    QObject::connect( this,            &Window::shepherdChanged,        _prepareTab,     &PrepareTab::setShepherd         );
    QObject::connect( this,            &Window::shepherdChanged,        _printTab,       &PrintTab::setShepherd           );
    QObject::connect( this,            &Window::shepherdChanged,        _statusTab,      &StatusTab::setShepherd          );
    QObject::connect( this,            &Window::shepherdChanged,        _advancedTab,    &AdvancedTab::setShepherd        );
    QObject::connect( this,            &Window::shepherdChanged,        _maintenanceTab, &MaintenanceTab::setShepherd     );

    QObject::connect( _shepherd,       &Shepherd::shepherd_started,     this,            &Window::shepherd_started        );
    QObject::connect( _shepherd,       &Shepherd::shepherd_startFailed, this,            &Window::shepherd_startFailed    );
    QObject::connect( _shepherd,       &Shepherd::shepherd_terminated,  this,            &Window::shepherd_terminated     );

    for ( auto tabA : tabs ) {
        QObject::connect( tabA, &TabBase::uiStateChanged, this, &Window::tab_uiStateChanged );
        for ( auto tabB : tabs ) {
            QObject::connect( tabA, &TabBase::uiStateChanged, tabB, &TabBase::tab_uiStateChanged );
        }
    }

    emit shepherdChanged( _shepherd );
    emit printJobChanged( _printJob );

    _shepherd->start( );

    //
    // "Select" tab
    //

    _fileTab->setContentsMargins( { } );
    _fileTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _fileTab, &FileTab::modelSelected, this, &Window::fileTab_modelSelected );

    //
    // "Prepare" tab
    //

    _prepareTab->setContentsMargins( { } );
    _prepareTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _prepareTab->setShepherd( _shepherd );
    QObject::connect( _prepareTab, &PrepareTab::sliceStarted,           this, &Window::prepareTab_sliceStarted           );
    QObject::connect( _prepareTab, &PrepareTab::sliceComplete,          this, &Window::prepareTab_sliceComplete          );
    QObject::connect( _prepareTab, &PrepareTab::renderStarted,          this, &Window::prepareTab_renderStarted          );
    QObject::connect( _prepareTab, &PrepareTab::renderComplete,         this, &Window::prepareTab_renderComplete         );
    QObject::connect( _prepareTab, &PrepareTab::preparePrinterStarted,  this, &Window::prepareTab_preparePrinterStarted  );
    QObject::connect( _prepareTab, &PrepareTab::preparePrinterComplete, this, &Window::prepareTab_preparePrinterComplete );
    QObject::connect( _prepareTab, &PrepareTab::slicingNeeded,          this, &Window::prepareTab_slicingNeeded          );

    //
    // "Print" tab
    //

    _printTab->setContentsMargins( { } );
    _printTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _printTab, &PrintTab::printButtonClicked, this, &Window::printTab_printButtonClicked );

    //
    // "Status" tab
    //

    _statusTab->setContentsMargins( { } );
    _statusTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _statusTab, &StatusTab::stopButtonClicked,    this, &Window::statusTab_stopButtonClicked    );
    QObject::connect( _statusTab, &StatusTab::reprintButtonClicked, this, &Window::statusTab_reprintButtonClicked );
    QObject::connect( _statusTab, &StatusTab::printComplete,        this, &Window::statusTab_cleanUpAfterPrint    );

    //
    // "Advanced" tab
    //

    _advancedTab->setContentsMargins( { } );
    _advancedTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // Tab widget
    //

    double pointSize = _tabWidget->font( ).pointSizeF( );
    _tabWidget->setFont( ModifyFont( _tabWidget->font( ), 20.0 ) );
    QObject::connect( _tabWidget, &QTabWidget::currentChanged, this, &Window::tabs_currentChanged );

    auto font9pt = ModifyFont( _fileTab->font( ), pointSize );
    for ( TabIndex index = TabIndex::File; index <= TabIndex::Maintenance; index = static_cast<TabIndex>( +index + 1 ) ) {
        _tabWidget->addTab( tabs[+index], ToString( index ) );
        tabs[+index]->setFont( font9pt );
    }

    setCentralWidget( _tabWidget );
}

Window::~Window( ) {
    QObject::disconnect( g_signalHandler, nullptr, this, nullptr );
    g_signalHandler->unsubscribe( signalList );
}

void Window::closeEvent( QCloseEvent* event ) {
    debug( "+ Window::closeEvent\n" );
    if ( _printManager ) {
        _printManager->terminate( );
    }
    _shepherd->doTerminate( );
    event->accept( );
}

//void Window::_initialShowEvent( QShowEvent* event ) {
//    debug( "+ Window::_initialShowEvent\n" );
//    event->ignore( );
//}

void Window::_startPrinting( ) {
    _tabWidget->setCurrentIndex( +TabIndex::Status );
    debug(
        "+ Window::_startPrinting\n"
        "  + Print job:\n"
        "    + modelFileName:       '%s'\n"
        "    + jobWorkingDirectory: '%s'\n"
        "    + layerCount:          %d\n"
        "    + layerThickness:      %d\n"
        "    + exposureTime:        %f\n"
        "    + powerLevel:          %d\n"
        "",
        _printJob->modelFileName.toUtf8( ).data( ),
        _printJob->jobWorkingDirectory.toUtf8( ).data( ),
        _printJob->layerCount,
        _printJob->layerThickness,
        _printJob->exposureTime,
        _printJob->powerLevel
    );

    PrintJob* job = _printJob;
    _printJob = new PrintJob( *_printJob );

    _printManager = new PrintManager( _shepherd, this );
    _printManager->print( job );

    emit printJobChanged( job );
    emit printManagerChanged( _printManager );

    _printTab->setPrintButtonEnabled( false );
    _statusTab->setStopButtonEnabled( true );
    _statusTab->setReprintButtonEnabled( false );
}

void Window::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ Window::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch ( state ) {
        case UiState::SelectStarted:
            /*empty*/
            break;

        case UiState::SelectCompleted:
            if ( _tabWidget->currentIndex( ) == +TabIndex::File ) {
                _tabWidget->setCurrentIndex( +TabIndex::Prepare );
            }
            break;

        case UiState::SliceStarted:
        case UiState::SliceCompleted:
        case UiState::PrintStarted:
        case UiState::PrintCompleted:
            break;
    }
}

void Window::shepherd_started( ) {
    debug( "+ Window::shepherd_started\n" );
}

void Window::shepherd_startFailed( ) {
    debug( "+ Window::shepherd_startFailed\n" );
    // TODO panic!
}

void Window::shepherd_terminated( bool const expected, bool const cleanExit ) {
    debug( "+ Window::shepherd_terminated: expected? %s; clean? %s\n", ToString( expected ), ToString( cleanExit ) );
    // TODO restart shepherd if not expected
}

void Window::fileTab_modelSelected( ModelSelectionInfo* modelSelection ) {
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
    _isModelRendered = false;
    debug( "  + isModelRendered set to false.\n" );

    _printJob->vertexCount     = _modelSelection->vertexCount;
    _printJob->x               = _modelSelection->x;
    _printJob->y               = _modelSelection->y;
    _printJob->z               = _modelSelection->z;
    _printJob->estimatedVolume = _modelSelection->estimatedVolume;
    _printJob->modelFileName   = _modelSelection->fileName;
}

void Window::prepareTab_sliceStarted( ) {
    debug( "+ Window::prepareTab_sliceStarted: isModelRendered set to false.\n" );

    _printTab->setPrintButtonEnabled( false );
    _statusTab->setReprintButtonEnabled( false );
}

void Window::prepareTab_sliceComplete( bool const success ) {
    debug( "+ Window::prepareTab_sliceComplete: success: %s\n", ToString( success ) );
    if ( !success ) {
        return;
    }
}

void Window::prepareTab_renderStarted( ) {
    debug( "+ Window::prepareTab_renderStarted\n" );
}

void Window::prepareTab_renderComplete( bool const success ) {
    debug( "+ Window::prepareTab_renderComplete: success: %s\n", ToString( success ) );
    _isModelRendered = success;
    debug( "  + isModelRendered set to %s.\n", ToString( success ) );

    _printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    _statusTab->setReprintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    if ( _isModelRendered && _isPrinterPrepared && ( _tabWidget->currentIndex( ) == +TabIndex::Prepare ) ) {
        _tabWidget->setCurrentIndex( +TabIndex::Print );
    }
}

void Window::prepareTab_preparePrinterStarted( ) {
    debug( "+ Window::prepareTab_preparePrinterStarted\n" );
    _isPrinterPrepared = false;
}

void Window::prepareTab_preparePrinterComplete( bool const success ) {
    _isPrinterPrepared =
#if defined _DEBUG
        g_settings.pretendPrinterIsPrepared ? true :
#endif // _DEBUG
        success;
    debug( "+ Window::prepareTab_preparePrinterComplete: isPrinterPrepared set to %s [%s].\n", ToString( _isPrinterPrepared ), ToString( success ) );

    _printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    _statusTab->setReprintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    if ( _isModelRendered && _isPrinterPrepared && ( _tabWidget->currentIndex( ) == +TabIndex::Prepare ) ) {
        _tabWidget->setCurrentIndex( +TabIndex::Print );
    }
}

void Window::prepareTab_slicingNeeded( bool const needed ) {
    debug( "+ Window::prepareTab_slicingNeeded: needed? %s\n", ToString( needed ) );
    _isModelRendered = !needed;

    _printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    _statusTab->setReprintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    if ( _isModelRendered && _isPrinterPrepared && ( _tabWidget->currentIndex( ) == +TabIndex::Prepare ) ) {
        _tabWidget->setCurrentIndex( +TabIndex::Print );
    }
}

void Window::printTab_printButtonClicked( ) {
    _startPrinting( );
}

void Window::tabs_currentChanged( int index ) {
    debug( "+ Window::tabs_currentChanged: new tab is '%s' [%d]\n", ToString( static_cast<TabIndex>( index ) ), index );
}

void Window::statusTab_stopButtonClicked( ) {
    debug( "+ Window::statusTab_stopButtonClicked\n" );
    _statusTab->setStopButtonEnabled( false );
    if ( _printManager ) {
        _printManager->abort( );
    }
}

void Window::statusTab_reprintButtonClicked( ) {
    _startPrinting( );
}

void Window::statusTab_cleanUpAfterPrint( ) {
    debug( "+ Window::statusTab_cleanUpAfterPrint: is model rendered? %s; is printer prepared? %s\n", ToString( _isModelRendered ), ToString( _isPrinterPrepared ) );

    if ( _printManager ) {
        _printManager->deleteLater( );
        _printManager = nullptr;
        emit printManagerChanged( nullptr );
    }

    _printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    _statusTab->setReprintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    _statusTab->setStopButtonEnabled( false );
}

#if defined _DEBUG
void Window::signalHandler_signalReceived( int const signalNumber ) {
    debug( "+ Window::signalHandler_signalReceived: received signal %s [%d]\n", ::strsignal( signalNumber ), signalNumber );

    if ( SIGUSR1 == signalNumber ) {
        debug( "+ Window::signalHandler_signalReceived: object information dump:\n" );
        dumpObjectInfo( );
        debug( "+ Window::signalHandler_signalReceived: object tree dump:\n" );
        dumpObjectTree( );
    } else {
        close( );
    }
}
#else
void Window::signalHandler_signalReceived( int const signalNumber ) {
    close( );
}
#endif // defined _DEBUG
