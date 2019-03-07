#include "pch.h"

#include "window.h"

#include "app.h"
#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"
#include "signalhandler.h"
#include "strings.h"
#include "utils.h"

#include "welcometab.h"
#include "selecttab.h"
#include "preparetab.h"
#include "printtab.h"
#include "statustab.h"

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

    //welcomeTab = new WelcomeTab;
    selectTab  = new SelectTab;
    prepareTab = new PrepareTab;
    printTab   = new PrintTab;
    statusTab  = new StatusTab;

    //QObject::connect( this, &Window::printJobChanged, welcomeTab, &WelcomeTab::setPrintJob );
    QObject::connect( this, &Window::printJobChanged, selectTab,  &SelectTab::setPrintJob  );
    QObject::connect( this, &Window::printJobChanged, prepareTab, &PrepareTab::setPrintJob );
    QObject::connect( this, &Window::printJobChanged, printTab,   &PrintTab::setPrintJob   );
    QObject::connect( this, &Window::printJobChanged, statusTab,  &StatusTab::setPrintJob  );

    printJob = new PrintJob;
    emit printJobChanged( printJob );

    //QObject::connect( this, &Window::shepherdChanged, welcomeTab, &WelcomeTab::setShepherd );
    QObject::connect( this, &Window::shepherdChanged, selectTab,  &SelectTab::setShepherd  );
    QObject::connect( this, &Window::shepherdChanged, prepareTab, &PrepareTab::setShepherd );
    QObject::connect( this, &Window::shepherdChanged, printTab,   &PrintTab::setShepherd   );
    QObject::connect( this, &Window::shepherdChanged, statusTab,  &StatusTab::setShepherd  );

    shepherd = new Shepherd( parent );
    QObject::connect( shepherd, &Shepherd::shepherd_started,     this, &Window::shepherd_started     );
    QObject::connect( shepherd, &Shepherd::shepherd_startFailed, this, &Window::shepherd_startFailed );
    QObject::connect( shepherd, &Shepherd::shepherd_terminated,  this, &Window::shepherd_terminated  );
    shepherd->start( );
    emit shepherdChanged( shepherd );

    //
    // "Welcome" tab
    //

    //welcomeTab->setContentsMargins( { } );
    //welcomeTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    //
    // "Select" tab
    //

    selectTab->setContentsMargins( { } );
    selectTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( selectTab, &SelectTab::modelSelected,        this, &Window::selectTab_modelSelected        );
    QObject::connect( selectTab, &SelectTab::modelSelectionFailed, this, &Window::selectTab_modelSelectionFailed );

    //
    // "Prepare" tab
    //

    prepareTab->setContentsMargins( { } );
    prepareTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    prepareTab->setShepherd( shepherd );
    QObject::connect( prepareTab, &PrepareTab::sliceStarted,           this, &Window::prepareTab_sliceStarted           );
    QObject::connect( prepareTab, &PrepareTab::sliceComplete,          this, &Window::prepareTab_sliceComplete          );
    QObject::connect( prepareTab, &PrepareTab::renderStarted,          this, &Window::prepareTab_renderStarted          );
    QObject::connect( prepareTab, &PrepareTab::renderComplete,         this, &Window::prepareTab_renderComplete         );
    QObject::connect( prepareTab, &PrepareTab::preparePrinterComplete, this, &Window::prepareTab_preparePrinterComplete );

    //
    // "Print" tab
    //

    printTab->setContentsMargins( { } );
    printTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( printTab, &PrintTab::printButtonClicked, this, &Window::printTab_printButtonClicked );
    QObject::connect( printTab, &PrintTab::raiseBuildPlatform, this, &Window::printTab_raiseBuildPlatform );
    QObject::connect( printTab, &PrintTab::lowerBuildPlatform, this, &Window::printTab_lowerBuildPlatform );
    QObject::connect( printTab, &PrintTab::homePrinter,        this, &Window::printTab_homePrinter        );

    //
    // "Status" tab
    //

    statusTab->setContentsMargins( { } );
    statusTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    QObject::connect( statusTab, &StatusTab::stopButtonClicked, this, &Window::statusTab_stopButtonClicked );
    QObject::connect( statusTab, &StatusTab::printComplete,     this, &Window::statusTab_cleanUpAfterPrint );

    //
    // Tab widget
    //

    QObject::connect( tabs, &QTabWidget::currentChanged, this, &Window::tabs_currentChanged );
    double pointSize = tabs->font( ).pointSizeF( );
    tabs->setFont( ModifyFont( tabs->font( ), 22.0 ) );
    tabs->setContentsMargins( { } );
    auto font9pt = ModifyFont( selectTab->font( ), pointSize );
    //tabs->addTab( welcomeTab, "Welcome" ); welcomeTab->setFont( font9pt );
    tabs->addTab( selectTab,  "Select"  ); selectTab ->setFont( font9pt );
    tabs->addTab( prepareTab, "Prepare" ); prepareTab->setFont( font9pt );
    tabs->addTab( printTab,   "Print"   ); printTab  ->setFont( font9pt );
    tabs->addTab( statusTab,  "Status"  ); statusTab ->setFont( font9pt );
    tabs->setCurrentIndex( +TabIndex::Select/*+TabIndex::Welcome*/ );

    setCentralWidget( tabs );
}

Window::~Window( ) {
    QObject::disconnect( g_signalHandler, nullptr, this, nullptr );
    g_signalHandler->unsubscribe( signalList );
}

void Window::closeEvent( QCloseEvent* event ) {
    debug( "+ Window::closeEvent\n" );
    if ( printManager ) {
        printManager->terminate( );
    }
    shepherd->doTerminate( );
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

void Window::selectTab_modelSelected( ModelSelectionInfo* modelSelection ) {
    debug(
        "+ Window::selectTab_modelSelected:\n"
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

    prepareTab->resetState( );
    prepareTab->setSliceButtonEnabled( true );
    printJob->vertexCount     = modelSelection->vertexCount;
    printJob->x               = modelSelection->x;
    printJob->y               = modelSelection->y;
    printJob->z               = modelSelection->z;
    printJob->estimatedVolume = modelSelection->estimatedVolume;
    printJob->modelFileName   = modelSelection->fileName;
    if ( tabs->currentIndex( ) == +TabIndex::Select ) {
        tabs->setCurrentIndex( +TabIndex::Prepare );
    }
}

void Window::selectTab_modelSelectionFailed( ) {
    debug( "+ Window::selectTab_modelSelectionFailed\n" );
    prepareTab->resetState( );
    prepareTab->setSliceButtonEnabled( false );
    if ( !_isModelRendered ) {
        printTab->setPrintButtonEnabled( false );
    }
}

void Window::prepareTab_sliceStarted( ) {
    debug( "+ Window::prepareTab_sliceStarted\n" );
    _isModelRendered = false;
    debug( "  + isModelRendered set to false.\n" );

    prepareTab->setSliceButtonEnabled( false );
    printTab->setPrintButtonEnabled( false );
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

    prepareTab->setSliceButtonEnabled( true );
    printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    if ( success && ( tabs->currentIndex( ) == +TabIndex::Prepare ) && _isPrinterPrepared ) {
        tabs->setCurrentIndex( +TabIndex::Print );
    }
}

void Window::prepareTab_preparePrinterComplete( bool const success ) {
    debug( "+ Window::prepareTab_renderStarted\n" );
#if defined _DEBUG
    _isPrinterPrepared = g_settings.pretendPrinterIsPrepared ? true : success;
#else
    _isPrinterPrepared = success;
#endif // _DEBUG
    debug( "  + isPrinterPrepared set to %s.\n", ToString( success ) );

    printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    if ( success && ( tabs->currentIndex( ) == +TabIndex::Prepare ) && _isModelRendered ) {
        tabs->setCurrentIndex( +TabIndex::Print );
    }
}

void Window::printTab_printButtonClicked( ) {
    debug( "+ Window::printTab_printButtonClicked\n" );
    tabs->setCurrentIndex( +TabIndex::Status );

    debug(
        "  + Print job:\n"
        "    + modelFileName:       '%s'\n"
        "    + jobWorkingDirectory: '%s'\n"
        "    + layerCount:          %d\n"
        "    + layerThickness:      %d\n"
        "    + exposureTime:        %f\n"
        "    + powerLevel:          %d\n"
        "",
        printJob->modelFileName.toUtf8( ).data( ),
        printJob->jobWorkingDirectory.toUtf8( ).data( ),
        printJob->layerCount,
        printJob->layerThickness,
        printJob->exposureTime,
        printJob->powerLevel
    );

    PrintJob* newJob = new PrintJob( *printJob );
    printManager = new PrintManager( shepherd, this );

    statusTab->setPrintManager( printManager );
    printManager->print( printJob );

    printJob = newJob;
    emit printJobChanged( printJob );

    prepareTab->setPrepareButtonEnabled( false );
    printTab->setPrintButtonEnabled( false );
    statusTab->setStopButtonEnabled( true );
}

void Window::printTab_raiseBuildPlatform( ) {
    debug( "+ Window::printTab_raiseBuildPlatform\n" );

    QObject::connect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_raiseBuildPlatformMoveToComplete );
    shepherd->doMoveTo( PrinterMaximumZ );
}

void Window::shepherd_raiseBuildPlatformMoveToComplete( bool const success ) {
    debug( "+ Window::shepherd_raiseBuildPlatformMoveToComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_raiseBuildPlatformMoveToComplete );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Raise of build platform failed."
        );
    }

    printTab->raiseBuildPlatformComplete( success );
}

void Window::printTab_homePrinter( ) {
    debug( "+ Window::printTab_homePrinter\n" );

    QObject::connect( shepherd, &Shepherd::action_homeComplete, this, &Window::shepherd_homeComplete );
    shepherd->doHome( );
}

void Window::shepherd_homeComplete( bool const success ) {
    debug( "+ Window::shepherd_homeComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_homeComplete, this, &Window::shepherd_homeComplete );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Homing failed."
        );
    }

    printTab->homeComplete( success );
}

void Window::printTab_lowerBuildPlatform( ) {
    debug( "+ Window::printTab_lowerBuildPlatform\n" );

    QObject::connect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_lowerBuildPlatformMoveToComplete );
    shepherd->doMoveTo( std::max( 100, printJob->layerThickness ) / 1000.0 );
}

void Window::shepherd_lowerBuildPlatformMoveToComplete( bool const success ) {
    debug( "+ Window::shepherd_lowerBuildPlatformMoveToComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_lowerBuildPlatformMoveToComplete );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Extension of build platform failed."
        );
    }

    printTab->lowerBuildPlatformComplete( success );
}

void Window::tabs_currentChanged( int index ) {
    debug( "+ Window::tabs_currentChanged: new tab is '%s' [%d]\n", ToString( static_cast<TabIndex>( index ) ), index );
}

void Window::statusTab_stopButtonClicked( ) {
    debug( "+ Window::statusTab_stopButtonClicked\n" );
    statusTab->setStopButtonEnabled( false );
    if ( printManager ) {
        printManager->abort( );
    }
}

void Window::statusTab_cleanUpAfterPrint( ) {
    debug( "+ Window::statusTab_cleanUpAfterPrint\n" );
    if ( printManager ) {
        printManager->deleteLater( );
        printManager = nullptr;
        statusTab->setPrintManager( nullptr );
    }

    debug( "+ Window::statusTab_cleanUpAfterPrint: is model rendered? %s; is printer prepared? %s\n", ToString( _isModelRendered ), ToString( _isPrinterPrepared ) );
    prepareTab->setPrepareButtonEnabled( true );
    printTab->setPrintButtonEnabled( _isModelRendered && _isPrinterPrepared );
    statusTab->setStopButtonEnabled( false );
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
