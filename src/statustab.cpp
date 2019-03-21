#include "pch.h"

#include <numeric>

#include "statustab.h"

#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"
#include "strings.h"
#include "utils.h"

namespace {

    double const PrintSolutionRecommendedScaleFactor = 2.0;

    QStringList const PrinterInitializationCommands {
        "M18",
        //"M140 S40",
        "M106",
    };

}

StatusTab::StatusTab( QWidget* parent ): QWidget( parent ) {
    _initialShowEventFunc = std::bind( &StatusTab::_initialShowEvent, this );

    auto boldFont = ModifyFont( font( ), font( ).pointSizeF( ), QFont::Bold );

    printerStateLabel->setText( "Printer state:" );

    printerStateDisplay->setFont( boldFont );
    printerStateDisplay->setText( "offline" );

    projectorLampStateLabel->setText( "Projector state:" );

    projectorLampStateDisplay->setFont( boldFont );
    projectorLampStateDisplay->setText( "off" );

    jobStateLabel->setText( "Print job:" );

    jobStateDisplay->setFont( boldFont );
    jobStateDisplay->setText( "idle" );

    currentLayerLabel->setText( "Current layer:" );

    currentLayerDisplay->setFont( boldFont );

    elapsedTimeLabel->setText( "Elapsed time:" );

    elapsedTimeDisplay->setFont( boldFont );

    estimatedTimeLeftLabel->setText( "Time left:" );

    estimatedTimeLeftDisplay->setFont( boldFont );

    percentageCompleteLabel->setText( "Percent complete:" );

    percentageCompleteDisplay->setFont( boldFont );


    loadPrintSolutionLabel->setAlignment( Qt::AlignHCenter );
    loadPrintSolutionLabel->setTextFormat( Qt::RichText );
    loadPrintSolutionLabel->setWordWrap( true );

    printSolutionLoadedButton->setText( "Continue" );
    QObject::connect( printSolutionLoadedButton, &QPushButton::clicked, this, &StatusTab::printSolutionLoadedButton_clicked );

    loadPrintSolutionGroup->setTitle( "Dispense print solution" );
    loadPrintSolutionGroup->setContentsMargins( { } );
    loadPrintSolutionGroup->setEnabled( false );
    loadPrintSolutionGroup->setFixedWidth( MainButtonSize.width( ) );
    loadPrintSolutionGroup->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    loadPrintSolutionGroup->setLayout( WrapWidgetsInVBox( { nullptr, loadPrintSolutionLabel, nullptr, printSolutionLoadedButton, nullptr } ) );


    _warningHotImage = new QPixmap { QString { ":images/warning-hot.png" } };
    _warningUvImage  = new QPixmap { QString { ":images/warning-uv.png"  } };

    warningHotLabel->setAlignment( Qt::AlignCenter );
    warningHotLabel->setContentsMargins( { } );
    warningHotLabel->setFixedSize( 43, 43 );
    warningHotLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    warningUvLabel->setAlignment( Qt::AlignCenter );
    warningUvLabel->setContentsMargins( { } );
    warningUvLabel->setFixedSize( 43, 43 );
    warningUvLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );


    progressControlsLayout->setContentsMargins( { } );
    progressControlsLayout->addLayout( WrapWidgetsInHBox( { printerStateLabel,       nullptr, printerStateDisplay       } ) );
    progressControlsLayout->addLayout( WrapWidgetsInHBox( { projectorLampStateLabel, nullptr, projectorLampStateDisplay } ) );
    progressControlsLayout->addLayout( WrapWidgetsInHBox( { jobStateLabel,           nullptr, jobStateDisplay           } ) );
    progressControlsLayout->addLayout( WrapWidgetsInHBox( { currentLayerLabel,       nullptr, currentLayerDisplay       } ) );
    progressControlsLayout->addLayout( WrapWidgetsInHBox( { elapsedTimeLabel,        nullptr, elapsedTimeDisplay        } ) );
    progressControlsLayout->addLayout( WrapWidgetsInHBox( { estimatedTimeLeftLabel,  nullptr, estimatedTimeLeftDisplay  } ) );
    progressControlsLayout->addLayout( WrapWidgetsInHBox( { percentageCompleteLabel, nullptr, percentageCompleteDisplay } ) );
    progressControlsLayout->addWidget( loadPrintSolutionGroup );
    progressControlsLayout->addStretch( );
    progressControlsLayout->addLayout( WrapWidgetsInHBox( { nullptr, warningHotLabel, nullptr, warningUvLabel, nullptr } ) );
    progressControlsLayout->addStretch( );

    progressControlsContainer->setContentsMargins( { } );
    progressControlsContainer->setLayout( progressControlsLayout );
    progressControlsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );


    currentLayerImage->setAlignment( Qt::AlignCenter );
    currentLayerImage->setContentsMargins( { } );
    currentLayerImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    currentLayerImage->setStyleSheet( QString( "QWidget { background: black }" ) );

    currentLayerLayout = WrapWidgetInVBox( currentLayerImage );
    currentLayerLayout->setAlignment( Qt::AlignCenter );
    currentLayerLayout->setContentsMargins( { } );

    currentLayerGroup->setTitle( "Current layer" );
    currentLayerGroup->setContentsMargins( { } );
    currentLayerGroup->setMinimumSize( MaximalRightHandPaneSize );
    currentLayerGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    currentLayerGroup->setLayout( currentLayerLayout );


    {
        _stopButtonEnabledPalette  =  stopButton->palette( );
        _stopButtonDisabledPalette = _stopButtonEnabledPalette;
        _stopButtonEnabledPalette.setColor( QPalette::Button,     Qt::red    );
        _stopButtonEnabledPalette.setColor( QPalette::ButtonText, Qt::yellow );
    }
    stopButton->setEnabled( false );
    stopButton->setFixedSize( MainButtonSize );
    stopButton->setFont( ModifyFont( stopButton->font( ), 22.25, QFont::Bold ) );
    stopButton->setText( "STOP" );
    QObject::connect( stopButton, &QPushButton::clicked, this, &StatusTab::stopButton_clicked );

    _layout->setContentsMargins( { } );
    _layout->addWidget( progressControlsContainer, 0, 0, 1, 1 );
    _layout->addWidget( stopButton,                1, 0, 1, 1 );
    _layout->addWidget( currentLayerGroup,         0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );

    _updatePrintTimeInfo = new QTimer( this );
    _updatePrintTimeInfo->setInterval( 1000 );
    _updatePrintTimeInfo->setSingleShot( false );
    _updatePrintTimeInfo->setTimerType( Qt::PreciseTimer );
    QObject::connect( _updatePrintTimeInfo, &QTimer::timeout, this, &StatusTab::updatePrintTimeInfo_timeout );
}

StatusTab::~StatusTab( ) {
    /*empty*/
}

void StatusTab::showEvent( QShowEvent* event ) {
    if ( _initialShowEventFunc ) {
        _initialShowEventFunc( );
        _initialShowEventFunc = nullptr;
    }
    event->ignore( );
}

void StatusTab::_initialShowEvent( ) {
    currentLayerImage->setFixedWidth( currentLayerImage->width( ) );
    currentLayerImage->setFixedHeight( currentLayerImage->width( ) / AspectRatio16to10 + 0.5 );

    debug(
        "+ StatusTab::_initialShowEvent:\n"
        "  + warningHotImage size: %s\n"
        "  + warningUvImage size:  %s\n"
        "  + warningHotLabel size: %s\n"
        "  + warningUvLabel size:  %s\n"
        "",
        ToString( _warningHotImage->size( ) ).toUtf8( ).data( ),
        ToString( _warningUvImage ->size( ) ).toUtf8( ).data( ),
        ToString(  warningHotLabel->size( ) ).toUtf8( ).data( ),
        ToString(   warningUvLabel->size( ) ).toUtf8( ).data( )
    );
}

void StatusTab::printer_online( ) {
    debug( "+ StatusTab::printer_online\n" );
    _isPrinterOnline = true;
    printerStateDisplay->setText( "online" );

    if ( PrinterInitializationCommands.isEmpty( ) || _isFirstOnlineTaskDone ) {
        return;
    }

    debug( "+ StatusTab::printer_online: printer has come online for the first time; sending initialization commands\n" );
    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &StatusTab::initializationCommands_sendComplete );
    _shepherd->doSend( PrinterInitializationCommands );
}

void StatusTab::printer_offline( ) {
    debug( "+ StatusTab::printer_offline\n" );
    _isPrinterOnline = false;
    printerStateDisplay->setText( "offline" );
}

void StatusTab::stopButton_clicked( bool ) {
    debug( "+ StatusTab::stopButton_clicked\n" );
    _updatePrintTimeInfo->stop( );
    emit stopButtonClicked( );
}

void StatusTab::printManager_printStarting( ) {
    debug( "+ StatusTab::printManager_printStarting\n" );
    jobStateDisplay->setText( "print started" );
    estimatedTimeLeftDisplay->setText( QString( "calculating..." ) );
}

void StatusTab::printManager_startingLayer( int const layer ) {
    debug( "+ StatusTab::printManager_startingLayer: layer %d/%d\n", layer + 1, _printJob->layerCount );
    currentLayerDisplay->setText( QString( "%1/%2" ).arg( layer + 1 ).arg( _printJob->layerCount ) );

    _previousLayerStartTime = _currentLayerStartTime;
    _currentLayerStartTime  = GetBootTimeClock( );

    if ( 0 == layer ) {
        _printJobStartTime = _currentLayerStartTime;
        _updatePrintTimeInfo->start( );
    } else {
        auto delta = _currentLayerStartTime - _previousLayerStartTime;
        debug( "  + layer time: %.3f\n", delta );
        _layerElapsedTimes.emplace_back( delta );

        auto average = std::accumulate<std::vector<double>::iterator, double>( _layerElapsedTimes.begin( ), _layerElapsedTimes.end( ), 0 ) / _layerElapsedTimes.size( );
        debug( "  + average:    %.3f\n", average );

        _estimatedPrintJobTime = average * _printJob->layerCount;
        debug( "  + estimate:   %.3f\n", _estimatedPrintJobTime );
    }

    percentageCompleteDisplay->setText( QString( "%1%" ).arg( static_cast<int>( static_cast<double>( _printManager->currentLayer( ) + 1 ) / static_cast<double>( _printJob->layerCount ) * 100.0 + 0.5 ) ) );

    auto pixmap = QPixmap( _printJob->jobWorkingDirectory + QString( "/%2.png" ).arg( layer, 6, 10, DigitZero ) );
    if ( ( pixmap.width( ) > currentLayerImage->width( ) ) || ( pixmap.height( ) > currentLayerImage->height( ) ) ) {
        pixmap = pixmap.scaled( currentLayerImage->size( ), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }
    currentLayerImage->setPixmap( pixmap );
}

void StatusTab::printManager_lampStatusChange( bool const on ) {
    debug( "+ StatusTab::printManager_lampStatusChange: lamp %s\n", on ? "ON" : "off" );
    projectorLampStateDisplay->setText( QString( on ? "ON" : "off" ) );
    if ( on ) {
        warningUvLabel->setPixmap( *_warningUvImage );
    } else {
        warningUvLabel->clear( );
    }
}

void StatusTab::printManager_printComplete( bool const success ) {
    debug( "+ StatusTab::printManager_printComplete: %s\n", success ? "print complete" : "print failed" );

    _updatePrintTimeInfo->stop( );

    jobStateDisplay->setText( QString( success ? "print complete" : "print failed" ) );
    currentLayerDisplay->clear( );
    estimatedTimeLeftDisplay->clear( );
    percentageCompleteDisplay->clear( );
    currentLayerImage->clear( );

    emit printComplete( );
}

void StatusTab::printManager_printAborted( ) {
    debug( "+ StatusTab::printManager_printAborted\n" );

    _updatePrintTimeInfo->stop( );

    jobStateDisplay->setText( QString( "print aborted" ) );
    currentLayerDisplay->clear( );
    estimatedTimeLeftDisplay->clear( );
    percentageCompleteDisplay->clear( );
    currentLayerImage->clear( );

    emit printComplete( );
}

void StatusTab::shepherd_temperatureReport( double const bedCurrentTemperature, double const /*bedTargetTemperature*/, int const /*bedPwm*/ ) {
    if ( bedCurrentTemperature >= 30.0 ) {
        warningHotLabel->setPixmap( *_warningHotImage );
    } else {
        warningHotLabel->clear( );
    }
}

void StatusTab::initializationCommands_sendComplete( bool const success ) {
    debug( "+ StatusTab::initializationCommands_sendComplete: success %s\n", ToString( success ) );
    QObject::disconnect( _shepherd, &Shepherd::action_sendComplete, this, &StatusTab::initializationCommands_sendComplete );

    if ( success ) {
        debug( "+ StatusTab::setFanSpeed_sendComplete: first-online tasks completed\n" );
        _isFirstOnlineTaskDone = true;
    }
}

void StatusTab::updatePrintTimeInfo_timeout( ) {
    if ( !_printManager ) {
        debug( "+ StatusTab::updatePrintTimeInfo_timeout: no print manager. don't know why timer is even running, but stopping it.\n" );
        _updatePrintTimeInfo->stop( );
        return;
    }

    double delta = GetBootTimeClock( ) - _printJobStartTime;
    debug( "+ StatusTab::updatePrintTimeInfo_timeout: delta %f\n", delta );
    elapsedTimeDisplay->setText( TimeDeltaToString( delta ) );
    estimatedTimeLeftDisplay->setText( TimeDeltaToString( _estimatedPrintJobTime - delta ) );
}

void StatusTab::printManager_requestLoadPrintSolution( ) {
    loadPrintSolutionLabel->setText( QString( "Dispense <b>%1 mL</b> of print solution, then tap <b>Continue</b> to start printing." ).arg( std::max( 1.0, PrintSolutionRecommendedScaleFactor * _printJob->estimatedVolume / 1000.0 ), 0, 'f', 2 ) );
    loadPrintSolutionGroup->setEnabled( true );
}

void StatusTab::printSolutionLoadedButton_clicked( bool ) {
    loadPrintSolutionGroup->setEnabled( false );

    _printManager->printSolutionLoaded( );
}

void StatusTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ StatusTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;
}

void StatusTab::setPrintManager( PrintManager* printManager ) {
    debug( "+ StatusTab::setPrintManager: old %p, new %p\n", _printManager, printManager );
    if ( _printManager ) {
        QObject::disconnect( _printManager, nullptr, this, nullptr );
    }

    _printManager = printManager;

    if ( _printManager ) {
        QObject::connect( _printManager, &PrintManager::printStarting,            this, &StatusTab::printManager_printStarting            );
        QObject::connect( _printManager, &PrintManager::printComplete,            this, &StatusTab::printManager_printComplete            );
        QObject::connect( _printManager, &PrintManager::printAborted,             this, &StatusTab::printManager_printAborted             );
        QObject::connect( _printManager, &PrintManager::startingLayer,            this, &StatusTab::printManager_startingLayer            );
        QObject::connect( _printManager, &PrintManager::lampStatusChange,         this, &StatusTab::printManager_lampStatusChange         );
        QObject::connect( _printManager, &PrintManager::requestLoadPrintSolution, this, &StatusTab::printManager_requestLoadPrintSolution );
    }
}

void StatusTab::setShepherd( Shepherd* newShepherd ) {
    if ( _shepherd ) {
        QObject::disconnect( _shepherd, nullptr, this, nullptr );
    }

    _shepherd = newShepherd;

    if ( _shepherd ) {
        QObject::connect( _shepherd, &Shepherd::printer_online,  this, &StatusTab::printer_online  );
        QObject::connect( _shepherd, &Shepherd::printer_offline, this, &StatusTab::printer_offline );
    }
}

void StatusTab::setStopButtonEnabled( bool value ) {
    stopButton->setEnabled( value );
    stopButton->setPalette( value ? _stopButtonEnabledPalette : _stopButtonDisabledPalette );
}
