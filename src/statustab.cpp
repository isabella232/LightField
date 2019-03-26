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
        "M106",
        "M155 S5",
    };

}

StatusTab::StatusTab( QWidget* parent ): TabBase( parent ) {
    _initialShowEventFunc = std::bind( &StatusTab::_initialShowEvent, this, _1 );

    auto origFont = font( );
    auto boldFont = ModifyFont( origFont, QFont::Bold );
    auto font22pt = ModifyFont( origFont, 22.0 );


    _printerStateLabel->setText( "Printer state:" );

    _printerStateDisplay->setFont( boldFont );
    _printerStateDisplay->setText( "offline" );

    _projectorLampStateLabel->setText( "Projector state:" );

    _projectorLampStateDisplay->setFont( boldFont );
    _projectorLampStateDisplay->setText( "off" );

    _jobStateLabel->setText( "Print job:" );

    _jobStateDisplay->setFont( boldFont );
    _jobStateDisplay->setText( "idle" );

    _currentLayerLabel->setText( "Current layer:" );

    _currentLayerDisplay->setFont( boldFont );

    _elapsedTimeLabel->setText( "Elapsed time:" );

    _elapsedTimeDisplay->setFont( boldFont );

    _estimatedTimeLeftLabel->setText( "Time left:" );

    _estimatedTimeLeftDisplay->setFont( boldFont );

    _percentageCompleteLabel->setText( "Percent complete:" );

    _percentageCompleteDisplay->setFont( boldFont );


    _loadPrintSolutionLabel->setAlignment( Qt::AlignHCenter );
    _loadPrintSolutionLabel->setTextFormat( Qt::RichText );
    _loadPrintSolutionLabel->setWordWrap( true );

    _printSolutionLoadedButton->setText( "Continue" );
    QObject::connect( _printSolutionLoadedButton, &QPushButton::clicked, this, &StatusTab::printSolutionLoadedButton_clicked );

    _loadPrintSolutionGroup->setTitle( "Dispense print solution" );
    _loadPrintSolutionGroup->setContentsMargins( { } );
    _loadPrintSolutionGroup->setEnabled( false );
    _loadPrintSolutionGroup->setFixedWidth( MainButtonSize.width( ) );
    _loadPrintSolutionGroup->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    _loadPrintSolutionGroup->setLayout( WrapWidgetsInVBox( { nullptr, _loadPrintSolutionLabel, nullptr, _printSolutionLoadedButton, nullptr } ) );


    _warningHotImage = new QPixmap { QString { ":images/warning-hot.png" } };
    _warningUvImage  = new QPixmap { QString { ":images/warning-uv.png"  } };

    _warningHotLabel->setAlignment( Qt::AlignCenter );
    _warningHotLabel->setContentsMargins( { } );
    _warningHotLabel->setFixedSize( 43, 43 );
    _warningHotLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _warningUvLabel->setAlignment( Qt::AlignCenter );
    _warningUvLabel->setContentsMargins( { } );
    _warningUvLabel->setFixedSize( 43, 43 );
    _warningUvLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );


    _progressControlsLayout->setContentsMargins( { } );
    _progressControlsLayout->addLayout( WrapWidgetsInHBox( { _printerStateLabel,       nullptr, _printerStateDisplay       } ) );
    _progressControlsLayout->addLayout( WrapWidgetsInHBox( { _projectorLampStateLabel, nullptr, _projectorLampStateDisplay } ) );
    _progressControlsLayout->addLayout( WrapWidgetsInHBox( { _jobStateLabel,           nullptr, _jobStateDisplay           } ) );
    _progressControlsLayout->addLayout( WrapWidgetsInHBox( { _currentLayerLabel,       nullptr, _currentLayerDisplay       } ) );
    _progressControlsLayout->addLayout( WrapWidgetsInHBox( { _elapsedTimeLabel,        nullptr, _elapsedTimeDisplay        } ) );
    _progressControlsLayout->addLayout( WrapWidgetsInHBox( { _estimatedTimeLeftLabel,  nullptr, _estimatedTimeLeftDisplay  } ) );
    _progressControlsLayout->addLayout( WrapWidgetsInHBox( { _percentageCompleteLabel, nullptr, _percentageCompleteDisplay } ) );
    _progressControlsLayout->addWidget( _loadPrintSolutionGroup );
    _progressControlsLayout->addStretch( );
    _progressControlsLayout->addLayout( WrapWidgetsInHBox( { nullptr, _warningHotLabel, nullptr, _warningUvLabel, nullptr } ) );
    _progressControlsLayout->addStretch( );

    _progressControlsContainer->setContentsMargins( { } );
    _progressControlsContainer->setLayout( _progressControlsLayout );
    _progressControlsContainer->setFixedWidth( MainButtonSize.width( ) );
    _progressControlsContainer->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );


    _currentLayerImage->setAlignment( Qt::AlignCenter );
    _currentLayerImage->setContentsMargins( { } );
    _currentLayerImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _currentLayerImage->setStyleSheet( QString( "QWidget { background: black }" ) );

    _currentLayerLayout = WrapWidgetInVBox( _currentLayerImage );
    _currentLayerLayout->setAlignment( Qt::AlignCenter );
    _currentLayerLayout->setContentsMargins( { } );

    _currentLayerGroup->setTitle( "Current layer" );
    _currentLayerGroup->setContentsMargins( { } );
    _currentLayerGroup->setMinimumSize( MaximalRightHandPaneSize );
    _currentLayerGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _currentLayerGroup->setLayout( _currentLayerLayout );


    {
        _stopButtonEnabledPalette  = _stopButton->palette( );
        _stopButtonDisabledPalette = _stopButtonEnabledPalette;
        _stopButtonEnabledPalette.setColor( QPalette::Button,     Qt::red    );
        _stopButtonEnabledPalette.setColor( QPalette::ButtonText, Qt::yellow );
    }
    _stopButton->setEnabled( false );
    _stopButton->setFixedSize( MainButtonSize );
    _stopButton->setFont( ModifyFont( font22pt, QFont::Bold ) );
    _stopButton->setVisible( true );
    _stopButton->setText( "STOP" );
    QObject::connect( _stopButton, &QPushButton::clicked, this, &StatusTab::stopButton_clicked );

    _reprintButton->setEnabled( false );
    _reprintButton->setFixedSize( MainButtonSize );
    _reprintButton->setFont( font22pt );
    _reprintButton->setVisible( false );
    _reprintButton->setText( "Reprint" );
    QObject::connect( _reprintButton, &QPushButton::clicked, this, &StatusTab::reprintButton_clicked );

    _layout->setContentsMargins( { } );
    _layout->addWidget( _progressControlsContainer, 0, 0, 1, 1 );
    _layout->addWidget( _stopButton,                1, 0, 1, 1 );
    _layout->addWidget( _reprintButton,             1, 0, 1, 1 );
    _layout->addWidget( _currentLayerGroup,         0, 1, 2, 1 );
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

void StatusTab::_initialShowEvent( QShowEvent* event ) {
    _currentLayerImage->setFixedWidth( _currentLayerImage->width( ) );
    _currentLayerImage->setFixedHeight( _currentLayerImage->width( ) / AspectRatio16to10 + 0.5 );

    event->accept( );
}

void StatusTab::printer_online( ) {
    debug( "+ StatusTab::printer_online\n" );
    _isPrinterOnline = true;
    _printerStateDisplay->setText( "online" );

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
    _printerStateDisplay->setText( "offline" );
}

void StatusTab::stopButton_clicked( bool ) {
    debug( "+ StatusTab::stopButton_clicked\n" );
    _updatePrintTimeInfo->stop( );
    emit stopButtonClicked( );
}

void StatusTab::reprintButton_clicked( bool ) {
    debug( "+ StatusTab::reprintButton_clicked\n" );
    emit reprintButtonClicked( );
}

void StatusTab::printManager_printStarting( ) {
    debug( "+ StatusTab::printManager_printStarting\n" );

    _reprintButton->setVisible( false );
    _stopButton->setVisible( true );

    _jobStateDisplay->setText( "print started" );
    _estimatedTimeLeftDisplay->setText( QString( "calculating..." ) );
}

void StatusTab::printManager_startingLayer( int const layer ) {
    debug( "+ StatusTab::printManager_startingLayer: layer %d/%d\n", layer + 1, _printJob->layerCount );
    _currentLayerDisplay->setText( QString( "%1/%2" ).arg( layer + 1 ).arg( _printJob->layerCount ) );

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

    _percentageCompleteDisplay->setText( QString( "%1%" ).arg( static_cast<int>( static_cast<double>( _printManager->currentLayer( ) + 1 ) / static_cast<double>( _printJob->layerCount ) * 100.0 + 0.5 ) ) );

    auto pixmap = QPixmap( _printJob->jobWorkingDirectory + QString( "/%2.png" ).arg( layer, 6, 10, DigitZero ) );
    if ( ( pixmap.width( ) > _currentLayerImage->width( ) ) || ( pixmap.height( ) > _currentLayerImage->height( ) ) ) {
        pixmap = pixmap.scaled( _currentLayerImage->size( ), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }
    _currentLayerImage->setPixmap( pixmap );
}

void StatusTab::printManager_lampStatusChange( bool const on ) {
    debug( "+ StatusTab::printManager_lampStatusChange: lamp %s\n", on ? "ON" : "off" );
    _projectorLampStateDisplay->setText( QString( on ? "ON" : "off" ) );
    if ( on ) {
        _warningUvLabel->setPixmap( *_warningUvImage );
    } else {
        _warningUvLabel->clear( );
    }
}

void StatusTab::printManager_printComplete( bool const success ) {
    debug( "+ StatusTab::printManager_printComplete: %s\n", success ? "print complete" : "print failed" );

    _stopButton->setVisible( false );
    _reprintButton->setVisible( true );

    _updatePrintTimeInfo->stop( );

    _jobStateDisplay->setText( QString( success ? "print complete" : "print failed" ) );
    _currentLayerDisplay->clear( );
    _estimatedTimeLeftDisplay->clear( );
    _percentageCompleteDisplay->clear( );
    _currentLayerImage->clear( );

    emit printComplete( );
}

void StatusTab::printManager_printAborted( ) {
    debug( "+ StatusTab::printManager_printAborted\n" );

    _loadPrintSolutionGroup->setEnabled( false );

    _stopButton->setVisible( false );
    _reprintButton->setVisible( true );

    _updatePrintTimeInfo->stop( );

    _jobStateDisplay->setText( QString( "print aborted" ) );
    _currentLayerDisplay->clear( );
    _estimatedTimeLeftDisplay->clear( );
    _percentageCompleteDisplay->clear( );
    _currentLayerImage->clear( );

    emit printComplete( );
}

void StatusTab::shepherd_temperatureReport( double const bedCurrentTemperature, double const /*bedTargetTemperature*/, int const /*bedPwm*/ ) {
    if ( bedCurrentTemperature >= 30.0 ) {
        _warningHotLabel->setPixmap( *_warningHotImage );
    } else {
        _warningHotLabel->clear( );
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
    debug( "+ StatusTab::updatePrintTimeInfo_timeout: delta %f; estimate %f; time left %f\n", delta, _estimatedPrintJobTime, _estimatedPrintJobTime - delta );
    _elapsedTimeDisplay->setText( TimeDeltaToString( delta ) );
    if ( _printManager->currentLayer( ) > 2 ) {
        _estimatedTimeLeftDisplay->setText( TimeDeltaToString( _estimatedPrintJobTime - delta ) );
    }
}

void StatusTab::printManager_requestLoadPrintSolution( ) {
    _loadPrintSolutionLabel->setText( QString( "Dispense <b>%1 mL</b> of print solution, then tap <b>Continue</b> to start printing." ).arg( std::max( 1.0, PrintSolutionRecommendedScaleFactor * _printJob->estimatedVolume / 1000.0 ), 0, 'f', 2 ) );
    _loadPrintSolutionGroup->setEnabled( true );
}

void StatusTab::printSolutionLoadedButton_clicked( bool ) {
    _loadPrintSolutionGroup->setEnabled( false );

    _printManager->printSolutionLoaded( );
}

void StatusTab::_connectPrintManager( ) {
    if ( _printManager ) {
        QObject::connect( _printManager, &PrintManager::printStarting,            this, &StatusTab::printManager_printStarting            );
        QObject::connect( _printManager, &PrintManager::printComplete,            this, &StatusTab::printManager_printComplete            );
        QObject::connect( _printManager, &PrintManager::printAborted,             this, &StatusTab::printManager_printAborted             );
        QObject::connect( _printManager, &PrintManager::startingLayer,            this, &StatusTab::printManager_startingLayer            );
        QObject::connect( _printManager, &PrintManager::lampStatusChange,         this, &StatusTab::printManager_lampStatusChange         );
        QObject::connect( _printManager, &PrintManager::requestLoadPrintSolution, this, &StatusTab::printManager_requestLoadPrintSolution );
    }
}

void StatusTab::_connectShepherd( ) {
    if ( _shepherd ) {
        QObject::connect( _shepherd, &Shepherd::printer_online,  this, &StatusTab::printer_online  );
        QObject::connect( _shepherd, &Shepherd::printer_offline, this, &StatusTab::printer_offline );
    }
}

void StatusTab::setStopButtonEnabled( bool value ) {
    _stopButton->setEnabled( value );
    _stopButton->setPalette( value ? _stopButtonEnabledPalette : _stopButtonDisabledPalette );
}

void StatusTab::setReprintButtonEnabled( bool value ) {
    _reprintButton->setEnabled( value );
}
