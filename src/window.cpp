#include "pch.h"

#include "window.h"

#include "signalhandler.h"
#include "strings.h"
#include "constants.h"
#include "debug.h"

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

    move( { 0, g_settings.debuggingPosition ? 560 : 800 } );
    if ( g_settings.fullScreen ) {
        showFullScreen( );
    } else {
        setFixedSize( 800, 480 );
    }

    shepherd = new Shepherd( parent );
    QObject::connect( shepherd, &Shepherd::shepherd_Started,      this,      &Window::shepherd_Started      );
    QObject::connect( shepherd, &Shepherd::shepherd_Finished,     this,      &Window::shepherd_Finished     );
    QObject::connect( shepherd, &Shepherd::shepherd_ProcessError, this,      &Window::shepherd_ProcessError );
    QObject::connect( shepherd, &Shepherd::printer_Online,        statusTab, &StatusTab::printer_Online     );
    QObject::connect( shepherd, &Shepherd::printer_Offline,       statusTab, &StatusTab::printer_Offline    );
    shepherd->start( );

    //
    // "Select" tab
    //

    selectTab->setContentsMargins( { } );
    selectTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    selectTab->setPrintJob( printJob );
    QObject::connect( selectTab, &SelectTab::modelSelected, this, &Window::selectTab_modelSelected );
    QObject::connect( this, &Window::printJobChanged, selectTab, &SelectTab::setPrintJob );

    //
    // "Prepare" tab
    //

    prepareTab->setContentsMargins( { } );
    prepareTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    prepareTab->setPrintJob( printJob );
    QObject::connect( prepareTab, &PrepareTab::sliceStarting,  this, &Window::prepareTab_sliceStarting  );
    QObject::connect( prepareTab, &PrepareTab::sliceComplete,  this, &Window::prepareTab_sliceComplete  );
    QObject::connect( prepareTab, &PrepareTab::renderStarting, this, &Window::prepareTab_renderStarting );
    QObject::connect( prepareTab, &PrepareTab::renderComplete, this, &Window::prepareTab_renderComplete );
    QObject::connect( this, &Window::printJobChanged, prepareTab, &PrepareTab::setPrintJob );

    //
    // "Print" tab
    //

    printTab->setContentsMargins( { } );
    printTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    printTab->setPrintJob( printJob );
    QObject::connect( printTab, &PrintTab::printButtonClicked, this, &Window::printTab_printButtonClicked );
    QObject::connect( this, &Window::printJobChanged, printTab, &PrintTab::setPrintJob );

    //
    // "Status" tab
    //

    statusTab->setContentsMargins( { } );
    statusTab->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    statusTab->setPrintJob( printJob );
    QObject::connect( statusTab, &StatusTab::stopButtonClicked, this, &Window::statusTab_stopButtonClicked );

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
    fprintf( stderr, "+ Window::closeEvent\n" );
    if ( printManager ) {
        printManager->terminate( );
    }
    shepherd->doTerminate( );
    event->accept( );
}

void Window::shepherd_Started( ) {
    fprintf( stderr, "+ Window::shepherd_Started\n" );
}

void Window::shepherd_Finished( int exitCode, QProcess::ExitStatus exitStatus ) {
    fprintf( stderr, "+ Window::shepherd_Finished: exitStatus %d, exitCode %d\n", exitStatus, exitCode );
}

void Window::shepherd_ProcessError( QProcess::ProcessError error ) {
    fprintf( stderr, "+ Window::shepherd_ProcessError: %d\n", error );
}

void Window::selectTab_modelSelected( bool success, QString const& fileName ) {
    fprintf( stderr, "+ Window::selectTab_modelSelected: success: %s, fileName: '%s'\n", success ? "true" : "false", fileName.toUtf8( ).data( ) );
    if ( success ) {
        prepareTab->setSliceButtonEnabled( true );
        printJob->modelFileName = fileName;
        tabs->setCurrentIndex( TabIndex::Prepare );
    } else {
        prepareTab->setSliceButtonEnabled( false );
    }
}

void Window::prepareTab_sliceStarting( ) {
    fprintf( stderr, "+ Window::prepareTab_sliceStarting\n" );
}

void Window::prepareTab_sliceComplete( bool success ) {
    fprintf( stderr, "+ Window::prepareTab_sliceComplete: success: %s\n", success ? "true" : "false" );
    if ( !success ) {
        return;
    }
}

void Window::prepareTab_renderStarting( ) {
    fprintf( stderr, "+ Window::prepareTab_renderStarting\n" );
}

void Window::prepareTab_renderComplete( bool success ) {
    fprintf( stderr, "+ Window::prepareTab_renderComplete: success: %s\n", success ? "true" : "false" );
    if ( !success ) {
        return;
    }

    printTab->setPrintButtonEnabled( true );
    tabs->setCurrentIndex( TabIndex::Print );
}

void Window::printTab_printButtonClicked( ) {
    fprintf( stderr, "+ PrintTab::printButton_clicked\n" );
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

    statusTab->setStopButtonEnabled( true );
}

void Window::statusTab_stopButtonClicked( ) {
    printManager->abortJob( );
}

void Window::signalHandler_quit( int signalNumber ) {
    fprintf( stderr, "+ Window::signalHandler_quit: received signal %d\n", signalNumber );
    close( );
}
