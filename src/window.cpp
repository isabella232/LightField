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

    class TabIndex {
    public:
        enum {
            Select,
            Prepare,
            Print,
            Status,
        };
    };

}

Window::Window( QWidget *parent ): QMainWindow( parent ) {
    QObject::connect( g_signalHandler, &SignalHandler::quit, this, &Window::signalHandler_quit );

    printJob   = new PrintJob;
    selectTab  = new SelectTab;
    prepareTab = new PrepareTab;
    printTab   = new PrintTab;
    statusTab  = new StatusTab;

    setWindowFlags( windowFlags( ) | Qt::BypassWindowManagerHint );
    setFixedSize( 800, 480 );
    move( { 0, g_settings.startY } );

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
    QObject::connect( prepareTab, &PrepareTab::sliceStarting,  this,       &Window::prepareTab_sliceStarting  );
    QObject::connect( prepareTab, &PrepareTab::sliceComplete,  this,       &Window::prepareTab_sliceComplete  );
    QObject::connect( prepareTab, &PrepareTab::renderStarting, this,       &Window::prepareTab_renderStarting );
    QObject::connect( prepareTab, &PrepareTab::renderComplete, this,       &Window::prepareTab_renderComplete );
    QObject::connect( this,       &Window::printJobChanged,    prepareTab, &PrepareTab::setPrintJob           );

    //
    // "Print" tab
    //

    printTab->setContentsMargins( { } );
    printTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    printTab->setPrintJob( printJob );
    QObject::connect( printTab, &PrintTab::printButtonClicked, this,     &Window::printTab_printButtonClicked );
    QObject::connect( printTab, &PrintTab::adjustBedHeight,    this,     &Window::printTab_adjustBedHeight    );
    QObject::connect( this,     &Window::printJobChanged,      printTab, &PrintTab::setPrintJob               );

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

    tabs->setContentsMargins( { } );
    tabs->addTab( selectTab,  "Select"  );
    tabs->addTab( prepareTab, "Prepare" );
    tabs->addTab( printTab,   "Print"   );
    tabs->addTab( statusTab,  "Status"  );
    tabs->setCurrentIndex( TabIndex::Select );

    setCentralWidget( tabs );
}

Window::~Window( ) {
    if ( g_signalHandler ) {
        QObject::disconnect( g_signalHandler, &SignalHandler::quit, this, &Window::signalHandler_quit );
    }
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

void Window::selectTab_modelSelected( bool success, QString const& fileName ) {
    debug( "+ Window::selectTab_modelSelected: success: %s, fileName: '%s'\n", success ? "true" : "false", fileName.toUtf8( ).data( ) );
    if ( success ) {
        prepareTab->setSliceButtonEnabled( true );
        printJob->modelFileName = fileName;
        tabs->setCurrentIndex( TabIndex::Prepare );
    } else {
        prepareTab->setSliceButtonEnabled( false );
    }
}

void Window::prepareTab_sliceStarting( ) {
    debug( "+ Window::prepareTab_sliceStarting\n" );
    prepareTab->setSliceButtonEnabled( false );
    printTab->setPrintButtonEnabled( false );
}

void Window::prepareTab_sliceComplete( bool success ) {
    debug( "+ Window::prepareTab_sliceComplete: success: %s\n", success ? "true" : "false" );
    if ( !success ) {
        return;
    }
}

void Window::prepareTab_renderStarting( ) {
    debug( "+ Window::prepareTab_renderStarting\n" );
}

void Window::prepareTab_renderComplete( bool success ) {
    debug( "+ Window::prepareTab_renderComplete: success: %s\n", success ? "true" : "false" );
    if ( !success ) {
        return;
    }

    prepareTab->setSliceButtonEnabled( true );
    printTab->setPrintButtonEnabled( true );
    tabs->setCurrentIndex( TabIndex::Print );
}

void Window::printTab_printButtonClicked( ) {
    debug( "+ PrintTab::printButton_clicked\n" );
    tabs->setCurrentIndex( TabIndex::Status );

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

void Window::shepherd_adjustBedHeightMoveToComplete( bool success ) {
    debug( "+ Window::shepherd_adjustBedHeightMoveToComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_adjustBedHeightMoveToComplete );

    printTab->adjustBedHeightComplete( success );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Move to new bed height position failed."
        );
        return;
    }

    shepherd->doSend( "G92 X0" );
}

void Window::printTab_retractBuildPlatform( ) {
    debug( "+ Window::printTab_retractBuildPlatform\n" );

    QObject::connect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_retractBuildPlatformMoveToComplete );
    shepherd->doMoveTo( 50.0 );
}

void Window::shepherd_retractBuildPlatformMoveToComplete( bool success ) {
    debug( "+ Window::shepherd_retractBuildPlatformMoveToComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_retractBuildPlatformMoveToComplete );

    printTab->retractBuildPlatformComplete( success );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Retraction of build platform failed."
        );
    }
}

void Window::printTab_extendBuildPlatform( ) {
    debug( "+ Window::printTab_extendBuildPlatform\n" );

    QObject::connect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_extendBuildPlatformMoveToComplete );
    shepherd->doMoveTo( 0.1 );
}

void Window::shepherd_extendBuildPlatformMoveToComplete( bool success ) {
    debug( "+ Window::shepherd_extendBuildPlatformMoveToComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_extendBuildPlatformMoveToComplete );

    printTab->extendBuildPlatformComplete( success );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Extension of build platform failed."
        );
        return;
    }
}

void Window::printTab_moveBuildPlatformUp( ) {
    debug( "+ Window::printTab_moveBuildPlatformUp\n" );

    QObject::connect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_moveBuildPlatformUpMoveComplete );
    shepherd->doMove( 0.1 );
}

void Window::shepherd_moveBuildPlatformUpMoveComplete( bool success ) {
    debug( "+ Window::shepherd_moveBuildPlatformUpMoveComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_moveBuildPlatformUpMoveComplete );

    printTab->moveBuildPlatformUpComplete( success );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Moving build platform up failed."
        );
    }
}

void Window::printTab_moveBuildPlatformDown( ) {
    debug( "+ Window::printTab_moveBuildPlatformDown\n" );

    QObject::connect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_moveBuildPlatformDownMoveComplete );
    shepherd->doMove( -0.1 );
}

void Window::shepherd_moveBuildPlatformDownMoveComplete( bool success ) {
    debug( "+ Window::shepherd_moveBuildPlatformDownMoveComplete: %s\n", success ? "succeeded" : "failed" );
    QObject::disconnect( shepherd, &Shepherd::action_moveToComplete, this, &Window::shepherd_moveBuildPlatformDownMoveComplete );

    printTab->moveBuildPlatformUpComplete( success );

    if ( !success ) {
        QMessageBox::critical( this, "Error",
            "<b>Error:</b><br>"
            "Moving build platform down failed."
        );
    }
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
    statusTab->setStopButtonEnabled( false );
}

void Window::signalHandler_quit( int signalNumber ) {
    debug( "+ Window::signalHandler_quit: received signal %d\n", signalNumber );
    close( );
}
