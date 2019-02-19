#include "pch.h"

#include "window.h"

#include "app.h"
#include "signalhandler.h"
#include "shepherd.h"
#include "printmanager.h"
#include "printjob.h"
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
    QObject::connect( g_signalHandler, &SignalHandler::signalReceived, this, &Window::signalHandler_signalReceived );
    g_signalHandler->subscribe( signalList );

    printJob   = new PrintJob;

    selectTab  = new SelectTab;
    prepareTab = new PrepareTab;
    printTab   = new PrintTab;
    statusTab  = new StatusTab;

    setWindowFlags( windowFlags( ) | ( g_settings.frameless ? Qt::FramelessWindowHint : Qt::BypassWindowManagerHint ) );
    setFixedSize( MainWindowSize );
    move( g_settings.mainWindowPosition );

    shepherd = new Shepherd( parent );
    QObject::connect( shepherd, &Shepherd::shepherd_started,      this,      &Window::shepherd_started      );
    QObject::connect( shepherd, &Shepherd::shepherd_finished,     this,      &Window::shepherd_finished     );
    QObject::connect( shepherd, &Shepherd::shepherd_processError, this,      &Window::shepherd_processError );
    QObject::connect( shepherd, &Shepherd::printer_online,        statusTab, &StatusTab::printer_online     );
    QObject::connect( shepherd, &Shepherd::printer_offline,       statusTab, &StatusTab::printer_offline    );
    shepherd->start( );

    //
    // "Select" tab
    //

    selectTab->setContentsMargins( { } );
    selectTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    selectTab->setPrintJob( printJob );
    QObject::connect( selectTab, &SelectTab::modelSelected, this,      &Window::selectTab_modelSelected );
    QObject::connect( this,      &Window::printJobChanged,  selectTab, &SelectTab::setPrintJob          );

    //
    // "Prepare" tab
    //

    prepareTab->setContentsMargins( { } );
    prepareTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    prepareTab->setPrintJob( printJob );
    prepareTab->setShepherd( shepherd );
    QObject::connect( prepareTab, &PrepareTab::sliceStarted,   this,       &Window::prepareTab_sliceStarted   );
    QObject::connect( prepareTab, &PrepareTab::sliceComplete,  this,       &Window::prepareTab_sliceComplete  );
    QObject::connect( prepareTab, &PrepareTab::renderStarted,  this,       &Window::prepareTab_renderStarted  );
    QObject::connect( prepareTab, &PrepareTab::renderComplete, this,       &Window::prepareTab_renderComplete );
    QObject::connect( this,       &Window::printJobChanged,    prepareTab, &PrepareTab::setPrintJob           );

    //
    // "Print" tab
    //

    printTab->setContentsMargins( { } );
    printTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    printTab->setPrintJob( printJob );
    QObject::connect( printTab, &PrintTab::printButtonClicked,    this,     &Window::printTab_printButtonClicked    );
    QObject::connect( printTab, &PrintTab::adjustBedHeight,       this,     &Window::printTab_adjustBedHeight       );
    QObject::connect( printTab, &PrintTab::raiseBuildPlatform,    this,     &Window::printTab_raiseBuildPlatform    );
    QObject::connect( printTab, &PrintTab::lowerBuildPlatform,    this,     &Window::printTab_lowerBuildPlatform    );
    QObject::connect( printTab, &PrintTab::homePrinter,           this,     &Window::printTab_homePrinter           );
    QObject::connect( printTab, &PrintTab::moveBuildPlatformUp,   this,     &Window::printTab_moveBuildPlatformUp   );
    QObject::connect( printTab, &PrintTab::moveBuildPlatformDown, this,     &Window::printTab_moveBuildPlatformDown );
    QObject::connect( this,     &Window::printJobChanged,         printTab, &PrintTab::setPrintJob                  );

    //
    // "Status" tab
    //

    statusTab->setContentsMargins( { } );
    statusTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    statusTab->setPrintJob( printJob );
    QObject::connect( statusTab, &StatusTab::stopButtonClicked, this,      &Window::statusTab_stopButtonClicked );
    QObject::connect( statusTab, &StatusTab::printComplete,     this,      &Window::statusTab_cleanUpAfterPrint );
    QObject::connect( this,      &Window::printJobChanged,      statusTab, &StatusTab::setPrintJob              );

    //
    // Tab widget
    //

    QObject::connect( tabs, &QTabWidget::currentChanged, this, &Window::tabs_currentChanged );
    tabs->setContentsMargins( { } );
    tabs->addTab( selectTab,      "Select"    );
    tabs->addTab( prepareTab,     "Prepare"   );
    tabs->addTab( printTab,       "Print"     );
    tabs->addTab( statusTab,      "Status"    );
    tabs->setCurrentIndex( +TabIndex::Select );

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

void Window::shepherd_started( ) {
    debug( "+ Window::shepherd_started\n" );
}

void Window::shepherd_finished( int exitCode, QProcess::ExitStatus exitStatus ) {
    debug( "+ Window::shepherd_finished: exitStatus %d, exitCode %d\n", exitStatus, exitCode );
}

void Window::shepherd_processError( QProcess::ProcessError error ) {
    debug( "+ Window::shepherd_processError: %d\n", error );
}

void Window::selectTab_modelSelected( bool const success, QString const& fileName ) {
    debug( "+ Window::selectTab_modelSelected: success: %s, fileName: '%s'\n", success ? "true" : "false", fileName.toUtf8( ).data( ) );
    if ( success ) {
        prepareTab->setSliceButtonEnabled( true );
        printJob->modelFileName = fileName;
        if ( tabs->currentIndex( ) == +TabIndex::Select ) {
            tabs->setCurrentIndex( +TabIndex::Prepare );
        }
    } else {
        prepareTab->setSliceButtonEnabled( false );
    }
}

void Window::prepareTab_sliceStarted( ) {
    debug( "+ Window::prepareTab_sliceStarted\n" );
    prepareTab->setSliceButtonEnabled( false );
    printTab->setPrintButtonEnabled( false );
}

void Window::prepareTab_sliceComplete( bool const success ) {
    debug( "+ Window::prepareTab_sliceComplete: success: %s\n", success ? "true" : "false" );
    if ( !success ) {
        return;
    }
}

void Window::prepareTab_renderStarted( ) {
    debug( "+ Window::prepareTab_renderStarted\n" );
}

void Window::prepareTab_renderComplete( bool const success ) {
    debug( "+ Window::prepareTab_renderComplete: success: %s\n", success ? "true" : "false" );
    if ( !success ) {
        return;
    }

    prepareTab->setSliceButtonEnabled( true );
    printTab->setPrintButtonEnabled( true );
    if ( tabs->currentIndex( ) == +TabIndex::Prepare ) {
        tabs->setCurrentIndex( +TabIndex::Print );
    }
}

void Window::printTab_printButtonClicked( ) {
    debug( "+ Window::printTab_printButtonClicked\n" );
    tabs->setCurrentIndex( +TabIndex::Status );

    fprintf( stderr,
        "  + Print job:\n"
        "    + modelFileName:     '%s'\n"
        "    + slicedSvgFileName: '%s'\n"
        "    + pngFilesPath:      '%s'\n"
        "    + layerCount:        %d\n"
        "    + layerThickness:    %d\n"
        "    + exposureTime:      %f\n"
        "    + powerLevel:        %d\n"
        "",
        printJob->modelFileName.toUtf8( ).data( ),
        printJob->slicedSvgFileName.toUtf8( ).data( ),
        printJob->pngFilesPath.toUtf8( ).data( ),
        printJob->layerCount,
        printJob->layerThickness,
        printJob->exposureTime,
        printJob->powerLevel
    );

    PrintJob* newJob = new PrintJob;
    *newJob = *printJob;

    printManager = new PrintManager( shepherd, this );
    QObject::connect( printManager, &PrintManager::printStarting,    statusTab, &StatusTab::printManager_printStarting    );
    QObject::connect( printManager, &PrintManager::startingLayer,    statusTab, &StatusTab::printManager_startingLayer    );
    QObject::connect( printManager, &PrintManager::lampStatusChange, statusTab, &StatusTab::printManager_lampStatusChange );
    QObject::connect( printManager, &PrintManager::printComplete,    statusTab, &StatusTab::printManager_printComplete    );
    printManager->print( printJob );

    printJob = newJob;
    emit printJobChanged( printJob );

    printTab->setPrintButtonEnabled( false );
    statusTab->setStopButtonEnabled( true );
}

void Window::printTab_adjustBedHeight( double const newHeight ) {
    debug( "+ Window::printTab_adjustBedHeight: new bed height %f\n", newHeight );

    QObject::connect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_adjustBedHeightMoveToComplete );
    shepherd->doMoveTo( newHeight );
}

void Window::shepherd_adjustBedHeightMoveToComplete( bool const success ) {
    debug( "+ Window::shepherd_adjustBedHeightMoveToComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_adjustBedHeightMoveToComplete );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Move to new bed height position failed."
        );
    } else {
        shepherd->doSend( "G92 X0" );
    }

    printTab->adjustBedHeightComplete( success );
}

void Window::printTab_raiseBuildPlatform( ) {
    debug( "+ Window::printTab_raiseBuildPlatform\n" );

    QObject::connect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_raiseBuildPlatformMoveToComplete );
    shepherd->doMoveTo( PrinterMaximumHeight );
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

void Window::printTab_moveBuildPlatformUp( ) {
    debug( "+ Window::printTab_moveBuildPlatformUp\n" );

    QObject::connect( shepherd, &Shepherd::action_moveComplete, this, &Window::shepherd_moveBuildPlatformUpMoveComplete );
    shepherd->doMove( 0.1 );
}

void Window::shepherd_moveBuildPlatformUpMoveComplete( bool const success ) {
    debug( "+ Window::shepherd_moveBuildPlatformUpMoveComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveComplete, this, &Window::shepherd_moveBuildPlatformUpMoveComplete );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Moving build platform up failed."
        );
    }

    printTab->moveBuildPlatformUpComplete( success );
}

void Window::printTab_moveBuildPlatformDown( ) {
    debug( "+ Window::printTab_moveBuildPlatformDown\n" );

    QObject::connect( shepherd, &Shepherd::action_moveComplete, this, &Window::shepherd_moveBuildPlatformDownMoveComplete );
    shepherd->doMove( -0.1 );
}

void Window::shepherd_moveBuildPlatformDownMoveComplete( bool const success ) {
    debug( "+ Window::shepherd_moveBuildPlatformDownMoveComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveComplete, this, &Window::shepherd_moveBuildPlatformDownMoveComplete );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Moving build platform down failed."
        );
    }

    printTab->moveBuildPlatformUpComplete( success );
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
    }
    printTab->setPrintButtonEnabled( false );
    statusTab->setStopButtonEnabled( false );
}

#if defined _DEBUG
void Window::signalHandler_signalReceived( int const signalNumber ) {
    debug( "+ Window::signalHandler_signalReceived: received signal %s [%d]\n", strsignal( signalNumber ), signalNumber );

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
