#include "pch.h"

#include "statustab.h"

#include "printjob.h"
#include "shepherd.h"
#include "strings.h"
#include "utils.h"

StatusTab::StatusTab( QWidget* parent ): QWidget( parent ) {
    _initialShowEventFunc = std::bind( &StatusTab::_initialShowEvent, this );

    printerStateLabel->setText( "Printer status:" );
    printerStateLabel->setBuddy( printerStateDisplay );

    printerStateDisplay->setText( "Offline" );
    printerStateDisplay->setFrameShadow( QFrame::Sunken );
    printerStateDisplay->setFrameStyle( QFrame::StyledPanel );

    projectorLampStateLabel->setText( "Projector lamp status:" );
    projectorLampStateLabel->setBuddy( projectorLampStateDisplay );

    projectorLampStateDisplay->setText( "off" );
    projectorLampStateDisplay->setFrameShadow( QFrame::Sunken );
    projectorLampStateDisplay->setFrameStyle( QFrame::StyledPanel );

    jobStateLabel->setText( "Print status:" );
    jobStateLabel->setBuddy( jobStateDisplay );

    jobStateDisplay->setText( "Not printing" );
    jobStateDisplay->setFrameShadow( QFrame::Sunken );
    jobStateDisplay->setFrameStyle( QFrame::StyledPanel );

    currentLayerLabel->setText( "Current layer:" );
    currentLayerLabel->setBuddy( currentLayerDisplay );

    currentLayerDisplay->setFrameShadow( QFrame::Sunken );
    currentLayerDisplay->setFrameStyle( QFrame::StyledPanel );

    progressControlsLayout->setContentsMargins( { } );
    progressControlsLayout->addWidget( printerStateLabel );
    progressControlsLayout->addWidget( printerStateDisplay );
    progressControlsLayout->addWidget( projectorLampStateLabel );
    progressControlsLayout->addWidget( projectorLampStateDisplay );
    progressControlsLayout->addWidget( jobStateLabel );
    progressControlsLayout->addWidget( jobStateDisplay );
    progressControlsLayout->addWidget( currentLayerLabel );
    progressControlsLayout->addWidget( currentLayerDisplay );
    progressControlsLayout->addStretch( );

    progressControlsContainer->setContentsMargins( { } );
    progressControlsContainer->setLayout( progressControlsLayout );
    progressControlsContainer->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    currentLayerImage->setAlignment( Qt::AlignCenter );
    currentLayerImage->setContentsMargins( { } );
    currentLayerImage->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    currentLayerImage->setStyleSheet( QString( "QWidget { background: black }" ) );

    auto imageBackground = new QWidget( );
    imageBackground->setContentsMargins( { } );
    imageBackground->setFixedWidth( MaximalRightHandPaneSize.width( ) );
    imageBackground->setFixedHeight( MaximalRightHandPaneSize.width( ) / AspectRatio16to10 + 0.5 );
    imageBackground->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    imageBackground->setLayout( WrapWidgetInVBox( currentLayerImage ) );
    imageBackground->setStyleSheet( QString( "QWidget { background: black }" ) );

    currentLayerImageGroup->setTitle( "Current layer" );
    currentLayerImageGroup->setContentsMargins( { } );
    currentLayerImageGroup->setMinimumSize( MaximalRightHandPaneSize );
    currentLayerImageGroup->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    currentLayerImageGroup->setLayout( WrapWidgetInVBox( imageBackground ) );

    {
        _stopButtonEnabledPalette  =  stopButton->palette( );
        _stopButtonDisabledPalette = _stopButtonEnabledPalette;
        _stopButtonEnabledPalette.setColor( QPalette::Button,     Qt::red    );
        _stopButtonEnabledPalette.setColor( QPalette::ButtonText, Qt::yellow );
    }
    stopButton->setFont( ModifyFont( stopButton->font( ), 22.25f, QFont::Bold ) );
    stopButton->setText( "STOP" );
    stopButton->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::MinimumExpanding );
    stopButton->setEnabled( false );
    QObject::connect( stopButton, &QPushButton::clicked, this, &StatusTab::stopButton_clicked );

    _layout->setContentsMargins( { } );
    _layout->addWidget( progressControlsContainer,  0, 0, 1, 1 );
    _layout->addWidget( stopButton,                 1, 0, 1, 1 );
    _layout->addWidget( currentLayerImageGroup,     0, 1, 2, 1 );
    _layout->setRowStretch( 0, 4 );
    _layout->setRowStretch( 1, 1 );

    setLayout( _layout );
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
    currentLayerImageGroup->setFixedSize( currentLayerImageGroup->size( ) );
    currentLayerImageGroup->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );

    _maxLayerImageWidth = std::min( currentLayerImage->width( ), currentLayerImage->height( ) );
}

void StatusTab::printer_online( ) {
    debug( "+ StatusTab::printer_online\n" );
    _isPrinterOnline = true;
    printerStateDisplay->setText( "Online" );

    if ( !_isFirstOnlineTaskDone ) {
        debug( "+ StatusTab::printer_online: printer has come online for the first time; sending 'disable steppers' command\n" );
        QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &StatusTab::disableSteppers_sendComplete );
        _shepherd->doSend( QString( "M18" ) );
    }
}

void StatusTab::printer_offline( ) {
    debug( "+ StatusTab::printer_offline\n" );
    _isPrinterOnline = false;
    printerStateDisplay->setText( "Offline" );
}

void StatusTab::stopButton_clicked( bool ) {
    debug( "+ StatusTab::stopButton_clicked\n" );
    emit stopButtonClicked( );
}

void StatusTab::printManager_printStarting( ) {
    debug( "+ StatusTab::printManager_printStarting\n" );
    jobStateDisplay->setText( "Print started" );
}

void StatusTab::printManager_startingLayer( int const layer ) {
    debug( "+ StatusTab::printManager_startingLayer: layer %d/%d\n", layer, _printJob->layerCount );
    currentLayerDisplay->setText( QString( "%1/%2" ).arg( layer + 1 ).arg( _printJob->layerCount ) );
    auto pixmap = QPixmap( QString( "%1/%2.png" ).arg( _printJob->pngFilesPath ).arg( layer, 6, 10, DigitZero ) );
    int scaledWidth  = pixmap.width( )  / 1280.0 * static_cast<double>( currentLayerImageGroup->width( )  ) + 0.5;
    int scaledHeight = pixmap.height( ) /  800.0 * static_cast<double>( currentLayerImageGroup->height( ) ) + 0.5;
    currentLayerImage->setPixmap( pixmap.scaled( scaledWidth, scaledHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
}

void StatusTab::printManager_lampStatusChange( bool const on ) {
    debug( "+ StatusTab::printManager_lampStatusChange: lamp %s\n", on ? "ON" : "off" );
    projectorLampStateDisplay->setText( QString( on ? "ON" : "off" ) );
}

void StatusTab::printManager_printComplete( bool const success ) {
    debug( "+ StatusTab::printManager_printComplete: %s\n", success ? "Print complete" : "Print failed" );
    jobStateDisplay->setText( QString( success ? "Print complete" : "Print failed" ) );
    emit printComplete( );
}

void StatusTab::disableSteppers_sendComplete( bool const success ) {
    debug( "+ StatusTab::disableSteppers_sendComplete: success %s\n", ToString( success ) );
    QObject::disconnect( _shepherd, &Shepherd::action_sendComplete, this, &StatusTab::disableSteppers_sendComplete );

    if ( success ) {
        debug( "+ StatusTab::disableSteppers_sendComplete: sending 'set fan speed' command\n" );
        QObject::connect( _shepherd, &Shepherd::action_sendComplete, this, &StatusTab::setFanSpeed_sendComplete );
        _shepherd->doSend( QString( "M106 S220" ) );
    }
}

void StatusTab::setFanSpeed_sendComplete( bool const success ) {
    debug( "+ StatusTab::setFanSpeed_sendComplete: success %s\n", ToString( success ) );
    QObject::disconnect( _shepherd, &Shepherd::action_sendComplete, this, &StatusTab::setFanSpeed_sendComplete );

    if ( success ) {
        debug( "+ StatusTab::setFanSpeed_sendComplete: first-online tasks completed\n" );
        _isFirstOnlineTaskDone = true;
    }
}

void StatusTab::setPrintJob( PrintJob* printJob ) {
    debug( "+ StatusTab::setPrintJob: printJob %p\n", printJob );
    _printJob = printJob;
}

void StatusTab::setShepherd( Shepherd* newShepherd ) {
    if ( _shepherd ) {
        QObject::disconnect( _shepherd, nullptr, this, nullptr );
    }

    _shepherd = newShepherd;

    QObject::connect( _shepherd, &Shepherd::printer_online,  this, &StatusTab::printer_online  );
    QObject::connect( _shepherd, &Shepherd::printer_offline, this, &StatusTab::printer_offline );
}

void StatusTab::setStopButtonEnabled( bool value ) {
    stopButton->setEnabled( value );
    stopButton->setPalette( value ? _stopButtonEnabledPalette : _stopButtonDisabledPalette );
}
