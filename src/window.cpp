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

Window::Window( QWidget *parent ): QMainWindow( parent ) {
    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
    setFixedSize( MainWindowSize );
    move( g_settings.mainWindowPosition );

    QObject::connect( g_signalHandler, &SignalHandler::signalReceived, this, &Window::signalHandler_signalReceived );
    g_signalHandler->subscribe( signalList );

    _initialShowEventFunc = std::bind( &Window::_initialShowEvent, this );

#if defined _DEBUG
    if ( g_settings.pretendPrinterIsPrepared ) {
        _isPrinterPrepared = true;
    }
#endif // _DEBUG

    _fileTab        = new FileTab;
    _prepareTab     = new PrepareTab;
    _printTab       = new PrintTab;
    _statusTab      = new StatusTab;
    _advancedTab    = new AdvancedTab;
    _maintenanceTab = new MaintenanceTab;
    _printJob       = new PrintJob;
    _shepherd       = new Shepherd { parent };

    QObject::connect( this, &Window::printJobChanged,     _fileTab,        &FileTab::setPrintJob            );
    QObject::connect( this, &Window::printJobChanged,     _prepareTab,     &PrepareTab::setPrintJob         );
    QObject::connect( this, &Window::printJobChanged,     _printTab,       &PrintTab::setPrintJob           );
    QObject::connect( this, &Window::printJobChanged,     _statusTab,      &StatusTab::setPrintJob          );
    QObject::connect( this, &Window::printJobChanged,     _advancedTab,    &AdvancedTab::setPrintJob        );
    QObject::connect( this, &Window::printJobChanged,     _maintenanceTab, &MaintenanceTab::setPrintJob     );

    QObject::connect( this, &Window::printManagerChanged, _fileTab,        &FileTab::setPrintManager        );
    QObject::connect( this, &Window::printManagerChanged, _prepareTab,     &PrepareTab::setPrintManager     );
    QObject::connect( this, &Window::printManagerChanged, _printTab,       &PrintTab::setPrintManager       );
    QObject::connect( this, &Window::printManagerChanged, _statusTab,      &StatusTab::setPrintManager      );
    QObject::connect( this, &Window::printManagerChanged, _advancedTab,    &AdvancedTab::setPrintManager    );
    QObject::connect( this, &Window::printManagerChanged, _maintenanceTab, &MaintenanceTab::setPrintManager );

    QObject::connect( this, &Window::shepherdChanged,     _fileTab,        &FileTab::setShepherd            );
    QObject::connect( this, &Window::shepherdChanged,     _prepareTab,     &PrepareTab::setShepherd         );
    QObject::connect( this, &Window::shepherdChanged,     _printTab,       &PrintTab::setShepherd           );
    QObject::connect( this, &Window::shepherdChanged,     _statusTab,      &StatusTab::setShepherd          );
    QObject::connect( this, &Window::shepherdChanged,     _advancedTab,    &AdvancedTab::setShepherd        );
    QObject::connect( this, &Window::shepherdChanged,     _maintenanceTab, &MaintenanceTab::setShepherd     );

    QObject::connect( _shepherd, &Shepherd::shepherd_started,     this, &Window::shepherd_started     );
    QObject::connect( _shepherd, &Shepherd::shepherd_startFailed, this, &Window::shepherd_startFailed );
    QObject::connect( _shepherd, &Shepherd::shepherd_terminated,  this, &Window::shepherd_terminated  );

    _shepherd->start( );

    emit printJobChanged( _printJob );
    emit shepherdChanged( _shepherd );

    //
    // "Select" tab
    //

    _fileTab->setContentsMargins( { } );
    _fileTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( _fileTab, &FileTab::modelSelected,        this, &Window::fileTab_modelSelected        );
    QObject::connect( _fileTab, &FileTab::modelSelectionFailed, this, &Window::fileTab_modelSelectionFailed );

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
    QObject::connect( _prepareTab, &PrepareTab::alreadySliced,          this, &Window::prepareTab_alreadySliced          );

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
    QObject::connect( _statusTab, &StatusTab::stopButtonClicked, this, &Window::statusTab_stopButtonClicked );
    QObject::connect( _statusTab, &StatusTab::printComplete,     this, &Window::statusTab_cleanUpAfterPrint );

    //
    // "Advanced" tab
    //

    _advancedTab->setContentsMargins( { } );
    _advancedTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // Tab widget
    //

    QObject::connect( _tabs, &QTabWidget::currentChanged, this, &Window::tabs_currentChanged );
    double pointSize = _tabs->font( ).pointSizeF( );
    _tabs->setFont( ModifyFont( _tabs->font( ), 22.0 ) );
    _tabs->setContentsMargins( { } );
    auto font9pt = ModifyFont( _fileTab->font( ), pointSize );
    _tabs->addTab( _fileTab,        "File"        ); _fileTab       ->setFont( font9pt );
    _tabs->addTab( _prepareTab,     "Prepare"     ); _prepareTab    ->setFont( font9pt );
    _tabs->addTab( _printTab,       "Print"       ); _printTab      ->setFont( font9pt );
    _tabs->addTab( _statusTab,      "Status"      ); _statusTab     ->setFont( font9pt );
    _tabs->addTab( _advancedTab,    "Advanced"    ); _advancedTab   ->setFont( font9pt );
    _tabs->addTab( _maintenanceTab, "Maintenance" ); _maintenanceTab->setFont( font9pt );
    _tabs->setCurrentIndex( +TabIndex::File );

    setCentralWidget( _tabs );
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

void Window::showEvent( QShowEvent* event ) {
    //if ( _initialShowEventFunc ) {
    //    _initialShowEventFunc( );
    //    _initialShowEventFunc = nullptr;
    //} else {
        event->ignore( );
    //}
}

void Window::_initialShowEvent( ) {
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
    debug(
        "+ Window::fileTab_modelSelected:\n"
        "  + file name:        '%s'\n"
        "  + vertex count:     %5zu\n"
        "  + X min, max, size: %.2f..%.2f, %.2f\n"
        "  + Y min, max, size: %.2f..%.2f, %.2f\n"
        "  + Z min, max, size: %.2f..%.2f, %.2f\n"
        "  + estimated volume: %.2f mL\n"
        "",
        modelSelection->fileName.toUtf8( ).data( ),
        modelSelection->vertexCount,
        modelSelection->x.min, modelSelection->x.max, modelSelection->x.size,
        modelSelection->y.min, modelSelection->y.max, modelSelection->y.size,
        modelSelection->z.min, modelSelection->z.max, modelSelection->z.size,
        modelSelection->estimatedVolume
    );
    _isModelRendered = false;
    debug( "  + isModelRendered set to false.\n" );

    _printJob->vertexCount     = modelSelection->vertexCount;
    _printJob->x               = modelSelection->x;
    _printJob->y               = modelSelection->y;
    _printJob->z               = modelSelection->z;
    _printJob->estimatedVolume = modelSelection->estimatedVolume;
    _printJob->modelFileName   = modelSelection->fileName;

    if ( _tabs->currentIndex( ) == +TabIndex::File ) {
        _tabs->setCurrentIndex( +TabIndex::Prepare );
    }
    _prepareTab->modelSelected( );
}

void Window::fileTab_modelSelectionFailed( ) {
    debug( "+ Window::fileTab_modelSelectionFailed\n" );
    _prepareTab->resetState( );
    _prepareTab->setSliceButtonEnabled( false );
    if ( !_isModelRendered ) {
        _printTab->setPrintButtonEnabled( false );
    }
}

void Window::prepareTab_sliceStarted( ) {
    debug( "+ Window::prepareTab_sliceStarted\n" );
    _isModelRendered = false;
    debug( "  + isModelRendered set to false.\n" );

    _prepareTab->setSliceButtonEnabled( false );
    _printTab->setPrintButtonEnabled( false );
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

    _prepareTab->setSliceButtonEnabled( true );
    _printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    if ( success && ( _tabs->currentIndex( ) == +TabIndex::Prepare ) && _isPrinterPrepared ) {
        _tabs->setCurrentIndex( +TabIndex::Print );
    }
}

void Window::prepareTab_preparePrinterStarted( ) {
    debug( "+ Window::prepareTab_preparePrinterStarted\n" );
    _isPrinterPrepared = false;
}

void Window::prepareTab_preparePrinterComplete( bool const success ) {
    debug( "+ Window::prepareTab_preparePrinterComplete\n" );
#if defined _DEBUG
    _isPrinterPrepared = g_settings.pretendPrinterIsPrepared ? true : success;
#else
    _isPrinterPrepared = success;
#endif // _DEBUG
    debug( "  + isPrinterPrepared set to %s.\n", ToString( success ) );

    _printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    if ( success && ( _tabs->currentIndex( ) == +TabIndex::Prepare ) && _isModelRendered ) {
        _tabs->setCurrentIndex( +TabIndex::Print );
    }
}

void Window::prepareTab_alreadySliced( ) {
    debug( "  + Window::prepareTab_alreadySliced\n" );
    _isModelRendered = true;
    _printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    if ( ( _tabs->currentIndex( ) == +TabIndex::Prepare ) && _isPrinterPrepared ) {
        _tabs->setCurrentIndex( +TabIndex::Print );
    }
}

void Window::printTab_printButtonClicked( ) {
    debug( "+ Window::printTab_printButtonClicked\n" );
    _tabs->setCurrentIndex( +TabIndex::Status );

    debug(
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

    PrintJob* newJob = new PrintJob( *_printJob );
    _printManager = new PrintManager( _shepherd, this );

    _statusTab->setPrintManager( _printManager );
    _printManager->print( _printJob );

    _printJob = newJob;
    emit printJobChanged( _printJob );

    _prepareTab->setPrepareButtonEnabled( false );
    _printTab->setPrintButtonEnabled( false );
    _statusTab->setStopButtonEnabled( true );
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

void Window::statusTab_cleanUpAfterPrint( ) {
    debug( "+ Window::statusTab_cleanUpAfterPrint\n" );
    if ( _printManager ) {
        _printManager->deleteLater( );
        _printManager = nullptr;
        _statusTab->setPrintManager( nullptr );
    }

    debug( "+ Window::statusTab_cleanUpAfterPrint: is model rendered? %s; is printer prepared? %s\n", ToString( _isModelRendered ), ToString( _isPrinterPrepared ) );
    _prepareTab->setPrepareButtonEnabled( true );
    _printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
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
