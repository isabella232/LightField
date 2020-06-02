#include "pch.h"

#include <numeric>

#include "statustab.h"

#include "ordermanifestmanager.h"
#include "printjob.h"
#include "printmanager.h"
#include "shepherd.h"

namespace {

    double const PrintSolutionRecommendedScaleFactor = 2.0;

    QStringList const PrinterInitializationCommands {
        "M18",
        "M106",
        "M155 S5",
    };

    void _HideAndClear( QLabel* label ) {
        label->setVisible( false );
        label->clear( );
        label->update( );
    }

    void _SetTextAndShow( QLabel* label, QString const& text ) {
        label->setText( text );
        label->setVisible( true );
        label->update( );
    }

}

StatusTab::StatusTab( QWidget* parent ): InitialShowEventMixin<StatusTab, TabBase>( parent ) {
#if defined _DEBUG
    _isPrinterPrepared = g_settings.pretendPrinterIsPrepared;
#endif // _DEBUG

    QFont origFont { font( ) };
    QFont font22pt { ModifyFont( origFont, LargeFontSize ) };

    _boldFont = ModifyFont( origFont, QFont::Bold );

    _italicFont = _boldFont;
    _italicFont.setItalic( true );


    _currentLayerDisplay->setFont( _boldFont );
    _currentLayerDisplay->setVisible( false );

    _elapsedTimeDisplay->setFont( _boldFont );
    _elapsedTimeDisplay->setVisible( false );

    _estimatedTimeLeftDisplay->setFont( _boldFont );
    _estimatedTimeLeftDisplay->setVisible( false );

    _percentageCompleteDisplay->setFont( _boldFont );
    _percentageCompleteDisplay->setVisible( false );

    _printerStateDisplay->setFont( _boldFont );
    _printerStateDisplay->setText( "Printer is OFFLINE" );

    _zPositionDisplay->setFont( _boldFont );
    _zPositionDisplay->setText( "Z position " );

    _temperatureDisplay->setFont( _boldFont );
    _temperatureDisplay->setVisible( false );

    _projectorLampStateDisplay->setFont( _boldFont );
    _projectorLampStateDisplay->setText( "Projector is off" );


    _warningHotImage = new QPixmap { QString { ":images/warning-hot.png" } };
    _warningHotLabel->setAlignment( Qt::AlignCenter );
    _warningHotLabel->setContentsMargins( { } );
    _warningHotLabel->setFixedSize( 43, 43 );
    _warningHotLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _warningUvImage  = new QPixmap { QString { ":images/warning-uv.png"  } };
    _warningUvLabel->setAlignment( Qt::AlignCenter );
    _warningUvLabel->setContentsMargins( { } );
    _warningUvLabel->setFixedSize( 43, 43 );
    _warningUvLabel->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );


    _pauseButton->setFixedSize( MainButtonSize );
    _pauseButton->setFont( font22pt );
    _pauseButton->setEnabled( false );
    _pauseButton->setText( "Pause" );
    QObject::connect( _pauseButton, &QPushButton::clicked, this, &StatusTab::pauseButton_clicked );


    {
        auto palette = _stopButton->palette( );
        palette.setColor( QPalette::Button,     Qt::red    );
        palette.setColor( QPalette::ButtonText, Qt::yellow );
        _stopButton->setPalette( palette );
    }
    _stopButton->setFixedSize( MainButtonSize );
    _stopButton->setFont( ModifyFont( font22pt, QFont::Bold ) );
    _stopButton->setVisible( false );
    _stopButton->setText( "STOP" );
    QObject::connect( _stopButton, &QPushButton::clicked, this, &StatusTab::stopButton_clicked );

    _reprintButton->setFixedSize( MainButtonSize );
    _reprintButton->setFont( font22pt );
    _reprintButton->setVisible( true );
    _reprintButton->setText( "Reprint…" );
    QObject::connect( _reprintButton, &QPushButton::clicked, this, &StatusTab::reprintButton_clicked );


    _leftColumn->setContentsMargins( { } );
    _leftColumn->setFixedWidth( MainButtonSize.width( ) );
    _leftColumn->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
    _leftColumn->setLayout( WrapWidgetsInVBox(
        WrapWidgetsInVBox(
            WrapWidgetsInHBox( _currentLayerDisplay,       nullptr ),
            WrapWidgetsInHBox( _elapsedTimeDisplay,        nullptr ),
            WrapWidgetsInHBox( _estimatedTimeLeftDisplay,  nullptr ),
            WrapWidgetsInHBox( _percentageCompleteDisplay, nullptr ),
            WrapWidgetsInHBox( _printerStateDisplay,       nullptr ),
            WrapWidgetsInHBox( _zPositionDisplay,          nullptr ),
            WrapWidgetsInHBox( _temperatureDisplay,        nullptr ),
            WrapWidgetsInHBox( _projectorLampStateDisplay, nullptr ),
            nullptr,
            WrapWidgetsInHBox( nullptr, _warningHotLabel, nullptr, _warningUvLabel, nullptr )
        ),
        _pauseButton,
        _stopButton,
        _reprintButton
    ) );


    _currentLayerImage->setAlignment( Qt::AlignCenter );
    _currentLayerImage->setContentsMargins( { } );
    _currentLayerImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _currentLayerImage->setStyleSheet( "QWidget { background: black }" );

    _modelFileNameLabel->setAlignment( Qt::AlignLeft );
    _modelFileNameLabel->setContentsMargins( { } );
    _modelFileNameLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _modelFileNameLabel->setTextFormat( Qt::RichText );
    _modelFileNameLabel->setWordWrap( true );

    _imageFileNameLabel->setAlignment( Qt::AlignLeft );
    _imageFileNameLabel->setContentsMargins( { } );
    _imageFileNameLabel->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _imageFileNameLabel->setTextFormat( Qt::RichText );
    _imageFileNameLabel->setWordWrap( true );


    _currentLayerLayout = WrapWidgetsInVBox(
        _currentLayerImage,
        nullptr,
        WrapWidgetsInHBox(_modelFileNameLabel, _imageFileNameLabel)
    );
    _currentLayerLayout->setAlignment( Qt::AlignHCenter | Qt::AlignTop );

    _currentLayerGroup->setContentsMargins( { } );
    _currentLayerGroup->setMinimumSize( MaximalRightHandPaneSize );
    _currentLayerGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _currentLayerGroup->setLayout( _currentLayerLayout );
    _currentLayerGroup->setTitle( "Current layer" );


    _dispensePrintSolutionLabel->setAlignment( Qt::AlignHCenter );
    _dispensePrintSolutionLabel->setTextFormat( Qt::RichText );
    _dispensePrintSolutionLabel->setWordWrap( true );

    _startThePrintButton->setFont( font22pt );
    _startThePrintButton->setFixedSize( MainButtonSize );
    _startThePrintButton->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    _startThePrintButton->setText( "Start the print" );
    QObject::connect( _startThePrintButton, &QPushButton::clicked, this, &StatusTab::startThePrintButton_clicked );

    _dispensePrintSolutionGroup->setContentsMargins( { } );
    _dispensePrintSolutionGroup->setMinimumSize( MaximalRightHandPaneSize );
    _dispensePrintSolutionGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _dispensePrintSolutionGroup->setVisible( false );
    _dispensePrintSolutionGroup->setTitle( "Dispense print solution" );
    _dispensePrintSolutionGroup->setLayout( WrapWidgetsInVBox(
        nullptr,
        _dispensePrintSolutionLabel,
        nullptr,
        WrapWidgetsInHBox( nullptr, _startThePrintButton, nullptr ),
        nullptr
    ) );


    _rightColumn->setContentsMargins( { } );
    _rightColumn->setMinimumSize( MaximalRightHandPaneSize );
    _rightColumn->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    _rightColumn->setLayout( WrapWidgetsInVBox(
        _currentLayerGroup,
        _dispensePrintSolutionGroup,
        nullptr
    ) );


    setLayout( WrapWidgetsInHBox( _leftColumn, _rightColumn ) );


    _updatePrintTimeInfo = new QTimer( this );
    _updatePrintTimeInfo->setInterval( 1000 );
    _updatePrintTimeInfo->setSingleShot( false );
    _updatePrintTimeInfo->setTimerType( Qt::PreciseTimer );
    QObject::connect( _updatePrintTimeInfo, &QTimer::timeout, this, &StatusTab::updatePrintTimeInfo_timeout );


    _printerOnlineTimer = new QTimer( this );
    _printerOnlineTimer->setInterval( 2000 );
    _printerOnlineTimer->setSingleShot( true );
    _printerOnlineTimer->setTimerType( Qt::PreciseTimer );
    QObject::connect( _printerOnlineTimer, &QTimer::timeout, this, &StatusTab::printerOnlineTimer_timeout );
}

StatusTab::~StatusTab( ) {
    /*empty*/
}

void StatusTab::_updateReprintButtonState( ) {
    _reprintButton->setEnabled( ( UiState::PrintCompleted == _uiState ) && _isPrinterOnline && _isPrinterAvailable && _isPrinterPrepared && _isModelRendered );

    update( );
}

void StatusTab::_initialShowEvent( QShowEvent* event ) {
    _currentLayerImage->setFixedSize( _currentLayerImage->width( ), _currentLayerImage->width( ) / AspectRatio16to10 + 0.5 );
    update( );

    event->accept( );
}

void StatusTab::setModelRendered( bool const value ) {
    _isModelRendered = value;
    //debug( "+ StatusTab::setModelRendered: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _updateReprintButtonState( );
}

void StatusTab::setPrinterPrepared( bool const value ) {
    _isPrinterPrepared = value;
    debug( "+ StatusTab::setPrinterPrepared: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _updateReprintButtonState( );
}

void StatusTab::setPrinterAvailable( bool const value ) {
    _isPrinterAvailable = value;
    debug( "+ StatusTab::setPrinterAvailable: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _updateReprintButtonState( );
}

void StatusTab::printer_online( ) {
    _isPrinterOnline = true;
    debug( "+ StatusTab::printer_online: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _printerStateDisplay->setText( "Printer is online" );
    update( );

    if ( PrinterInitializationCommands.isEmpty( ) || _isFirstOnlineTaskDone ) {
        return;
    }
    debug( "+ StatusTab::printer_online: printer has come online for the first time; starting timer\n" );
    _printerOnlineTimer->start( );
}

void StatusTab::printer_offline( ) {
    _isPrinterOnline = false;
    debug( "+ StatusTab::printer_offline: PO? %s PA? %s PP? %s MR? %s\n", YesNoString( _isPrinterOnline ), YesNoString( _isPrinterAvailable ), YesNoString( _isPrinterPrepared ), YesNoString( _isModelRendered ) );

    _printerStateDisplay->setText( "Printer is OFFLINE" );
    update( );
}

void StatusTab::pauseButton_clicked( bool ) {
    auto paused = _isPaused;
    _isPaused = !_isPaused;
    _pauseButton->setEnabled( false );
    _pauseButton->setText("Pausing...");

    update( );

    if ( !paused ) {
        _currentPauseStartTime = GetBootTimeClock( );
        _printManager->pause( );
    } else {
        auto pausedTime = GetBootTimeClock( ) - _currentPauseStartTime;

        _totalPausedTime        += pausedTime;
        _previousLayerStartTime += pausedTime;
        _currentLayerStartTime  += pausedTime;
        _printJobStartTime      += pausedTime;

        _printManager->resume( );
    }
}

void StatusTab::stopButton_clicked( bool ) {
    debug( "+ StatusTab::stopButton_clicked\n" );

    _pauseButton->setEnabled( false );
    _stopButton->setEnabled( false );
    _stopButton->setText( "Stopping…" );
    _updatePrintTimeInfo->stop( );

    if ( _printManager ) {
        _printManager->abort( );
    }

    update( );
}

void StatusTab::reprintButton_clicked( bool ) {
    debug( "+ StatusTab::reprintButton_clicked\n" );
    emit printRequested( );
    emit uiStateChanged( TabIndex::Status, UiState::PrintStarted );

    update( );
}

void StatusTab::printManager_printStarting( ) {
    debug( "+ StatusTab::printManager_printStarting\n" );

    _printerStateDisplay->setText( "Waiting" );
    _HideAndClear( _elapsedTimeDisplay );

    _layerElapsedTimes.clear( );

    update( );
}

void StatusTab::printManager_printComplete( bool const success ) {
    debug( "+ StatusTab::printManager_printComplete: print %s\n", success ? "complete" : "failed" );

    _updatePrintTimeInfo->stop( );

    _printerStateDisplay->setText( success ? "Print is complete" : "Print failed" );
    _HideAndClear( _currentLayerDisplay       );
    _HideAndClear( _estimatedTimeLeftDisplay  );
    _HideAndClear( _percentageCompleteDisplay );
    _currentLayerImage->clear( );

    emit uiStateChanged( TabIndex::Status, UiState::PrintCompleted );

    update( );
}

void StatusTab::printManager_printAborted( ) {
    debug( "+ StatusTab::printManager_printAborted\n" );

    _dispensePrintSolutionGroup->setVisible( false );
    _currentLayerGroup->setVisible( true );

    _updatePrintTimeInfo->stop( );

    _printerStateDisplay->setText( "Print was canceled" );
    _HideAndClear( _currentLayerDisplay       );
    _HideAndClear( _estimatedTimeLeftDisplay  );
    _HideAndClear( _percentageCompleteDisplay );
    _currentLayerImage->clear( );

    emit uiStateChanged( TabIndex::Status, UiState::PrintCompleted );

    update( );
}

void StatusTab::printManager_printPausable( bool const pausable ) {
    debug( "+ StatusTab::printManager_printPausable: pausable? %s\n", ToString( pausable ) );

    _pauseButton->setEnabled( pausable );

    update( );
}

void StatusTab::printManager_printPaused( ) {
    debug( "+ StatusTab::printManager_printPaused\n" );

    _printerStateDisplay->setText( "Print is paused" );
    _pauseButton->setEnabled( true );
    _pauseButton->setText( "Resume" );

    update( );
}

void StatusTab::printManager_printResumed( ) {
    debug( "+ StatusTab::printManager_printResumed\n" );

    _printerStateDisplay->setText( "Printing" );
    _pauseButton->setEnabled( true );
    _pauseButton->setText( "Pause" );

    update( );
}

void StatusTab::printManager_startingLayer( int const layer ) {
    debug( "+ StatusTab::printManager_startingLayer: layer %d/%d\n", layer + 1, _printJob->totalLayerCount());
    if(_printJob->isTiled()){
        int realLayer = (layer+_printJob->tilingCount())/_printJob->tilingCount();
        int realLayersTotal = _printJob->totalLayerCount()/_printJob->tilingCount();
        int currentElement = (layer % _printJob->tilingCount()) + 1;
        _SetTextAndShow( _currentLayerDisplay, QString { "Printing layer %1 of %2, elements %3 of %4" }.arg( realLayer ).arg( realLayersTotal ).arg( currentElement ).arg( _printJob->tilingCount() ) );
    }else{
 _SetTextAndShow( _currentLayerDisplay, QString { "Printing layer %1 of %2" }.arg( layer + 1 ).arg( _printJob->totalLayerCount() ) );
    }

    _previousLayerStartTime = _currentLayerStartTime;
    _currentLayerStartTime  = GetBootTimeClock( );

    //TODO time estimation will be wrong in case of tiled
    if ( 0 == layer ) {
        _printerStateDisplay->setText( "Printing" );
        _estimatedTimeLeftDisplay->setFont( _italicFont );
        _SetTextAndShow( _estimatedTimeLeftDisplay, "Estimating time remaining…" );

        _printJobStartTime = _currentLayerStartTime;
        _updatePrintTimeInfo->start( );
    }

    if ( layer > 1 ) {
        auto delta = _currentLayerStartTime - _previousLayerStartTime;
        debug( "  + layer time: %.3f\n", delta );
        _layerElapsedTimes.emplace_back( delta );
    }

    if ( layer > 3 ) {
        // TODO
        // argh, the time estimate needs to be updated to deal with the base and
        // body layers, because they could have completely different settings
        auto average = std::accumulate<std::vector<double>::iterator, double>( _layerElapsedTimes.begin( ), _layerElapsedTimes.end( ), 0 ) / _layerElapsedTimes.size( );
        debug( "  + average:    %.3f\n", average );

        _estimatedPrintJobTime = average * _printJob->totalLayerCount();
        debug( "  + estimate:   %.3f\n", _estimatedPrintJobTime );
    }

    _SetTextAndShow( _percentageCompleteDisplay, QString { "%1% complete" }.arg( static_cast<int>( static_cast<double>( _printManager->currentLayer( ) ) / static_cast<double>( _printJob->totalLayerCount() ) * 100.0 + 0.5 ) ) );

    auto pixmap = QPixmap( _printJob->getLayerPath( layer ) );
    _imageFileNameLabel->setText("Layer image: " % ( _printJob->getLayerFileName( layer ) ));

    if ( ( pixmap.width( ) > _currentLayerImage->width( ) ) || ( pixmap.height( ) > _currentLayerImage->height( ) ) ) {
        pixmap = pixmap.scaled( _currentLayerImage->size( ), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    }
    _currentLayerImage->setPixmap( pixmap );

    update( );
}

void StatusTab::printManager_lampStatusChange( bool const on ) {
    _projectorLampStateDisplay->setText( on ? "Projector is ON" : "Projector is off" );
    if ( on ) {
        _warningUvLabel->setPixmap( *_warningUvImage );
    } else {
        _warningUvLabel->clear( );
    }

    update( );
}

void StatusTab::printManager_requestDispensePrintSolution( ) {
    double solutionRequired = PrintSolutionRecommendedScaleFactor * _printJob->estimatedVolume / 1000.0;

    QString dispenseText = QString("Dispense <b>%1 mL</b> of print solution,<br>then tap <b>Start the print</b>." ).arg( std::clamp(solutionRequired, 1.0, 10.0 ), 0, 'f', 2 );
    if(solutionRequired > 10.0){
     dispenseText.append(QString("<br>Total print solution needed: <b>%1 mL</b>" ).arg( solutionRequired, 0, 'f', 2 ));
    }
    _dispensePrintSolutionLabel->setText(dispenseText);


    _currentLayerGroup->setVisible( false );
    _dispensePrintSolutionGroup->setVisible( true );

    update( );
}

void StatusTab::printer_temperatureReport( double const bedCurrentTemperature, double const bedTargetTemperature, int const /*bedPwm*/ ) {
    if ( bedCurrentTemperature >= 30.0 ) {
        _warningHotLabel->setPixmap( *_warningHotImage );
    } else {
        _warningHotLabel->clear( );
    }

    if ( bedTargetTemperature != 0.0 ) {
        _SetTextAndShow( _temperatureDisplay, QString { "Print bed is %1 °C/%2 °C" }.arg( bedCurrentTemperature, 0, 'f', 1 ).arg( bedTargetTemperature, 0, 'f', 1 ) );
    } else {
        _SetTextAndShow( _temperatureDisplay, "Print bed heating is off." );
    }

    update( );
}

void StatusTab::printer_positionReport( double const px, int const cx ) {
    debug( "+ StatusTab::printer_positionReport: px %.2f mm, cx %d\n", px, cx );
    _zPositionDisplay->setText( QString { "Z position: %1 mm" }.arg( px, 0, 'f', 2 ) );

    update( );
}



void StatusTab::initializationCommands_sendComplete( bool const success ) {
    QObject::disconnect( _shepherd, &Shepherd::action_sendComplete, this, &StatusTab::initializationCommands_sendComplete );

    debug( "+ StatusTab::initializationCommands_sendComplete: first-online tasks %s\n", SucceededString( success ) );
    _isFirstOnlineTaskDone = success;
}

void StatusTab::updatePrintTimeInfo_timeout( ) {
    if ( !_printManager ) {
        debug( "+ StatusTab::updatePrintTimeInfo_timeout: no print manager. don't know why timer is even running, but stopping it.\n" );
        _updatePrintTimeInfo->stop( );
        return;
    }

    auto const delta             = GetBootTimeClock( ) - _printJobStartTime;
    auto const estimatedTimeLeft = _estimatedPrintJobTime - delta;
    auto const currentLayer      = _printManager->currentLayer( );

    debug( "+ StatusTab::updatePrintTimeInfo_timeout: delta %f; estimate %f; time left %f; isPaused? %s; currentLayer %d\n", delta, _estimatedPrintJobTime, estimatedTimeLeft, YesNoString( _isPaused ), currentLayer );

    _SetTextAndShow( _elapsedTimeDisplay, TimeDeltaToString( delta + _totalPausedTime ) % " elapsed" );

    update( );

    if ( _isPaused || ( currentLayer < 4 ) || ( estimatedTimeLeft < 0.5 ) ) {
        return;
    }

    if ( currentLayer == 4 ) {
        _estimatedTimeLeftDisplay->setFont( _boldFont );
    }
    _SetTextAndShow( _estimatedTimeLeftDisplay, TimeDeltaToString( estimatedTimeLeft ) % " remaining" );

    update( );
}

void StatusTab::printerOnlineTimer_timeout( ) {
    _printerOnlineTimer->stop( );

    debug( "+ StatusTab::printerOnlineTimer_timeout: sending initialization commands\n" );
    QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &StatusTab::initializationCommands_sendComplete );
    _shepherd->doSend( PrinterInitializationCommands );
}

void StatusTab::startThePrintButton_clicked( bool ) {
    _dispensePrintSolutionGroup->setVisible( false );
    _currentLayerGroup->setVisible( true );
    _printerStateDisplay->setText( "Print solution is settling" );

    _printManager->printSolutionDispensed( );

    update( );
}

void StatusTab::_connectPrintManager( ) {
    if ( _printManager ) {
        QObject::connect( _printManager, &PrintManager::printStarting,                this, &StatusTab::printManager_printStarting                );
        QObject::connect( _printManager, &PrintManager::printComplete,                this, &StatusTab::printManager_printComplete                );
        QObject::connect( _printManager, &PrintManager::printAborted,                 this, &StatusTab::printManager_printAborted                 );
        QObject::connect( _printManager, &PrintManager::printPausable,                this, &StatusTab::printManager_printPausable                );
        QObject::connect( _printManager, &PrintManager::printPaused,                  this, &StatusTab::printManager_printPaused                  );
        QObject::connect( _printManager, &PrintManager::printResumed,                 this, &StatusTab::printManager_printResumed                 );
        QObject::connect( _printManager, &PrintManager::startingLayer,                this, &StatusTab::printManager_startingLayer                );
        QObject::connect( _printManager, &PrintManager::lampStatusChange,             this, &StatusTab::printManager_lampStatusChange             );
        QObject::connect( _printManager, &PrintManager::requestDispensePrintSolution, this, &StatusTab::printManager_requestDispensePrintSolution );
    }
}

void StatusTab::_connectShepherd( ) {
    if ( _shepherd ) {
        QObject::connect( _shepherd, &Shepherd::printer_online,            this, &StatusTab::printer_online            );
        QObject::connect( _shepherd, &Shepherd::printer_offline,           this, &StatusTab::printer_offline           );
        QObject::connect( _shepherd, &Shepherd::printer_temperatureReport, this, &StatusTab::printer_temperatureReport );
        QObject::connect( _shepherd, &Shepherd::printer_positionReport,    this, &StatusTab::printer_positionReport    );
    }
}

void StatusTab::tab_uiStateChanged( TabIndex const sender, UiState const state ) {
    debug( "+ StatusTab::tab_uiStateChanged: from %sTab: %s => %s\n", ToString( sender ), ToString( _uiState ), ToString( state ) );
    _uiState = state;

    switch (_uiState) {
    case UiState::SelectStarted:
    case UiState::SelectCompleted:
    case UiState::SliceStarted:
    case UiState::SliceCompleted:
        _modelFileNameLabel->clear( );
        _imageFileNameLabel->clear();
        _stopButton->setVisible( false );
        _reprintButton->setVisible( true );
        break;

    case UiState::PrintStarted:
        _modelFileNameLabel->setText( "Model: " % _printJob->modelFileName.right( _printJob->modelFileName.length( ) - _printJob->modelFileName.lastIndexOf( Slash ) - 1 ) );
        _stopButton->setEnabled( true );
        _stopButton->setText( "STOP" );
        _stopButton->setVisible( true );
        _reprintButton->setVisible( false );
        break;

    case UiState::PrintCompleted:
        _stopButton->setVisible( false );
        _reprintButton->setVisible( true );
        break;

    default:
        break;
    }

    _updateReprintButtonState( );
}
